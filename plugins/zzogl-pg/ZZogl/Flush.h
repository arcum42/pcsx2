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
 
#ifndef ZZOGLFLUSH_H_INCLUDED
#define ZZOGLFLUSH_H_INCLUDED

#ifndef ZEROGS_DEVBUILD

#define INC_GENVARS()
#define INC_TEXVARS()
#define INC_ALPHAVARS()
#define INC_RESOLVE()

#define g_bUpdateEffect false
#define g_bSaveTex false
#define g_bSaveResolved false

#else // defined(ZEROGS_DEVBUILD)

#define INC_GENVARS() ++g_nGenVars
#define INC_TEXVARS() ++g_nTexVars
#define INC_ALPHAVARS() ++g_nAlphaVars
#define INC_RESOLVE() ++g_nResolve

extern bool g_bUpdateEffect;
extern bool g_bSaveTex;	// saves the current texture
extern bool g_bSaveResolved;
#endif // !defined(ZEROGS_DEVBUILD)

enum StencilBits
{
	STENCIL_ALPHABIT = 1,		// if set, dest alpha >= 0x80
	STENCIL_PIXELWRITE = 2,		// if set, pixel just written (reset after every Flush)
	STENCIL_FBA = 4,			// if set, just written pixel's alpha >= 0 (reset after every Flush)
	STENCIL_SPECIAL = 8		// if set, indicates that pixel passed its alpha test (reset after every Flush)
	//STENCIL_PBE = 16	
};
#define STENCIL_CLEAR	   (2|4|8|16)

enum ColorMask 
{
	COLORMASK_RED = 1,
	COLORMASK_GREEN = 2,
	COLORMASK_BLUE = 4,
	COLORMASK_ALPHA = 8
	
};
#define GL_COLORMASK(mask) glColorMask(!!((mask)&COLORMASK_RED), !!((mask)&COLORMASK_GREEN), !!((mask)&COLORMASK_BLUE), !!((mask)&COLORMASK_ALPHA))

// extern int g_nDepthBias;
extern float g_fBlockMult; // used for old cards, that do not support Alpha-32float textures. We store block data in u16 and use it. Note: only ever gets set to 1, so probably not working.
extern u32 g_nCurVBOIndex; // From GSCreate.cpp.
extern u8* g_pbyGSClut; // From HostMemory.cpp. Used in ZZClut.cpp, and ZZSave.cpp. (but not Flush.cpp.)
extern int ppf; // From GSmain.cpp.

extern bool s_bTexFlush; // From ZZClut.cpp. Is in ZZClut.h.

extern vector<u32> s_vecTempTextures;		   // temporary textures, released at the end of every frame. From RenderCRTC.cpp.
extern GLuint g_vboBuffers[VB_NUMBUFFERS]; // VBOs for all drawing commands. From GSCreate.cpp.
extern CRangeManager s_RangeMngr; // manages overwritten memory				// zz // From targets.h. :(

void FlushTransferRanges(const tex0Info* ptex);						//zz // From targets.cpp. Used in RenderCRTC.cpp, Flush.cpp.

// use to update the state
void SetTexVariables(int context, FRAGMENTSHADER* pfragment);			// zz
void SetTexInt(int context, FRAGMENTSHADER* pfragment, int settexint);		// zz
void SetAlphaVariables(const alphaInfo& ainfo);					// zzz
//void ResetAlphaVariables();

inline void SetAlphaTestInt(pixTest curtest);

inline void RenderAlphaTest(const VB& curvb, FRAGMENTSHADER* pfragment);
inline void RenderStencil(const VB& curvb, u32 dwUsingSpecialTesting);
inline void ProcessStencil(const VB& curvb);
inline void RenderFBA(const VB& curvb, FRAGMENTSHADER* pfragment);
inline void ProcessFBA(const VB& curvb, FRAGMENTSHADER* pfragment);			// zz

void SetContextTarget(int context);

void SetWriteDepth();
bool IsWriteDepth();
void SetDestAlphaTest();

// flush current vertices, call before setting new registers (the main render method)
// All from ZZoglFlush.cpp.
extern void Flush(int context); // Used in HostMemory.cpp, Regs.cpp, ZZClut.cpp, Drawing.cpp, Flush.cpp, VB.cpp.
extern void FlushBoth(); // Used in RenderCRTC.cpp, targets.cpp, Regs.cpp.
extern void SetTexFlush(); // Only used in Regs.cpp.

#endif // ZZOGLFLUSH_H_INCLUDED
