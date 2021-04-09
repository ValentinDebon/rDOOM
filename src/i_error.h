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

#ifndef RDOOM_I_ERROR_H
#define RDOOM_I_ERROR_H

#include <stdnoreturn.h>

noreturn void
I_Error(char *error, ...);

/* RDOOM_I_ERROR_H */
#endif
