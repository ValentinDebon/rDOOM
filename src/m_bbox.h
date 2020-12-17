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

#ifndef __M_BBOX__
#define __M_BBOX__

#ifdef LINUX
#include <values.h>
#endif

#include "m_fixed.h"

// Bounding box coordinate storage.
enum {
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
}; // bbox coordinates

// Bounding box functions.
void
M_ClearBox(fixed_t *box);

void
M_AddToBox(fixed_t *box,
	fixed_t x,
	fixed_t y);

#endif
