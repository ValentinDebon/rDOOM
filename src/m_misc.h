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
//-----------------------------------------------------------------------------

#ifndef RDOOM_M_MISC_H
#define RDOOM_M_MISC_H

#ifdef __has_attribute
#if __has_attribute(format)
#define PRINTF_LIKE(x, y) __attribute__((format(printf, x, y)))
#endif
#endif

#ifndef PRINTF_LIKE
#define PRINTF_LIKE(x, y)
#endif

#include <stdbool.h>

void
M_LoadDefaults(void);

const char *
M_Savefile(int slot);

const char * PRINTF_LIKE(2, 3)
M_Format(size_t *lenp, const char *format, ...);

bool
M_WriteFile(char const *name,
	void *source,
	int length);

int
M_ReadFile(char const *name,
	uint8_t **buffer);

void
M_ScreenShot(void);

int
M_DrawText(int x,
	int y,
	bool direct,
	char *string);

/* RDOOM_M_MISC_H */
#endif
