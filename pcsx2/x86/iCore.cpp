/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#include "PrecompiledHeader.h"

#include "System.h"
#include "iR5900.h"
#include "Vif.h"
#include "VU.h"
#include "R3000A.h"

using namespace x86Emitter;

__tls_emit u8 *j8Ptr[32];
__tls_emit u32 *j32Ptr[32];

u16 g_x86AllocCounter = 0;

EEINST *g_pCurInstInfo = NULL;

// used to make sure regs don't get changed while in recompiler
// use FreezeXMMRegs
u32 g_recWriteback = 0;

// XMM Caching
#define VU_VFx_ADDR(x) (uptr) & VU->VF[x].UL[0]
#define VU_ACCx_ADDR (uptr) & VU->ACC.UL[0]

XMM_Regs XMM_Reg;

// Clear current register mapping structure
// Clear allocation counter
void XMM_Regs::init()
{
    xmmregs.fill({0});
    g_xmmAllocCounter = 0;
    s_xmmchecknext = 0;
}

// Get the index of a free register.
// 1: Check any available register. (inuse == 0)
// 2: Check registers that are not live. (both EEINST_LIVE* are cleared)
// 3: Check registers that won't use SSE in the future. (likely broken as EEINST_XMM isn't set properly)
// 4: Take a random register.
//
// Note: I don't understand why we don't check register that aren't useful anymore.
// (i.e EEINST_USED is cleared)
int XMM_Regs::getFreeReg()
{
    int tempi;
    u32 bestcount = 0x10000;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[(i + s_xmmchecknext) % iREGCNT_XMM].inuse == 0)
        {
            int ret = (s_xmmchecknext + i) % iREGCNT_XMM;
            s_xmmchecknext = (s_xmmchecknext + i + 1) % iREGCNT_XMM;
            return ret;
        }
    }

    // check for dead regs
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].needed) continue;
        if (xmmregs[i].type == XMMTYPE_GPRREG)
        {
            if (!(EEINST_ISLIVEXMM(xmmregs[i].reg)))
            {
                freeReg(i);
                return i;
            }
        }
    }

    // check for future xmm usage
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].needed) continue;
        if (xmmregs[i].type == XMMTYPE_GPRREG)
        {
            if (!(g_pCurInstInfo->regs[XMM_Reg.xmmregs[i].reg] & EEINST_XMM))
            {
                freeReg(i);
                return i;
            }
        }
    }

    tempi = -1;
    bestcount = 0xffff;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].needed) continue;
        if (xmmregs[i].type != XMMTYPE_TEMP)
        {
            if (xmmregs[i].counter < bestcount)
            {
                tempi = i;
                bestcount = xmmregs[i].counter;
            }
            continue;
        }

        freeReg(i);
        return i;
    }

    if (tempi != -1)
    {
        freeReg(tempi);
        return tempi;
    }

    pxFailDev("*PCSX2*: XMM Reg Allocation Error in XMM_Reg.getFreeReg()!");
    throw Exception::FailedToAllocateRegister();
}

// Reserve a XMM register for temporary operation.
int XMM_Regs::allocTemp(XMMSSEType type, int xmmreg)
{
    if (xmmreg == -1)
        xmmreg = getFreeReg();
    else
        freeReg(xmmreg);

    xmmregs[xmmreg].inuse = 1;
    xmmregs[xmmreg].type = XMMTYPE_TEMP;
    xmmregs[xmmreg].needed = 1;
    xmmregs[xmmreg].counter = g_xmmAllocCounter++;
    g_xmmtypes[xmmreg] = type;

    return xmmreg;
}

// Get a pointer to the physical register (GPR / FPU / VU etc..)
// Only used in XMM_Reg.checkReg.
__fi void *XMM_Regs::GetAddr(_xmmregs &r)
{
    const VURegs *VU = r.VU ? &VU1 : &VU0;

    switch (r.type)
    {
        case XMMTYPE_VFREG:
            return (void *)VU_VFx_ADDR(r.reg);

        case XMMTYPE_ACC:
            return (void *)VU_ACCx_ADDR;

        case XMMTYPE_GPRREG:
            if (r.reg < 32)
                pxAssert(!(g_cpuHasConstReg & (1 << r.reg)) || (g_cpuFlushedConstReg & (1 << r.reg)));
            return &cpuRegs.GPR.r[r.reg].UL[0];

        case XMMTYPE_FPREG:
            return &fpuRegs.fpr[r.reg];

        case XMMTYPE_FPACC:
            return &fpuRegs.ACC.f;

            jNO_DEFAULT
    }

    return NULL;
}

