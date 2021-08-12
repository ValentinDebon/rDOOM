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
// DESCRIPTION: Internal parameters
//
//-----------------------------------------------------------------------------

#include "m_param.h"
#include "m_array.h"
#include "i_error.h"

#include <string.h>

struct m_parameter {
	const char *name;
	const char *value;
};

M_TemplateArray(Parameter, struct m_parameter, 4);

static struct m_param {
	struct m_arrayParameter parameters;
} m_param;

void
M_AppendParam(const char *name, const char *value) {
	const struct m_parameter param = {
		.name = name, .value = value
	};

	M_ArrayParameterAppend(&m_param.parameters, param);
}

bool
M_CheckParam(const char *name) {
	const struct m_parameter * const params = m_param.parameters.begin;
	int index = m_param.parameters.end - m_param.parameters.begin;

	while(--index != -1 && strcmp(params[index].name, name) != 0);

	return index != -1;
}

bool
M_GetValueParam(const char *name, const char **valuep) {
	const struct m_parameter * const params = m_param.parameters.begin;
	int index = m_param.parameters.end - m_param.parameters.begin;

	while(--index != -1 && strcmp(params[index].name, name) != 0);

	if(index != -1) {
		*valuep = params[index].value;
		return true;
	} else {
		return false;
	}
}

bool
M_GetNumericParam(const char *name, int min, int max, int *numericp) {
	const char *value;

	if(!M_GetValueParam(name, &value)) {
		return false;
	}

	if(value == NULL) {
		I_Error("M_GetNumericParam %s: Missing value for argument", name);
	}

	char *endptr;
	const long int numeric = strtol(value, &endptr, 10);

	if(*endptr != '\0') {
		I_Error("M_GetNumericParam %s: Invalid character while parsing argument '%s'", name, value);
	}

	if(numeric < min) {
		I_Error("M_GetNumericParam %s: Numeric argument %ld is smaller than required minimum %d", name, numeric, min);
	}

	if(numeric > max) {
		I_Error("M_GetNumericParam %s: Numeric argument %ld is bigger than required maximum %d", name, numeric, max);
	}

	*numericp = numeric;

	return true;
}

