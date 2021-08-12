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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"

void
D_DoomMain(char **files, char **filesend);

// Called by IO functions when input is detected.
void
D_PostEvent(const event_t *ev);

//
// BASE LEVEL
//
void
D_PageTicker(void);
void
D_PageDrawer(void);
void
D_AdvanceDemo(void);
void
D_StartTitle(void);

#endif
