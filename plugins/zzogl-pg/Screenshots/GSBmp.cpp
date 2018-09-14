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

// Save image as BMP - based on example code:
// https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
bool SaveBMP(const char* filename, int width, int height, void* pdata)
{
    FILE *f;
    u32 imgsize = 3 * width * height;
    u32 filesize = 54 + imgsize;

    u8* image_buffer = new u8[imgsize];
	u8* psrc = (u8*)pdata;

    u8 bmp_fileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    u8 bmp_infoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    u8 bmp_padding[3] = {0,0,0};

	f = fopen(filename, "wb");
	if (f == NULL) return false;

	// input data is rgba format, so convert to rgb
	u8* p = image_buffer;

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			p[0] = psrc[2];
			p[1] = psrc[1];
			p[2] = psrc[0];
			p += 3;
			psrc += 4;
		}
	}

    bmp_fileheader[ 2] = (u8)(filesize);
    bmp_fileheader[ 3] = (u8)(filesize>>8);
    bmp_fileheader[ 4] = (u8)(filesize>>16);
    bmp_fileheader[ 5] = (u8)(filesize>>24);

    bmp_infoheader[ 4] = (u8)(width);
    bmp_infoheader[ 5] = (u8)(width>>8);
    bmp_infoheader[ 6] = (u8)(width>>16);
    bmp_infoheader[ 7] = (u8)(width>>24);
    bmp_infoheader[ 8] = (u8)(height);
    bmp_infoheader[ 9] = (u8)(height>>8);
    bmp_infoheader[10] = (u8)(height>>16);
    bmp_infoheader[11] = (u8)(height>>24);

	fwrite(bmp_fileheader, 1, 14, f);
    fwrite(bmp_infoheader, 1, 40, f);

    for(int i = 0; i < height; i++)
    {
        fwrite(image_buffer + (width * (height - i - 1) * 3), 3, width, f);
        fwrite(bmp_padding, 1, (4 - (width * 3) % 4) % 4, f);
    }

    fclose(f);
    delete []image_buffer;

	return true;
}
