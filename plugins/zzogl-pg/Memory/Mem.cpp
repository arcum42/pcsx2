/*  ZZ Open GL graphics plugin
 *  Copyright (c)2009-2010 zeydlitz@gmail.com, arcum42@gmail.com
 *  Based on Zerofrog's ZeroGS KOSMOS (c)2005-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "GS.h"
#include "Util.h"
#include "Targets/targets.h"

#include "Memory/Mem.h"
#include "Memory/Mem_Swizzle.h"

BLOCK m_Blocks[0x40]; // do so blocks are indexable

PCSX2_ALIGNED16(u32 tempblock[64]);

// Add a bunch of local variables that used to be in the TransferHostLocal
// functions, in order to de-macro the TransmitHostLocal macros.
// May be in a class or namespace eventually.
int tempX, tempY;
int pitch, area, fracX;
int nSize;
u8* pstart;

// For debugging.
extern const char* psm_name[64];

//#define TRANSMISSION_LOG

// transfers whole rows
template <class T>
static __forceinline const T *TransmitHostLocalY_(_writePixel_0 wp, s32 widthlimit, int endY, const T *buf)
{
	assert((nSize % widthlimit) == 0 && widthlimit <= 4);

	if ((gs.imageEnd.x - gs.trxpos.dx) % widthlimit)
	{
		#ifdef TRANSMISSION_LOG
		ZZLog::GS_Log("Bad Transmission! %d %d, psm: %s", gs.trxpos.dx, gs.imageEnd.x, psm_name[gs.dstbuf.psm]);
		#endif

		for (; tempY < endY; ++tempY)
		{
			for (; tempX < gs.imageEnd.x && nSize > 0; tempX += 1, nSize -= 1, buf += 1)
			{
				/* write as many pixel at one time as possible */
				wp(pstart, tempX % 2048, tempY % 2048, buf[0], gs.dstbuf.bw);
			}
		}
	}

	for (; tempY < endY; ++tempY)
	{
		for (; tempX < gs.imageEnd.x && nSize > 0; tempX += widthlimit, nSize -= widthlimit, buf += widthlimit)
		{

			/* write as many pixel at one time as possible */
			if (nSize < widthlimit) return NULL;

			wp(pstart, tempX % 2048, tempY % 2048, buf[0], gs.dstbuf.bw);

			if (widthlimit > 1)
			{
				wp(pstart, (tempX + 1) % 2048, tempY % 2048, buf[1], gs.dstbuf.bw);

				if (widthlimit > 2)
				{
					wp(pstart, (tempX + 2) % 2048, tempY % 2048, buf[2], gs.dstbuf.bw);

					if (widthlimit > 3)
					{
						wp(pstart, (tempX + 3) % 2048, tempY % 2048, buf[3], gs.dstbuf.bw);
					}
				}
			}
		}

		if (tempX >= gs.imageEnd.x)
		{
			assert(tempX == gs.imageEnd.x);
			tempX = gs.trxpos.dx;
		}
		else
		{
			assert(gs.transferring == false || nSize*sizeof(T) / 4 == 0);
			return NULL;
		}
	}

	return buf;
}

