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
//	Archiving: SaveGame I/O.
//
//-----------------------------------------------------------------------------

#include <stdint.h>

#include "i_system.h"
#include "z_zone.h"
#include "p_local.h"

// State.
#include "doomstat.h"
#include "r_state.h"

uint8_t *save_p;

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#define PADSAVEP() save_p += (4 - ((uintptr_t)save_p & 3)) & 3

//
// P_ArchivePlayers
//
void
P_ArchivePlayers(void) {
	int i;
	int j;
	player_t *dest;

	for(i = 0; i < MAXPLAYERS; i++) {
		if(!playeringame[i])
			continue;

		PADSAVEP();

		dest = (player_t *)save_p;
		memcpy(dest, &players[i], sizeof(player_t));
		save_p += sizeof(player_t);
		for(j = 0; j < NUMPSPRITES; j++) {
			if(dest->psprites[j].state) {
				dest->psprites[j].state
					= (state_t *)(dest->psprites[j].state - states);
			}
		}
	}
}

//
// P_UnArchivePlayers
//
void
P_UnArchivePlayers(void) {
	int i;
	int j;

	for(i = 0; i < MAXPLAYERS; i++) {
		if(!playeringame[i])
			continue;

		PADSAVEP();

		memcpy(&players[i], save_p, sizeof(player_t));
		save_p += sizeof(player_t);

		// will be set when unarc thinker
		players[i].mo       = NULL;
		players[i].message  = NULL;
		players[i].attacker = NULL;

		for(j = 0; j < NUMPSPRITES; j++) {
			if(players[i].psprites[j].state) {
				players[i].psprites[j].state
					= &states[(intptr_t)players[i].psprites[j].state];
			}
		}
	}
}

//
// P_ArchiveWorld
//
void
P_ArchiveWorld(void) {
	int i;
	int j;
	const struct p_sector *sec;
	const struct p_line *li;
	const struct p_side *si;
	short *put;

	put = (short *)save_p;

	// do sectors
	for(i = 0, sec = p_level.sectors; i < p_level.sectors_count; i++, sec++) {
		*put++ = sec->floor_height >> FRACBITS;
		*put++ = sec->ceiling_height >> FRACBITS;
		*put++ = sec->floor;
		*put++ = sec->ceiling;
		*put++ = sec->lighting;
		*put++ = sec->special_type; // needed?
		*put++ = sec->tag;     // needed?
	}

	// do lines
	for(i = 0, li = p_level.lines; i < p_level.lines_count; i++, li++) {
		*put++ = li->flags;
		*put++ = li->special_type;
		*put++ = li->sector_tag;
		for(j = 0; j < 2; j++) {
			if(li->sides[j] == -1)
				continue;

			si = p_level.sides + li->sides[j];

			*put++ = si->offset_x >> FRACBITS;
			*put++ = si->offset_y >> FRACBITS;
			*put++ = si->top_texture;
			*put++ = si->bottom_texture;
			*put++ = si->middle_texture;
		}
	}

	save_p = (uint8_t *)put;
}

//
// P_UnArchiveWorld
//
void
P_UnArchiveWorld(void) {
	int i;
	int j;
	struct p_sector *sec;
	struct p_line *li;
	struct p_side *si;
	short *get;

	get = (short *)save_p;

	// do sectors
	for(i = 0, sec = p_level.sectors; i < p_level.sectors_count; i++, sec++) {
		sec->floor_height   = *get++ << FRACBITS;
		sec->ceiling_height = *get++ << FRACBITS;
		sec->floor          = *get++;
		sec->ceiling        = *get++;
		sec->lighting       = *get++;
		sec->special_type   = *get++; // needed?
		sec->tag            = *get++; // needed?
		sec->special_data   = 0;
		sec->sound_target   = 0;
	}

	// do lines
	for(i = 0, li = p_level.lines; i < p_level.lines_count; i++, li++) {
		li->flags        = *get++;
		li->special_type = *get++;
		li->sector_tag   = *get++;
		for(j = 0; j < 2; j++) {
			if(li->sides[j] == -1)
				continue;
			si                 = p_level.sides + li->sides[j];
			si->offset_x       = *get++ << FRACBITS;
			si->offset_y       = *get++ << FRACBITS;
			si->top_texture    = *get++;
			si->bottom_texture = *get++;
			si->middle_texture = *get++;
		}
	}
	save_p = (uint8_t *)get;
}

//
// Thinkers
//
typedef enum {
	tc_end,
	tc_mobj

} thinkerclass_t;

