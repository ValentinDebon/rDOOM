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
//	Handle Sector base lighting effects.
//	Muzzle flash?
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "m_random.h"

#include "doomdef.h"
#include "p_local.h"

// State.
#include "r_state.h"

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//
void
T_FireFlicker(fireflicker_t *flick) {
	int amount;

	if(--flick->count)
		return;

	amount = (P_Random() & 3) * 16;

	if(flick->sector->lighting - amount < flick->minlight)
		flick->sector->lighting = flick->minlight;
	else
		flick->sector->lighting = flick->maxlight - amount;

	flick->count = 4;
}

//
// P_SpawnFireFlicker
//
void
P_SpawnFireFlicker(struct p_sector *sector) {
	fireflicker_t *flick;

	// Note that we are resetting sector attributes.
	// Nothing special about it during gameplay.
	sector->special_type = 0;

	flick = Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);

	P_AddThinker(&flick->thinker);

	flick->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
	flick->sector                = sector;
	flick->maxlight              = sector->lighting;
	flick->minlight              = P_FindMinSurroundingLight(sector, sector->lighting) + 16;
	flick->count                 = 4;
}

//
// BROKEN LIGHT FLASHING
//

//
// T_LightFlash
// Do flashing lights.
//
void
T_LightFlash(lightflash_t *flash) {
	if(--flash->count)
		return;

	if(flash->sector->lighting == flash->maxlight) {
		flash->sector->lighting = flash->minlight;
		flash->count              = (P_Random() & flash->mintime) + 1;
	} else {
		flash->sector->lighting = flash->maxlight;
		flash->count              = (P_Random() & flash->maxtime) + 1;
	}
}

//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void
P_SpawnLightFlash(struct p_sector *sector) {
	lightflash_t *flash;

	// nothing special about it during gameplay
	sector->special_type = 0;

	flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);

	P_AddThinker(&flash->thinker);

	flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
	flash->sector                = sector;
	flash->maxlight              = sector->lighting;

	flash->minlight = P_FindMinSurroundingLight(sector, sector->lighting);
	flash->maxtime  = 64;
	flash->mintime  = 7;
	flash->count    = (P_Random() & flash->maxtime) + 1;
}

//
// STROBE LIGHT FLASHING
//

//
// T_StrobeFlash
//
void
T_StrobeFlash(strobe_t *flash) {
	if(--flash->count)
		return;

	if(flash->sector->lighting == flash->minlight) {
		flash->sector->lighting = flash->maxlight;
		flash->count              = flash->brighttime;
	} else {
		flash->sector->lighting = flash->minlight;
		flash->count              = flash->darktime;
	}
}

//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void
P_SpawnStrobeFlash(struct p_sector *sector,
	int fastOrSlow,
	int inSync) {
	strobe_t *flash;

	flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);

	P_AddThinker(&flash->thinker);

	flash->sector                = sector;
	flash->darktime              = fastOrSlow;
	flash->brighttime            = STROBEBRIGHT;
	flash->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
	flash->maxlight              = sector->lighting;
	flash->minlight              = P_FindMinSurroundingLight(sector, sector->lighting);

	if(flash->minlight == flash->maxlight)
		flash->minlight = 0;

	// nothing special about it during gameplay
	sector->special_type = 0;

	if(!inSync)
		flash->count = (P_Random() & 7) + 1;
	else
		flash->count = 1;
}

//
// Start strobing lights (usually from a trigger)
//
void
EV_StartLightStrobing(const struct p_line *line) {
	int secnum;
	struct p_sector *sec;

	secnum = -1;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
		sec = p_level.sectors + secnum;
		if(sec->special_data)
			continue;

		P_SpawnStrobeFlash(sec, SLOWDARK, 0);
	}
}

//
// TURN LINE'S TAG LIGHTS OFF
//
void
EV_TurnTagLightsOff(const struct p_line *line) {
	int i;
	int j;
	int min;
	struct p_sector *sector;
	const struct p_sector *tsec;
	const struct p_line *templine;

	sector = p_level.sectors;

	for(j = 0; j < p_level.sectors_count; j++, sector++) {
		if(sector->tag == line->sector_tag) {
			min = sector->lighting;
			for(i = 0; i < sector->lines_count; i++) {
				templine = sector->lines[i];
				tsec     = getNextSector(templine, sector);
				if(!tsec)
					continue;
				if(tsec->lighting < min)
					min = tsec->lighting;
			}
			sector->lighting = min;
		}
	}
}

//
// TURN LINE'S TAG LIGHTS ON
//
void
EV_LightTurnOn(const struct p_line *line,
	int bright) {
	int i;
	int j;
	struct p_sector *sector;
	const struct p_sector *temp;
	const struct p_line *templine;

	sector = p_level.sectors;

	for(i = 0; i < p_level.sectors_count; i++, sector++) {
		if(sector->tag == line->sector_tag) {
			// bright = 0 means to search
			// for highest light level
			// surrounding sector
			if(!bright) {
				for(j = 0; j < sector->lines_count; j++) {
					templine = sector->lines[j];
					temp     = getNextSector(templine, sector);

					if(!temp)
						continue;

					if(temp->lighting > bright)
						bright = temp->lighting;
				}
			}
			sector->lighting = bright;
		}
	}
}

//
// Spawn glowing light
//

void
T_Glow(glow_t *g) {
	switch(g->direction) {
	case -1:
		// DOWN
		g->sector->lighting -= GLOWSPEED;
		if(g->sector->lighting <= g->minlight) {
			g->sector->lighting += GLOWSPEED;
			g->direction = 1;
		}
		break;

	case 1:
		// UP
		g->sector->lighting += GLOWSPEED;
		if(g->sector->lighting >= g->maxlight) {
			g->sector->lighting -= GLOWSPEED;
			g->direction = -1;
		}
		break;
	}
}

void
P_SpawnGlowingLight(struct p_sector *sector) {
	glow_t *g;

	g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);

	P_AddThinker(&g->thinker);

	g->sector                = sector;
	g->minlight              = P_FindMinSurroundingLight(sector, sector->lighting);
	g->maxlight              = sector->lighting;
	g->thinker.function.acp1 = (actionf_p1)T_Glow;
	g->direction             = -1;

	sector->special_type = 0;
}
