//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
//	DOOM stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

#include "i_xcb.h"

#include "doomdef.h"
#include "i_system.h"
#include "d_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X_KEYSYM_SPECIAL_NOSYMBOL   0x00000000
#define X_KEYSYM_SPECIAL_VOIDSYMBOL 0x00FFFFFF

#define X_KEYSYM_LATIN_1_SPACE ' '
#define X_KEYSYM_LATIN_1_TILDE '~'

#define X_KEYSYM_FUNCTION_BACKSPACE         0xFF08
#define X_KEYSYM_FUNCTION_TAB               0xFF09
#define X_KEYSYM_FUNCTION_RETURN            0xFF0D
#define X_KEYSYM_FUNCTION_PAUSE             0xFF13
#define X_KEYSYM_FUNCTION_ESCAPE            0xFF1B
#define X_KEYSYM_FUNCTION_LEFT              0xFF51
#define X_KEYSYM_FUNCTION_UP                0xFF52
#define X_KEYSYM_FUNCTION_RIGHT             0xFF53
#define X_KEYSYM_FUNCTION_DOWN              0xFF54
#define X_KEYSYM_FUNCTION_KEYPAD_MINUS_SIGN 0xFFAD
#define X_KEYSYM_FUNCTION_KEYPAD_EQUALS     0xFFBD
#define X_KEYSYM_FUNCTION_F1                0xFFBE
#define X_KEYSYM_FUNCTION_F2                0xFFBF
#define X_KEYSYM_FUNCTION_F3                0xFFC0
#define X_KEYSYM_FUNCTION_F4                0xFFC1
#define X_KEYSYM_FUNCTION_F5                0xFFC2
#define X_KEYSYM_FUNCTION_F6                0xFFC3
#define X_KEYSYM_FUNCTION_F7                0xFFC4
#define X_KEYSYM_FUNCTION_F8                0xFFC5
#define X_KEYSYM_FUNCTION_F9                0xFFC6
#define X_KEYSYM_FUNCTION_F10               0xFFC7
#define X_KEYSYM_FUNCTION_F11               0xFFC8
#define X_KEYSYM_FUNCTION_F12               0xFFC9
#define X_KEYSYM_FUNCTION_LEFT_SHIFT        0xFFE1
#define X_KEYSYM_FUNCTION_RIGHT_SHIFT       0xFFE2
#define X_KEYSYM_FUNCTION_LEFT_CONTROL      0xFFE3
#define X_KEYSYM_FUNCTION_RIGHT_CONTROL     0xFFE4
#define X_KEYSYM_FUNCTION_LEFT_ALT          0xFFE9
#define X_KEYSYM_FUNCTION_RIGHT_ALT         0xFFEA
#define X_KEYSYM_FUNCTION_DELETE            0xFFFF

struct i_xcb i_xcb = {
	.window = {
		.width = 640,
		.height = 400,
	},
	.framebuffer = {
		.width = 320,
		.height = 200,
	},
};

static void
I_SetXCBKeyboardMappingFrom(xcb_get_keyboard_mapping_cookie_t get_keyboard_mapping_cookie) {
	xcb_get_keyboard_mapping_reply_t *get_keyboard_mapping_reply
		= xcb_get_keyboard_mapping_reply(i_xcb.connection, get_keyboard_mapping_cookie, NULL);

	if(get_keyboard_mapping_reply == NULL) {
		I_Error("I_SetXCBKeyboardMappingFrom: Unable to get reply");
	}

	xcb_keysym_t *keysyms = xcb_get_keyboard_mapping_keysyms(get_keyboard_mapping_reply);
	i_xcb.keyboard_mapping.keysyms = realloc(i_xcb.keyboard_mapping.keysyms, get_keyboard_mapping_reply->length * sizeof(*i_xcb.keyboard_mapping.keysyms));
	if(i_xcb.keyboard_mapping.keysyms == NULL) {
		I_Error("I_SetXCBKeyboardMappingFrom: Unable to allocate keysyms table");
	}

	/* At each remap we remap the whole table, so we can be sure
	the request holds the maximum possible keycodes count */
	memcpy(i_xcb.keyboard_mapping.keysyms, keysyms, get_keyboard_mapping_reply->length * sizeof(*keysyms));

	i_xcb.keyboard_mapping.keysyms_per_keycode = get_keyboard_mapping_reply->keysyms_per_keycode;

	free(get_keyboard_mapping_reply);
}

