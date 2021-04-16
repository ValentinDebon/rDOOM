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

#ifndef RDOOM_M_ARRAY_H
#define RDOOM_M_ARRAY_H

#include <stdlib.h>

#include <assert.h>

#define M_TemplateArray(S, T, N) \
struct m_array##S { \
	T *begin; \
	T *end; \
	T *edge; \
}; \
\
static inline void \
M_Array##S##Append(struct m_array##S *array, T element) { \
	if((array)->end == (array)->edge) { \
		const size_t length = (array)->end - (array)->begin; \
		const size_t capacity = length != 0 ? length * 2 : (N); \
		T * const newbegin = realloc((array)->begin, capacity * sizeof(T)); \
		assert(newbegin != NULL); \
		(array)->begin = newbegin; \
		(array)->end = newbegin + length; \
		(array)->edge = newbegin + capacity; \
	} \
	*(array)->end = element; \
	(array)->end++; \
} \
\
static inline void \
M_Array##S##Fit(struct m_array##S *array) { \
	const size_t length = (array)->end - (array)->begin; \
	if(length != 0) { \
		T * const newbegin = realloc((array)->begin, length * sizeof(T)); \
		assert(newbegin != NULL); \
		(array)->begin = newbegin; \
		(array)->end = newbegin + length; \
		(array)->edge = (array)->end; \
	} \
} \
\
static inline void \
M_Array##S##Free(struct m_array##S *array) { \
	free((array)->begin); \
} \
\
_Static_assert((N) > 0, "Invalid default capacity for template Array"#S)

/* RDOOM_M_ARRAY_H */
#endif
