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

#ifndef ZZOGLCREATE_H_INCLUDED
#define ZZOGLCREATE_H_INCLUDED

extern int GPU_TEXWIDTH; // ZZoglCreate.cpp - seriously, a bunch of unreated files are relying on these externs being here because they happen to use targets.h?
extern float g_fiGPU_TEXWIDTH; // ZZoglCreate.cpp

#define VB_BUFFERSIZE			   0x4000 // Used in ZZoglCreate.cpp and ZZoglSave.cpp.

extern void ZZGSStateReset();
extern u32 g_nCurVBOIndex;

#endif