// transfers whole rows
template <class T>
static __forceinline const T *TransmitHostLocalY_24(_writePixel_0 wp, s32 widthlimit, int endY, const T *buf)
{
	if (widthlimit != 8 || ((gs.imageEnd.x - gs.trxpos.dx) % widthlimit))
	{
		#ifdef TRANSMISSION_LOG
		ZZLog::GS_Log("Bad Transmission! %d %d, psm: %s", gs.trxpos.dx, gs.imageEnd.x, psm_name[gs.dstbuf.psm]);
		#endif

		for (; tempY < endY; ++tempY)
		{
			for (; tempX < gs.imageEnd.x && nSize > 0; tempX += 1, nSize -= 1, buf += 3)
			{
				wp(pstart, tempX % 2048, tempY % 2048, *(u32*)(buf), gs.dstbuf.bw);
			}

			if (tempX >= gs.imageEnd.x)
			{
				assert(gs.transferring == false || tempX == gs.imageEnd.x);
				tempX = gs.trxpos.dx;
			}
			else
			{
				assert(gs.transferring == false || nSize == 0);
				return NULL;
			}
		}
	}
	else
	{
		assert(/*(nSize%widthlimit) == 0 &&*/ widthlimit == 8);

		for (; tempY < endY; ++tempY)
		{
			for (; tempX < gs.imageEnd.x && nSize > 0; tempX += widthlimit, nSize -= widthlimit, buf += 3 * widthlimit)
			{
				if (nSize < widthlimit) return NULL;

				/* write as many pixel at one time as possible */

				wp(pstart, tempX % 2048, tempY % 2048, *(u32*)(buf + 0), gs.dstbuf.bw);
				wp(pstart, (tempX + 1) % 2048, tempY % 2048, *(u32*)(buf + 3), gs.dstbuf.bw);
				wp(pstart, (tempX + 2) % 2048, tempY % 2048, *(u32*)(buf + 6), gs.dstbuf.bw);
				wp(pstart, (tempX + 3) % 2048, tempY % 2048, *(u32*)(buf + 9), gs.dstbuf.bw);
				wp(pstart, (tempX + 4) % 2048, tempY % 2048, *(u32*)(buf + 12), gs.dstbuf.bw);
				wp(pstart, (tempX + 5) % 2048, tempY % 2048, *(u32*)(buf + 15), gs.dstbuf.bw);
				wp(pstart, (tempX + 6) % 2048, tempY % 2048, *(u32*)(buf + 18), gs.dstbuf.bw);
				wp(pstart, (tempX + 7) % 2048, tempY % 2048, *(u32*)(buf + 21), gs.dstbuf.bw);
			}

			if (tempX >= gs.imageEnd.x)
			{
				assert(gs.transferring == false || tempX == gs.imageEnd.x);
				tempX = gs.trxpos.dx;
			}
			else
			{
				if (nSize < 0)
				{
					/* extracted too much */
					assert((nSize % 3) == 0 && nSize > -24);
					tempX += nSize / 3;
					nSize = 0;
				}

				assert(gs.transferring == false || nSize == 0);

				return NULL;
			}
		}
	}

	return buf;
}

// meant for 4bit transfers
template <class T>
static __forceinline const T *TransmitHostLocalY_4(_writePixel_0 wp, s32 widthlimit, int endY, const T *buf)
{
	for (; tempY < endY; ++tempY)
	{
		for (; tempX < gs.imageEnd.x && nSize > 0; tempX += widthlimit, nSize -= widthlimit)
		{
			/* write as many pixel at one time as possible */
			wp(pstart, tempX % 2048, tempY % 2048, *buf&0x0f, gs.dstbuf.bw);
			wp(pstart, (tempX + 1) % 2048, tempY % 2048, *buf >> 4, gs.dstbuf.bw);
			buf++;

			if (widthlimit > 2)
			{
				wp(pstart, (tempX + 2) % 2048, tempY % 2048, *buf&0x0f, gs.dstbuf.bw);
				wp(pstart, (tempX + 3) % 2048, tempY % 2048, *buf >> 4, gs.dstbuf.bw);
				buf++;

				if (widthlimit > 4)
				{
					wp(pstart, (tempX + 4) % 2048, tempY % 2048, *buf&0x0f, gs.dstbuf.bw);
					wp(pstart, (tempX + 5) % 2048, tempY % 2048, *buf >> 4, gs.dstbuf.bw);
					buf++;

					if (widthlimit > 6)
					{
						wp(pstart, (tempX + 6) % 2048, tempY % 2048, *buf&0x0f, gs.dstbuf.bw);
						wp(pstart, (tempX + 7) % 2048, tempY % 2048, *buf >> 4, gs.dstbuf.bw);
						buf++;
					}
				}
			}
		}

		if (tempX >= gs.imageEnd.x)
		{
			tempX = gs.trxpos.dx;
		}
		else
		{
			assert(gs.transferring == false || (nSize / 32) == 0);
			return NULL;
		}
	}

	return buf;
}

template <class T>
static __forceinline const T *TransmitHostLocalX_(_writePixel_0 wp, u32 widthlimit, u32 blockheight, u32 startX, const T *buf)
{
	for (u32 tempi = 0; tempi < blockheight; ++tempi)
	{
		for (tempX = startX; tempX < gs.imageEnd.x; tempX++, buf++)
		{
			wp(pstart, tempX % 2048, (tempY + tempi) % 2048, buf[0], gs.dstbuf.bw);
		}

		buf += pitch - fracX;
	}

	return buf;
}

