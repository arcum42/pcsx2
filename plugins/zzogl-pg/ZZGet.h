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

//--------------------------- Inlines for bitwise ops
//--------------------------- textures

// Function for calculating overal height from frame data.
inline int ZZOgl_fbh_Calc(int fbp, int fbw, int psm)
{
	int fbh = (1024 * 1024 - 64 * fbp) / fbw;
	fbh &= ~0x1f;

	if (PSMT_ISHALF(psm)) fbh *= 2;
	if (fbh > 1024) fbh = 1024;

	//ZZLog::Debug_Log("ZZOgl_fbh_Calc: 0x%x", fbh);
	return fbh;
}

inline int ZZOgl_fbh_Calc(frameInfo frame)
{
	return ZZOgl_fbh_Calc(frame.fbp, frame.fbw, frame.psm);
}

namespace ZZGet 
{

// Tex0Info (TEXD_x registers) bits, lower word
// The register is really 64-bit, but we use 2 32bit ones to represent it
// Obtain tbp0 -- Texture Buffer Base Pointer (Word Address/64) -- from data. Bits 0-13.

static __forceinline int tbp0_TexBits(u32 data)
{
	//return tex_0_info(data).tbp0;
	return (data) & 0x3fff;
}

// Obtain tbw -- Texture Buffer Width (Texels/64) -- from data, do not multiply to 64. Bits 14-19
// ( data & 0xfc000 ) >> 14
static __forceinline int tbw_TexBits(u32 data)
{
	//return tex_0_info(data).tbw;
	return (data >> 14) & 0x3f;
}

// Obtain tbw -- Texture Buffer Width (Texels) -- from data, do multiply to 64, never return 0.
static __forceinline int tbw_TexBitsMult(u32 data)
{
	//return text_0_info(data).tbw_mult();
	int result = tbw_TexBits(data);

	if (result == 0)
		return 64;
	else
		return (result << 6);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25.
// (data & 0x3f00000) >> 20
static __forceinline int psm_TexBits(u32 data)
{
	//return tex_0_info(data).psm;
	return	((data >> 20) & 0x3f);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25. Fix incorrect psm == 9
static __forceinline int psm_TexBitsFix(u32 data)
{
	//return tex_0_info(data).psm_fix();
	int result = psm_TexBits(data) ;
//	ZZLog::Debug_Log("result %d", result);

	if (result == 9) result = 1;

	return result;
}

// Obtain tw -- Texture Width (Width = 2^TW) -- from data. Bits 26-29
// (data & 0x3c000000)>>26
static __forceinline u16 tw_TexBits(u32 data)
{
	//return tex_0_info(data).tw;
	return	((data >> 26) & 0xf);
}

// Obtain tw -- Texture Width (Width = TW) -- from data. Width could newer be more than 1024.
static __forceinline u16 tw_TexBitsExp(u32 data)
{
	//return tex_0_info(data).tw_exp();
	u16 result = tw_TexBits(data);

	if (result > 10) result = 10;

	return (1 << result);
}

// TH set at the border of upper and higher words.
// Obtain th -- Texture Height (Height = 2^TH) -- from data. Bits 30-31 lower, 0-1 higher
// (dataLO & 0xc0000000) >> 30 + (dataHI & 0x3) * 0x4
static __forceinline u16 th_TexBits(u32 dataLO, u32 dataHI)
{
	//return tex_0_info(dataLO, dataHI).th;
	return (((dataLO >> 30) & 0x3) | ((dataHI & 0x3) << 2));
}

// Obtain th --Texture Height (Height = 2^TH) -- from data. Height could newer be more than 1024.
static __forceinline u16 th_TexBitsExp(u32 dataLO, u32 dataHI)
{
	//return tex_0_info(dataLO, dataHI).th_exp();
	u16 result = th_TexBits(dataLO, dataHI);

	if (result > 10) result = 10;

	return	(1 << result);
}

// Tex0Info bits, higher word.
// Obtain tcc -- Texture Color Component 0=RGB, 1=RGBA + use Alpha from TEXA reg when not in PSM -- from data. Bit 3
// (data & 0x4)>>2
static __forceinline u8 tcc_TexBits(u32 data)
{
	//return tex_0_info(0, data).tcc;
	return ((data >>  2) & 0x1);
}

// Obtain tfx -- Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2) -- from data. Bit 4-5
// (data & 0x18)>>3
static __forceinline u8 tfx_TexBits(u32 data)
{
	//return tex_0_info(0, data).tfx;
	return ((data >>  3) & 0x3);
}

// Obtain cbp from data -- Clut Buffer Base Pointer (Address/256) -- Bits 5-18
// (data & 0x7ffe0)>>5
static __forceinline int cbp_TexBits(u32 data)
{
	//return tex_0_info(0, data).cbp;
	return ((data >>  5) & 0x3fff);
}

// Obtain cpsm from data -- Clut pixel Storage Format -- Bits 19-22. 22nd is at no use.
// (data & 0x700000)>>19
// 0000 - psmct32; 0010 - psmct16; 1010 - psmct16s.
static __forceinline u8 cpsm_TexBits(u32 data)
{
	//return (tex_0_info(0, data).cpsm & 0xe);
	return ((data >> 19) & 0xe);
}

// Obtain csm -- I don't know what is it -- from data. Bit 23
// (data & 0x800000)>>23
// csm is the clut storage mode. 0 for CSM1, 1 for CSM2.
static __forceinline u8 csm_TexBits(u32 data)
{
	//return tex_0_info(0, data).csm;
	return ((data >> 23) & 0x1);
}

// Obtain csa -- -- from data. Bits 24-28
// (data & 0x1f000000)>>24
static __forceinline u8 csa_TexBits(u32 data)
{
	//return tex_0_info(0, data).csa_fix();

	if ((data & 0x700000) == 0)  // it is cpsm < 2 check
		return ((data >> 24) & 0xf);
	else
		return ((data >> 24) & 0x1f);
}

// Obtain cld --   -- from data. Bits 29-31
// (data & 0xe0000000)>>29
static __forceinline u8 cld_TexBits(u32 data)
{
	//return tex_0_info(0, data).cld;
	return ((data >> 29) & 0x7);
}

//-------------------------- frames
// FrameInfo bits.
// Obtain fbp -- frame Buffer Base Pointer (Word Address/2048) -- from data. Bits 0-15
inline int fbp_FrameBits(u32 data)
{
	return ((data) & 0x1ff);
}

// So we got address / 64, henceby frame fbp and tex tbp have the same dimension -- "real address" is x64.
inline int fbp_FrameBitsMult(u32 data)
{
	return (fbp_FrameBits(data) << 5);
}

// Obtain fbw -- width (Texels/64) -- from data. Bits 16-23
inline int fbw_FrameBits(u32 data)
{
	return ((data >> 16) & 0x3f);
}

inline int fbw_FrameBitsMult(u32 data)
{
	return (fbw_FrameBits(data) << 6);
}


// Obtain psm -- Pixel Storage Format -- from data. Bits 24-29.
// (data & 0x3f000000) >> 24
inline int psm_FrameBits(u32 data)
{
	return	((data >> 24) & 0x3f);
}

// Calculate fbh from data, It does not set in register
inline int fbh_FrameBitsCalc(u32 data)
{
	int fbh = 0;
	int fbp = fbp_FrameBits(data);
	int fbw = fbw_FrameBits(data);
	int psm = psm_FrameBits(data);

	if (fbw > 0) fbh = ZZOgl_fbh_Calc(fbp, fbw, psm) ;

	return fbh ;
}

// Obtain fbm -- frame mask -- from data. All higher word.
inline u32 fbm_FrameBits(u32 data)
{
	return (data);
}

// Obtain fbm -- frame mask -- from data. All higher word. Fixed from psm == PCMT24 (without alpha)
inline u32 fbm_FrameBitsFix(u32 dataLO, u32 dataHI)
{
	if (PSMT_BITMODE(psm_FrameBits(dataLO)) == 1)
		return (dataHI | 0xff000000);
	else
		return dataHI;
}

// obtain colormask RED
inline u32 fbmRed_FrameBits(u32 data)
{
	return (data & 0xff);
}

// obtain colormask Green
inline u32 fbmGreen_FrameBits(u32 data)
{
	return ((data >> 8) & 0xff);
}

// obtain colormask Blue
inline u32 fbmBlue_FrameBits(u32 data)
{
	return ((data >> 16) & 0xff);
}

// obtain colormask Alpha
inline u32 fbmAlpha_FrameBits(u32 data)
{
	return ((data >> 24) & 0xff);
}

// obtain colormask Alpha
inline u32 fbmHighByte(u32 data)
{
	return (!!(data & 0x80000000));
}
};

//-------------------------- tex0 comparison
// Check if old and new tex0 registers have only clut difference
inline bool ZZOglAllExceptClutIsSame(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[0] == newtex[0]) && ((oldtex[1] & 0x1f) == (newtex[1] & 0x1f)));
}

// Check if the CLUT registers are same, except CLD
inline bool ZZOglClutMinusCLDunchanged(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[1] & 0x1fffffe0) == (newtex[1] & 0x1fffffe0));
}

// Check if CLUT storage mode is not changed (CSA, CSM and CSPM)
inline bool ZZOglClutStorageUnchanged(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[1] & 0x1ff10000) == (newtex[1] & 0x1ff10000));
}

// call to load CLUT data (depending on CLD) - Regs.cpp
extern void texClutWrite(int ctx);

// Perform clutting for flushed texture. Better check if it needs a prior call.
inline void CluttingForFlushedTex(tex0Info* tex0, u32 Data, int ictx)
{
	tex0->cbp  = ZZGet::cbp_TexBits(Data);
	tex0->cpsm = ZZGet::cpsm_TexBits(Data);
	tex0->csm  = ZZGet::csm_TexBits(Data);
	tex0->csa  = ZZGet::csa_TexBits(Data);
	tex0->cld  = ZZGet::cld_TexBits(Data);

	texClutWrite(ictx);
 };