static void
I_ShutdownXCB(void) {
	xcb_render_free_picture(i_xcb.connection, i_xcb.window.picture);
	xcb_render_free_picture(i_xcb.connection, i_xcb.framebuffer.picture);
	xcb_destroy_window(i_xcb.connection, i_xcb.window.drawable);
	xcb_free_pixmap(i_xcb.connection, i_xcb.framebuffer.drawable);
	xcb_disconnect(i_xcb.connection);
}

void
I_InitXCB(void) {
	/* Let's connect to X11 */
	int screen_number;

	i_xcb.connection = xcb_connect(NULL, &screen_number);

	if(xcb_connection_has_error(i_xcb.connection) != 0) {
		I_Error("I_InitXCB: Unable to connect to X11 display");
	}

	/* Access screen */
	const xcb_setup_t *setup = xcb_get_setup(i_xcb.connection);
	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);

	while(screen_number > 0) {
		xcb_screen_next(&screen_iterator);
		screen_number--;
	}

	i_xcb.screen = screen_iterator.data;

	/* Get root window visual */
	xcb_depth_iterator_t depth_iterator
		= xcb_screen_allowed_depths_iterator(i_xcb.screen);

	while(depth_iterator.rem != 0) {
		xcb_visualtype_iterator_t visualtype_iterator
			= xcb_depth_visuals_iterator(depth_iterator.data);

		while(visualtype_iterator.rem != 0) {
			const xcb_visualtype_t *visualtype = visualtype_iterator.data;

			if(visualtype->visual_id == i_xcb.screen->root_visual) {
				i_xcb.visualtype = visualtype;
			}

			xcb_visualtype_next(&visualtype_iterator);
		}

		xcb_depth_next(&depth_iterator);
	}

	if(i_xcb.visualtype == NULL) {
		I_Error("I_InitXCB: Unable to find root visual type");
	}

	/* Get image format */
	xcb_format_iterator_t format_iterator = xcb_setup_pixmap_formats_iterator(setup);

	while(format_iterator.rem != 0) {

		if(format_iterator.data->depth == i_xcb.screen->root_depth) {
			i_xcb.format = format_iterator.data;
			break;
		}

		xcb_format_next(&format_iterator);
	}

	if(format_iterator.rem == 0) {
		I_Error("I_InitXCB: Unable to find suitable format for depth %d\n", i_xcb.screen->root_depth);
	}

	/* Create our graphic context */
	i_xcb.graphic_context = xcb_generate_id(i_xcb.connection);
	xcb_create_gc(i_xcb.connection, i_xcb.graphic_context, i_xcb.screen->root,
		XCB_GC_FOREGROUND, &i_xcb.screen->white_pixel);

	/* Create our window */
	const uint32_t window_value_list[] = {
		i_xcb.screen->black_pixel,
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};
	i_xcb.window.drawable = xcb_generate_id(i_xcb.connection);
	xcb_create_window(i_xcb.connection, XCB_COPY_FROM_PARENT,
		i_xcb.window.drawable, i_xcb.screen->root,
		0, 0,
		i_xcb.window.width, i_xcb.window.height,
		0, XCB_WINDOW_CLASS_COPY_FROM_PARENT,
		XCB_COPY_FROM_PARENT,
		XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
		window_value_list);

	/* Create our pixmap */
	i_xcb.framebuffer.drawable = xcb_generate_id(i_xcb.connection);
	xcb_create_pixmap(i_xcb.connection, i_xcb.screen->root_depth,
		i_xcb.framebuffer.drawable, i_xcb.window.drawable,
		i_xcb.framebuffer.width, i_xcb.framebuffer.height);

	/* Asynchronous requests */

	/* - X Render PictFormats cookie */
	xcb_render_query_pict_formats_cookie_t render_query_pict_formats_cookie
		= xcb_render_query_pict_formats(i_xcb.connection);

	/* - WM_PROTOCOLS atom cookie */
	xcb_intern_atom_cookie_t wm_protocols_atom_cookie
		= xcb_intern_atom(i_xcb.connection, 1, 12, "WM_PROTOCOLS");

	/* - WM_DELETE_WINDOW atom cookie */
	xcb_intern_atom_cookie_t wm_delete_window_atom_cookie
		= xcb_intern_atom(i_xcb.connection, 0, 16, "WM_DELETE_WINDOW");

	/* - Get Keyboard Mapping cookie */
	xcb_get_keyboard_mapping_cookie_t get_keyboard_mapping_cookie
		= xcb_get_keyboard_mapping(i_xcb.connection, setup->min_keycode,
			setup->max_keycode - setup->min_keycode + 1);

	/* Asynchronous replies. */

	/* - X Render PictFormats reply */
	xcb_render_query_pict_formats_reply_t *render_query_pict_formats_reply
		= xcb_render_query_pict_formats_reply(i_xcb.connection, render_query_pict_formats_cookie, NULL);
	if(render_query_pict_formats_reply == NULL) {
		I_Error("I_InitXCB: Required X Render PictFormats not available");
	}

	xcb_render_pictforminfo_iterator_t render_pictforminfo_iterator
		= xcb_render_query_pict_formats_formats_iterator(render_query_pict_formats_reply);

	while(render_pictforminfo_iterator.rem != 0) {
		const xcb_render_pictforminfo_t *render_pictforminfo
			= render_pictforminfo_iterator.data;

		if(i_xcb.screen->root_depth == render_pictforminfo->depth
			&& i_xcb.visualtype->red_mask == render_pictforminfo->direct.red_mask << render_pictforminfo->direct.red_shift
			&& i_xcb.visualtype->green_mask == render_pictforminfo->direct.green_mask << render_pictforminfo->direct.green_shift
			&& i_xcb.visualtype->blue_mask == render_pictforminfo->direct.blue_mask << render_pictforminfo->direct.blue_shift) {
			i_xcb.render_pictforminfo = render_pictforminfo;
		}

		xcb_render_pictforminfo_next(&render_pictforminfo_iterator);
	}

	if(i_xcb.render_pictforminfo == NULL) {
		I_Error("I_InitXCB: Unable to find suitable PictFormat for root-depth based drawables");
	}

	/* - WM_PROTOCOLS atom reply */
	xcb_intern_atom_reply_t *wm_protocols_atom_reply
		= xcb_intern_atom_reply(i_xcb.connection, wm_protocols_atom_cookie, NULL);
	if(wm_protocols_atom_reply == NULL) {
		I_Error("I_InitXCB: Required WM_PROTOCOLS not available");
	}

	/* - WM_DELETE_WINDOW atom reply */
	xcb_intern_atom_reply_t *wm_delete_window_atom_reply
		= xcb_intern_atom_reply(i_xcb.connection, wm_delete_window_atom_cookie, NULL);
	if(wm_delete_window_atom_reply == NULL) {
		I_Error("I_InitXCB: Required WM_DELETE_WINDOW not available");
	}

	i_xcb.atoms.wm_delete_window = wm_delete_window_atom_reply->atom;

	/* - Get Keyboard Mapping reply */
	I_SetXCBKeyboardMappingFrom(get_keyboard_mapping_cookie);

	/* We finally have our PictFormats, our drawables, create our pictures */
	i_xcb.window.picture = xcb_generate_id(i_xcb.connection);
	xcb_render_create_picture(i_xcb.connection, i_xcb.window.picture,
		i_xcb.window.drawable, i_xcb.render_pictforminfo->id, 0, NULL);

	i_xcb.framebuffer.picture = xcb_generate_id(i_xcb.connection);
	xcb_render_create_picture(i_xcb.connection, i_xcb.framebuffer.picture,
		i_xcb.framebuffer.drawable, i_xcb.render_pictforminfo->id, 0, NULL);

	/* Final setups before start */
	xcb_change_property(i_xcb.connection, XCB_PROP_MODE_REPLACE,
		i_xcb.window.drawable, wm_protocols_atom_reply->atom,
		XCB_ATOM_ATOM, 32, 1, &i_xcb.atoms.wm_delete_window);

	xcb_map_window(i_xcb.connection, i_xcb.window.drawable);

	xcb_flush(i_xcb.connection);

	free(render_query_pict_formats_reply);
	free(wm_protocols_atom_reply);
	free(wm_delete_window_atom_reply);

	atexit(I_ShutdownXCB);
}

