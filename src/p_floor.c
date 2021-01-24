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
//	Floor animation: raising stairs.
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"
// Data.
#include "sounds.h"

//
// FLOORS
//

//
// Move a plane (floor or ceiling) and check for crushing
//
result_e
T_MovePlane(struct p_sector *sector,
	fixed_t speed,
	fixed_t dest,
	boolean crush,
	int floorOrCeiling,
	int direction) {
	boolean flag;
	fixed_t lastpos;

	switch(floorOrCeiling) {
	case 0:
		// FLOOR
		switch(direction) {
		case -1:
			// DOWN
			if(sector->floor_height - speed < dest) {
				lastpos             = sector->floor_height;
				sector->floor_height = dest;
				flag                = P_ChangeSector(sector, crush);
				if(flag == true) {
					sector->floor_height = lastpos;
					P_ChangeSector(sector, crush);
					//return crushed;
				}
				return pastdest;
			} else {
				lastpos = sector->floor_height;
				sector->floor_height -= speed;
				flag = P_ChangeSector(sector, crush);
				if(flag == true) {
					sector->floor_height = lastpos;
					P_ChangeSector(sector, crush);
					return crushed;
				}
			}
			break;

		case 1:
			// UP
			if(sector->floor_height + speed > dest) {
				lastpos             = sector->floor_height;
				sector->floor_height = dest;
				flag                = P_ChangeSector(sector, crush);
				if(flag == true) {
					sector->floor_height = lastpos;
					P_ChangeSector(sector, crush);
					//return crushed;
				}
				return pastdest;
			} else {
				// COULD GET CRUSHED
				lastpos = sector->floor_height;
				sector->floor_height += speed;
				flag = P_ChangeSector(sector, crush);
				if(flag == true) {
					if(crush == true)
						return crushed;
					sector->floor_height = lastpos;
					P_ChangeSector(sector, crush);
					return crushed;
				}
			}
			break;
		}
		break;

	case 1:
		// CEILING
		switch(direction) {
		case -1:
			// DOWN
			if(sector->ceiling_height - speed < dest) {
				lastpos               = sector->ceiling_height;
				sector->ceiling_height = dest;
				flag                  = P_ChangeSector(sector, crush);

				if(flag == true) {
					sector->ceiling_height = lastpos;
					P_ChangeSector(sector, crush);
					//return crushed;
				}
				return pastdest;
			} else {
				// COULD GET CRUSHED
				lastpos = sector->ceiling_height;
				sector->ceiling_height -= speed;
				flag = P_ChangeSector(sector, crush);

				if(flag == true) {
					if(crush == true)
						return crushed;
					sector->ceiling_height = lastpos;
					P_ChangeSector(sector, crush);
					return crushed;
				}
			}
			break;

		case 1:
			// UP
			if(sector->ceiling_height + speed > dest) {
				lastpos               = sector->ceiling_height;
				sector->ceiling_height = dest;
				flag                  = P_ChangeSector(sector, crush);
				if(flag == true) {
					sector->ceiling_height = lastpos;
					P_ChangeSector(sector, crush);
					//return crushed;
				}
				return pastdest;
			} else {
				lastpos = sector->ceiling_height;
				sector->ceiling_height += speed;
				flag = P_ChangeSector(sector, crush);
// UNUSED
#if 0
		if (flag == true)
		{
		    sector->ceiling_height = lastpos;
		    P_ChangeSector(sector,crush);
		    return crushed;
		}
#endif
			}
			break;
		}
		break;
	}
	return ok;
}

//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void
T_MoveFloor(floormove_t *floor) {
	result_e res;

	res = T_MovePlane(floor->sector,
		floor->speed,
		floor->floordestheight,
		floor->crush,
		0,
		floor->direction);

	if(!(leveltime & 7))
		S_StartSound((mobj_t *)&floor->sector->sound_origin,
			sfx_stnmov);

	if(res == pastdest) {
		floor->sector->special_data = NULL;

		if(floor->direction == 1) {
			switch(floor->type) {
			case donutRaise:
				floor->sector->special_type = floor->newspecial;
				floor->sector->floor = floor->texture;
			default:
				break;
			}
		} else if(floor->direction == -1) {
			switch(floor->type) {
			case lowerAndChange:
				floor->sector->special_type = floor->newspecial;
				floor->sector->floor = floor->texture;
			default:
				break;
			}
		}
		P_RemoveThinker(&floor->thinker);

		S_StartSound((mobj_t *)&floor->sector->sound_origin,
			sfx_pstop);
	}
}

