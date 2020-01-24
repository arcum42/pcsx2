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

// This is basically the core of the USBNull plugin, though I've cleaned up and changed things a bit.

#include "usbNull.h"

USBcallback USBirq = nullptr;
s8 *usbregs = nullptr, *ram = nullptr;

s32 usbNull_USBinit()
{
    Console.WriteLn("Initializing built-in null USB plugin.");

    // Initialize memory structures here.
    usbregs = (s8 *)calloc(0x10000, 1);

    if (usbregs == nullptr) 
    {
        Console.WriteLn("Error allocating memory to USB.");
        return -1;
    }

    return 0;
}

void usbNull_USBshutdown()
{
    free(usbregs);
    usbregs = nullptr;
}

s32 usbNull_USBopen(void *pDsp)
{
    Console.WriteLn("Opening built-in null USB plugin.");

    // Take care of anything else we need on opening, other then initialization.
    return 0;
}

void usbNull_USBclose()
{
    Console.WriteLn("Closing built-in null USB plugin.");
}

// Note: actually uncommenting the read/write functions provided here
// caused uLauncher.elf to hang on startup, so careful when experimenting.
u8 usbNull_USBread8(u32 addr)
{
    u8 value = 0;

    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 8 bit read at address %lx", addr);
            break;

        default:
            //value = usbRu8(addr);
            Console.WriteLn("*(USBnull) 8 bit read at address %lx", addr);
            break;
    }
    return value;
}

u16 usbNull_USBread16(u32 addr)
{
    u16 value = 0;

    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 16 bit read at address %lx", addr);
            break;

        default:
            //value = usbRu16(addr);
            Console.WriteLn("(USBnull) 16 bit read at address %lx", addr);
    }
    return value;
}

u32 usbNull_USBread32(u32 addr)
{
    u32 value = 0;

    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 32 bit read at address %lx", addr);
            break;

        default:
            //value = usbRu32(addr);
            Console.WriteLn("(USBnull) 32 bit read at address %lx", addr);
    }
    return value;
}

void usbNull_USBwrite8(u32 addr, u8 value)
{
    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 8 bit write at address %lx value %x", addr, value);
            break;

        default:
            //usbRu8(addr) = value;
            Console.WriteLn("(USBnull) 8 bit write at address %lx value %x", addr, value);
    }
}

void usbNull_USBwrite16(u32 addr, u16 value)
{
    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 16 bit write at address %lx value %x", addr, value);
            break;

        default:
            //usbRu16(addr) = value;
            Console.WriteLn("(USBnull) 16 bit write at address %lx value %x", addr, value);
    }
}

void usbNull_USBwrite32(u32 addr, u32 value)
{
    switch (addr)
    {
        // Handle any appropriate addresses here.
        case 0x1f801600:
            Console.WriteLn("(USBnull) 16 bit write at address %lx value %x", addr, value);
            break;

        default:
            //usbRu32(addr) = value;
            Console.WriteLn("(USBnull) 32 bit write at address %lx value %x", addr, value);
    }
}

void usbNull_USBirqCallback(USBcallback callback)
{
    // Register USBirq, so we can trigger an interrupt with it later.
    // It will be called as USBirq(cycles); where cycles is the number
    // of cycles before the irq is triggered.
    USBirq = callback;
}

int _USBirqHandler(void)
{
    // This is our USB irq handler, so if an interrupt gets triggered,
    // deal with it here.
    return 0;
}

USBhandler usbNull_USBirqHandler(void)
{
    // Pass our handler to pcsx2.
    return (USBhandler)_USBirqHandler;
}

void usbNull_USBsetRAM(void *mem)
{
    ram = (s8 *)mem;
    Console.WriteLn("*Setting ram.");
}

s32 usbNull_USBfreeze(int mode, freezeData *data)
{
    // This should store or retrieve any information, for if emulation
    // gets suspended, or for savestates.
    switch (mode)
    {
        case FREEZE_LOAD:
            // Load previously saved data.
            break;
        case FREEZE_SAVE:
            // Save data.
            break;
        case FREEZE_SIZE:
            // return the size of the data.
            break;
    }
    return 0;
}

s32 usbNull_USBtest()
{
    // 0 if the plugin works, non-0 if it doesn't.
    return 0;
}