// Search register "reg" of type "type" which is in use.
// If register doesn't have the read flag but mode is read,
// then populate the register from the memory.
// Note: There is a special HALF mode (to handle low 64 bits copy) but it seems to be unused.
//
// So basically it is mostly used to set the mode of the register, and load value if we need to read it.
int XMM_Regs::checkReg(int type, int reg, int mode)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse && (r.type == (type & 0xff)) && (r.reg == reg))
        {
            if (!(r.mode & MODE_READ))
            {
                if (mode & MODE_READ)
                {
                    xMOVDQA(xRegisterSSE(i), ptr[GetAddr(r)]);
                }
                else if (mode & MODE_READHALF)
                {
                    if (g_xmmtypes[i] == XMMT_INT)
                        xMOVQZX(xRegisterSSE(i), ptr[(void *)(uptr)GetAddr(r)]);
                    else
                        xMOVL.PS(xRegisterSSE(i), ptr[(void *)(uptr)GetAddr(r)]);
                }
            }

            r.mode |= mode & ~MODE_READHALF;
            r.counter = g_xmmAllocCounter++; // update counter
            r.needed = 1;
            return i;
        }
    }

    return -1;
}

// Fully allocate a FPU register:
// First trial:
//     Search an already reserved reg, then populate it if we read it.
// Second trial:
//     Reserve a new reg, then populate it if we read it.
//
// Note: FPU are always in XMM register
int XMM_Regs::allocFP(int xmmreg, int fpreg, int mode)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_FPREG) continue;
        if (r.reg != fpreg) continue;

        if (!(r.mode & MODE_READ) && (mode & MODE_READ))
        {
            xMOVSSZX(xRegisterSSE(i), ptr[&fpuRegs.fpr[fpreg].f]);
            r.mode |= MODE_READ;
        }

        g_xmmtypes[i] = XMMT_FPS;
        reg_reuse(r, mode);
        return i;
    }

    if (xmmreg == -1) xmmreg = getFreeReg();
    _xmmregs &r = xmmregs[xmmreg];

    g_xmmtypes[xmmreg] = XMMT_FPS;
    reg_use(r, XMMTYPE_FPREG, fpreg, mode);

    if (mode & MODE_READ)
        xMOVSSZX(xRegisterSSE(xmmreg), ptr[&fpuRegs.fpr[fpreg].f]);

    return xmmreg;
}

// In short, try to allocate a GPR register. Code is an utter mess,
// due to XMM/MMX/X86 crazyness!
int XMM_Regs::allocGPR(int xmmreg, int gprreg, int mode)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_GPRREG) continue;
        if (r.reg != gprreg) continue;

        if (!(r.mode & MODE_READ) && (mode & MODE_READ))
        {
            if (gprreg == 0)
            {
                xPXOR(xRegisterSSE(i), xRegisterSSE(i));
            }
            else
            {
                //pxAssert( !(g_cpuHasConstReg & (1<<gprreg)) || (g_cpuFlushedConstReg & (1<<gprreg)) );
                _flushConstReg(gprreg);
                xMOVDQA(xRegisterSSE(i), ptr[&cpuRegs.GPR.r[gprreg].UL[0]]);
            }
            xmmregs[i].mode |= MODE_READ;
        }

        if ((mode & MODE_WRITE) && (gprreg < 32))
        {
            g_cpuHasConstReg &= ~(1 << gprreg);
            //pxAssert( !(g_cpuHasConstReg & (1<<gprreg)) );
        }

        g_xmmtypes[i] = XMMT_INT;
        reg_reuse(r, mode);
        return i;
    }

    // currently only gpr regs are const
    // fixme - do we really need to execute this both here and in the loop?
    if ((mode & MODE_WRITE) && gprreg < 32)
    {
        //pxAssert( !(g_cpuHasConstReg & (1<<gprreg)) );
        g_cpuHasConstReg &= ~(1 << gprreg);
    }

    if (xmmreg == -1) xmmreg = getFreeReg();
    _xmmregs &r = xmmregs[xmmreg];

    g_xmmtypes[xmmreg] = XMMT_INT;
    reg_use(r, XMMTYPE_GPRREG, gprreg, mode);

    if (mode & MODE_READ)
    {
        if (gprreg == 0)
        {
            xPXOR(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg));
        }
        else
        {
            // DOX86
            if (mode & MODE_READ) _flushConstReg(gprreg);

            xMOVDQA(xRegisterSSE(xmmreg), ptr[&cpuRegs.GPR.r[gprreg].UL[0]]);
        }
    }

    return xmmreg;
}

