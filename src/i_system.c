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
//
//-----------------------------------------------------------------------------

#define _POSIX_THREAD_CPUTIME

#include "doomdef.h"
#include "i_system.h"

#include "i_xcb.h"
#include "i_sound.h"
#include "g_game.h"
#include "m_misc.h"
#include "d_net.h"

#include <stdlib.h>
#include <time.h>

int mb_used = 6;

void
I_Init(void) {
	I_InitSound();
	I_InitXCB();
}

uint8_t *
I_ZoneBase(int *size) {

	*size = mb_used * 1024 * 1024;

	return malloc(*size);
}

int
I_GetTime(void) {
	struct timespec now;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);

	return now.tv_sec * TICRATE + now.tv_nsec * TICRATE / 1000000000;
}

void
I_StartFrame(void) {
}

void
I_StartTic(void) {
	xcb_generic_event_t *event;
	int flush = 0;

	while(event = xcb_poll_for_event(i_xcb.connection), event != NULL) {
		flush += I_PostXCBEvent(event);
		free(event);
	}

	if(i_xcb.grab_mouse != 0) {
		xcb_warp_pointer(i_xcb.connection, XCB_WINDOW_NONE, i_xcb.window.drawable,
			0, 0, 0, 0, i_xcb.window.width / 2, i_xcb.window.height / 2);
		flush++;
	}

	if(flush != 0) {
		xcb_flush(i_xcb.connection);
	}
}

ticcmd_t *
I_BaseTiccmd(void) {
	static ticcmd_t empty;

	return &empty;
}

noreturn void
I_Quit(void) {
	D_QuitNetGame();
	exit(EXIT_SUCCESS);
}

uint8_t *
I_AllocLow(int length) {
	return calloc(length, 1);
}

void
I_Tactile(int on,
	int off,
	int total) {
	on = off = total = 0;
}