// transmit until endX, don't check size since it has already been prevalidated
template <class T>
static __forceinline const T *TransmitHostLocalX_24(_writePixel_0 wp, u32 widthlimit, u32 blockheight, u32 startX, const T *buf)
{
	for (u32 tempi = 0; tempi < blockheight; ++tempi)
	{
		for (tempX = startX; tempX < gs.imageEnd.x; tempX++, buf += 3)
		{
			wp(pstart, tempX % 2048, (tempY + tempi) % 2048, *(u32*)buf, gs.dstbuf.bw);
		}

		buf += 3 * (pitch - fracX);
	}

	return buf;
}

// transmit until endX, don't check size since it has already been prevalidated
template <class T>
static __forceinline const T *TransmitHostLocalX_4(_writePixel_0 wp, u32 widthlimit, u32 blockheight, u32 startX, const T *buf)
{
	for (u32 tempi = 0; tempi < blockheight; ++tempi)
	{
		for (tempX = startX; tempX < gs.imageEnd.x; tempX += 2, buf++)
		{
			wp(pstart, tempX % 2048, (tempY + tempi) % 2048, buf[0]&0x0f, gs.dstbuf.bw);
			wp(pstart, (tempX + 1) % 2048, (tempY + tempi) % 2048, buf[0] >> 4, gs.dstbuf.bw);
		}

		buf += (pitch - fracX) / 2;
	}

	return buf;
}

template <class T>
static __forceinline const T *TransmitHostLocalY(u32 psm, _writePixel_0 wp, s32 widthlimit, int endY, const T *buf)
{

	#ifdef TRANSMISSION_LOG
	ZZLog::WriteLn("TransmitHostLocalY: psm == %s, bimode == 0x%x", psm_name[psm], PSMT_BITMODE(psm));
	#endif

	switch (PSMT_BITMODE(psm))
	{
		case 1:
			return TransmitHostLocalY_24<T>(wp, widthlimit, endY, buf);
		case 4:
			return TransmitHostLocalY_4<T>(wp, widthlimit, endY, buf);
		default:
			return TransmitHostLocalY_<T>(wp, widthlimit, endY, buf);
	}

	assert(0);

	return NULL;
}

template <class T>
static __forceinline const T *TransmitHostLocalX(u32 psm, _writePixel_0 wp, u32 widthlimit, u32 blockheight, u32 startX, const T *buf)
{
	#ifdef TRANSMISSION_LOG
	ZZLog::WriteLn("TransmitHostLocalX: psm == %s, bimode == 0x%x", psm_name[psm], PSMT_BITMODE(psm));
	#endif

	switch (PSMT_BITMODE(psm))
	{
		case 1:
			return TransmitHostLocalX_24<T>(wp, widthlimit, blockheight, startX, buf);
		case 4:
			return TransmitHostLocalX_4<T>(wp, widthlimit, blockheight, startX, buf);
		default:
			return TransmitHostLocalX_<T>(wp, widthlimit, blockheight, startX, buf);
	}

	assert(0);

	return NULL;
}

// calculate pitch in source buffer
static __forceinline u32 TransPitch(u32 pitch, u32 size)
{
	return pitch * size / 8;
}

//static __forceinline u32 TransPitch2(u32 pitch, u32 size)
//{
//	if (size == 4) return pitch / 2;
//	if (size == 24) return pitch * 3;
//	return pitch;
//}

// ------------------------
// |              Y       |
// ------------------------
// |        block     |   |
// |   aligned area   | X |
// |                  |   |
// ------------------------
// |              Y       |
// ------------------------

