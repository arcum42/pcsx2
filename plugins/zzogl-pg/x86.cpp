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
#include "Memory/Mem.h"

// Note: all codes of this files is deprecated. Keeping for reference.

#ifdef ZEROGS_SSE2

//void __fastcall WriteCLUT_T16_I8_CSM1_sse2(u32* vm, u32* clut)
//{
//  __asm {
//	  mov eax, vm
//	  mov ecx, clut
//	  mov edx, 8
//  }
//
//Extract32x2:
//  __asm {
//	  movdqa xmm0, qword ptr [eax]
//	  movdqa xmm1, qword ptr [eax+16]
//	  movdqa xmm2, qword ptr [eax+32]
//	  movdqa xmm3, qword ptr [eax+48]
//
//	  // rearrange
//	  pshuflw xmm0, xmm0, 0xd8
//	  pshufhw xmm0, xmm0, 0xd8
//	  pshuflw xmm1, xmm1, 0xd8
//	  pshufhw xmm1, xmm1, 0xd8
//	  pshuflw xmm2, xmm2, 0xd8
//	  pshufhw xmm2, xmm2, 0xd8
//	  pshuflw xmm3, xmm3, 0xd8
//	  pshufhw xmm3, xmm3, 0xd8
//
//	  movdqa xmm4, xmm0
//	  movdqa xmm6, xmm2
//
//	  shufps xmm0, xmm1, 0x88
//	  shufps xmm2, xmm3, 0x88
//
//	  shufps xmm4, xmm1, 0xdd
//	  shufps xmm6, xmm3, 0xdd
//
//	  pshufd xmm0, xmm0, 0xd8
//	  pshufd xmm2, xmm2, 0xd8
//	  pshufd xmm4, xmm4, 0xd8
//	  pshufd xmm6, xmm6, 0xd8
//
//	  // left column
//	  movhlps xmm1, xmm0
//	  movlhps xmm0, xmm2
//	  //movdqa xmm7, [ecx]
//
//	  movdqa [ecx], xmm0
//	  shufps xmm1, xmm2, 0xe4
//	  movdqa [ecx+16], xmm1
//
//	  // right column
//	  movhlps xmm3, xmm4
//	  movlhps xmm4, xmm6
//	  movdqa [ecx+32], xmm4
//	  shufps xmm3, xmm6, 0xe4
//	  movdqa [ecx+48], xmm3
//
//	  add eax, 16*4
//	  add ecx, 16*8
//	  sub edx, 1
//	  cmp edx, 0
//	  jne Extract32x2
//  }
//}


#endif // ZEROGS_SSE2


void SSE2_UnswizzleZ16Target(u16* dst, u16* src, int iters)
{

#if defined(_MSC_VER)
	__asm
	{
		mov edx, iters
		pxor xmm7, xmm7
		mov eax, dst
		mov ecx, src

Z16Loop:
		// unpack 64 bytes at a time
		movdqa xmm0, [ecx]
		movdqa xmm2, [ecx+16]
		movdqa xmm4, [ecx+32]
		movdqa xmm6, [ecx+48]

		movdqa xmm1, xmm0
		movdqa xmm3, xmm2
		movdqa xmm5, xmm4

		punpcklwd xmm0, xmm7
		punpckhwd xmm1, xmm7
		punpcklwd xmm2, xmm7
		punpckhwd xmm3, xmm7

		// start saving
		movdqa [eax], xmm0
		movdqa [eax+16], xmm1

		punpcklwd xmm4, xmm7
		punpckhwd xmm5, xmm7

		movdqa [eax+32], xmm2
		movdqa [eax+48], xmm3

		movdqa xmm0, xmm6
		punpcklwd xmm6, xmm7

		movdqa [eax+64], xmm4
		movdqa [eax+80], xmm5

		punpckhwd xmm0, xmm7

		movdqa [eax+96], xmm6
		movdqa [eax+112], xmm0

		add ecx, 64
		add eax, 128
		sub edx, 1
		jne Z16Loop
	}
#else // _MSC_VER

	__asm__ __volatile__(".intel_syntax\n"
	"pxor %%xmm7, %%xmm7\n"

	"Z16Loop:\n"
	// unpack 64 bytes at a time
	"movdqa %%xmm0, [%[src]]\n"
	"movdqa %%xmm2, [%[src]+16]\n"
	"movdqa %%xmm4, [%[src]+32]\n"
	"movdqa %%xmm6, [%[src]+48]\n"

	"movdqa %%xmm1, %%xmm0\n"
	"movdqa %%xmm3, %%xmm2\n"
	"movdqa %%xmm5, %%xmm4\n"

	"punpcklwd %%xmm0, %%xmm7\n"
	"punpckhwd %%xmm1, %%xmm7\n"
	"punpcklwd %%xmm2, %%xmm7\n"
	"punpckhwd %%xmm3, %%xmm7\n"

	// start saving
	"movdqa [%[dst]], %%xmm0\n"
	"movdqa [%[dst]+16], %%xmm1\n"

	"punpcklwd %%xmm4, %%xmm7\n"
	"punpckhwd %%xmm5, %%xmm7\n"

	"movdqa [%[dst]+32], %%xmm2\n"
	"movdqa [%[dst]+48], %%xmm3\n"

	"movdqa %%xmm0, %%xmm6\n"
	"punpcklwd %%xmm6, %%xmm7\n"

	"movdqa [%[dst]+64], %%xmm4\n"
	"movdqa [%[dst]+80], %%xmm5\n"

	"punpckhwd %%xmm0, %%xmm7\n"

	"movdqa [%[dst]+96], %%xmm6\n"
	"movdqa [%[dst]+112], %%xmm0\n"

	"add %[src], 64\n"
	"add %[dst], 128\n"
	"sub %[iters], 1\n"
	"jne Z16Loop\n"

".att_syntax\n" 
    : "=&r"(src), "=&r"(dst), "=&r"(iters)
    : [src] "0"(src), [dst] "1"(dst), [iters] "2"(iters)
    : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
       );
#endif // _MSC_VER
}