//
// P_ArchiveThinkers
//
void
P_ArchiveThinkers(void) {
	thinker_t *th;
	mobj_t *mobj;

	// save off the current thinkers
	for(th = thinkercap.next; th != &thinkercap; th = th->next) {
		if(th->function.acp1 == (actionf_p1)P_MobjThinker) {
			*save_p++ = tc_mobj;
			PADSAVEP();
			mobj = (mobj_t *)save_p;
			memcpy(mobj, th, sizeof(*mobj));
			save_p += sizeof(*mobj);
			mobj->state = (state_t *)(mobj->state - states);

			if(mobj->player)
				mobj->player = (player_t *)((mobj->player - players) + 1);
			continue;
		}

		// I_Error ("P_ArchiveThinkers: Unknown thinker function");
	}

	// add a terminating marker
	*save_p++ = tc_end;
}

//
// P_UnArchiveThinkers
//
void
P_UnArchiveThinkers(void) {
	uint8_t tclass;
	thinker_t *currentthinker;
	thinker_t *next;
	mobj_t *mobj;

	// remove all the current thinkers
	currentthinker = thinkercap.next;
	while(currentthinker != &thinkercap) {
		next = currentthinker->next;

		if(currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
			P_RemoveMobj((mobj_t *)currentthinker);
		else
			Z_Free(currentthinker);

		currentthinker = next;
	}
	P_InitThinkers();

	// read in saved thinkers
	while(1) {
		tclass = *save_p++;
		switch(tclass) {
		case tc_end:
			return; // end of list

		case tc_mobj:
			PADSAVEP();
			mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
			memcpy(mobj, save_p, sizeof(*mobj));
			save_p += sizeof(*mobj);
			mobj->state  = &states[(intptr_t)mobj->state];
			mobj->target = NULL;
			if(mobj->player) {
				mobj->player     = &players[(intptr_t)mobj->player - 1];
				mobj->player->mo = mobj;
			}
			P_SetThingPosition(mobj);
			mobj->info                  = &mobjinfo[mobj->type];
			mobj->floorz                = mobj->subsector->sector->floor_height;
			mobj->ceilingz              = mobj->subsector->sector->ceiling_height;
			mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
			P_AddThinker(&mobj->thinker);
			break;

		default:
			I_Error("Unknown tclass %i in savegame", tclass);
		}
	}
}

//
// P_ArchiveSpecials
//
enum {
	tc_ceiling,
	tc_door,
	tc_floor,
	tc_plat,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_endspecials

} specials_e;

//
// Things to handle:
//
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
//
void
P_ArchiveSpecials(void) {
	thinker_t *th;
	ceiling_t *ceiling;
	vldoor_t *door;
	floormove_t *floor;
	plat_t *plat;
	lightflash_t *flash;
	strobe_t *strobe;
	glow_t *glow;
	int i;

	// save off the current thinkers
	for(th = thinkercap.next; th != &thinkercap; th = th->next) {
		if(th->function.acv == (actionf_v)NULL) {
			for(i = 0; i < MAXCEILINGS; i++)
				if(activeceilings[i] == (ceiling_t *)th)
					break;

			if(i < MAXCEILINGS) {
				*save_p++ = tc_ceiling;
				PADSAVEP();
				ceiling = (ceiling_t *)save_p;
				memcpy(ceiling, th, sizeof(*ceiling));
				save_p += sizeof(*ceiling);
				ceiling->sector = (struct p_sector *)(ceiling->sector - p_level.sectors);
			}
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_MoveCeiling) {
			*save_p++ = tc_ceiling;
			PADSAVEP();
			ceiling = (ceiling_t *)save_p;
			memcpy(ceiling, th, sizeof(*ceiling));
			save_p += sizeof(*ceiling);
			ceiling->sector = (struct p_sector *)(ceiling->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_VerticalDoor) {
			*save_p++ = tc_door;
			PADSAVEP();
			door = (vldoor_t *)save_p;
			memcpy(door, th, sizeof(*door));
			save_p += sizeof(*door);
			door->sector = (struct p_sector *)(door->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_MoveFloor) {
			*save_p++ = tc_floor;
			PADSAVEP();
			floor = (floormove_t *)save_p;
			memcpy(floor, th, sizeof(*floor));
			save_p += sizeof(*floor);
			floor->sector = (struct p_sector *)(floor->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_PlatRaise) {
			*save_p++ = tc_plat;
			PADSAVEP();
			plat = (plat_t *)save_p;
			memcpy(plat, th, sizeof(*plat));
			save_p += sizeof(*plat);
			plat->sector = (struct p_sector *)(plat->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_LightFlash) {
			*save_p++ = tc_flash;
			PADSAVEP();
			flash = (lightflash_t *)save_p;
			memcpy(flash, th, sizeof(*flash));
			save_p += sizeof(*flash);
			flash->sector = (struct p_sector *)(flash->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_StrobeFlash) {
			*save_p++ = tc_strobe;
			PADSAVEP();
			strobe = (strobe_t *)save_p;
			memcpy(strobe, th, sizeof(*strobe));
			save_p += sizeof(*strobe);
			strobe->sector = (struct p_sector *)(strobe->sector - p_level.sectors);
			continue;
		}

		if(th->function.acp1 == (actionf_p1)T_Glow) {
			*save_p++ = tc_glow;
			PADSAVEP();
			glow = (glow_t *)save_p;
			memcpy(glow, th, sizeof(*glow));
			save_p += sizeof(*glow);
			glow->sector = (struct p_sector *)(glow->sector - p_level.sectors);
			continue;
		}
	}

	// add a terminating marker
	*save_p++ = tc_endspecials;
}

//
// P_UnArchiveSpecials
//
void
P_UnArchiveSpecials(void) {
	uint8_t tclass;
	ceiling_t *ceiling;
	vldoor_t *door;
	floormove_t *floor;
	plat_t *plat;
	lightflash_t *flash;
	strobe_t *strobe;
	glow_t *glow;

	// read in saved thinkers
	while(1) {
		tclass = *save_p++;
		switch(tclass) {
		case tc_endspecials:
			return; // end of list

		case tc_ceiling:
			PADSAVEP();
			ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);
			memcpy(ceiling, save_p, sizeof(*ceiling));
			save_p += sizeof(*ceiling);
			ceiling->sector              = p_level.sectors + (intptr_t)ceiling->sector;
			ceiling->sector->special_data = ceiling;

			if(ceiling->thinker.function.acp1)
				ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

			P_AddThinker(&ceiling->thinker);
			P_AddActiveCeiling(ceiling);
			break;

		case tc_door:
			PADSAVEP();
			door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);
			memcpy(door, save_p, sizeof(*door));
			save_p += sizeof(*door);
			door->sector                = p_level.sectors + (intptr_t)door->sector;
			door->sector->special_data   = door;
			door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
			P_AddThinker(&door->thinker);
			break;

		case tc_floor:
			PADSAVEP();
			floor = Z_Malloc(sizeof(*floor), PU_LEVEL, NULL);
			memcpy(floor, save_p, sizeof(*floor));
			save_p += sizeof(*floor);
			floor->sector                = p_level.sectors + (intptr_t)floor->sector;
			floor->sector->special_data   = floor;
			floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
			P_AddThinker(&floor->thinker);
			break;

		case tc_plat:
			PADSAVEP();
			plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);
			memcpy(plat, save_p, sizeof(*plat));
			save_p += sizeof(*plat);
			plat->sector              = p_level.sectors + (intptr_t)plat->sector;
			plat->sector->special_data = plat;

			if(plat->thinker.function.acp1)
				plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

			P_AddThinker(&plat->thinker);
			P_AddActivePlat(plat);
			break;

		case tc_flash:
			PADSAVEP();
			flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);
			memcpy(flash, save_p, sizeof(*flash));
			save_p += sizeof(*flash);
			flash->sector                = p_level.sectors + (intptr_t)flash->sector;
			flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
			P_AddThinker(&flash->thinker);
			break;

		case tc_strobe:
			PADSAVEP();
			strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);
			memcpy(strobe, save_p, sizeof(*strobe));
			save_p += sizeof(*strobe);
			strobe->sector                = p_level.sectors + (intptr_t)strobe->sector;
			strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
			P_AddThinker(&strobe->thinker);
			break;

		case tc_glow:
			PADSAVEP();
			glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);
			memcpy(glow, save_p, sizeof(*glow));
			save_p += sizeof(*glow);
			glow->sector                = p_level.sectors + (intptr_t)glow->sector;
			glow->thinker.function.acp1 = (actionf_p1)T_Glow;
			P_AddThinker(&glow->thinker);
			break;

		default:
			I_Error("P_UnarchiveSpecials:Unknown tclass %i "
					"in savegame",
				tclass);
		}
	}
}