template <class T>
static __forceinline const T* AlignOnBlockBoundry(TransferData data, TransferFuncts fun, Point alignedPt, int& endY, const T* pbuf)
{
	bool bCanAlign = ((MOD_POW2(gs.trxpos.dx, data.blockwidth) == 0) && (gs.image.x == gs.trxpos.dx) &&
					  (alignedPt.y > endY) && (alignedPt.x > gs.trxpos.dx));

	if ((gs.imageEnd.x - gs.trxpos.dx) % data.widthlimit)
	{
		/* hack */
		int testwidth = (int)nSize -
						(gs.imageEnd.y - gs.image.y) * (gs.imageEnd.x - gs.trxpos.dx)
						+ (gs.image.x - gs.trxpos.dx);

		if ((testwidth <= data.widthlimit) && (testwidth >= -data.widthlimit))
		{
			/* don't transfer */
			ZZLog::Debug_Log("Bad texture %s: %d %d %d", psm_name[gs.dstbuf.psm], gs.trxpos.dx, gs.imageEnd.x, (int)nSize);
			ZZLog::Error_Log("Bad texture: testwidth = %d; data.widthlimit = %d", testwidth, data.widthlimit);
			gs.transferring = false;
		}

		bCanAlign = false;
	}

	/* first align on block boundary */
	if (MOD_POW2(gs.image.y, data.blockheight) || !bCanAlign)
	{
		u32 transwidth;

		if (!bCanAlign)
			endY = gs.imageEnd.y; /* transfer the whole image */
		else
			assert(endY < gs.imageEnd.y);  /* part of alignment condition */

		if (((gs.imageEnd.x - gs.trxpos.dx) % data.widthlimit) || ((gs.imageEnd.x - gs.image.x) % data.widthlimit))
		{
			/* transmit with a width of 1 */
			transwidth = (1 + (gs.dstbuf.psm == PSMT4));
		}
		else
		{
			transwidth = data.widthlimit;
		}

		pbuf = TransmitHostLocalY<T>(data.psm, fun.wp, transwidth, endY, pbuf);

		if (pbuf == NULL) return NULL;

		if (nSize == 0 || tempY == gs.imageEnd.y) return NULL;
	}

	return pbuf;
}

template <class T>
static __forceinline const T* TransferAligningToBlocks(TransferData data, TransferFuncts fun, Point alignedPt, const T* pbuf)
{
	bool bAligned;
	const u32 TSize = sizeof(T);
	_SwizzleBlock swizzle;

	/* can align! */
	pitch = gs.imageEnd.x - gs.trxpos.dx;
	area = pitch * data.blockheight;
	fracX = gs.imageEnd.x - alignedPt.x;

	/* on top of checking whether pbuf is aligned, make sure that the width is at least aligned to its limits (due to bugs in pcsx2) */
	bAligned = !((uptr)pbuf & 0xf) && (TransPitch(pitch, data.transfersize) & 0xf) == 0;

	if (bAligned || ((gs.dstbuf.psm == PSMCT24) || (gs.dstbuf.psm == PSMT8H) || (gs.dstbuf.psm == PSMT4HH) || (gs.dstbuf.psm == PSMT4HL)))
		swizzle = (fun.Swizzle);
	else
		swizzle = (fun.Swizzle_u);

	//Transfer aligning to blocks.
	for (; tempY < alignedPt.y && nSize >= area; tempY += data.blockheight, nSize -= area)
	{
		for (int tempj = gs.trxpos.dx; tempj < alignedPt.x; tempj += data.blockwidth, pbuf += TransPitch(data.blockwidth, data.transfersize) / TSize)
		{
			u8 *temp = pstart + fun.gp(tempj, tempY, gs.dstbuf.bw) * data.blockbits / 8;
			swizzle(temp, (u8*)pbuf, TransPitch(pitch, data.transfersize));
		}
#ifdef ZEROGS_SSE2
        // Note: swizzle function uses some non temporal move (mm_stream) instruction.
        // store fence insures that previous store are finish before execute new one.
        _mm_sfence();

#endif

		/* transfer the rest */
		if (alignedPt.x < gs.imageEnd.x)
		{
			pbuf = TransmitHostLocalX<T>(data.psm, fun.wp, data.widthlimit, data.blockheight, alignedPt.x, pbuf);

			if (pbuf == NULL) return NULL;

			pbuf -= TransPitch((alignedPt.x - gs.trxpos.dx), data.transfersize) / TSize;
		}
		else
		{
			pbuf += (data.blockheight - 1) * TransPitch(pitch, data.transfersize) / TSize;
		}

		tempX = gs.trxpos.dx;
	}

	return pbuf;
}

static __forceinline int FinishTransfer(TransferData data, int nLeftOver)
{
	if (tempY >= gs.imageEnd.y)
	{
		assert( gs.transferring == false || tempY == gs.imageEnd.y);
		gs.transferring = false;
		/*int start, end;
		GetRectMemAddress(start, end, gs.dstbuf.psm, gs.trxpos.dx, gs.trxpos.dy, gs.imageNew, gs.dstbuf.bp, gs.dstbuf.bw);
		g_MemTargs.ClearRange(start, end);*/
	}
	else
	{
		/* update new params */
		gs.image.y = tempY;
		gs.image.x = tempX;
	}

	return (nSize * TransPitch(2, data.transfersize) + nLeftOver) / 2;
}