static int
I_XCBKeycodeToKey(xcb_keycode_t keycode) {
	/* Let's get the keysym, TODO: Handle Group Modifier and modifiers */
	const xcb_setup_t *setup = xcb_get_setup(i_xcb.connection);
	const xcb_keysym_t *keysyms = i_xcb.keyboard_mapping.keysyms
		+ (keycode - setup->min_keycode) * i_xcb.keyboard_mapping.keysyms_per_keycode;
	const xcb_keysym_t * const keysymsend = keysyms + i_xcb.keyboard_mapping.keysyms_per_keycode;
	xcb_keysym_t keysym = X_KEYSYM_SPECIAL_NOSYMBOL;

	while(keysyms != keysymsend && *keysyms == X_KEYSYM_SPECIAL_NOSYMBOL) {
		keysyms++;
	}

	if(keysyms != keysymsend) {
		keysym = *keysyms;
	}

	/* Now keysym to DOOM key */
	int key;

	switch(keysym) {
	case X_KEYSYM_FUNCTION_RIGHT:             key = KEY_RIGHTARROW; break;
	case X_KEYSYM_FUNCTION_LEFT:              key = KEY_LEFTARROW;  break;
	case X_KEYSYM_FUNCTION_UP:                key = KEY_UPARROW;    break;
	case X_KEYSYM_FUNCTION_DOWN:              key = KEY_DOWNARROW;  break;
	case X_KEYSYM_FUNCTION_ESCAPE:            key = KEY_ESCAPE;     break;
	case X_KEYSYM_FUNCTION_RETURN:            key = KEY_ENTER;      break;
	case X_KEYSYM_FUNCTION_TAB:               key = KEY_TAB;        break;
	case X_KEYSYM_FUNCTION_F1:                key = KEY_F1;         break;
	case X_KEYSYM_FUNCTION_F2:                key = KEY_F2;         break;
	case X_KEYSYM_FUNCTION_F3:                key = KEY_F3;         break;
	case X_KEYSYM_FUNCTION_F4:                key = KEY_F4;         break;
	case X_KEYSYM_FUNCTION_F5:                key = KEY_F5;         break;
	case X_KEYSYM_FUNCTION_F6:                key = KEY_F6;         break;
	case X_KEYSYM_FUNCTION_F7:                key = KEY_F7;         break;
	case X_KEYSYM_FUNCTION_F8:                key = KEY_F8;         break;
	case X_KEYSYM_FUNCTION_F9:                key = KEY_F9;         break;
	case X_KEYSYM_FUNCTION_F10:               key = KEY_F10;        break;
	case X_KEYSYM_FUNCTION_F11:               key = KEY_F11;        break;
	case X_KEYSYM_FUNCTION_F12:               key = KEY_F12;        break;
	case X_KEYSYM_FUNCTION_BACKSPACE:
	case X_KEYSYM_FUNCTION_DELETE:            key = KEY_BACKSPACE;  break;
	case X_KEYSYM_FUNCTION_PAUSE:             key = KEY_PAUSE;      break;
	case X_KEYSYM_FUNCTION_KEYPAD_EQUALS:     key = KEY_EQUALS;     break;
	case X_KEYSYM_FUNCTION_KEYPAD_MINUS_SIGN: key = KEY_MINUS;      break;
	case X_KEYSYM_FUNCTION_LEFT_SHIFT:
	case X_KEYSYM_FUNCTION_RIGHT_SHIFT:       key = KEY_RSHIFT;     break;
	case X_KEYSYM_FUNCTION_LEFT_CONTROL:
	case X_KEYSYM_FUNCTION_RIGHT_CONTROL:     key = KEY_RCTRL;      break;
	case X_KEYSYM_FUNCTION_LEFT_ALT:
	case X_KEYSYM_FUNCTION_RIGHT_ALT:         key = KEY_RALT;       break;
	default:
		/* Check if we're a printable ascii character */
		if(keysym >= X_KEYSYM_LATIN_1_SPACE && keysym <= X_KEYSYM_LATIN_1_TILDE) {
			if(keysym - 0x40 <= 26) { /* If we're ASCII maj character */
				keysym += 0x20; /* Lowercase it */
			}
			key = keysym;
			break;
		}
		key = -1; /* Unrecognized keysym */
	}

	return key;
}

