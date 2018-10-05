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

#include <list>
#include <map>

#define TARGET_VIRTUAL_KEY 0x80000000

inline u32 GetFrameKey(int fbp, int fbw);

// manages render-to-texture targets
class CRenderTarget
{

	public:
		CRenderTarget();
		virtual ~CRenderTarget();

		virtual bool Create(const frameInfo& frame);
		virtual void Destroy();

		// set the GPU_POSXY variable, scissor rect, and current render target
		void SetTarget(int fbplocal, const Rect2& scissor, int context);
		void SetViewport();

		// copies/creates the feedback contents
		inline void CreateFeedback()
		{
			if (ptexFeedback == 0 || !(status&TS_FeedbackReady))
				_CreateFeedback();
		}

		virtual void Resolve();
		virtual void Resolve(int startrange, int endrange); // resolves only in the allowed range
		virtual void Update(int context, CRenderTarget* pdepth);
		virtual void ConvertTo32(); // converts a psm==2 target, to a psm==0
		virtual void ConvertTo16(); // converts a psm==0 target, to a psm==2

		virtual bool IsDepth() { return false; }

		void SetRenderTarget(int targ);

		void* psys;   // system data used for comparison
		u32 ptex;

		int fbp, fbw, fbh, fbhCalc; // if fbp is negative, virtual target (not mapped to any real addr)
		int start, end; // in bytes
		u32 lastused;	// time stamp since last used
		float4 vposxy;

		u32 fbm;
		u16 status;
		u8 psm;
		u8 resv0;
		Rect scissorrect;

		u8 created;	// Check for object destruction/creating for r201.

		//int startresolve, endresolve;
		u32 nUpdateTarg; // use this target to update the texture if non 0 (one time only)

		// this is optionally used when feedback effects are used (render target is used as a texture when rendering to itself)
		u32 ptexFeedback;

		enum TargetStatus
		{
			TS_Resolved = 1,
			TS_NeedUpdate = 2,
			TS_Virtual = 4, // currently not mapped to memory
			TS_FeedbackReady = 8, // feedback effect is ready and doesn't need to be updated
			TS_NeedConvert32 = 16,
			TS_NeedConvert16 = 32
		};
		float4 DefaultBitBltPos();
		float4 DefaultBitBltTex();

	private:
		void _CreateFeedback();
		inline bool InitialiseDefaultTexture(u32 *p_ptr, int fbw, int fbh) ;
};

// manages render targets
class CRenderTargetMngr
{
	public:
		typedef map<u32, CRenderTarget*> MAPTARGETS;

		enum TargetOptions
		{
			TO_DepthBuffer = 1,
			TO_StrictHeight = 2, // height returned has to be the same as requested
			TO_Virtual = 4
		};

		~CRenderTargetMngr() { Destroy(); }

		void Destroy();
		static MAPTARGETS::iterator GetOldestTarg(MAPTARGETS& m);
		
		bool isFound(const frameInfo& frame, MAPTARGETS::iterator& it, u32 opts, u32 key, int maxposheight);
		
		CRenderTarget* GetTarg(const frameInfo& frame, u32 Options, int maxposheight);
		inline CRenderTarget* GetTarg(int fbp, int fbw)
		{
			MAPTARGETS::iterator it = mapTargets.find(GetFrameKey(fbp, fbw));

			/*			if (fbp == 0x3600 && fbw == 0x100 && it == mapTargets.end())
						{
							ZZLog::Debug_Log("%x", GetFrameKey(fbp, fbw)) ;
							ZZLog::Debug_Log("%x %x", fbp, fbw);
							for(MAPTARGETS::iterator it1 = mapTargets.begin(); it1 != mapTargets.end(); ++it1)
								ZZLog::Debug_Log("\t %x %x %x %x", it1->second->fbw, it1->second->fbh, it1->second->psm, it1->second->fbp);
						}*/
			return it != mapTargets.end() ? it->second : NULL;
		}

		// gets all targets with a range
		void GetTargs(int start, int end, list<CRenderTarget*>& listTargets) const;

		// resolves all targets within a range
		__forceinline void Resolve(int start, int end);
		__forceinline void ResolveAll()
		{
			for (auto& it : mapTargets)
				it.second->Resolve();
		}

		void DestroyAllTargs(int start, int end, int fbw);
		void DestroyIntersecting(CRenderTarget* prndr);

		// promotes a target from virtual to real
		inline CRenderTarget* Promote(u32 key)
		{
			assert(!(key & TARGET_VIRTUAL_KEY));

			// promote to regular targ
			CRenderTargetMngr::MAPTARGETS::iterator it = mapTargets.find(key | TARGET_VIRTUAL_KEY);
			assert(it != mapTargets.end());

			CRenderTarget* ptarg = it->second;
			mapTargets.erase(it);

			DestroyIntersecting(ptarg);

			it = mapTargets.find(key);

			if (it != mapTargets.end())
			{
				DestroyTarg(it->second);
				it->second = ptarg;
			}
			else
				mapTargets[key] = ptarg;

			if (conf.settings().resolve_promoted)
				ptarg->status = CRenderTarget::TS_Resolved;
			else
				ptarg->status = CRenderTarget::TS_NeedUpdate;

			return ptarg;
		}

		void DestroyTarg(CRenderTarget* ptarg);
		void PrintTargets();
		MAPTARGETS mapTargets, mapDummyTargs;
	private:
		
		void DestroyAllTargetsHelper(void* ptr);
};