// Same code as allocFP but for the FPU ACC register
// (seriously boy you could have factorized it)
int XMM_Regs::allocFPACC(int xmmreg, int mode)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_FPACC) continue;

        if (!(r.mode & MODE_READ) && (mode & MODE_READ))
        {
            xMOVSSZX(xRegisterSSE(i), ptr[&fpuRegs.ACC.f]);
            r.mode |= MODE_READ;
        }

        g_xmmtypes[i] = XMMT_FPS;
        reg_reuse(r, mode);
        return i;
    }

    if (xmmreg == -1) xmmreg = getFreeReg();
    _xmmregs &r = xmmregs[xmmreg];

    g_xmmtypes[xmmreg] = XMMT_FPS;
    reg_use(r, XMMTYPE_FPACC, 0, mode);

    if (mode & MODE_READ)
    {
        xMOVSSZX(xRegisterSSE(xmmreg), ptr[&fpuRegs.ACC.f]);
    }

    return xmmreg;
}

// Mark reserved GPR reg as needed. It won't be evicted anymore.
// You must use XMM_Reg.clearNeededRegs to clear the flag.
void XMM_Regs::addNeededGPR(int gprreg)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_GPRREG) continue;
        if (r.reg != gprreg) continue;

        r.counter = g_xmmAllocCounter++; // update counter
        r.needed = 1;
        break;
    }
}

// Mark reserved FPU reg as needed. It won't be evicted anymore.
// You must use clearNeededRegs to clear the flag
void XMM_Regs::addNeededFP(int fpreg)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_FPREG) continue;
        if (r.reg != fpreg) continue;

        r.counter = g_xmmAllocCounter++; // update counter
        r.needed = 1;
        break;
    }
}

// Mark reserved FPU ACC reg as needed. It won't be evicted anymore.
// You must use XMM_Reg.clearNeededRegs to clear the flag
void XMM_Regs::addNeededFPACC()
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_FPACC) continue;

        r.counter = g_xmmAllocCounter++; // update counter
        r.needed = 1;
        break;
    }
}

// Clear needed flags of all registers
// Written register will set MODE_READ (aka data is valid, no need to load it)
void XMM_Regs::clearNeededRegs()
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.needed)
        {
            // setup read to any just written regs
            if (r.inuse && (r.mode & MODE_WRITE))
                r.mode |= MODE_READ;
            r.needed = 0;
        }

        if (r.inuse)
        {
            pxAssert(r.type != XMMTYPE_TEMP);
        }
    }
}

// Flush is 0: XMM_Reg.freeReg. Flush in memory if MODE_WRITE. Clear inuse.
// Flush is 1: Flush in memory. But register is still valid.
// Flush is 2: like 0 ...
// Flush is 3: drop register content.
void XMM_Regs::deleteGPR(int reg, int flush)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse && r.type == XMMTYPE_GPRREG && r.reg == reg)
        {
            switch (flush)
            {
                case 0:
                    freeReg(i);
                    break;

                case 1:
                case 2:
                    if (r.mode & MODE_WRITE)
                    {
                        pxAssert(reg != 0);

                        //pxAssert( g_xmmtypes[i] == XMMT_INT );
                        xMOVDQA(ptr[&cpuRegs.GPR.r[reg].UL[0]], xRegisterSSE(i));

                        // get rid of MODE_WRITE since don't want to flush again
                        r.mode &= ~MODE_WRITE;
                        r.mode |= MODE_READ;
                    }

                    if (flush == 2) r.inuse = 0;
                    break;

                case 3:
                    r.inuse = 0;
                    break;
            }

            return;
        }
    }
}

