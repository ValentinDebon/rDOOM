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
#include "m_array.h"
#include "m_swap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

M_TemplateArray(Filemap, struct i_fileMap, 4);
M_TemplateArray(Lump, struct w_lump, 2048);

static struct w_wad {
	struct m_arrayFilemap filemaps;
	struct m_arrayLump lumps;
} w_wad;

static void
W_InitFile(const char *filename, struct i_fileMap *filemap) {
	I_FileMap(filename, filemap);

	printf(" adding %s\n", filename);

	const char * const extension = strrchr(filename, '.');
	if(extension != NULL && strcasecmp(extension, ".wad") == 0) { /* WAD files */
		const struct w_wadInfo *wadinfo = filemap->address;

		/* Check magic number */
		if(memcmp(wadinfo->identification, "IWAD", 4) != 0
			&& memcmp(wadinfo->identification, "PWAD", 4) != 0) {
			I_Error("W_InitFile: WAD files %s doesn't have IWAD or PWAD id\n", filename);
		}

		/* Get count of added lumps and info table */
		const size_t count = LE_U32(wadinfo->lumps_count);
		const struct w_lumpInfo *lumpinfo = (const struct w_lumpInfo *)
			((const uint8_t *)filemap->address + LE_U32(wadinfo->info_table_offset));
		const struct w_lumpInfo * const lumpinfoend = lumpinfo + count;

		while(lumpinfo != lumpinfoend) {
			struct w_lump lump;

			strncpy(lump.name, lumpinfo->name, sizeof(lump.name));
			lump.size = lumpinfo->size;
			lump.data = (const uint8_t *)filemap->address + lumpinfo->position;

			M_ArrayLumpAppend(&w_wad.lumps, lump);

			lumpinfo++;
		}
	} else { /* Standalone lump patch */
		struct w_lump lump;

		strncpy(lump.name, filename, sizeof(lump.name));
		lump.size = filemap->size;
		lump.data = filemap->address;

		M_ArrayLumpAppend(&w_wad.lumps, lump);
	}
}

static void
W_Shutdown(void) {

	for(struct i_fileMap *current = w_wad.filemaps.begin; current != w_wad.filemaps.end; current++) {
		I_FileUnMap(current);
	}

	M_ArrayFilemapFree(&w_wad.filemaps);
	M_ArrayLumpFree(&w_wad.lumps);
}

void
W_Init(const char * const *begin, const char * const *end) {

	if(begin == end) {
		I_Error("W_Init: No files found");
	}

	while(begin != end) {
		const char *filename = *begin;
		struct i_fileMap filemap;

		W_InitFile(filename, &filemap);
		M_ArrayFilemapAppend(&w_wad.filemaps, filemap);

		begin++;
	}

	M_ArrayFilemapFit(&w_wad.filemaps);
	M_ArrayLumpFit(&w_wad.lumps);

	atexit(W_Shutdown);
}

lumpId_t
W_FindIdForName(const char *name) {
	const struct w_lump *lumpsend = w_wad.lumps.end;

	while(--lumpsend >= w_wad.lumps.begin) {
		if(strncasecmp(lumpsend->name, name, sizeof(lumpsend->name)) == 0) {
			return lumpsend - w_wad.lumps.begin;
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

	if(id < 0 || id >= w_wad.lumps.end - w_wad.lumps.begin) {
		I_Error("W_LumpForId: Invalid lump id '%d'", id);
	}

	return w_wad.lumps.begin + id;
}

const struct w_lump *
W_LumpForName(const char *name) {
	const lumpId_t id = W_FindIdForName(name);

	if(id == -1) {
		I_Error("W_LumpForId: Invalid lump name '%s'", name);
	}

	return w_wad.lumps.begin + id;
}
