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

#include "Util.h"
#include "GS.h"
//#include "Targets/ZZTargets.h"

extern int g_TransferredToGPU;

extern int VALIDATE_THRESH;
extern u32 TEXDESTROY_THRESH;
#define FORCE_TEXDESTROY_THRESH (3) // destroy texture after FORCE_TEXDESTROY_THRESH frames

// manages contiguous chunks of memory (width is always 1024)
class CMemoryTarget
{
	public:
		struct TEXTURE
		{
			inline TEXTURE() : tex(0), memptr(NULL), ref(0) {}
			inline ~TEXTURE() { glDeleteTextures(1, &tex); _aligned_free(memptr); }

			u32 tex;
			u8* memptr;  // GPU memory used for comparison
			int ref;
		};

		inline CMemoryTarget() : ptex(NULL), starty(0), height(0), realy(0), realheight(0), usedstamp(0), psm(0), cpsm(0), channels(0), clearminy(0), clearmaxy(0), validatecount(0), clut(NULL), clutsize(0) {}

		inline CMemoryTarget(const CMemoryTarget& r)
		{
			ptex = r.ptex;

			if (ptex != NULL) ptex->ref++;

			starty = r.starty;
			height = r.height;
			realy = r.realy;
			realheight = r.realheight;
			usedstamp = r.usedstamp;
			psm = r.psm;
			cpsm = r.cpsm;
			clut = r.clut;
			clearminy = r.clearminy;
			clearmaxy = r.clearmaxy;
			widthmult = r.widthmult;
			texH = r.texH;
			texW = r.texW;
			channels = r.channels;
			validatecount = r.validatecount;
			fmt = r.fmt;
		}

		~CMemoryTarget() { Destroy(); }

		inline void Destroy()
		{
			if (ptex != NULL && ptex->ref > 0)
			{
				if (--ptex->ref <= 0) delete ptex;
			}

			ptex = NULL;

            _aligned_free(clut);
            clut = NULL;
            clutsize = 0;
		}

		// returns true if clut data is synced
		bool ValidateClut(const tex0Info& tex0);
		// returns true if tex data is synced
		bool ValidateTex(const tex0Info& tex0, int starttex, int endtex, bool bDeleteBadTex);

		// realy is offset in pixels from start of valid region
		// so texture in memory is [realy,starty+height]
		// valid texture is [starty,starty+height]
		// offset in mem [starty-realy, height]
		TEXTURE* ptex; // can be 16bit

		int starty, height; // assert(starty >= realy)
		int realy, realheight; // this is never touched once allocated
		// realy is start pointer of data in 4M data block (start) and size (end-start).
		
		u32 usedstamp;
		u8 psm, cpsm; // texture and clut format. For psm, only 16bit/32bit differentiation matters

		u32 fmt;

		int widthmult;	// Either 1 or 2.
		int channels;	// The number of pixels per PSM format word. channels == PIXELS_PER_WORD(psm)
						// This is the real drawing size in pixels of the texture in renderbuffer.
		int texW;		// (realheight + widthmult - 1)/widthmult == realheight or [(realheight+1)/2]
		int texH;		//  GPU_TEXWIDTH *widthmult * channels;			

		int clearminy, clearmaxy;	// when maxy > 0, need to check for clearing

		int validatecount; // count how many times has been validated, if too many, destroy

		u8* clut;        // Clut texture data. Null otherwise
        int clutsize;    // size of the clut array. 0 otherwise 
};

class CMemoryTargetMngr
{
	public:
		CMemoryTargetMngr() : curstamp(0) {}

		CMemoryTarget* GetMemoryTarget(const tex0Info& tex0, bool forcevalidate); // pcbp is pointer to start of clut

		void Destroy(); // destroy all targs

		void ClearRange(int starty, int endy); // set all targets to cleared
		void DestroyCleared(); // flush all cleared targets
		void DestroyOldest();

		list<CMemoryTarget> listTargets, listClearedTargets;
		u32 curstamp;

	private:
		CMemoryTarget* ClearedTargetsSearch(u32 fmt, int widthmult, int channels, int height);
		int CompareTarget(list<CMemoryTarget>::iterator& it, const tex0Info& tex0, int clutsize);
		list<CMemoryTarget>::iterator DestroyTargetIter(list<CMemoryTarget>::iterator& it);
		void GetClutVariables(int& clutsize, const tex0Info& tex0);
		void GetMemAddress(int& start, int& end,  const tex0Info& tex0);
		CMemoryTarget* SearchExistTarget(int start, int end, int clutsize, const tex0Info& tex0);
		CMemoryTarget* SearchExistForceTarget(int start, int end, int clutsize, const tex0Info& tex0);
        CMemoryTarget* CreateMemoryTarget(int start, int end, int clutsize, const tex0Info& tex0);
};
