/*  GSOne
 *  Copyright (C) 2004-2018 PCSX2 Team
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

#ifndef __GS_H__
#define __GS_H__

struct _keyEvent;
typedef struct _keyEvent keyEvent;

#include <stdio.h>
#include "Pcsx2Defs.h"

#ifdef _WIN32
#include "Windows/GSwin.h"
#endif

#ifdef __linux__
#include "Linux/GSLinux.h"
#endif

#define GSdefs
#include "PS2Edefs.h"
#include "PS2Eext.h"

#include "GifTransfer.h"
#include "null/GSnull.h"

#ifdef _MSC_VER
#define EXPORT_C_(type) extern "C" type CALLBACK
#else
#define EXPORT_C_(type) extern "C" __attribute__((stdcall, externally_visible, visibility("default"))) type
#endif

//#define GS_LOG GSLog::Log

enum GS_ONE_RENDERER
{
    GS_ONE_NULL = 0,
    GS_ONE_VULKAN
};

extern const unsigned char version;
extern const unsigned char revision;
extern const unsigned char build;

typedef struct
{
    int Log;
    bool path3;
    GS_ONE_RENDERER renderer;
} Config;

typedef struct
{
    //u32 CSRw; // Divided CSRw into the two values we check.
    bool CSRsignal;
    bool CSRfinish;

    pathInfo path[4];
    bool Path3transfer;
    float q;
    u32 imageTransfer;
    int MultiThreaded;
    int nPath3Hack;

    GIFReg regs;
    GIFCTXTReg ctxt_regs[2];
} GSVars;

extern GSVars gs;

extern Config conf;
extern u32 GSKeyEvent;
extern bool GSShift, GSAlt;

extern void (*GSirq)();

namespace GSLog
{
extern bool Open();
extern void Close();
extern void Log(const char *fmt, ...);
extern void Message(const char *fmt, ...);
extern void Print(const char *fmt, ...);
extern void WriteLn(const char *fmt, ...);
};

extern void SaveConfig();
extern void LoadConfig();

#endif
