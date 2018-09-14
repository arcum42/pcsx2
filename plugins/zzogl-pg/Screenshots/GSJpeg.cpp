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

//extern "C"
//{
#ifdef _WIN32
#	define XMD_H
#	undef FAR
#define HAVE_BOOLEAN
#endif

#include "jpeglib.h"
//}

// save image as JPEG
// Clearly adapted from an example. I'm removing most of the comments.
bool SaveJPEG(const char* filename, int image_width, int image_height, const void* pdata, int quality)
{
	u8* image_buffer = new u8[image_width * image_height * 3];
	u8* psrc = (u8*)pdata;

	// Compression parameters.
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;	 /* physical row width in image buffer */
    
	// input data is rgba format, so convert to rgb
	u8* p = image_buffer;

	for (int i = 0; i < image_height; ++i)
	{
		for (int j = 0; j < image_width; ++j)
		{
			p[0] = psrc[0];
			p[1] = psrc[1];
			p[2] = psrc[2];
			p += 3;
			psrc += 4;
		}
	}

	// Set up error handling and compression object.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) // Must be binary.
	{
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width;	
	cinfo.image_height = image_height;
	cinfo.input_components = 3;	 /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;	 /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, true /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, true);


	row_stride = image_width * 3;   /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	delete []image_buffer;

	return true;
}