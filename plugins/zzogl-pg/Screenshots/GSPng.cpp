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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include <png.h>

// Save image as PNG
bool SavePNG(const char* filename, int width, int height, void* pdata)
{
    FILE *f;
    png_image image; /* The control structure used by libpng */
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

	//u8* image_buffer = new u8[width * height * 4];
    u8* psrc = (u8*)pdata;
    int pitch = 4 * width;

    memset(&image, 0, (sizeof image));

    f = fopen(filename, "wb");
	if (f == NULL) return false;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return false;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) return false;

    if (setjmp(png_jmpbuf(png_ptr))) return false;

    png_init_io(png_ptr, f);
    if (setjmp(png_jmpbuf(png_ptr))) return false;
     png_set_IHDR(png_ptr, info_ptr, width, height,
                     8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))) return false;

   // Write image data
   row = (png_bytep) malloc(pitch * sizeof(png_byte));

    for (int y = 0; y < height; ++y) 
    {
        for (int x = 0; x < width; ++x)
            for (int i = 0; i < 4 ; ++i)
            {
                row[4 * x + i] = psrc[y * pitch + 4 * x + i + 0];
            }
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, nullptr);
    if (setjmp(png_jmpbuf(png_ptr))) return false;

    if (png_ptr) png_destroy_write_struct(&png_ptr, info_ptr ? &info_ptr : nullptr);

    fclose(f);
    
	return true;
}