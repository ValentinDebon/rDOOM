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
//	DOOM graphics stuff for UNIX.
//
//-----------------------------------------------------------------------------

#include "i_video.h"

#include "v_video.h"
#include "i_system.h"
#include "i_xcb.h"

#include <stdlib.h>
#include <errno.h>

static struct {
	size_t format_bytes_per_pixel;

	/* This colormap is used to index colors when the server
	supports TrueColor or DirectColor only, it contains pixel values */
	uint8_t *colormap;

	size_t render_directformat_red_index;
	size_t render_directformat_green_index;
	size_t render_directformat_blue_index;

	size_t framebuffer_stride;
} i_video;

static void
I_ShutdownGraphics(void) {
	free(i_video.colormap);
}

void
I_InitGraphics(void) {
	switch(i_xcb.visualtype->_class) {
	case XCB_VISUAL_CLASS_TRUE_COLOR:
	case XCB_VISUAL_CLASS_DIRECT_COLOR:
		printf("I_InitGraphics: Initializing for TrueColor/DirectColor visual\n");
		break;
	default:
		I_Error("I_InitGraphics: Unsupported class of visual\n");
	}

	/* Code doesn't handle (yet) non byte based rgb values */
	if(i_xcb.visualtype->bits_per_rgb_value != 8) {
		I_Error("I_InitGraphics: Unsupported %d bits rgb value", i_xcb.visualtype->bits_per_rgb_value);
	}

	/* Keep it somewhere, need it everywhere */
	i_video.format_bytes_per_pixel = i_xcb.format->bits_per_pixel / 8;

	/* The indices are better pre-computed here */
	switch(xcb_get_setup(i_xcb.connection)->image_byte_order) {
	case XCB_IMAGE_ORDER_LSB_FIRST:
		i_video.render_directformat_red_index   = i_xcb.render_pictforminfo->direct.red_shift / 8;
		i_video.render_directformat_green_index = i_xcb.render_pictforminfo->direct.green_shift / 8;
		i_video.render_directformat_blue_index  = i_xcb.render_pictforminfo->direct.blue_shift / 8;
		break;
	case XCB_IMAGE_ORDER_MSB_FIRST:
		i_video.render_directformat_red_index   = i_video.format_bytes_per_pixel - i_xcb.render_pictforminfo->direct.red_shift / 8;
		i_video.render_directformat_green_index = i_video.format_bytes_per_pixel - i_xcb.render_pictforminfo->direct.green_shift / 8;
		i_video.render_directformat_blue_index  = i_video.format_bytes_per_pixel - i_xcb.render_pictforminfo->direct.blue_shift / 8;
		break;
	default:
		I_Error("I_InitGraphics: Invalid format's image byte order");
	}

	/* Initialize colormap full black */
	i_video.colormap = calloc(i_video.format_bytes_per_pixel * 256, sizeof(*i_video.colormap));

	/* Keep stride somewhere */
	i_video.framebuffer_stride = ((((i_xcb.framebuffer.width * i_xcb.format->bits_per_pixel) + i_xcb.format->scanline_pad - 1) & -i_xcb.format->scanline_pad) + 7) >> 3;

	atexit(I_ShutdownGraphics);
}

void
I_SetPalette(const uint8_t *palette) {
	const uint8_t* const paletteend = palette + 768;
	uint8_t *colormap = i_video.colormap;

	while(palette != paletteend) {
		colormap[i_video.render_directformat_red_index]   = gammatable[usegamma][palette[0]];
		colormap[i_video.render_directformat_green_index] = gammatable[usegamma][palette[1]];
		colormap[i_video.render_directformat_blue_index]  = gammatable[usegamma][palette[2]];

		colormap += i_video.format_bytes_per_pixel;
		palette += 3;
	}
}

void
I_UpdateNoBlit(void) {
}

void
I_FinishUpdate(void) {
	/* Synchronizing framebuffer from client with server */
	uint8_t image[i_video.framebuffer_stride * i_xcb.framebuffer.height];
	const uint8_t *framebuffer = screens[0];
	uint8_t *scanline = image;

	const uint8_t * const framebufferend = framebuffer + i_xcb.framebuffer.width * i_xcb.framebuffer.height;
	const size_t scanline_padding = i_video.framebuffer_stride - i_xcb.framebuffer.width * i_video.format_bytes_per_pixel;
	while(framebuffer != framebufferend) {
		const uint8_t * const framebufferrowend = framebuffer + i_xcb.framebuffer.width;

		while(framebuffer != framebufferrowend) {
			const uint8_t * const rgb_pixel = i_video.colormap + *framebuffer * i_video.format_bytes_per_pixel;

			__builtin_memcpy(scanline, rgb_pixel, i_video.format_bytes_per_pixel);

			scanline += i_video.format_bytes_per_pixel;
			framebuffer++;
		}

		scanline += scanline_padding;
	}

	xcb_put_image(i_xcb.connection,
		XCB_IMAGE_FORMAT_Z_PIXMAP, i_xcb.framebuffer.drawable, i_xcb.graphic_context,
		i_xcb.framebuffer.width, i_xcb.framebuffer.height, 0, 0, 0, i_xcb.format->depth,
		sizeof(image), image);

	/* Ask X Rendering extension to handle composition natively, avoids client resize */
	xcb_render_composite(i_xcb.connection, XCB_RENDER_PICT_OP_SRC,
		i_xcb.framebuffer.picture, XCB_RENDER_PICTURE_NONE, i_xcb.window.picture,
		0, 0, 0, 0, 0, 0, i_xcb.window.width, i_xcb.window.height);

	xcb_flush(i_xcb.connection);
}

void
I_WaitVBL(int count) {
	struct timespec req = {
		.tv_nsec = 14285714 * count,
	}, rem;

	if(req.tv_nsec > 1000000000) {
		req.tv_sec = req.tv_nsec / 1000000000;
		req.tv_nsec %= 1000000000;
	}

	while(errno = 0, nanosleep(&req, &rem) == -1 && errno == EINTR) {
		req = rem;
	}
}

void
I_ReadScreen(uint8_t *scr) {
	memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

void
I_BeginRead(void) {
}

void
I_EndRead(void) {
}

