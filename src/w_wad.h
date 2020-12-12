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
//
//-----------------------------------------------------------------------------

#ifndef __W_WAD__
#define __W_WAD__

#ifdef __GNUG__
#pragma interface
#endif

//
// TYPES
//
typedef struct __attribute__((packed)) {
	// Should be "IWAD" or "PWAD".
	char identification[4];
	int numlumps;
	int infotableofs;

} wadinfo_t;

typedef struct __attribute__((packed)) {
	int filepos;
	int size;
	char name[8];

} filelump_t;

//
// WADFILE I/O related stuff.
//
typedef struct __attribute__((packed)) {
	char name[8];
	int handle;
	int position;
	int size;
} lumpinfo_t;

extern void **lumpcache;
extern lumpinfo_t *lumpinfo;
extern int numlumps;

void
W_InitMultipleFiles(char **filenames);
void
W_Reload(void);

int
W_CheckNumForName(char *name);
int
W_GetNumForName(char *name);

int
W_LumpLength(int lump);
void
W_ReadLump(int lump, void *dest);

void *
W_CacheLumpNum(int lump, int tag);
void *
W_CacheLumpName(char *name, int tag);

#endif
