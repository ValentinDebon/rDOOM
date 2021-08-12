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
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_param.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_error.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

#include "l_strings.h"

// State.
#include "doomstat.h"

// Data.

#include "m_misc.h"

struct m_defaultNumeric {
	const char *name;
	int *location;
	int defaultvalue;
};

struct m_defaultString {
	const char *name;
	char **location;
	enum l_string defaultvalue;
};

/* Imported Defaults */
int usemouse;
int usejoystick;

extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;

extern int key_strafeleft;
extern int key_straferight;

extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;

extern int viewwidth;
extern int viewheight;

extern int mouseSensitivity;
extern int showMessages;

extern int detailLevel;

extern int screenblocks;

extern int showMessages;

extern int numChannels;

extern int mb_used;

extern char *chat_macros[];

static struct m_misc {
	struct {
		char *file;
		struct m_defaultNumeric numerics[28];
		struct m_defaultString strings[10];
	} defaults;
} m_misc = {
	.defaults = {
		.numerics = {
			{ "mouse_sensitivity", &mouseSensitivity, 5 },
			{ "sfx_volume", &snd_SfxVolume, 8 },
			{ "music_volume", &snd_MusicVolume, 8 },
			{ "show_messages", &showMessages, 1 },

			{ "key_right", &key_right, KEY_RIGHTARROW },
			{ "key_left", &key_left, KEY_LEFTARROW },
			{ "key_up", &key_up, KEY_UPARROW },
			{ "key_down", &key_down, KEY_DOWNARROW },
			{ "key_strafeleft", &key_strafeleft, ',' },
			{ "key_straferight", &key_straferight, '.' },

			{ "key_fire", &key_fire, KEY_RCTRL },
			{ "key_use", &key_use, ' ' },
			{ "key_strafe", &key_strafe, KEY_RALT },
			{ "key_speed", &key_speed, KEY_RSHIFT },

			{ "mb_used", &mb_used, 2 },

			{ "use_mouse", &usemouse, 1 },
			{ "mouseb_fire", &mousebfire, 0 },
			{ "mouseb_strafe", &mousebstrafe, 1 },
			{ "mouseb_forward", &mousebforward, 2 },

			{ "use_joystick", &usejoystick, 0 },
			{ "joyb_fire", &joybfire, 0 },
			{ "joyb_strafe", &joybstrafe, 1 },
			{ "joyb_use", &joybuse, 3 },
			{ "joyb_speed", &joybspeed, 2 },

			{ "screenblocks", &screenblocks, 9 },
			{ "detaillevel", &detailLevel, 0 },

			{ "snd_channels", &numChannels, 3 },

			{ "usegamma", &usegamma, 0 },
		},
		.strings = {
			{ "chatmacro0", chat_macros + 0, STRING_HU_CHATMACRO0 },
			{ "chatmacro1", chat_macros + 1, STRING_HU_CHATMACRO1 },
			{ "chatmacro2", chat_macros + 2, STRING_HU_CHATMACRO2 },
			{ "chatmacro3", chat_macros + 3, STRING_HU_CHATMACRO3 },
			{ "chatmacro4", chat_macros + 4, STRING_HU_CHATMACRO4 },
			{ "chatmacro5", chat_macros + 5, STRING_HU_CHATMACRO5 },
			{ "chatmacro6", chat_macros + 6, STRING_HU_CHATMACRO6 },
			{ "chatmacro7", chat_macros + 7, STRING_HU_CHATMACRO7 },
			{ "chatmacro8", chat_macros + 8, STRING_HU_CHATMACRO8 },
			{ "chatmacro9", chat_macros + 9, STRING_HU_CHATMACRO9 },
		},
	},
};

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern const patch_t *hu_font[HU_FONTSIZE];

int
M_DrawText(int x,
	int y,
	bool direct,
	char *string) {
	int c;
	int w;

	while(*string) {
		c = toupper(*string) - HU_FONTSTART;
		string++;
		if(c < 0 || c > HU_FONTSIZE) {
			x += 4;
			continue;
		}

		w = LE_U16(hu_font[c]->width);
		if(x + w > SCREENWIDTH)
			break;
		if(direct)
			V_DrawPatchDirect(x, y, 0, hu_font[c]);
		else
			V_DrawPatch(x, y, 0, hu_font[c]);
		x += w;
	}

	return x;
}

