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
#ifndef RDOOM_I_FILEMAP_H
#define RDOOM_I_FILEMAP_H

#include <stddef.h>

struct i_fileMap {
	void *address;
	size_t size;
};

void
I_FileMap(const char *filename, struct i_fileMap *filemap);

void
I_FileUnMap(struct i_fileMap *filemap);

/* RDOOM_I_FILEMAP_H */
#endif
