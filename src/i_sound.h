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
//	System interface, sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "sounds.h"

// Init at program start...
void
I_InitSound(void);

// ... update sound buffer and audio device at runtime...
void
I_UpdateSound(void);

void
I_SubmitSound(void);

//
//  SFX I/O
//

// Initialize channels?
void
I_SetChannels(void);

// Get raw data lump index for sound descriptor.
int
I_GetSfxLumpNum(sfxinfo_t *sfxinfo);

// Starts a sound in a particular sound channel.
int
I_StartSound(int id,
	int vol,
	int sep,
	int pitch,
	int priority);

// Stops a sound channel.
void
I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int
I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void
I_UpdateSoundParams(int handle,
	int vol,
	int sep,
	int pitch);

//
//  MUSIC I/O
//

// Volume.
void
I_SetMusicVolume(int volume);

// Registers a song handle to song data.
int
I_RegisterSong(const void *data);

// See above (register), then think backwards
void
I_UnRegisterSong(int handle);

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void
I_PlaySong(int handle,
	int looping);

// Stops a song over 3 seconds.
void
I_StopSong(int handle);

// PAUSE game handling.
void
I_PauseSong(int handle);

void
I_ResumeSong(int handle);

#endif
