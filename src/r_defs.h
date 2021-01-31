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
//      Refresh/rendering module, shared data struct definitions.
//
//-----------------------------------------------------------------------------

#ifndef __R_DEFS__
#define __R_DEFS__

#include <stdbool.h>

// Screenwidth.
#include "doomdef.h"

// Some more or less basic data types
// we depend on.
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"
// SECTORS do store MObjs anyway.
#include "p_mobj.h"

#include "p_local.h"

#ifdef __GNUG__
#pragma interface
#endif

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE 0
#define SIL_BOTTOM 1
#define SIL_TOP 2
#define SIL_BOTH 3

#define MAXDRAWSEGS 256

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

// posts are runs of non masked source pixels
typedef struct
{
	uint8_t topdelta; // -1 is the last post in a column
	uint8_t length;   // length data bytes follows
} post_t;

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

// PC direct to screen pointers
//B UNUSED - keep till detailshift in r_draw.c resolved
//extern byte*	destview;
//extern byte*	destscreen;

//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even us emore than 32 levels.
typedef uint8_t lighttable_t;

//
// ?
//
typedef struct drawseg_s {
	const struct p_segment *curline;
	int x1;
	int x2;

	fixed_t scale1;
	fixed_t scale2;
	fixed_t scalestep;

	// 0=none, 1=bottom, 2=top, 3=both
	int silhouette;

	// do not clip sprites above this
	fixed_t bsilheight;

	// do not clip sprites below this
	fixed_t tsilheight;

	// Pointers to lists for sprite clipping,
	//  all three adjusted so [x1] is first value.
	short *sprtopclip;
	short *sprbottomclip;
	short *maskedtexturecol;

} drawseg_t;

// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
typedef struct
{
	short width; // bounding box size
	short height;
	short leftoffset; // pixels to the left of origin
	short topoffset;  // pixels below the origin
	int columnofs[8]; // only [width] used
					  // the [0] is &columnofs[width]
} patch_t;

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s {
	// Doubly linked list.
	struct vissprite_s *prev;
	struct vissprite_s *next;

	int x1;
	int x2;

	// for line side calculation
	fixed_t gx;
	fixed_t gy;

	// global bottom / top for silhouette clipping
	fixed_t gz;
	fixed_t gzt;

	// horizontal position of x1
	fixed_t startfrac;

	fixed_t scale;

	// negative if flipped
	fixed_t xiscale;

	fixed_t texturemid;
	int patch;

	// for color translation and shadow draw,
	//  maxbright frames as well
	const lighttable_t *colormap;

	int mobjflags;

} vissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct
{
	// If false use 0 for any position.
	// Note: as eight entries are available,
	//  we might as well insert the same name eight times.
	int rotate;

	// Lump to use for view angles 0-7.
	short lump[8];

	// Flip bit (1 = flip) to use for view angles 0-7.
	uint8_t flip[8];

} spriteframe_t;

//
// A sprite definition:
//  a number of animation frames.
//
typedef struct
{
	int numframes;
	spriteframe_t *spriteframes;

} spritedef_t;

//
// Now what is a visplane, anyway?
//
typedef struct
{
	fixed_t height;
	int picnum;
	int lightlevel;
	int minx;
	int maxx;

	// leave pads for [minx-1]/[maxx+1]

	uint8_t pad1;
	// Here lies the rub for all
	//  dynamic resize/change of resolution.
	uint8_t top[SCREENWIDTH];
	uint8_t pad2;
	uint8_t pad3;
	// See above.
	uint8_t bottom[SCREENWIDTH];
	uint8_t pad4;

} visplane_t;

#endif
