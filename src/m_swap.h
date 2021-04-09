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
//	Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------

#ifndef RDOOM_M_SWAP_H
#define RDOOM_M_SWAP_H

#include <stdint.h>

/* Endianess handling.
 * WAD files are stored little endian.
 */

#if defined(__LITTLE_ENDIAN__)

#define LE_U16(x) ((uint16_t)(x))
#define LE_S16(x) ((int16_t)(x))
#define LE_U32(x) ((uint32_t)(x))
#define LE_S32(x) ((int32_t)(x))

#elif defined(__BIG_ENDIAN__)

uint16_t
SwapU16(uint16_t value);

uint32_t
SwapU32(uint32_t value);

#define LE_U16(x) (SwapU16(x))
#define LE_S16(x) ((int16_t)SwapU16(x))
#define LE_U32(x) (SwapU32(x))
#define LE_S32(x) ((int32_t)SwapU32(x))

#else

#error "Unsupported/undefined byte order"

#endif

/* RDOOM_M_SWAP_H */
#endif