void
I_PostXCBEvent(const xcb_generic_event_t *generic_event) {
		uint8_t const response_type = generic_event->response_type & ~0x80;

		if(response_type != 0) {
			switch(response_type) {
			case XCB_KEY_PRESS: {
				const xcb_key_press_event_t *key_press_event
					= (const xcb_key_press_event_t *)generic_event;
				const event_t event = {
					.type = ev_keydown,
					.data1 = I_XCBKeycodeToKey(key_press_event->detail),
				};

				if(event.data1 != -1) {
					D_PostEvent(&event);
				}
			}	break;
			case XCB_KEY_RELEASE: {
				const xcb_key_release_event_t *key_release_event
					= (const xcb_key_release_event_t *)generic_event;
				const event_t event = {
					.type = ev_keyup,
					.data1 = I_XCBKeycodeToKey(key_release_event->detail),
				};

				if(event.data1 != -1) {
					D_PostEvent(&event);
				}
			}	break;
			case XCB_BUTTON_PRESS: {
				const xcb_button_press_event_t *button_press_event
					= (const xcb_button_press_event_t *)generic_event;
				const event_t event = {
					.type = ev_mouse,
					.data1 = (button_press_event->state >> 8 & 0x07) |
						1 << (button_press_event->detail - 1 & 0x07),
				};

				D_PostEvent(&event);
			}	break;
			case XCB_BUTTON_RELEASE: {
				const xcb_button_release_event_t *button_release_event
					= (const xcb_button_release_event_t *)generic_event;
				const event_t event = {
					.type = ev_mouse,
					.data1 = (button_release_event->state >> 8 & 0x07) ^
						1 << (button_release_event->detail - 1 & 0x07),
				};

				D_PostEvent(&event);
			}	break;
			case XCB_MOTION_NOTIFY: {
				static int16_t lastx, lasty;
				const xcb_motion_notify_event_t *motion_notify_event
					= (const xcb_motion_notify_event_t *)generic_event;
				const event_t event = {
					.type = ev_mouse,
					.data1 = 1 << (motion_notify_event->detail - 1 & 0x07),
					.data2 = (motion_notify_event->event_x - lastx) << 2,
					.data3 = (lasty - motion_notify_event->event_y) << 2,
				};

				lastx = motion_notify_event->event_x;
				lasty = motion_notify_event->event_y;

				D_PostEvent(&event);
			}	break;
			case XCB_DESTROY_NOTIFY:
				break;
			case XCB_UNMAP_NOTIFY:
				break;
				break;
			case XCB_REPARENT_NOTIFY:
				break;
			case XCB_CONFIGURE_NOTIFY: {
				const xcb_configure_notify_event_t *configure_notify_event
					= (const xcb_configure_notify_event_t *)generic_event;

				i_xcb.window.width = configure_notify_event->width;
				i_xcb.window.height = configure_notify_event->height;
			}	/* fallthrough */
			case XCB_MAP_NOTIFY: {
				/* When mapping is requested or size changed, set framebuffer picture transform */
				xcb_render_transform_t transform = {
					.matrix11 = (float)i_xcb.framebuffer.width / i_xcb.window.width * (1 << 16),
					.matrix22 = (float)i_xcb.framebuffer.height / i_xcb.window.height * (1 << 16),
					.matrix33 = 1 << 16,
				};

				xcb_render_set_picture_transform(i_xcb.connection,
					i_xcb.framebuffer.picture, transform);
			}	break;
			case XCB_GRAVITY_NOTIFY:
				break;
			case XCB_CIRCULATE_NOTIFY:
				break;
			case XCB_MAPPING_NOTIFY: {
				const xcb_mapping_notify_event_t *mapping_notify_event
					= (const xcb_mapping_notify_event_t *)generic_event;

				if(mapping_notify_event->request == XCB_MAPPING_KEYBOARD) { 
					const xcb_setup_t *setup = xcb_get_setup(i_xcb.connection);
					/* We could only remap the new mappings, but it's a naive implementation */
					xcb_get_keyboard_mapping_cookie_t get_keyboard_mapping_cookie
						= xcb_get_keyboard_mapping(i_xcb.connection, setup->min_keycode,
							setup->max_keycode - setup->min_keycode + 1);

					/* Synchronous call, but shouldn't happen often */
					I_SetXCBKeyboardMappingFrom(get_keyboard_mapping_cookie);
				}
			}	break;
			case XCB_CLIENT_MESSAGE: {
				const xcb_client_message_event_t *client_message_event
					= (const xcb_client_message_event_t *)generic_event;

				if(client_message_event->data.data32[0] == i_xcb.atoms.wm_delete_window) {
					I_Quit();
				}
			}	break;
			}
		} else {
			const xcb_generic_error_t *generic_error = (const xcb_generic_error_t *)generic_event;
			fprintf(stderr, "I_PostXCBEvent: X11 Error, code: %d\n", generic_error->error_code);
		}
}