//
// HANDLE FLOOR TYPES
//
int
EV_DoFloor(const struct p_line *line,
	floor_e floortype) {
	int secnum;
	int rtn;
	int i;
	struct p_sector *sec;
	floormove_t *floor;

	secnum = -1;
	rtn    = 0;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
		sec = p_level.sectors + secnum;

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if(sec->special_data)
			continue;

		// new floor thinker
		rtn   = 1;
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker(&floor->thinker);
		sec->special_data             = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->type                  = floortype;
		floor->crush                 = false;

		switch(floortype) {
		case lowerFloor:
			floor->direction       = -1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = P_FindHighestFloorSurrounding(sec);
			break;

		case lowerFloorToLowest:
			floor->direction       = -1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = P_FindLowestFloorSurrounding(sec);
			break;

		case turboLower:
			floor->direction       = -1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED * 4;
			floor->floordestheight = P_FindHighestFloorSurrounding(sec);
			if(floor->floordestheight != sec->floor_height)
				floor->floordestheight += 8 * FRACUNIT;
			break;

		case raiseFloorCrush:
			floor->crush = true;
		case raiseFloor:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
			if(floor->floordestheight > sec->ceiling_height)
				floor->floordestheight = sec->ceiling_height;
			floor->floordestheight -= (8 * FRACUNIT) * (floortype == raiseFloorCrush);
			break;

		case raiseFloorTurbo:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED * 4;
			floor->floordestheight = P_FindNextHighestFloor(sec, sec->floor_height);
			break;

		case raiseFloorToNearest:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = P_FindNextHighestFloor(sec, sec->floor_height);
			break;

		case raiseFloor24:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = floor->sector->floor_height + 24 * FRACUNIT;
			break;
		case raiseFloor512:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = floor->sector->floor_height + 512 * FRACUNIT;
			break;

		case raiseFloor24AndChange:
			floor->direction       = 1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = floor->sector->floor_height + 24 * FRACUNIT;
			sec->floor          = line->front_sector->floor;
			sec->special_type           = line->front_sector->special_type;
			break;

		case raiseToTexture: {
			int minsize = MAXINT;
			const struct p_side *side;

			floor->direction = 1;
			floor->sector    = sec;
			floor->speed     = FLOORSPEED;
			for(i = 0; i < sec->lines_count; i++) {
				if(twoSided(secnum, i)) {
					side = getSide(secnum, i, 0);
					if(side->bottom_texture >= 0)
						if(textureheight[side->bottom_texture] < minsize)
							minsize = textureheight[side->bottom_texture];
					side = getSide(secnum, i, 1);
					if(side->bottom_texture >= 0)
						if(textureheight[side->bottom_texture] < minsize)
							minsize = textureheight[side->bottom_texture];
				}
			}
			floor->floordestheight = floor->sector->floor_height + minsize;
		} break;

		case lowerAndChange:
			floor->direction       = -1;
			floor->sector          = sec;
			floor->speed           = FLOORSPEED;
			floor->floordestheight = P_FindLowestFloorSurrounding(sec);
			floor->texture         = sec->floor;

			for(i = 0; i < sec->lines_count; i++) {
				if(twoSided(secnum, i)) {
					if(getSide(secnum, i, 0)->sector - p_level.sectors == secnum) {
						sec = getSector(secnum, i, 1);

						if(sec->floor_height == floor->floordestheight) {
							floor->texture    = sec->floor;
							floor->newspecial = sec->special_type;
							break;
						}
					} else {
						sec = getSector(secnum, i, 0);

						if(sec->floor_height == floor->floordestheight) {
							floor->texture    = sec->floor;
							floor->newspecial = sec->special_type;
							break;
						}
					}
				}
			}
		default:
			break;
		}
	}
	return rtn;
}

//
// BUILD A STAIRCASE!
//
int
EV_BuildStairs(const struct p_line *line,
	stair_e type) {
	int secnum;
	int height;
	int i;
	int newsecnum;
	int texture;
	int ok;
	int rtn;

	struct p_sector *sec;
	struct p_sector *tsec;

	floormove_t *floor;

	fixed_t stairsize;
	fixed_t speed;

	secnum = -1;
	rtn    = 0;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
		sec = p_level.sectors + secnum;

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if(sec->special_data)
			continue;

		// new floor thinker
		rtn   = 1;
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker(&floor->thinker);
		sec->special_data             = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->direction             = 1;
		floor->sector                = sec;
		switch(type) {
		case build8:
			speed     = FLOORSPEED / 4;
			stairsize = 8 * FRACUNIT;
			break;
		case turbo16:
			speed     = FLOORSPEED * 4;
			stairsize = 16 * FRACUNIT;
			break;
		}
		floor->speed           = speed;
		height                 = sec->floor_height + stairsize;
		floor->floordestheight = height;

		texture = sec->floor;

		// Find next sector to raise
		// 1.	Find 2-sided line with same sector side[0]
		// 2.	Other side is the next sector to raise
		do {
			ok = 0;
			for(i = 0; i < sec->lines_count; i++) {
				if(!((sec->lines[i])->flags & ML_TWOSIDED))
					continue;

				tsec      = (sec->lines[i])->front_sector;
				newsecnum = tsec - p_level.sectors;

				if(secnum != newsecnum)
					continue;

				tsec      = (sec->lines[i])->back_sector;
				newsecnum = tsec - p_level.sectors;

				if(tsec->floor != texture)
					continue;

				height += stairsize;

				if(tsec->special_data)
					continue;

				sec    = tsec;
				secnum = newsecnum;
				floor  = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

				P_AddThinker(&floor->thinker);

				sec->special_data             = floor;
				floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
				floor->direction             = 1;
				floor->sector                = sec;
				floor->speed                 = speed;
				floor->floordestheight       = height;
				ok                           = 1;
				break;
			}
		} while(ok);
	}
	return rtn;
}
