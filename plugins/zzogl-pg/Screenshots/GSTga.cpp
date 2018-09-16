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

//Windows have no snprintf
#if defined(_WIN32)
#	define snprintf sprintf_s
#endif

#define 	TGA_FILE_NAME_MAX_LENGTH 	 20
#define		MAX_NUMBER_SAVED_TGA		200

#if defined(_MSC_VER)
#	pragma pack(push, 1)
#endif
// This is the definition of a TGA header. We need it to function below
struct TGA_HEADER
{
	u8  identsize;		  	// size of ID field that follows 18 u8 header (0 usually)
	u8  colourmaptype;	  	// type of colour map 0=none, 1=has palette
	u8  imagetype;		  	// type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	s16 colourmapstart;	 	// first colour map entry in palette
	s16 colourmaplength;		// number of colours in palette
	u8  colourmapbits;	  	// number of bits per palette entry 15,16,24,32

	s16 xstart;			// image x origin
	s16 ystart;			// image y origin
	s16 width;			// image width in pixels
	s16 height;			// image height in pixels
	u8  bits;			// image bits per pixel 8,16,24,32
	u8  descriptor;		 	// image descriptor bits (vh flip bits)

	// pixel data follows header
#if defined(_MSC_VER)
};

#	pragma pack(pop)
#	else
}

__attribute__((packed));
#endif

int TexNumber = 0;

// Save image as TGA
bool SaveTGA(const char* filename, int width, int height, void* pdata)
{
	TGA_HEADER hdr;
	FILE* f = fopen(filename, "wb");

	if (f == NULL) return false;

	assert(sizeof(TGA_HEADER) == 18 && sizeof(hdr) == 18);

	memset(&hdr, 0, sizeof(hdr));

	hdr.imagetype = 2;
	hdr.bits = 32;
	hdr.width = width;
	hdr.height = height;
	hdr.descriptor |= 8 | (1 << 5); 	// 8bit alpha, flip vertical

	fwrite(&hdr, sizeof(hdr), 1, f);
	fwrite(pdata, width * height * 4, 1, f);

	fclose(f);

	return true;
}

