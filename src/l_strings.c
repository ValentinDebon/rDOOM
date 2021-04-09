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
// DESCRIPTION:
//	Localized strings.
//
//-----------------------------------------------------------------------------

#include "l_strings.h"

#include <assert.h>

static struct {
	const char * const localized[LANGUAGE_LAST][STRING_LAST];
	const char * const *current;
} l_localization = {
	.localized = {
		{ /* LANGUAGE_ENGLISH */
			#include "l_english.inc"
		},
		{ /* LANGUAGE_FRENCH */
			#include "l_french.inc"
		},
	},
	.current = l_localization.localized[LANGUAGE_ENGLISH],
};

enum l_language
L_CurrentLanguage(void) {
	return l_localization.current - l_localization.localized[0];
}

void
L_SetCurrentLanguage(enum l_language language) {
	assert(language >= 0 && language < LANGUAGE_LAST);
	l_localization.current = l_localization.localized[language];
}

const char *
L_String(enum l_string identifier) {
	assert(identifier >= 0 && identifier < STRING_LAST);
	return l_localization.current[identifier];
}

