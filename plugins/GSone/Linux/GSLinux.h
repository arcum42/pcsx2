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
#ifndef __GSLINUX_H__
#define __GSLINUX_H__

#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

extern int GSOpenWindow(void *pDsp, const char *Title);
extern int GSOpenWindow2(void *pDsp, u32 flags);
extern void GSCloseWindow();
extern void GSProcessMessages();
extern void HandleKeyEvent(keyEvent *ev);

#endif
