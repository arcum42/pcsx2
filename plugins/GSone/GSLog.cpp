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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>

#include <stdio.h>
#include <assert.h>

using namespace std;

#include "GS.h"
#include "GifTransfer.h"
#include "null/GSnull.h"

std::string s_strLogPath("logs");

namespace GSLog
{
FILE *gsLog;

bool Open()
{
    bool result = true;

    const std::string LogFile(s_strLogPath + "/GSOne.log");

    gsLog = fopen(LogFile.c_str(), "w");

    if (gsLog != NULL) {
        setvbuf(gsLog, NULL, _IONBF, 0);
    } else {
        Message("Can't create log file %s.", LogFile.c_str());
        result = false;
    }

    WriteLn("GSOne plugin version %d,%d", revision, build);
    WriteLn("GS init.");

    return result;
}

void Close()
{
    if (gsLog) {
        fclose(gsLog);
        gsLog = NULL;
    }
}

void Log(const char *fmt, ...)
{
    va_list list;

    if (!conf.Log || gsLog == NULL)
        return;

    va_start(list, fmt);
    vfprintf(gsLog, fmt, list);
    va_end(list);
}

void Message(const char *fmt, ...)
{
    va_list list;
    char msg[512];

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    SysMessage("%s\n", msg);
}

void Print(const char *fmt, ...)
{
    va_list list;
    char msg[512];

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    Log(msg);
    fprintf(stderr, "GSOne:%s", msg);
}


void WriteLn(const char *fmt, ...)
{
    va_list list;
    char msg[512];

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    Log("%s\n", msg);
    fprintf(stderr, "GSOne:%s\n", msg);
}
};
