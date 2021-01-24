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
//	Fixed point arithmetics, implementation.
//
//-----------------------------------------------------------------------------

#ifndef __M_FIXED__
#define __M_FIXED__

/*
 * Fixed point, 32bit as 16.16.
 */

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

#define FixedFor(a) ((fixed_t)(a) << FRACBITS)

typedef int fixed_t;

fixed_t
FixedMul(fixed_t a, fixed_t b);
fixed_t
FixedDiv(fixed_t a, fixed_t b);
fixed_t
FixedDiv2(fixed_t a, fixed_t b);

#endif
