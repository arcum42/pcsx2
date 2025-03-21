// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#pragma once

#include "Common.h"
#include "deci2.h"

void D2_DBGP(const u8 *inbuffer, u8 *outbuffer, char *message, char *eepc, char *ioppc, char *eecy, char *iopcy);
void sendBREAK(u8 source, u16 id, u8 code, u8 result, u8 count);
