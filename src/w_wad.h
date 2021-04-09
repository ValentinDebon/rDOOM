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
//	WAD I/O functions.
//
//-----------------------------------------------------------------------------

#ifndef RDOOM_W_WAD_H
#define RDOOM_W_WAD_H

/*
 * The WAD file module was revised to support file mappings,
 * in 2020 most OSes support a facility to directly map the file in memory,
 * effectively sharing this data in the whole system, this requires
 * less system memory and less system calls. But it requires extra caring,
 * This module's code was also revised to be more legible and safer.
 * NB: `char` type is not guaranteed to be 8-bits, but I don't see what crazy
 * compiler wouldn't do that, more understandable in the code to keep it.
 */

#include <stddef.h>
#include <stdint.h>

#if defined(__has_attribute) && __has_attribute(packed)
#define WAD __attribute__((packed))
#else
#warning "Packed attribute not available, WAD parsing might not work correctly"
#define WAD
#endif

/*************************************************
 * WAD structures, used internally in the engine *
 *************************************************/

/* WAD file header */
struct WAD w_wadInfo {
	char     identification[4]; /* Should be "IWAD" or "PWAD". */
	uint32_t lumps_count;       /* Available lumps in this WAD. */
	uint32_t info_table_offset; /* Offset to the info table, from the beginning of the file. */
};

/* NB: In the original source, lumpinfo_t represents the in-engine lump, while
filelump_t was the in-file lump, here w_lumpInfo is the in-file, w_lump the in-engine. */
struct WAD w_lumpInfo {
	uint32_t position; /* Lump offset in the file, from its beginning. */
	uint32_t size;     /* Lump size, in bytes. */
	char     name[8];  /* Lump name, zero filled at the end. */
};

/* Layout of the PNAMES lump */
struct WAD w_lumpPNAMES {
	uint32_t patches_count; /* Patches' count in the lump */
	char     names[][8];    /* Array of patches' names */
};

/* Layout of the TEXTURE1/TEXTURE2 lumps */
struct WAD w_lumpTEXTUREx {
	uint32_t textures_count; /* Textures' count in the lump */
	uint32_t offsets[];      /* Offsets of each map texture, in the lump */
};

/* The following structs were originaly defined in doomdata.h */

/* A single Vertex. VERTEXES array lump */
struct WAD w_mapVertex {
	int16_t x; /* Vertex x position */
	int16_t y; /* Vertex y position */
};

/* A SideDef, defining the visual appearance of a wall,
 * by setting textures and offsets.
 */
struct WAD w_mapSide {
	int16_t offset_x;
	int16_t offset_y;
	char    top_texture[8];
	char    bottom_texture[8];
	char    middle_texture[8];
	int16_t faced_sector;
};

/* A LineDef, as used for editing, and as input
 * to the BSP builder.
 */
struct WAD w_mapLine {
	int16_t first_vertex;
	int16_t last_vertex;
	int16_t flags;
	int16_t special_type;
	int16_t sector_tag;
	int16_t sides[2];
};

/* Basic component of a map */
struct WAD w_mapSector {
	int16_t floor_height;
	int16_t ceiling_height;
	char    floor[8];
	char    ceiling[8];
	int16_t lighting;
	int16_t special_type;
	int16_t tag;
};

/* Sub sectors compose sectors */
struct WAD w_mapSubSector {
	int16_t segments_count;
	int16_t first_segment;
};

/* Segment, basic component of a wall, with orientation */
struct WAD w_mapSegment {
	int16_t first_vertex;
	int16_t last_vertex;
	int16_t angle;
	int16_t line;
	int16_t side;
	int16_t offset;
};

/* Node is used when traversing the BSP to accelerate map sorting */
struct WAD w_mapNode {
	int16_t x;
	int16_t y;
	int16_t dx;
	int16_t dy;
	int16_t bounding_boxes[2][4];
	int16_t children[2];
};

/* Element in the map, for actions, etc... */
struct WAD w_mapThing {
	int16_t x;
	int16_t y;
	int16_t angle;
	int16_t type;
	int16_t flags;
};

/* Component of a texture */
struct WAD w_mapPatch {
	int16_t offset_x; /* Horizontal offset from the upper-left corner of the texture */
	int16_t offset_y; /* Vertical offset from the upper-left corner of the texture */
	int16_t patch;    /* Patch number (index in PNAMES) */
	int16_t stepdir;  /* Unused */
	int16_t colormap; /* Unused */
};

/* A texture is basically a picture, but is composed of several patches */
struct WAD w_mapTexture {
	char              name[8];         /* Texture name */
	uint32_t          masked;          /* Is it masked? 0 or 1 */
	int16_t           width;           /* Texture width */
	int16_t           height;          /* Texture height */
	uint32_t          columndirectory; /* Obsolete, unused */
	int16_t           patches_count;   /* Patches's count in texture */
	struct w_mapPatch patches[];       /* List of patches composing the texture, patches_count elements */
};

/* The following were originaly defined in r_defs.h */

/* Patches.
 * A patch holds one or more columns.
 * Patches are used for sprites and all masked pictures,
 * and we compose textures from the TEXTURE1/2 lists
 * of patches.
 */
struct WAD w_patch {
	int16_t  width;             /* Width of the image */
	int16_t  height;            /* Height of the image */
	int16_t  offset_x;          /* Pixels to the left of the origin */
	int16_t  offset_y;          /* Pixels below the origin */
	uint32_t columns_offsets[]; /* Indices to columns (posts list) */
};

/* NB: post_t is nearly unused in the original code,
but represents the atom of a column_t, so we stick with this name.
Note the last one in a column always have offset set to 255. */
struct WAD w_post {
	uint8_t offset;   /* Height offset of the post (Y position) */
	uint8_t length;   /* Bytes in the post */
	uint8_t pixels[]; /* Post pixels, size of 1 + length + 1 with two padding bytes */
};

/************************************
 * Generic purpose WAD manipulation *
 ************************************/

struct w_lump {
	char        name[8]; /* Lump name */
	size_t      size;    /* Size of the lump in bytes */
	const void *data;    /* Read-only pointer to the lump's data */
};

typedef int32_t lumpId_t; /* Id used for loaded lumps */

/* Load WAD files and adds lumps according to array order */
void
W_Init(const char * const *files);

/* Looks up lump id for name, returns -1 if missing */
lumpId_t
W_FindIdForName(const char *name);

/* Looks up lump id for name, fails if missing */
lumpId_t
W_GetIdForName(const char *name);

/* Finds lump for id, fails if invalid id */
const struct w_lump *
W_LumpForId(lumpId_t id);

/* Finds lump for name, fails if invalid id */
const struct w_lump *
W_LumpForName(const char *name);

/* RDOOM_W_WAD_H */
#endif
