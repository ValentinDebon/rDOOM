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
//	System interface for sound.
//
//-----------------------------------------------------------------------------
#include "i_sound.h"

#include "i_system.h"
#include "i_error.h"
#include "m_swap.h"
#include "w_wad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct WAD w_lump_D_x {
	uint8_t magic[4];
	uint16_t size;
	uint16_t offset;
	uint16_t primary_channels;
	uint16_t secondary_channels;
	uint16_t instrument_patches;
	uint16_t unused; /* zero */
	uint16_t patches[];

};

static void
I_ShutdownSound(void) {
}

void
I_InitSound(void) {

	atexit(I_ShutdownSound);
}

void
I_UpdateSound(void) {
}

void
I_SubmitSound(void) {
}

void
I_SetChannels(void) {
}

int
I_GetSfxLumpNum(sfxinfo_t *sfxinfo) {
	char name[9];

	snprintf(name, sizeof(name), "ds%s", sfxinfo->name);

	return W_GetIdForName(name);
}

int
I_StartSound(int id,
	int vol,
	int sep,
	int pitch,
	int priority) {

	return id;
}

void
I_StopSound(int handle) {
}

int
I_SoundIsPlaying(int handle) {
	return 0;
}

void
I_UpdateSoundParams(int handle,
	int vol,
	int sep,
	int pitch) {
}

void
I_SetMusicVolume(int volume) {
}

int
I_RegisterSong(const void *data) {
	static const uint8_t magic[] = { 'M', 'U', 'S', 0x1A };
	const struct w_lump_D_x *music = data;

	if(memcmp(music->magic, magic, sizeof(magic)) != 0) {
		I_Error("I_RegisterSong: Invalid Music lump");
	}

	return 0;
}

void
I_UnRegisterSong(int handle) {
}

void
I_PlaySong(int handle, int looping) {
}

void
I_StopSong(int handle) {
}

void
I_PauseSong(int handle) {
}

void
I_ResumeSong(int handle) {
}

