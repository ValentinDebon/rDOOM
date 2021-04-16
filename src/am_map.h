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
//  AutoMap module.
//
//-----------------------------------------------------------------------------

#ifndef RDOOM_AM_MAP_H
#define RDOOM_AM_MAP_H

#include <stdbool.h>

#include "d_event.h"

/* Used by ST StatusBar stuff. */
#define AM_MESSAGE_HEADER (('a' << 24) + ('m' << 16))
enum am_eventMessage {
	AM_MESSAGE_ENTERED = (AM_MESSAGE_HEADER | ('e' << 8)),
	AM_MESSAGE_EXITED  = (AM_MESSAGE_HEADER | ('x' << 8)),
};

/* Called by main loop. */
bool
AM_Responder(event_t *ev);

// Called by main loop.
void
AM_Ticker(void);

// Called by main loop,
// called instead of view drawer if automap active.
void
AM_Drawer(void);

// Called to force the automap to quit
// if the level is completed while it is up.
void
AM_Stop(void);

/* RDOOM_AM_MAP_H */
#endif