//
// M_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

bool
M_WriteFile(char const *name,
	void *source,
	int length) {
	int handle;
	int count;

	handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

	if(handle == -1)
		return false;

	count = write(handle, source, length);
	close(handle);

	if(count < length)
		return false;

	return true;
}

//
// M_ReadFile
//
int
M_ReadFile(char const *name,
	uint8_t **buffer) {
	int handle, count, length;
	struct stat fileinfo;
	uint8_t *buf;

	handle = open(name, O_RDONLY | O_BINARY, 0666);
	if(handle == -1)
		I_Error("Couldn't read file %s", name);
	if(fstat(handle, &fileinfo) == -1)
		I_Error("Couldn't read file %s", name);
	length = fileinfo.st_size;
	buf    = Z_Malloc(length, PU_STATIC, NULL);
	count  = read(handle, buf, length);
	close(handle);

	if(count < length)
		I_Error("Couldn't read file %s", name);

	*buffer = buf;
	return length;
}

static void
M_SaveDefaults(void) {
	FILE *filep = fopen(m_misc.defaults.file, "w");

	if(filep == NULL) {
		/* can't write the file, but don't complain */
		return;
	}

	for(int i = 0; i < sizeof(m_misc.defaults.numerics) / sizeof(*m_misc.defaults.numerics); i++) {
		const int value = *m_misc.defaults.numerics[i].location;
		fprintf(filep, "%s\t\t%i\n", m_misc.defaults.numerics[i].name, value);
	}

	for(int i = 0; i < sizeof(m_misc.defaults.strings) / sizeof(*m_misc.defaults.strings); i++) {
		char * const value = *m_misc.defaults.strings[i].location;
		fprintf(filep, "%s\t\t\"%s\"\n", m_misc.defaults.strings[i].name, value);
		free(value);
	}

	fclose(filep);

	free(m_misc.defaults.file);
}

void
M_LoadDefaults(void) {

	/* Set everything to base values */
	for(int i = 0; i < sizeof(m_misc.defaults.numerics) / sizeof(*m_misc.defaults.numerics); i++) {
		*m_misc.defaults.numerics[i].location = m_misc.defaults.numerics[i].defaultvalue;
	}

	for(int i = 0; i < sizeof(m_misc.defaults.strings) / sizeof(*m_misc.defaults.strings); i++) {
		*m_misc.defaults.strings[i].location = strdup(L_String(m_misc.defaults.strings[i].defaultvalue));
	}

	/* Check for a custom default file */
	const char *defaultfile;

	if(M_GetValueParam("config", &defaultfile)) {
		printf(" default file: %s\n", defaultfile);
	} else if(devparm) {
		defaultfile = DEVDATA "default.cfg";
	} else {
		const char *home = getenv("HOME");

		if(home == NULL) {
			I_Error("Please set $HOME to your home directory");
		}

		defaultfile = M_Format(NULL, "%s/.doomrc", home);
	}

	m_misc.defaults.file = strdup(defaultfile);

	/* Read the file in, overriding any set defaults */
	FILE *filep = fopen(m_misc.defaults.file, "r");
	char value[128], name[18]; /* 18 is the length + 1 of "mouse_sensitivity", the longest one */

	if(filep != NULL) {
		int line = 1;
		while(!feof(filep)) {
			if(fscanf(filep, "%17s %127[^\n]\n", name, value) == 2) {

				if(value[0] == '"') {
					char *endquote = strrchr(value, '"');
					if(endquote != NULL) {
						*endquote = '\0';
					}

					for(int i = 0; i < sizeof(m_misc.defaults.strings) / sizeof(*m_misc.defaults.strings); i++) {
						if(strcmp(m_misc.defaults.strings[i].name, name) == 0) {
							char ** const location = m_misc.defaults.strings[i].location;
							free(*location);
							*location = strdup(value + 1);
							break;
						}
					}
				} else {
					char *endptr;
					const long numeric = strtol(value, &endptr, 0);

					if(*endptr != '\0') {
						I_Error("Invalid numeric value for default '%s': '%s'", name, value);
					}

					for(int i = 0; i < sizeof(m_misc.defaults.numerics) / sizeof(*m_misc.defaults.numerics); i++) {
						if(strcmp(m_misc.defaults.numerics[i].name, name) == 0) {
							*m_misc.defaults.numerics[i].location = numeric;
							break;
						}
					}
				}
			} else {
				I_Error("Invalid line %d of defaults file", line);
			}
			line++;
		}

		fclose(filep);
	}

	atexit(M_SaveDefaults);
}

