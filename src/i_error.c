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
// DESCRIPTION: Error handling code.
//
//-----------------------------------------------------------------------------

#include "i_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

noreturn void
I_Error(const char *error, ...) {
	//extern bool demorecording;
	va_list ap;

	va_start(ap, error);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, error, ap);
	fputc('\n', stderr);
	va_end(ap);

/*
	if(demorecording)
		G_CheckDemoStatus();

	D_QuitNetGame();
*/

	exit(EXIT_FAILURE);
}
