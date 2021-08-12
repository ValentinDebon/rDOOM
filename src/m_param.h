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

#ifndef RDOOM_M_PARAM_H
#define RDOOM_M_PARAM_H

#include <stdbool.h>

/* Adds a parameter to the parsed parameters list */
void
M_AppendParam(const char *name, const char *value);

/* Returns true if it finds parameter name in the parameters list */
bool
M_CheckParam(const char *name);

/* Returns the value associated with parameter name in the parameters list, NULL else */
bool
M_GetValueParam(const char *name, const char **valuep);

/* Returns the numeric value associated with parameter name in the parameters list,
 * returns and error on failure or if the value is not between min and max inclusive */
bool
M_GetNumericParam(const char *name, int min, int max, int *numericp);

/* RDOOM_M_PARAM_H */
#endif
