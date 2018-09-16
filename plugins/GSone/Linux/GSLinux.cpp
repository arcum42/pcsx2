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

#include "GS.h"
#include "GSLinux.h"
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

Display *display;
int screen;
GtkScrolledWindow *win;
//GLFWwindow* window;

int GSOpenWindow(void *pDsp, const char *Title)
{
    //glfwInit();

    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    //return 0;
    display = XOpenDisplay(0);
    screen = DefaultScreen(display);

    if (pDsp != NULL)
        *(Display **)pDsp = display;
    else
        return -1;

    return 0;
}

int GSOpenWindow2(void *pDsp, u32 flags)
{
    if (pDsp != NULL)
        win = *(GtkScrolledWindow **)pDsp;
    else
        return -1;

    return 0;
}

void GSCloseWindow()
{
    if (display != NULL)
        XCloseDisplay(display);
}

void GSProcessMessages()
{
    //glfwPollEvents();
    if (GSKeyEvent) {
        int myKeyEvent = GSKeyEvent;
        bool myShift = GSShift;
        GSKeyEvent = 0;

        switch (myKeyEvent) {
            case XK_F5:
                OnKeyboardF5(myShift);
                break;
            case XK_F6:
                OnKeyboardF6(myShift);
                break;
            case XK_F7:
                OnKeyboardF7(myShift);
                break;
            case XK_F9:
                OnKeyboardF9(myShift);
                break;
        }
    }
}


void HandleKeyEvent(keyEvent *ev)
{
    switch (ev->evt) {
        case KEYPRESS:
            switch (ev->key) {
                case XK_F5:
                case XK_F6:
                case XK_F7:
                case XK_F9:
                    GSKeyEvent = ev->key;
                    break;
                case XK_Escape:
                    break;
                case XK_Shift_L:
                case XK_Shift_R:
                    GSShift = true;
                    break;
                case XK_Alt_L:
                case XK_Alt_R:
                    GSAlt = true;
                    break;
            }
            break;
        case KEYRELEASE:
            switch (ev->key) {
                case XK_Shift_L:
                case XK_Shift_R:
                    GSShift = false;
                    break;
                case XK_Alt_L:
                case XK_Alt_R:
                    GSAlt = false;
                    break;
            }
    }
}
