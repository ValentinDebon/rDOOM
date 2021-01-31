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
//	 separating modules.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

// Predefined with some OS.
#ifdef LINUX
#include <values.h>
#else
#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)

// Max pos 32-bit int.
#define MAXINT ((int)0x7fffffff)
#define MAXLONG ((long)0x7fffffff)
#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)

// Max negative 32-bit integer.
#define MININT ((int)0x80000000)
#define MINLONG ((long)0x80000000)
#endif

#endif
