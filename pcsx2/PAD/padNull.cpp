/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

// 

#include "padNull.h"

keyEvent event;
static keyEvent s_event;

s32 padNull_PADinit(u32 flags)
{
    return 0;
}

void padNull_PADshutdown()
{

}

s32 padNull_PADopen(void *pDsp)
{
    memset(&event, 0, sizeof(event));

    // In theory, grab the display here so we can get events from it, and set autorepeat to off on the keyboard.
    return 0;
}

void padNull_PADclose()
{
    // Close the plugin. Turn back on autorepeat.
}

// PADkeyEvent is called every vsync (return NULL if no event)
keyEvent * padNull_PADkeyEvent()
{
    s_event = event;
    event.evt = 0;
    event.key = 0;

    return &s_event;
}

u8 padNull_PADstartPoll(int pad)
{
    return 0;
}

u8 padNull_PADpoll(u8 value)
{
    return 0;
}


// call to give a hint to the PAD plugin to query for the keyboard state. A
// good plugin will query the OS for keyboard state ONLY in this function.
// This function is necessary when multithreading because otherwise
// the PAD plugin can get into deadlocks with the thread that really owns
// the window (and input). Note that PADupdate can be called from a different
// thread than the other functions, so mutex or other multithreading primitives
// have to be added to maintain data integrity.
u32 padNull_PADquery()
// returns: 1 if supported pad1
//			2 if supported pad2
//			3 if both are supported
{
    return 3;
}

void padNull_PADupdate(int pad)
{
    // This is where you actually check the keyboard/controller state and set event. If this was a real plugin.
}

void padNull_PADgsDriverInfo(GSdriverInfo *info)
{
}

s32 padNull_PADfreeze(int mode, freezeData *data)
{
    return 0;
}

s32 padNull_PADtest()
{
    return 0;
}