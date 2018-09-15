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

#pragma once

#include "PS2Edefs.h"
#include <list>
#include <map>
//#include "GS.h"
#include "ZZGl.h"
#include "ZZGet.h"

#include "Targets/ZZRenderTargets.h"
#include "Targets/ZZDepthTargets.h"
#include "Targets/ZZMemoryTargets.h"
//#include "VB.h"

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_NV
#endif

extern bool g_bSaveZUpdate; // Never actually set to true? For debugging, I'm assuming.

// all textures have this width
extern int GPU_TEXWIDTH; // GSCreate.cpp - seriously, a bunch of unrelated files are relying on these externs being here because they happen to use targets.h?
extern float g_fiGPU_TEXWIDTH; // GSCreate.cpp
#define MASKDIVISOR		0							// Used for decrement bitwise mask texture size if 1024 is too big; only used in targets.cpp.
#define GPU_TEXMASKWIDTH	(1024 >> MASKDIVISOR)	// bitwise mask width for region repeat mode; used in targets.cpp & Flush.cpp.

class CBitwiseTextureMngr
{
	public:
		~CBitwiseTextureMngr() { Destroy(); }

		void Destroy();

		// since GetTex can delete textures to free up mem, it is dangerous if using that texture, so specify at least one other tex to save
		__forceinline u32 GetTex(u32 bitvalue, u32 ptexDoNotDelete)
		{
			map<u32, u32>::iterator it = mapTextures.find(bitvalue);

			if (it != mapTextures.end()) return it->second;

			return GetTexInt(bitvalue, ptexDoNotDelete);
		}

	private:
		u32 GetTexInt(u32 bitvalue, u32 ptexDoNotDelete);

		map<u32, u32> mapTextures;
};

// manages

class CRangeManager
{
	public:
		CRangeManager()
		{
			ranges.reserve(16);
		}

		// [start, end)

		struct RANGE
		{
			RANGE() {}

			inline RANGE(int start, int end) : start(start), end(end) {}

			int start, end;
		};

		// works in semi logN
		void Insert(int start, int end);
		void RangeSanityCheck();
		inline void Clear()
		{
			ranges.resize(0);
		}

		vector<RANGE> ranges; // organized in ascending order, non-intersecting
};

extern CRenderTargetMngr s_RTs, s_DepthRTs;
extern CBitwiseTextureMngr s_BitwiseTextures;
extern CMemoryTargetMngr g_MemTargs;
extern CRangeManager s_RangeMngr; // manages overwritten memory

//extern u8 s_AAx, s_AAy;
extern Point AA;

// Real rendered width, depends on AA.
inline int RW(int tbw)
{
    return (tbw << AA.x);
}

// Real rendered height, depends on AA.
inline int RH(int tbh)
{
    return (tbh << AA.y);
}

// This pattern of functions is called 3 times, so I add creating Targets list into one.
inline list<CRenderTarget*> CreateTargetsList(int start, int end)
{
	list<CRenderTarget*> listTargs;
	s_DepthRTs.GetTargs(start, end, listTargs);
	s_RTs.GetTargs(start, end, listTargs);
	return listTargs;
}

extern int icurctx;
extern GLuint vboRect;

// Unworking
#define PSMPOSITION 28

// Code width and height of frame into key, that used in targetmanager
// This is 3 variants of one function, Key dependant on fbp and fbw.
inline u32 GetFrameKey(const frameInfo& frame)
{
	return (((frame.fbw) << 16) | (frame.fbp));
}

inline u32 GetFrameKey(CRenderTarget* frame)
{
	return (((frame->fbw) << 16) | (frame->fbp));
}

inline u32 GetFrameKey(int fbp, int fbw)
{
	return (((fbw) << 16) | (fbp));
}

inline u16 ShiftHeight(int fbh, int fbp, int fbhCalc)
{
	return fbh;
}

//#define FRAME_KEY_BY_FBH

//FIXME: this code is for P4 and KH1. It should not be so strange!
//Dummy targets was deleted from mapTargets, but not erased.
inline u32 GetFrameKeyDummy(int fbp, int fbw, int fbh, int psm)
{
//	if (fbp > 0x2000 && ZZOgl_fbh_Calc(fbp, fbw, psm) < 0x400 && ZZOgl_fbh_Calc(fbp, fbw, psm) != fbh)
//		ZZLog::Debug_Log("Z %x %x %x %x\n", fbh, fbhCalc, fbp, ZZOgl_fbh_Calc(fbp, fbw, psm));
	// height over 1024 would shrink to 1024, so dummy targets with calculated size more than 0x400 should be
	// distinct by real height. But in FFX there is 3e0 height target, so I put 0x300 as limit.

#ifndef FRAME_KEY_BY_FBH	
	int calc = ZZOgl_fbh_Calc(fbp, fbw, psm);
	if (/*fbp > 0x2000 && */calc < /*0x300*/0x2E0)
		return ((fbw << 16) | calc);
	else
#endif
		return ((fbw << 16) | fbh);
}

inline u32 GetFrameKeyDummy(const frameInfo& frame)
{
	return GetFrameKeyDummy(frame.fbp, frame.fbw, frame.fbh, frame.psm);
}

inline u32 GetFrameKeyDummy(CRenderTarget* frame)
{
	return GetFrameKeyDummy(frame->fbp, frame->fbw, frame->fbh, frame->psm);
}

//------------------------ Inlines -------------------------

// Calculate maximum height for target
inline int get_maxheight(int fbp, int fbw, int psm)
{
	int ret;

	if (fbw == 0) return 0;

	ret = (((0x00100000 - 64 * fbp) / fbw) & ~0x1f);
	if (PSMT_ISHALF(psm)) ret *= 2;

	return ret;
}

// memory size for one row of texture. It depends on width of texture and number of bytes
// per pixel
inline u32 Pitch(int fbw) { return (RW(fbw) * 4) ; }

// memory size of whole texture. It is number of rows multiplied by memory size of row
inline u32 Tex_Memory_Size(int fbw, int fbh) { return (RH(fbh) * Pitch(fbw)); }
