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

#include "m_swap.h"

/* Ironically, the original source code was compiling only when __BIG_ENDIAN__ was not defined,
I wonder if this was corrected in the Jaguar release. Which, if I recall, had an M68K */

/* Not needed with little endian. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

/* Swap 16bit, that is, MSB and LSB byte. */
uint16_t
SwapU16(uint16_t value) {
	return (value << 8) | (value >> 8);
}

/* Swapping 32bit. */
uint32_t
SwapU32(uint32_t value) {
	return ((uint32_t)SwapU16(value) << 16) | SwapU16(value >> 16);
}

#endif