template <class T>
static __forceinline int RealTransfer(u32 psm, const void* pbyMem, u32 nQWordSize)
{
	assert(gs.imageTransfer == XFER_HOST_TO_LOCAL);
	TransferData data = tData[psm];
	TransferFuncts fun(psm);

	pstart = g_pbyGSMemory + gs.dstbuf.bp * 256;
	//pstart = gs_mem._MemoryAddress<256>(gs.dstbuf.bp);

	const T* pbuf = (const T*)pbyMem;
	const int tp2 = TransPitch(2, data.transfersize);
	int nLeftOver = (nQWordSize * 4 * 2) % tp2;
	tempY = gs.image.y;
	tempX = gs.image.x;
	Point alignedPt;

	nSize = (nQWordSize * 4 * 2) / tp2;
	nSize = min(nSize, gs.imageNew.w * gs.imageNew.h);

	int endY = ROUND_UPPOW2(gs.image.y, data.blockheight);
	alignedPt.y = ROUND_DOWNPOW2(gs.imageEnd.y, data.blockheight);
	alignedPt.x = ROUND_DOWNPOW2(gs.imageEnd.x, data.blockwidth);

	pbuf = AlignOnBlockBoundry<T>(data, fun, alignedPt, endY, pbuf);
	if (pbuf == NULL) return FinishTransfer(data, nLeftOver);

	pbuf = TransferAligningToBlocks<T>(data, fun, alignedPt, pbuf);
	if (pbuf == NULL) return FinishTransfer(data, nLeftOver);

	if (TransPitch(nSize, data.transfersize) / 4 > 0)
	{
		pbuf = TransmitHostLocalY<T>(psm, fun.wp, data.widthlimit, gs.imageEnd.y, pbuf);

		if (pbuf == NULL) return FinishTransfer(data, nLeftOver);

		/* sometimes wrong sizes are sent (tekken tag) */
		assert(gs.transferring == false || TransPitch(nSize, data.transfersize) / 4 <= 2);
	}

	return FinishTransfer(data, nLeftOver);
}

// The TransferHostLocal/TranferLocalHost functions here are used outside of Mem.cpp.
int TransferHostLocal32(const void* pbyMem, u32 nQWordSize)  { return RealTransfer<u32>(PSMCT32, pbyMem, nQWordSize);  }
int TransferHostLocal32Z(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u32>(PSMT32Z, pbyMem, nQWordSize);  }
int TransferHostLocal24(const void* pbyMem, u32 nQWordSize)  { return RealTransfer<u8>(PSMCT24, pbyMem, nQWordSize);   }
int TransferHostLocal24Z(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u8>(PSMT24Z, pbyMem, nQWordSize);   }
int TransferHostLocal16(const void* pbyMem, u32 nQWordSize)  { return RealTransfer<u16>(PSMCT16, pbyMem, nQWordSize);  }
int TransferHostLocal16S(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u16>(PSMCT16S, pbyMem, nQWordSize); }
int TransferHostLocal16Z(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u16>(PSMT16Z, pbyMem, nQWordSize);  }
int TransferHostLocal16SZ(const void* pbyMem, u32 nQWordSize){ return RealTransfer<u16>(PSMT16SZ, pbyMem, nQWordSize); }
int TransferHostLocal8(const void* pbyMem, u32 nQWordSize)   { return RealTransfer<u8>(PSMT8, pbyMem, nQWordSize);     }
int TransferHostLocal4(const void* pbyMem, u32 nQWordSize)   { return RealTransfer<u8>(PSMT4, pbyMem, nQWordSize);     }
int TransferHostLocal8H(const void* pbyMem, u32 nQWordSize)  { return RealTransfer<u8>(PSMT8H, pbyMem, nQWordSize);    }
int TransferHostLocal4HL(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u8>(PSMT4HL, pbyMem, nQWordSize);   }
int TransferHostLocal4HH(const void* pbyMem, u32 nQWordSize) { return RealTransfer<u8>(PSMT4HH, pbyMem, nQWordSize);   }

void TransferLocalHost32(void* pbyMem, u32 nQWordSize) 		{FUNCLOG}
void TransferLocalHost24(void* pbyMem, u32 nQWordSize) 		{FUNCLOG}
void TransferLocalHost16(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost16S(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost8(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost4(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost8H(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost4HL(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost4HH(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost32Z(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost24Z(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost16Z(void* pbyMem, u32 nQWordSize)		{FUNCLOG}
void TransferLocalHost16SZ(void* pbyMem, u32 nQWordSize)	{FUNCLOG}