// Flush is 0: XMM_Reg.freeReg. Flush in memory if MODE_WRITE. Clear inuse.
// Flush is 1: Flush in memory. But register is still valid.
// Flush is 2: drop register content.
void XMM_Regs::deleteFP(int reg, int flush)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse && r.type == XMMTYPE_FPREG && r.reg == reg)
        {
            switch (flush)
            {
                case 0:
                    freeReg(i);
                    return;

                case 1:
                    if (r.mode & MODE_WRITE)
                    {
                        xMOVSS(ptr[&fpuRegs.fpr[reg].UL], xRegisterSSE(i));
                        // get rid of MODE_WRITE since don't want to flush again
                        r.mode &= ~MODE_WRITE;
                        r.mode |= MODE_READ;
                    }
                    return;

                case 2:
                    r.inuse = 0;
                    return;
            }
        }
    }
}

// Free cached register
// Step 1: flush content in memory if MODE_WRITE.
// Step 2: clear 'inuse' field.
void XMM_Regs::freeReg(u32 xmmreg)
{
    _xmmregs &r = xmmregs[xmmreg];

    pxAssert(xmmreg < iREGCNT_XMM);

    if (!r.inuse) return;

    if (r.mode & MODE_WRITE)
    {
        switch (r.type)
        {
            case XMMTYPE_VFREG:
            {
                const VURegs *VU = r.VU ? &VU1 : &VU0;

                if (r.mode & MODE_VUXYZ)
                {
                    if (r.mode & MODE_VUZ)
                    {
                        // don't destroy w
                        int t0reg = getAvailableReg();

                        if (t0reg >= 0)
                        {
                            xMOVHL.PS(xRegisterSSE(t0reg), xRegisterSSE(xmmreg));
                            xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(xmmreg));
                            xMOVSS(ptr[(void *)(VU_VFx_ADDR(r.reg) + 8)], xRegisterSSE(t0reg));
                        }
                        else
                        {
                            // no free reg
                            xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(xmmreg));
                            xSHUF.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg), 0xc6);
                            //xMOVHL.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg));
                            xMOVSS(ptr[(void *)(VU_VFx_ADDR(r.reg) + 8)], xRegisterSSE(xmmreg));
                            xSHUF.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg), 0xc6);
                        }
                    }
                    else
                    {
                        xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(xmmreg));
                    }
                }
                else
                {
                    xMOVAPS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(xmmreg));
                }
            }
            break;

            case XMMTYPE_ACC:
            {
                const VURegs *VU = r.VU ? &VU1 : &VU0;

                if (r.mode & MODE_VUXYZ)
                {
                    if (r.mode & MODE_VUZ)
                    {
                        // don't destroy w
                        int t0reg = getAvailableReg();

                        if (t0reg >= 0)
                        {
                            xMOVHL.PS(xRegisterSSE(t0reg), xRegisterSSE(xmmreg));
                            xMOVL.PS(ptr[(void *)(VU_ACCx_ADDR)], xRegisterSSE(xmmreg));
                            xMOVSS(ptr[(void *)(VU_ACCx_ADDR + 8)], xRegisterSSE(t0reg));
                        }
                        else
                        {
                            // no free reg
                            xMOVL.PS(ptr[(void *)(VU_ACCx_ADDR)], xRegisterSSE(xmmreg));
                            xSHUF.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg), 0xc6);
                            //xMOVHL.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg));
                            xMOVSS(ptr[(void *)(VU_ACCx_ADDR + 8)], xRegisterSSE(xmmreg));
                            xSHUF.PS(xRegisterSSE(xmmreg), xRegisterSSE(xmmreg), 0xc6);
                        }
                    }
                    else
                    {
                        xMOVL.PS(ptr[(void *)(VU_ACCx_ADDR)], xRegisterSSE(xmmreg));
                    }
                }
                else
                {
                    xMOVAPS(ptr[(void *)(VU_ACCx_ADDR)], xRegisterSSE(xmmreg));
                }
            }
            break;

            case XMMTYPE_GPRREG:
                pxAssert(XMM_Reg.xmmregs[xmmreg].reg != 0);
                //pxAssert( g_xmmtypes[xmmreg] == XMMT_INT );
                xMOVDQA(ptr[&cpuRegs.GPR.r[r.reg].UL[0]], xRegisterSSE(xmmreg));
                break;

            case XMMTYPE_FPREG:
                xMOVSS(ptr[&fpuRegs.fpr[r.reg]], xRegisterSSE(xmmreg));
                break;

            case XMMTYPE_FPACC:
                xMOVSS(ptr[&fpuRegs.ACC.f], xRegisterSSE(xmmreg));
                break;

            default:
                break;
        }
    }

    r.mode &= ~(MODE_WRITE | MODE_VUXYZ);
    r.inuse = 0;
}

