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

#include <stdlib.h>

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
	return 0;
}

int
I_StartSound(int id,
	int vol,
	int sep,
	int pitch,
	int priority) {
	return 0;
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

void
I_PauseSong(int handle) {
}

void
I_ResumeSong(int handle) {
}

int
I_RegisterSong(void *data) {
	return 0;
}

void
I_PlaySong(int handle,
	int looping) {
}

void
I_StopSong(int handle) {
}

void
I_UnRegisterSong(int handle) {
}

