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

#include "Util.h"
#include "Targets/targets.h"
#include "Memory/Mem.h"
#include "Screenshots/Screenshots.h"

// AVI Capture
int s_aviinit = 0;
int s_avicapturing = 0;

// AVI capture stuff
// AVI start -- set needed global variables
void StartCapture()
{
	if (conf.captureAvi()) return;
	if (!s_aviinit)
	{
#ifdef _WIN32
		START_AVI("zerogs.avi");
#else // linux
		//TODO
#endif
		s_aviinit = 1;
	}
	else
	{
		ZZLog::Error_Log("Continuing from previous capture.");
	}

	s_avicapturing = 1;
	conf.setCaptureAvi(true);
	ZZLog::Warn_Log("Started recording zerogs.avi.");
	
}

// Stop.
void StopCapture()
{
	if (!conf.captureAvi()) return;
	s_avicapturing = 0;
	conf.setCaptureAvi(false);
	ZZLog::Warn_Log("Stopped recording.");
}

// And capture frame does not work on linux.
void CaptureFrame()
{
	if ((!s_avicapturing) || (!s_aviinit)) return;

	vector<u32> data(GLWin.backbuffer.w * GLWin.backbuffer.h);
	glReadPixels(0, 0, GLWin.backbuffer.w, GLWin.backbuffer.h, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

	if (glGetError() != GL_NO_ERROR) return;

#ifdef _WIN32
	int fps = SMODE1->CMOD == 3 ? 50 : 60;

	bool bSuccess = ADD_FRAME_FROM_DIB_TO_AVI("AAAA", fps, GLWin.backbuffer.w, GLWin.backbuffer.h, 32, &data[0]);

	if (!bSuccess)
	{
		s_avicapturing = 0;
		STOP_AVI();
		ZZAddMessage("Failed to create avi");
		return;
	}

#else // linux
	//TODO
#endif // _WIN32
}

// Special function, which is safe to call from any other file, without aviutils problems.
void Stop_Avi()
{
#ifdef _WIN32
	STOP_AVI();
#else
// Does not support yet
#endif
}

void Delete_Avi_Capture()
{
	if (s_aviinit)
	{
		StopCapture();
		Stop_Avi();
		ZZLog::Error_Log("zerogs.avi stopped.");
		s_aviinit = 0;
	}
}
