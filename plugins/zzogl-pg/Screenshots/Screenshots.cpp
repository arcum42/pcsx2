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

// Texture and avi saving to file functions

//------------------ Includes
 #include "Util.h"
#if defined(_WIN32)
#	include "Utilities/RedtapeWindows.h"
#	include <aviUtil.h>
#	include "resource.h"
#endif
#include <stdlib.h>

#include "Targets/targets.h"
#include "Memory/Mem.h"
#include "Screenshots/Screenshots.h"

string strSnapshot;
bool g_bMakeSnapshot = false;

//------------------ Code

// Set variables need to made a snapshoot when it's possible
void SaveSnapshot(const char* filename)
{
	g_bMakeSnapshot = true;
	strSnapshot = filename;
}

// Save curent renderer.
bool SaveRenderTarget(const char* filename, int width, int height, int format)
{
	bool bflip = height < 0;
	height = abs(height);
	vector<u32> data(width*height);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

	if (glGetError() != GL_NO_ERROR) return false;

	if (bflip)
	{
		// swap scanlines
		vector<u32> scanline(width);

		for (int i = 0; i < height / 2; ++i)
		{
			memcpy(&scanline[0], &data[i * width], width * 4);
			memcpy(&data[i * width], &data[(height - i - 1) * width], width * 4);
			memcpy(&data[(height - i - 1) * width], &scanline[0], width * 4);
		}
	}

	switch (format)
	{
		case EXT_JPG:
			return SaveJPEG(filename, width, height, &data[0], 70);
			break;

		case EXT_TGA:
			return SaveTGA(filename, width, height, &data[0]);
			break;
		
		case EXT_BMP:
			return SaveBMP(filename, width, height, &data[0]);
			break;

		case EXT_PNG:
		default:
			return false;
	}
	return false;
}

// Save selected texture
bool SaveTexture(const char* filename, u32 textarget, u32 tex, int width, int height, int ext_format)
{
	vector<u32> data(width*height);
	glBindTexture(textarget, tex);
	glGetTexImage(textarget, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

	if (glGetError() != GL_NO_ERROR) return false;

	if (ext_format == EXT_BMP)
		return SaveBMP(filename, width, height, &data[0]);
	else if (ext_format == EXT_TGA)
		return SaveTGA(filename, width, height, &data[0]);
	else
		return false;
}
