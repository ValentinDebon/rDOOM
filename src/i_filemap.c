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
// DESCRIPTION: File mapping code for w_wad mmap support.
//
//-----------------------------------------------------------------------------

#include "i_filemap.h"
#include "i_error.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/stat.h>

void
I_FileMap(const char *filename, struct i_fileMap *filemap) {
	const int fd = open(filename, O_RDONLY);
	struct stat st;

	if(fd < 0) {
		I_Error("I_FileMap: Unable to open %s: %s", filename, strerror(errno));
	}

	if(fstat(fd, &st) != 0) {
		I_Error("I_FileMap: Unable to stat %s: %s", filename, strerror(errno));
	}

	filemap->size = st.st_size;
	filemap->address = mmap(0, filemap->size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(filemap->address == MAP_FAILED) {
		I_Error("I_FileMap: Unable to map %s: %s", filename, strerror(errno));
	}

	close(fd);
}

void
I_FileUnMap(struct i_fileMap *filemap) {
	munmap(filemap->address, filemap->size);
}
