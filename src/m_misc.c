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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t *hu_font[HU_FONTSIZE];

int
M_DrawText(int x,
	int y,
	boolean direct,
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

		w = SHORT(hu_font[c]->width);
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

boolean
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
	byte **buffer) {
	int handle, count, length;
	struct stat fileinfo;
	byte *buf;

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

//
// DEFAULTS
//
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

// machine-independent sound params
extern int numChannels;

// UNIX hack, to be removed.
#ifdef SNDSERV
extern char *sndserver_filename;
extern int mb_used;
#endif

#ifdef LINUX
char *mousetype;
char *mousedev;
#endif

extern char *chat_macros[];

typedef struct
{
	char *name;
	int *location;
	int defaultvalue;
} default_t;

default_t defaults[] = {
	{ "mouse_sensitivity", &mouseSensitivity, 5 },
	{ "sfx_volume", &snd_SfxVolume, 8 },
	{ "music_volume", &snd_MusicVolume, 8 },
	{ "show_messages", &showMessages, 1 },

#ifdef NORMALUNIX
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

// UNIX hack, to be removed.
#ifdef SNDSERV
	{ "mb_used", &mb_used, 2 },
#endif

#endif

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
};

typedef struct
{
	char *name;
	char **location;
	char *defaultvalue;
} default_str_t;

default_str_t defaultstrs[] = {
// UNIX hack, to be removed.
#ifdef SNDSERV
	{ "sndserver", &sndserver_filename, "sndserver" },
#endif

#ifdef LINUX
	{ "mousedev", &mousedev, "/dev/ttyS0" },
	{ "mousetype", &mousetype, "microsoft" },
#endif

	{ "chatmacro0", &chat_macros[0], HUSTR_CHATMACRO0 },
	{ "chatmacro1", &chat_macros[1], HUSTR_CHATMACRO1 },
	{ "chatmacro2", &chat_macros[2], HUSTR_CHATMACRO2 },
	{ "chatmacro3", &chat_macros[3], HUSTR_CHATMACRO3 },
	{ "chatmacro4", &chat_macros[4], HUSTR_CHATMACRO4 },
	{ "chatmacro5", &chat_macros[5], HUSTR_CHATMACRO5 },
	{ "chatmacro6", &chat_macros[6], HUSTR_CHATMACRO6 },
	{ "chatmacro7", &chat_macros[7], HUSTR_CHATMACRO7 },
	{ "chatmacro8", &chat_macros[8], HUSTR_CHATMACRO8 },
	{ "chatmacro9", &chat_macros[9], HUSTR_CHATMACRO9 }
};

int numdefaults;
int numdefaultstrs;
char *defaultfile;

//
// M_SaveDefaults
//
void
M_SaveDefaults(void) {
	int i;
	int v;
	char *s;
	FILE *f;

	f = fopen(defaultfile, "w");
	if(!f)
		return; // can't write the file, but don't complain

	for(i = 0; i < numdefaults; i++) {
		v = *defaults[i].location;
		fprintf(f, "%s\t\t%i\n", defaults[i].name, v);
	}

	for(i = 0; i < numdefaultstrs; i++) {
		s = *defaultstrs[i].location;
		fprintf(f, "%s\t\t\"%s\"\n", defaultstrs[i].name, s);
	}

	fclose(f);
}

//
// M_LoadDefaults
//
extern byte scantokey[128];

void
M_LoadDefaults(void) {
	int i;
	int len;
	FILE *f;
	char def[80];
	char strparm[100];
	char *newstring;
	int parm;
	boolean isstring;

	// set everything to base values
	numdefaults = sizeof(defaults) / sizeof(defaults[0]);
	for(i = 0; i < numdefaults; i++)
		*defaults[i].location = defaults[i].defaultvalue;
	numdefaultstrs = sizeof(defaultstrs) / sizeof(defaultstrs[0]);
	for(i = 0; i < numdefaultstrs; i++)
		*defaultstrs[i].location = defaultstrs[i].defaultvalue;

	// check for a custom default file
	i = M_CheckParm("-config");
	if(i && i < myargc - 1) {
		defaultfile = myargv[i + 1];
		printf("	default file: %s\n", defaultfile);
	} else
		defaultfile = basedefault;

	// read the file in, overriding any set defaults
	f = fopen(defaultfile, "r");
	if(f) {
		while(!feof(f)) {
			isstring = false;
			if(fscanf(f, "%79s %[^\n]\n", def, strparm) == 2) {
				if(strparm[0] == '"') {
					// get a string default
					isstring         = true;
					len              = strlen(strparm);
					newstring        = (char *)malloc(len);
					strparm[len - 1] = 0;
					strcpy(newstring, strparm + 1);
				} else if(strparm[0] == '0' && strparm[1] == 'x')
					sscanf(strparm + 2, "%x", &parm);
				else
					sscanf(strparm, "%i", &parm);

				if(!isstring) {
					for(i = 0; i < numdefaults; i++)
						if(!strcmp(def, defaults[i].name)) {
							*defaults[i].location = parm;
							break;
						}
				} else {
					for(i = 0; i < numdefaultstrs; i++)
						if(!strcmp(def, defaultstrs[i].name)) {
							*defaultstrs[i].location = newstring;
							break;
						}
				}
			}
		}

		fclose(f);
	}
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
	byte *data,
	int width,
	int height,
	byte *palette) {
	int i;
	int length;
	pcx_t *pcx;
	byte *pack;

	pcx = Z_Malloc(width * height * 2 + 1000, PU_STATIC, NULL);

	pcx->manufacturer   = 0x0a; // PCX id
	pcx->version        = 5;    // 256 color
	pcx->encoding       = 1;    // uncompressed
	pcx->bits_per_pixel = 8;    // 256 color
	pcx->xmin           = 0;
	pcx->ymin           = 0;
	pcx->xmax           = SHORT(width - 1);
	pcx->ymax           = SHORT(height - 1);
	pcx->hres           = SHORT(width);
	pcx->vres           = SHORT(height);
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes   = 1; // chunky image
	pcx->bytes_per_line = SHORT(width);
	pcx->palette_type   = SHORT(2); // not a grey scale
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
	length = pack - (byte *)pcx;
	M_WriteFile(filename, pcx, length);

	Z_Free(pcx);
}

//
// M_ScreenShot
//
void
M_ScreenShot(void) {
	int i;
	byte *linear;
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
	WritePCXfile(lbmname, linear, SCREENWIDTH, SCREENHEIGHT, W_CacheLumpName("PLAYPAL", PU_CACHE));

	players[consoleplayer].message = "screen shot";
}