// Return the first register not in use. Return -1 if all are in use.
int XMM_Regs::getAvailableReg()
{
    int num = -1;

    for (u32 i = 0; i < iREGCNT_XMM; ++i)
    {
        if (!xmmregs[i].inuse)
        {
            num = i;
            break;
        }
    }

    return num;
}

// Return the number of inuse XMM register that have a flag.
int XMM_Regs::getFlagCount(u8 flag)
{
    int num = 0;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].inuse && (xmmregs[i].mode & flag)) ++num;
    }

    return num;
}

// 1: Check any available register. (inuse == 0)
// 2: Check registers that are not live. (both EEINST_LIVE* are cleared)
// 3: Check registers that are not useful anymore. (EEINST_USED cleared)
u8 XMM_Regs::hasFreeReg()
{
    if (getAvailableReg() >= 0) return 1;

    // check for dead regs
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].needed) continue;
        if (xmmregs[i].type == XMMTYPE_GPRREG)
        {
            if (!EEINST_ISLIVEXMM(XMM_Reg.xmmregs[i].reg))
            {
                return 1;
            }
        }
    }

    // check for dead regs
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].needed) continue;
        if (xmmregs[i].type == XMMTYPE_GPRREG)
        {
            if (!(g_pCurInstInfo->regs[xmmregs[i].reg] & EEINST_USED))
            {
                return 1;
            }
        }
    }
    return 0;
}

// Flush in memory all inuse registers but registers are still valid
void XMM_Regs::flushRegs()
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].inuse == 0) continue;

        pxAssert(xmmregs[i].type != XMMTYPE_TEMP);
        pxAssert(xmmregs[i].mode & (MODE_READ | MODE_WRITE));

        freeReg(i);
        xmmregs[i].inuse = 1;
        xmmregs[i].mode &= ~MODE_WRITE;
        xmmregs[i].mode |= MODE_READ;
    }
}

// Flush in memory all inuse registers. All registers are invalid.
void XMM_Regs::freeRegs()
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        if (xmmregs[i].inuse == 0) continue;

        pxAssert(xmmregs[i].type != XMMTYPE_TEMP);
        //pxAssert(xmmregs[i].mode & (MODE_READ|MODE_WRITE) );

        freeReg(i);
    }
}

int _signExtendXMMtoM(uptr to, x86SSERegType from, int candestroy)
{
    int t0reg;
    g_xmmtypes[from] = XMMT_INT;

    if (candestroy)
    {
        if (g_xmmtypes[from] == XMMT_FPS)
            xMOVSS(ptr[(void *)(to)], xRegisterSSE(from));
        else
            xMOVD(ptr[(void *)(to)], xRegisterSSE(from));

        xPSRA.D(xRegisterSSE(from), 31);
        xMOVD(ptr[(void *)(to + 4)], xRegisterSSE(from));
        return 1;
    }
    else
    {
        // can't destroy and type is int
        pxAssert(g_xmmtypes[from] == XMMT_INT);


        if (XMM_Reg.hasFreeReg())
        {
            XMM_Reg.xmmregs[from].needed = 1;
            t0reg = XMM_Reg.allocTemp(XMMT_INT, -1);
            xMOVDQA(xRegisterSSE(t0reg), xRegisterSSE(from));
            xPSRA.D(xRegisterSSE(from), 31);
            xMOVD(ptr[(void *)(to)], xRegisterSSE(t0reg));
            xMOVD(ptr[(void *)(to + 4)], xRegisterSSE(from));

            // swap xmm regs.. don't ask
            XMM_Reg.xmmregs[t0reg] = XMM_Reg.xmmregs[from];
            XMM_Reg.xmmregs[from].inuse = 0;
        }
        else
        {
            xMOVD(ptr[(void *)(to + 4)], xRegisterSSE(from));
            xMOVD(ptr[(void *)(to)], xRegisterSSE(from));
            xSAR(ptr32[(u32 *)(to + 4)], 31);
        }

        return 0;
    }

    pxAssume(false);
}

