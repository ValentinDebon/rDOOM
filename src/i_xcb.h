//-----------------------------------------------------------------------------
//
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
//-----------------------------------------------------------------------------

#ifndef RDOOM_I_XCB_H
#define RDOOM_I_XCB_H

#include <xcb/xcb.h>
#include <xcb/render.h>

struct i_xcb_surface {
	xcb_drawable_t drawable;
	xcb_render_picture_t picture;
	uint16_t width, height;
};

extern struct i_xcb {
	xcb_connection_t *connection;
	const xcb_screen_t *screen;
	const xcb_visualtype_t *visualtype;
	const xcb_format_t *format;
	const xcb_render_pictforminfo_t *render_pictforminfo;

	xcb_gcontext_t graphic_context;

	struct {
		xcb_keysym_t *keysyms;
		unsigned keysyms_per_keycode;
	} keyboard_mapping;

	struct {
		xcb_atom_t wm_delete_window;
	} atoms;

	xcb_cursor_t cursor;

	struct i_xcb_surface window;
	struct i_xcb_surface framebuffer;

	int grab_mouse;
} i_xcb;

void
I_InitXCB(void);

int
I_PostXCBEvent(const xcb_generic_event_t *event);

/* RDOOM_I_XCB_H */
#endif
