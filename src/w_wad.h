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

#ifndef __W_WAD__
#define __W_WAD__

/*
 * The WAD file module was revised to support file mappings,
 * in 2020 most OSes support a facility to directly map the file in memory,
 * effectively sharing this data in the whole system, this requires
 * less system memory and less system calls. But it requires extra caring,
 * This module's code was also revised to be more legible and safer.
 * NB: `char` type is not guaranteed to be 8-bits, but I don't see what crazy
 * compiler wouldn't do that, more understandable in the code to keep it.
 */

#if __has_attribute(packed)
#define WAD __attribute__((packed))
#else
#warning "Packed attribute not available, WAD parsing might not work correctly"
#define WAD
#endif

#include <stddef.h>
#include <stdint.h>

#include "i_system.h"

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

struct w_lump {
	char name[8];     /* Lump name */
	size_t size;      /* Size of the lump in bytes */
	const void *data; /* Read-only pointer to the lump's data */
};

typedef int32_t lumpId_t; /* Id used for loaded lumps */

/* Load WAD files and choose lumps according to array order */
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

#endif