// Seem related to the mix between XMM/x86 in order to avoid a couple of move
// But it is quite obscure !!!
int XMM_Regs::allocCheckGPR(int gprreg, int mode)
{
    if (g_pCurInstInfo->regs[gprreg] & EEINST_XMM)
        return allocGPR(-1, gprreg, mode);

    return checkReg(XMMTYPE_GPRREG, gprreg, mode);
}

// Seem related to the mix between XMM/x86 in order to avoid a couple of move
// But it is quite obscure !!!
int XMM_Regs::allocCheckFPU(int fpureg, int mode)
{
    if (g_pCurInstInfo->fpuregs[fpureg] & EEINST_XMM)
        return XMM_Reg.allocFP(-1, fpureg, mode);

    return XMM_Reg.checkReg(XMMTYPE_FPREG, fpureg, mode);
}

int _allocCheckGPRtoX86(EEINST *pinst, int gprreg, int mode)
{
    if (pinst->regs[gprreg] & EEINST_USED)
        return X86_Reg.allocReg(xEmptyReg, X86TYPE_GPR, gprreg, mode);

    return X86_Reg.checkReg(X86TYPE_GPR, gprreg, mode);
}

void _recClearInst(EEINST *pinst)
{
    memzero(*pinst);
    memset8<EEINST_LIVE0 | EEINST_LIVE2>(pinst->regs);
    memset8<EEINST_LIVE0>(pinst->fpuregs);
}

// returns nonzero value if reg has been written between [startpc, endpc-4]
u32 _recIsRegWritten(EEINST *pinst, int size, u8 xmmtype, u8 reg)
{
    u32 inst = 1;

    while (size-- > 0)
    {
        for (u32 i = 0; i < ArraySize(pinst->writeType); ++i)
        {
            if ((pinst->writeType[i] == xmmtype) && (pinst->writeReg[i] == reg))
                return inst;
        }
        ++inst;
        pinst++;
    }

    return 0;
}

void _recFillRegister(EEINST &pinst, int type, int reg, int write)
{
    if (write)
    {
        for (u32 i = 0; i < ArraySize(pinst.writeType); ++i)
        {
            if (pinst.writeType[i] == XMMTYPE_TEMP)
            {
                pinst.writeType[i] = type;
                pinst.writeReg[i] = reg;
                return;
            }
        }
        pxAssume(false);
    }
    else
    {
        for (u32 i = 0; i < ArraySize(pinst.readType); ++i)
        {
            if (pinst.readType[i] == XMMTYPE_TEMP)
            {
                pinst.readType[i] = type;
                pinst.readReg[i] = reg;
                return;
            }
        }
        pxAssume(false);
    }
}

#ifndef DISABLE_SVU
int XMM_Regs::allocACC(VURegs *VU, int xmmreg, int mode)
{
    int readfromreg = -1;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_ACC) continue;
        if (r.VU != XMM_CONV_VU(VU)) continue;

        if (xmmreg >= 0)
        {
            // requested specific reg, so return that instead
            if ((int)i != xmmreg)
            {
                if (r.mode & MODE_READ) readfromreg = i;
                //if( XMM_Reg.xmmregs[i].mode & MODE_WRITE ) mode |= MODE_WRITE;
                mode |= r.mode & MODE_WRITE;
                r.inuse = 0;
                break;
            }
        }

        if (!(r.mode & MODE_READ) && (mode & MODE_READ))
        {
            xMOVAPS(xRegisterSSE(i), ptr[(void *)(VU_ACCx_ADDR)]);
            r.mode |= MODE_READ;
        }

        g_xmmtypes[i] = XMMT_FPS;
        reg_reuse(r, mode);
        return i;
    }

    if (xmmreg == -1)
        xmmreg = getFreeReg();
    else
        freeReg(xmmreg);
    _xmmregs &r = xmmregs[xmmreg];

    g_xmmtypes[xmmreg] = XMMT_FPS;
    r.VU = XMM_CONV_VU(VU);
    reg_use(r, XMMTYPE_ACC, 0, mode);

    if (mode & MODE_READ)
    {
        if (readfromreg >= 0)
            xMOVAPS(xRegisterSSE(xmmreg), xRegisterSSE(readfromreg));
        else
            xMOVAPS(xRegisterSSE(xmmreg), ptr[(void *)(VU_ACCx_ADDR)]);
    }

    return xmmreg;
}

