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

#include "../GS.h"

HINSTANCE HInst;
HWND GShwnd = NULL;

LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int GSOpenWindow(void *pDsp, const char *Title)
{
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                     GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                     "PS2EMU_GSNULL", NULL};
    RegisterClassEx(&wc);

    GShwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "PS2EMU_GSNULL", Title,
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, wc.hInstance, NULL);

    if (GShwnd == NULL) {
        GSLog::WriteLn("Failed to create window. Exiting...");
        return -1;
    }

    if (pDsp != NULL)
        *(uptr *)pDsp = (uptr)GShwnd;

    return 0;
}

void GSCloseWindow()
{
    DestroyWindow(GShwnd);
}

void GSProcessMessages()
{
}

// GSkeyEvent gets called when there is a keyEvent from the PAD plugin
void HandleKeyEvent(keyEvent *ev)
{
}
