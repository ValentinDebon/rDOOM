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
//	Plats (i.e. elevator platforms) code, raising/lowering.
//
//-----------------------------------------------------------------------------

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "sounds.h"

plat_t *activeplats[MAXPLATS];

//
// Move a plat up and down
//
void
T_PlatRaise(plat_t *plat) {
	result_e res;

	switch(plat->status) {
	case up:
		res = T_MovePlane(plat->sector,
			plat->speed,
			plat->high,
			plat->crush,
			0,
			1);

		if(plat->type == raiseAndChange
			|| plat->type == raiseToNearestAndChange) {
			if(!(leveltime & 7))
				S_StartSound((mobj_t *)&plat->sector->sound_origin,
					sfx_stnmov);
		}

		if(res == crushed && (!plat->crush)) {
			plat->count  = plat->wait;
			plat->status = down;
			S_StartSound((mobj_t *)&plat->sector->sound_origin,
				sfx_pstart);
		} else {
			if(res == pastdest) {
				plat->count  = plat->wait;
				plat->status = waiting;
				S_StartSound((mobj_t *)&plat->sector->sound_origin,
					sfx_pstop);

				switch(plat->type) {
				case blazeDWUS:
				case downWaitUpStay:
					P_RemoveActivePlat(plat);
					break;

				case raiseAndChange:
				case raiseToNearestAndChange:
					P_RemoveActivePlat(plat);
					break;

				default:
					break;
				}
			}
		}
		break;

	case down:
		res = T_MovePlane(plat->sector, plat->speed, plat->low, false, 0, -1);

		if(res == pastdest) {
			plat->count  = plat->wait;
			plat->status = waiting;
			S_StartSound((mobj_t *)&plat->sector->sound_origin, sfx_pstop);
		}
		break;

	case waiting:
		if(!--plat->count) {
			if(plat->sector->floor_height == plat->low)
				plat->status = up;
			else
				plat->status = down;
			S_StartSound((mobj_t *)&plat->sector->sound_origin, sfx_pstart);
		}
	case in_stasis:
		break;
	}
}

//
// Do Platforms
//  "amount" is only used for SOME platforms.
//
int
EV_DoPlat(const struct p_line *line,
	plattype_e type,
	int amount) {
	plat_t *plat;
	int secnum;
	int rtn;
	struct p_sector *sec;

	secnum = -1;
	rtn    = 0;

	//	Activate all <type> plats that are in_stasis
	switch(type) {
	case perpetualRaise:
		P_ActivateInStasis(line->sector_tag);
		break;

	default:
		break;
	}

	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
		sec = p_level.sectors + secnum;

		if(sec->special_data)
			continue;

		// Find lowest & highest floors around sector
		rtn  = 1;
		plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, 0);
		P_AddThinker(&plat->thinker);

		plat->type                  = type;
		plat->sector                = sec;
		plat->sector->special_data   = plat;
		plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
		plat->crush                 = false;
		plat->tag                   = line->sector_tag;

		switch(type) {
		case raiseToNearestAndChange:
			plat->speed   = PLATSPEED / 2;
			sec->floor    = p_level.sides[line->sides[0]].sector->floor;
			plat->high    = P_FindNextHighestFloor(sec, sec->floor_height);
			plat->wait    = 0;
			plat->status  = up;
			// NO MORE DAMAGE, IF APPLICABLE
			sec->special_type = 0;

			S_StartSound((mobj_t *)&sec->sound_origin, sfx_stnmov);
			break;

		case raiseAndChange:
			plat->speed   = PLATSPEED / 2;
			sec->floor    = p_level.sides[line->sides[0]].sector->floor;
			plat->high    = sec->floor_height + amount * FRACUNIT;
			plat->wait    = 0;
			plat->status  = up;

			S_StartSound((mobj_t *)&sec->sound_origin, sfx_stnmov);
			break;

		case downWaitUpStay:
			plat->speed = PLATSPEED * 4;
			plat->low   = P_FindLowestFloorSurrounding(sec);

			if(plat->low > sec->floor_height)
				plat->low = sec->floor_height;

			plat->high   = sec->floor_height;
			plat->wait   = 35 * PLATWAIT;
			plat->status = down;
			S_StartSound((mobj_t *)&sec->sound_origin, sfx_pstart);
			break;

		case blazeDWUS:
			plat->speed = PLATSPEED * 8;
			plat->low   = P_FindLowestFloorSurrounding(sec);

			if(plat->low > sec->floor_height)
				plat->low = sec->floor_height;

			plat->high   = sec->floor_height;
			plat->wait   = 35 * PLATWAIT;
			plat->status = down;
			S_StartSound((mobj_t *)&sec->sound_origin, sfx_pstart);
			break;

		case perpetualRaise:
			plat->speed = PLATSPEED;
			plat->low   = P_FindLowestFloorSurrounding(sec);

			if(plat->low > sec->floor_height)
				plat->low = sec->floor_height;

			plat->high = P_FindHighestFloorSurrounding(sec);

			if(plat->high < sec->floor_height)
				plat->high = sec->floor_height;

			plat->wait   = 35 * PLATWAIT;
			plat->status = P_Random() & 1;

			S_StartSound((mobj_t *)&sec->sound_origin, sfx_pstart);
			break;
		}
		P_AddActivePlat(plat);
	}
	return rtn;
}

void
P_ActivateInStasis(int tag) {
	int i;

	for(i = 0; i < MAXPLATS; i++)
		if(activeplats[i]
			&& (activeplats[i])->tag == tag
			&& (activeplats[i])->status == in_stasis) {
			(activeplats[i])->status = (activeplats[i])->oldstatus;
			(activeplats[i])->thinker.function.acp1
				= (actionf_p1)T_PlatRaise;
		}
}

void
EV_StopPlat(const struct p_line *line) {
	int j;

	for(j = 0; j < MAXPLATS; j++)
		if(activeplats[j]
			&& ((activeplats[j])->status != in_stasis)
			&& ((activeplats[j])->tag == line->sector_tag)) {
			(activeplats[j])->oldstatus            = (activeplats[j])->status;
			(activeplats[j])->status               = in_stasis;
			(activeplats[j])->thinker.function.acv = (actionf_v)NULL;
		}
}

void
P_AddActivePlat(plat_t *plat) {
	int i;

	for(i = 0; i < MAXPLATS; i++)
		if(activeplats[i] == NULL) {
			activeplats[i] = plat;
			return;
		}
	I_Error("P_AddActivePlat: no more plats!");
}

void
P_RemoveActivePlat(plat_t *plat) {
	int i;
	for(i = 0; i < MAXPLATS; i++)
		if(plat == activeplats[i]) {
			(activeplats[i])->sector->special_data = NULL;
			P_RemoveThinker(&(activeplats[i])->thinker);
			activeplats[i] = NULL;

			return;
		}
	I_Error("P_RemoveActivePlat: can't find plat!");
}