void XMM_Regs::addNeededVF(int vfreg)
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_VFREG) continue;
        if (r.reg != vfreg) continue;

        r.counter = g_xmmAllocCounter++; // update counter
        r.needed = 1;
    }
}

void XMM_Regs::addNeededACC()
{
    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse == 0) continue;
        if (r.type != XMMTYPE_ACC) continue;

        r.counter = g_xmmAllocCounter++; // update counter
        r.needed = 1;
        break;
    }
}

void XMM_Regs::deleteVF(int reg, int vu, int flush)
{
    VURegs *VU = vu ? &VU1 : &VU0;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if (r.inuse && (r.type == XMMTYPE_VFREG) && (r.reg == reg) && (r.VU == vu))
        {
            switch (flush)
            {
                case 0:
                    freeReg(i);
                    break;

                case 1:
                    if (r.mode & MODE_WRITE)
                    {
                        pxAssert(reg != 0);

                        if (r.mode & MODE_VUXYZ)
                        {
                            if (r.mode & MODE_VUZ)
                            {
                                // xyz, don't destroy w
                                int t0reg = getAvailableReg();

                                if (t0reg >= 0)
                                {
                                    xMOVHL.PS(xRegisterSSE(t0reg), xRegisterSSE(i));
                                    xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(i));
                                    xMOVSS(ptr[(void *)(VU_VFx_ADDR(r.reg) + 8)], xRegisterSSE(t0reg));
                                }
                                else
                                {
                                    // no free reg
                                    xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(i));
                                    xSHUF.PS(xRegisterSSE(i), xRegisterSSE(i), 0xc6);
                                    xMOVSS(ptr[(void *)(VU_VFx_ADDR(r.reg) + 8)], xRegisterSSE(i));
                                    xSHUF.PS(xRegisterSSE(i), xRegisterSSE(i), 0xc6);
                                }
                            }
                            else
                            {
                                // xy
                                xMOVL.PS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(i));
                            }
                        }
                        else
                            xMOVAPS(ptr[(void *)(VU_VFx_ADDR(r.reg))], xRegisterSSE(i));

                        // get rid of MODE_WRITE since don't want to flush again
                        r.mode &= ~MODE_WRITE;
                        r.mode |= MODE_READ;
                    }
                    break;

                case 2:
                    r.inuse = 0;
                    break;
            }

            return;
        }
    }
}

int XMM_Regs::allocVF(VURegs *VU, int xmmreg, int vfreg, int mode)
{
    int readfromreg = -1;

    for (u32 i = 0; i < iREGCNT_XMM; i++)
    {
        _xmmregs &r = xmmregs[i];

        if ((r.inuse == 0) || (r.type != XMMTYPE_VFREG) || (r.reg != vfreg) || (r.VU != XMM_CONV_VU(VU)))
            continue;

        if (xmmreg >= 0)
        {
            // requested specific reg, so return that instead
            if ((int)i != xmmreg)
            {
                if (r.mode & MODE_READ) readfromreg = i;
                //if( XMM_Reg.xmmregs[i].mode & MODE_WRITE ) mode |= MODE_WRITE;
                mode |= r.mode & MODE_WRITE;
                r.inuse = 0;
                break;
            }
        }

        r.needed = 1;

        if (!(r.mode & MODE_READ) && (mode & MODE_READ))
        {
            xMOVAPS(xRegisterSSE(i), ptr[(void *)(VU_VFx_ADDR(vfreg))]);
            r.mode |= MODE_READ;
        }

        g_xmmtypes[i] = XMMT_FPS;
        reg_reuse(r, mode);
        return i;
    }

    if (xmmreg == -1)
        xmmreg = getFreeReg();
    else
        freeReg(xmmreg);
    _xmmregs &r = xmmregs[xmmreg];

    g_xmmtypes[xmmreg] = XMMT_FPS;
    r.VU = XMM_CONV_VU(VU);
    reg_use(r, XMMTYPE_VFREG, vfreg, mode);

    if (mode & MODE_READ)
    {
        if (readfromreg >= 0)
            xMOVAPS(xRegisterSSE(xmmreg), xRegisterSSE(readfromreg));
        else
            xMOVAPS(xRegisterSSE(xmmreg), ptr[(void *)(VU_VFx_ADDR(r.reg))]);
    }

    return xmmreg;
}
#endif

