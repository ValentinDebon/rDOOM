//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2020 by Valentin Debon.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
//-----------------------------------------------------------------------------

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include <stdint.h>

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void
I_InitGraphics(void);

// Takes full 8 bit values.
void
I_SetPalette(const uint8_t *palette);

void
I_UpdateNoBlit(void);
void
I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void
I_WaitVBL(int count);

void
I_ReadScreen(uint8_t *scr);

void
I_BeginRead(void);
void
I_EndRead(void);

#endif
