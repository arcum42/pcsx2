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
#include "IopCommon.h"

#include "Counters.h"
#include "IPU/IPU.h"
#include "DebugTools/SymbolMap.h"

#include "AppConfig.h"
#include "Utilities/AsciiFile.h"

using namespace R5900;

#ifdef TEST_BROKEN_DUMP_ROUTINES
extern tIPU_BP g_BP;

#define VF_VAL(x) ((x==0x80000000)?0:(x))
#endif

void iDumpPsxRegisters(u32 startpc, u32 temp)
{
}

void iDumpRegisters(u32 startpc, u32 temp)
{
}

void iDumpVU0Registers()
{
}

void iDumpVU1Registers()
{
}

// This function is close of iDumpBlock but it doesn't rely too much on
// global variable. Beside it doesn't print the flag info.
//
// However you could call it anytime to dump any block. And we have both
// x86 and EE disassembly code
void iDumpBlock(u32 ee_pc, u32 ee_size, uptr x86_pc, u32 x86_size)
{
	u32 ee_end = ee_pc + ee_size;

	DbgCon.WriteLn( Color_Gray, "dump block %x:%x (x86:0x%x)", ee_pc, ee_end, x86_pc );

	g_Conf->Folders.Logs.Mkdir();
	wxString dump_filename = Path::Combine( g_Conf->Folders.Logs, wxsFormat(L"R5900dump_%.8X:%.8X.txt", ee_pc, ee_end) );
	AsciiFile eff( dump_filename, L"w" );

	// Print register content to detect the memory access type. Warning value are taken
	// during the call of this function. There aren't the real value of the block.
	eff.Printf("Dump register data: 0x%x\n", (uptr)&cpuRegs.GPR.r[0].UL[0]);
	for (int reg = 0; reg < 32; reg++) {
		// Only lower 32 bits (enough for address)
		eff.Printf("\t%2s <= 0x%08x_%08x\n", R5900::GPR_REG[reg], cpuRegs.GPR.r[reg].UL[1],cpuRegs.GPR.r[reg].UL[0]);
	}
	eff.Printf("\n");


	if (!symbolMap.GetLabelString(ee_pc).empty())
	{
		eff.Printf( "%s\n", symbolMap.GetLabelString(ee_pc).c_str() );
	}

	for ( u32 i = ee_pc; i < ee_end; i += 4 )
	{
		std::string output;
		//TLB Issue disR5900Fasm( output, memRead32( i ), i, false );
		disR5900Fasm( output, psMu32(i), i, false );
		eff.Printf( "0x%.X : %s\n", i, output.c_str() );
	}

	// Didn't find (search) a better solution
	eff.Printf( "\nRaw x86 dump (https://www.onlinedisassembler.com/odaweb/):\n");
	u8* x86 = (u8*)x86_pc;
	for (u32 i = 0; i < x86_size; i++) {
		eff.Printf("%.2X", x86[i]);
	}
	eff.Printf("\n\n");

	eff.Close(); // Close the file so it can be appended by objdump

	// handy but slow solution (system call)
#ifdef __linux__
	wxString obj_filename = Path::Combine(g_Conf->Folders.Logs, wxString(L"objdump_tmp.o"));
	wxFFile objdump(obj_filename , L"wb");
	objdump.Write(x86, x86_size);
	objdump.Close();

	int status = std::system(
			wxsFormat( L"objdump -D -b binary -mi386 --disassembler-options=intel --no-show-raw-insn --adjust-vma=%d %s >> %s",
				   (u32) x86_pc, WX_STR(obj_filename), WX_STR(dump_filename)).mb_str()
			);

	if (!WIFEXITED(status))
		Console.Error("IOP dump didn't terminate normally");
#endif
}