// It's nearly the same as save texture
void SaveTex(tex0Info* ptex, int usevid)
{
	vector<u32> data(ptex->tw*ptex->th);
	vector<u8> srcdata;

	u32* dst = &data[0];
	//u8* psrc = g_pbyGSMemory;
	u8* psrc = gs_mem.get();

	CMemoryTarget* pmemtarg = NULL;

	if (usevid)
	{
		pmemtarg = g_MemTargs.GetMemoryTarget(*ptex, false);
		assert(pmemtarg != NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE_NV, pmemtarg->ptex->tex);
		srcdata.resize(4 * pmemtarg->texW * pmemtarg->texH);

        // FIXME strangely this function call seem to crash pcsx2 on atelier of iris 1
        // Note: fmt is GL_UNSIGNED_SHORT_1_5_5_5_REV
		glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, pmemtarg->fmt, &srcdata[0]);

		u32 offset = gs_mem.MemorySize(pmemtarg->realy); /*4 * GPU_TEXWIDTH * x*/
		
		switch(ptex->psm)
		{
			case PSMT8:
				offset *= CLUT_PIXEL_SIZE(ptex->cpsm);
				break;
			case PSMT4:
				offset *= CLUT_PIXEL_SIZE(ptex->cpsm) * 2;
				break;
			default:
				break;
		}

		psrc = &srcdata[0] - offset;
	}

	for (int i = 0; i < ptex->th; ++i)
	{
		for (int j = 0; j < ptex->tw; ++j)
		{
			u32 u = 0;
			u32 addr;

			switch (ptex->psm)
			{
				case PSMCT32:
					addr = getPixelAddress32(j, i, ptex->tbp0, ptex->tbw);
					if (addr * 4 < MEMORY_END)
						u = readPixel32(psrc, j, i, ptex->tbp0, ptex->tbw);
					else
						u = 0;
					break;

				case PSMCT24:
					addr = getPixelAddress24(j, i, ptex->tbp0, ptex->tbw);
					if (addr * 4 < MEMORY_END)
						u = readPixel24(psrc, j, i, ptex->tbp0, ptex->tbw);
					else
						u = 0;
					break;

				case PSMCT16:
					addr = getPixelAddress16(j, i, ptex->tbp0, ptex->tbw);
					if (addr * 2 < MEMORY_END)
					{
						u = readPixel16(psrc, j, i, ptex->tbp0, ptex->tbw);
						u = RGBA16to32(u);
					}
					else
					{
						u = 0;
					}
					break;

				case PSMCT16S:
					addr = getPixelAddress16(j, i, ptex->tbp0, ptex->tbw);
					if (addr * 2 < MEMORY_END)
					{
						u = readPixel16S(psrc, j, i, ptex->tbp0, ptex->tbw);
						u = RGBA16to32(u);
					}
					else 
					{
						u = 0;
					}
					break;

				case PSMT8:
					addr = getPixelAddress8(j, i, ptex->tbp0, ptex->tbw);
					if (addr < MEMORY_END)
					{
						if (usevid)
						{
							if (PSMT_IS32BIT(ptex->cpsm))
								u = *(u32*)(psrc + 4 * addr);
							else
								u = RGBA16to32(*(u16*)(psrc + 2 * addr));
						}
						else
						{
							u = readPixel8(psrc, j, i, ptex->tbp0, ptex->tbw);
						}
					}
					else
					{
						u = 0;
					}
					break;

				case PSMT4:
					addr = getPixelAddress4(j, i, ptex->tbp0, ptex->tbw);
					if (addr < 2*MEMORY_END)
					{
						if (usevid)
						{
							if (PSMT_IS32BIT(ptex->cpsm))
								u = *(u32*)(psrc + 4 * addr);
							else
								u = RGBA16to32(*(u16*)(psrc + 2 * addr));
						}
						else
						{
							u = readPixel4(psrc, j, i, ptex->tbp0, ptex->tbw);
						}
					}
					else 
					{
						u = 0;
					}

					break;

				case PSMT8H:
					addr = getPixelAddress8H(j, i, ptex->tbp0, ptex->tbw);
					if (4*addr < MEMORY_END)
					{
						if (usevid)
						{
							if (PSMT_IS32BIT(ptex->cpsm))
								u = *(u32*)(psrc + 4 * addr);
							else
								u = RGBA16to32(*(u16*)(psrc + 2 * addr));
						}
						else
						{
							u = readPixel8H(psrc, j, i, ptex->tbp0, ptex->tbw);
						}
					}
					else 
					{
						u = 0;
					}
					break;

				case PSMT4HL:
					addr = getPixelAddress4HL(j, i, ptex->tbp0, ptex->tbw);
					if (4*addr < MEMORY_END)
					{
						if (usevid)
						{
							if (PSMT_IS32BIT(ptex->cpsm))
								u = *(u32*)(psrc + 4 * addr);
							else
								u = RGBA16to32(*(u16*)(psrc + 2 * addr));
						}
						else
						{
							u = readPixel4HL(psrc, j, i, ptex->tbp0, ptex->tbw);
						}
					}
					else 
					{
						u = 0;
					}
					break;

				case PSMT4HH:
					addr = getPixelAddress4HH(j, i, ptex->tbp0, ptex->tbw);
					if (4*addr < MEMORY_END)
					{
						if (usevid)
						{
							if (PSMT_IS32BIT(ptex->cpsm))
								u = *(u32*)(psrc + 4 * addr);
							else
								u = RGBA16to32(*(u16*)(psrc + 2 * addr));
						}
						else
						{
							u = readPixel4HH(psrc, j, i, ptex->tbp0, ptex->tbw);
						}
					}
					else
					{
						u = 0;
					}
					break;

				case PSMT32Z:
					addr = getPixelAddress32Z(j, i, ptex->tbp0, ptex->tbw);
					if (4*addr < MEMORY_END)
						u = readPixel32Z(psrc, j, i, ptex->tbp0, ptex->tbw);
					else 
						u = 0;
					break;

				case PSMT24Z:
					addr = getPixelAddress24Z(j, i, ptex->tbp0, ptex->tbw);
					if (4*addr < MEMORY_END)
						u = readPixel24Z(psrc, j, i, ptex->tbp0, ptex->tbw);
					else 
						u = 0;
					break;

				case PSMT16Z:
					addr = getPixelAddress16Z(j, i, ptex->tbp0, ptex->tbw);
					if (2*addr < MEMORY_END)
						u = readPixel16Z(psrc, j, i, ptex->tbp0, ptex->tbw);
					else 
						u = 0;
					break;

				case PSMT16SZ:
					addr = getPixelAddress16SZ(j, i, ptex->tbp0, ptex->tbw);
					if (2*addr < MEMORY_END)
						u = readPixel16SZ(psrc, j, i, ptex->tbp0, ptex->tbw);
					else 
						u = 0;
					break;

				default:
					assert(0);
			}

			*dst++ = u;
		}
	}

	char Name[TGA_FILE_NAME_MAX_LENGTH];

	snprintf(Name, TGA_FILE_NAME_MAX_LENGTH, "Tex.%d.tga", TexNumber);
	SaveTGA(Name, ptex->tw, ptex->th, &data[0]);

	TexNumber++;
	if (TexNumber > MAX_NUMBER_SAVED_TGA) TexNumber = 0;
}


// Do the save texture and return file name of it
// Do not forget to call free(), other wise there would be memory leak!
char* NamedSaveTex(tex0Info* ptex, int usevid)
{
	SaveTex(ptex, usevid);

	char* Name = (char*)malloc(TGA_FILE_NAME_MAX_LENGTH);
	snprintf(Name, TGA_FILE_NAME_MAX_LENGTH, "Tex.%d.tga", TexNumber);

	return Name;
}