#if 0
void _deleteACCtoXMMreg(int vu, int flush)
{
	int i;
	VURegs *VU = vu ? &VU1 : &VU0;

	for (i=0; (uint)i<iREGCNT_XMM; i++) {
		if (XMM_Reg.xmmregs[i].inuse && (XMM_Reg.xmmregs[i].type == XMMTYPE_ACC) && (XMM_Reg.xmmregs[i].VU == vu)) {

			switch(flush) {
				case 0:
					XMM_Reg.freeReg(i);
					break;
				case 1:
				case 2:
					if( XMM_Reg.xmmregs[i].mode & MODE_WRITE ) {

						if( XMM_Reg.xmmregs[i].mode & MODE_VUXYZ ) {

							if( XMM_Reg.xmmregs[i].mode & MODE_VUZ ) {
								// xyz, don't destroy w
								uint t0reg;
								for(t0reg = 0; t0reg < iREGCNT_XMM; ++t0reg ) {
									if( !XMM_Reg.xmmregs[t0reg].inuse ) break;
								}

								if( t0reg < iREGCNT_XMM ) {
									xMOVHL.PS(xRegisterSSE(t0reg), xRegisterSSE(i));
									xMOVL.PS(ptr[(void*)(VU_ACCx_ADDR)], xRegisterSSE(i));
									xMOVSS(ptr[(void*)(VU_ACCx_ADDR+8)], xRegisterSSE(t0reg));
								}
								else {
									// no free reg
									xMOVL.PS(ptr[(void*)(VU_ACCx_ADDR)], xRegisterSSE(i));
									xSHUF.PS(xRegisterSSE(i), xRegisterSSE(i), 0xc6);
									//xMOVHL.PS(xRegisterSSE(i), xRegisterSSE(i));
									xMOVSS(ptr[(void*)(VU_ACCx_ADDR+8)], xRegisterSSE(i));
									xSHUF.PS(xRegisterSSE(i), xRegisterSSE(i), 0xc6);
								}
							}
							else {
								// xy
								xMOVL.PS(ptr[(void*)(VU_ACCx_ADDR)], xRegisterSSE(i));
							}
						}
						else xMOVAPS(ptr[(void*)(VU_ACCx_ADDR)], xRegisterSSE(i));

						// get rid of MODE_WRITE since don't want to flush again
						XMM_Reg.xmmregs[i].mode &= ~MODE_WRITE;
						XMM_Reg.xmmregs[i].mode |= MODE_READ;
					}

					if( flush == 2 )
						XMM_Reg.xmmregs[i].inuse = 0;
					break;
			}

			return;
		}
	}
}

void _moveXMMreg(int xmmreg)
{
	int i;
	if( !XMM_Reg.xmmregs[xmmreg].inuse ) return;

	for (i=0; (uint)i<iREGCNT_XMM; i++) {
		if (XMM_Reg.xmmregs[i].inuse) continue;
		break;
	}

	if( i == iREGCNT_XMM ) {
		XMM_Reg.freeReg(xmmreg);
		return;
	}

	// move
	XMM_Reg.xmmregs[i] = XMM_Reg.xmmregs[xmmreg];
	XMM_Reg.xmmregs[xmmreg].inuse = 0;
	xMOVDQA(xRegisterSSE(i), xRegisterSSE(xmmreg));
}

u32 _recIsRegUsed(EEINST* pinst, int size, u8 xmmtype, u8 reg)
{
	u32 i, inst = 1;
	while(size-- > 0) {
		for(i = 0; i < ArraySize(pinst->writeType); ++i) {
			if( pinst->writeType[i] == xmmtype && pinst->writeReg[i] == reg )
				return inst;
		}
		for(i = 0; i < ArraySize(pinst->readType); ++i) {
			if( pinst->readType[i] == xmmtype && pinst->readReg[i] == reg )
				return inst;
		}
		++inst;
		pinst++;
	}

	return 0;
}
#endif