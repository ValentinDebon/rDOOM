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
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------

#include "w_wad.h"

#include "i_filemap.h"
#include "i_error.h"
#include "m_swap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct w_wad {
	struct i_fileMap *filemaps;
	size_t filemaps_count;

	struct w_lump *lumps;
	size_t lumps_count;
} w_wad;

static void
W_ReserveLumps(size_t count) {
	w_wad.lumps_count += count;
	w_wad.lumps = realloc(w_wad.lumps, w_wad.lumps_count * sizeof(*w_wad.lumps));
}

static void
W_InitFile(const char *filename, struct i_fileMap *filemap) {
	I_FileMap(filename, filemap);

	printf(" adding %s\n", filename);

	const char *extension = strrchr(filename, '.');
	if(extension != NULL && strcasecmp(extension, ".wad") == 0) { /* WAD files */
		const struct w_wadInfo *wadinfo = filemap->address;

		/* Check magic number */
		if(memcmp(wadinfo->identification, "IWAD", 4) != 0) {
			if(memcmp(wadinfo->identification, "PWAD", 4) != 0) {
				I_Error("W_InitFile: WAD files %s doesn't have IWAD or PWAD id\n", filename);
			}
		}

		/* Get count of added lumps and info table */
		const size_t count = LE_U32(wadinfo->lumps_count);
		const struct w_lumpInfo *lumpinfo = (const struct w_lumpInfo *)
			((const uint8_t *)filemap->address + LE_U32(wadinfo->info_table_offset));

		/* Reserve memory for new lumps, and get array bounds */
		W_ReserveLumps(count);
		struct w_lump * const lumpsend = w_wad.lumps + w_wad.lumps_count;
		struct w_lump *lumps = lumpsend - count;

		while(lumps != lumpsend) {

			strncpy(lumps->name, lumpinfo->name, sizeof(lumps->name));
			lumps->size = lumpinfo->size;
			lumps->data = (const uint8_t *)filemap->address + lumpinfo->position;

			lumpinfo++;
			lumps++;
		}
	} else { /* Standalone lump patch */
		W_ReserveLumps(1);
		struct w_lump *lump = w_wad.lumps + w_wad.lumps_count - 1;

		strncpy(lump->name, filename, sizeof(lump->name));
		lump->size = filemap->size;
		lump->data = filemap->address;
	}
}

static void
W_Shutdown(void) {

	while(w_wad.filemaps_count != 0) {

		w_wad.filemaps_count--;

		I_FileUnMap(w_wad.filemaps + w_wad.filemaps_count);
	}

	free(w_wad.filemaps);
	free(w_wad.lumps);
}

void
W_Init(const char * const *files) {

	if(*files != NULL) {
		const char * const *filesend = files;

		while(*++filesend != NULL);

		w_wad.filemaps_count = filesend - files;
		w_wad.filemaps = malloc(w_wad.filemaps_count * sizeof(*w_wad.filemaps));

		if(w_wad.filemaps == NULL) {
			I_Error("W_Init: Too many WAD files");
		}

		while(files != filesend) {
			const char *filename = *files;

			files++;

			W_InitFile(filename, filesend - files + w_wad.filemaps);
		}
	} else {
		I_Error("W_Init: No files found");
	}

	atexit(W_Shutdown);
}

lumpId_t
W_FindIdForName(const char *name) {
	const struct w_lump *lumpsend = w_wad.lumps + w_wad.lumps_count;

	while(--lumpsend >= w_wad.lumps) {
		if(strncasecmp(lumpsend->name, name, sizeof(lumpsend->name)) == 0) {
			return lumpsend - w_wad.lumps;
		}
	}

	return -1;
}

lumpId_t
W_GetIdForName(const char *name) {
	const lumpId_t id = W_FindIdForName(name);

	if(id == -1) {
		I_Error("W_GetIdForName: Invalid lump name '%s'", name);
	}

	return id;
}

const struct w_lump *
W_LumpForId(lumpId_t id) {

	if(id < 0 || id >= w_wad.lumps_count) {
		I_Error("W_LumpForId: Invalid lump id '%d'", id);
	}

	return w_wad.lumps + id;
}

const struct w_lump *
W_LumpForName(const char *name) {
	const lumpId_t id = W_FindIdForName(name);

	if(id == -1) {
		I_Error("W_LumpForId: Invalid lump name '%s'", name);
	}

	return w_wad.lumps + id;
}