//
// SCREEN SHOTS
//

typedef struct
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;

	unsigned short xmin;
	unsigned short ymin;
	unsigned short xmax;
	unsigned short ymax;

	unsigned short hres;
	unsigned short vres;

	unsigned char palette[48];

	char reserved;
	char color_planes;
	unsigned short bytes_per_line;
	unsigned short palette_type;

	char filler[58];
	unsigned char data; // unbounded
} pcx_t;

//
// WritePCXfile
//
void
WritePCXfile(char *filename,
	uint8_t *data,
	int width,
	int height,
	const uint8_t *palette) {
	int i;
	int length;
	pcx_t *pcx;
	uint8_t *pack;

	pcx = Z_Malloc(width * height * 2 + 1000, PU_STATIC, NULL);

	pcx->manufacturer   = 0x0a; // PCX id
	pcx->version        = 5;    // 256 color
	pcx->encoding       = 1;    // uncompressed
	pcx->bits_per_pixel = 8;    // 256 color
	pcx->xmin           = 0;
	pcx->ymin           = 0;
	pcx->xmax           = LE_U16(width - 1);
	pcx->ymax           = LE_U16(height - 1);
	pcx->hres           = LE_U16(width);
	pcx->vres           = LE_U16(height);
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes   = 1; // chunky image
	pcx->bytes_per_line = LE_U16(width);
	pcx->palette_type   = LE_U16(2); // not a grey scale
	memset(pcx->filler, 0, sizeof(pcx->filler));

	// pack the image
	pack = &pcx->data;

	for(i = 0; i < width * height; i++) {
		if((*data & 0xc0) != 0xc0)
			*pack++ = *data++;
		else {
			*pack++ = 0xc1;
			*pack++ = *data++;
		}
	}

	// write the palette
	*pack++ = 0x0c; // palette ID byte
	for(i = 0; i < 768; i++)
		*pack++ = *palette++;

	// write output file
	length = pack - (uint8_t *)pcx;
	M_WriteFile(filename, pcx, length);

	Z_Free(pcx);
}

//
// M_ScreenShot
//
void
M_ScreenShot(void) {
	int i;
	uint8_t *linear;
	char lbmname[12];

	// munge planar buffer to linear
	linear = screens[2];
	I_ReadScreen(linear);

	// find a file name to save it to
	strcpy(lbmname, "DOOM00.pcx");

	for(i = 0; i <= 99; i++) {
		lbmname[4] = i / 10 + '0';
		lbmname[5] = i % 10 + '0';
		if(access(lbmname, 0) == -1)
			break; // file doesn't exist
	}
	if(i == 100)
		I_Error("M_ScreenShot: Couldn't create a PCX");

	// save the pcx file
	WritePCXfile(lbmname, linear, SCREENWIDTH, SCREENHEIGHT, W_LumpForName("PLAYPAL")->data);

	players[consoleplayer].message = "screen shot";
}

const char *
M_Format(size_t *lenp, const char *format, ...) {
	static char *buffer = NULL;
	static size_t capacity = 0;
	size_t required;
	va_list ap;

	while(va_start(ap, format),
		required = vsnprintf(buffer, capacity, format, ap),
		va_end(ap), required >= capacity) {
		const size_t newcapacity = required + 1;
		char * const newbuffer = realloc(buffer, newcapacity);

		if(newbuffer == NULL) {
			I_Error("Unable to allocate memory for format buffer");
		}

		buffer = newbuffer;
		capacity = newcapacity;
	}

	if(lenp != NULL) {
		*lenp = required;
	}

	return buffer;
}
