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
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//	Line Tag handling. Line and Sector triggers.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"

#include "i_system.h"
#include "z_zone.h"
#include "m_argv.h"
#include "m_random.h"
#include "w_wad.h"

#include "r_local.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// State.
#include "r_state.h"

// Data.
#include "sounds.h"

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
	boolean istexture;
	int picnum;
	int basepic;
	int numpics;
	int speed;

} anim_t;

//
//      source animation definition
//
typedef struct
{
	boolean istexture; // if false, it is a flat
	char endname[9];
	char startname[9];
	int speed;
} animdef_t;

#define MAXANIMS 32

extern anim_t anims[MAXANIMS];
extern anim_t *lastanim;

//
// P_InitPicAnims
//

// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
animdef_t animdefs[] = {
	{ false, "NUKAGE3", "NUKAGE1", 8 },
	{ false, "FWATER4", "FWATER1", 8 },
	{ false, "SWATER4", "SWATER1", 8 },
	{ false, "LAVA4", "LAVA1", 8 },
	{ false, "BLOOD3", "BLOOD1", 8 },

	// DOOM II flat animations.
	{ false, "RROCK08", "RROCK05", 8 },
	{ false, "SLIME04", "SLIME01", 8 },
	{ false, "SLIME08", "SLIME05", 8 },
	{ false, "SLIME12", "SLIME09", 8 },

	{ true, "BLODGR4", "BLODGR1", 8 },
	{ true, "SLADRIP3", "SLADRIP1", 8 },

	{ true, "BLODRIP4", "BLODRIP1", 8 },
	{ true, "FIREWALL", "FIREWALA", 8 },
	{ true, "GSTFONT3", "GSTFONT1", 8 },
	{ true, "FIRELAVA", "FIRELAV3", 8 },
	{ true, "FIREMAG3", "FIREMAG1", 8 },
	{ true, "FIREBLU2", "FIREBLU1", 8 },
	{ true, "ROCKRED3", "ROCKRED1", 8 },

	{ true, "BFALL4", "BFALL1", 8 },
	{ true, "SFALL4", "SFALL1", 8 },
	{ true, "WFALL4", "WFALL1", 8 },
	{ true, "DBRAIN4", "DBRAIN1", 8 },

	{ -1 }
};

anim_t anims[MAXANIMS];
anim_t *lastanim;

//
//      Animating line specials
//
#define MAXLINEANIMS 64

extern short numlinespecials;
extern const struct p_line *linespeciallist[MAXLINEANIMS];

void
P_InitPicAnims(void) {
	int i;

	//	Init animation
	lastanim = anims;
	for(i = 0; animdefs[i].istexture != -1; i++) {
		if(animdefs[i].istexture) {
			// different episode ?
			if(R_CheckTextureNumForName(animdefs[i].startname) == -1)
				continue;

			lastanim->picnum  = R_TextureIdForName(animdefs[i].endname);
			lastanim->basepic = R_TextureIdForName(animdefs[i].startname);
		} else {
			if(W_FindIdForName(animdefs[i].startname) == -1)
				continue;

			lastanim->picnum  = R_FlatIdForName(animdefs[i].endname);
			lastanim->basepic = R_FlatIdForName(animdefs[i].startname);
		}

		lastanim->istexture = animdefs[i].istexture;
		lastanim->numpics   = lastanim->picnum - lastanim->basepic + 1;

		if(lastanim->numpics < 2)
			I_Error("P_InitPicAnims: bad cycle from %s to %s",
				animdefs[i].startname,
				animdefs[i].endname);

		lastanim->speed = animdefs[i].speed;
		lastanim++;
	}
}

//
// UTILITIES
//

//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
struct p_side *
getSide(int currentSector,
	int line,
	int side) {
	return p_level.sides + (p_level.sectors[currentSector].lines[line])->sides[side];
}

//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
struct p_sector *
getSector(int currentSector,
	int line,
	int side) {
	return p_level.sides[(p_level.sectors[currentSector].lines[line])->sides[side]].sector;
}

//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
int
twoSided(int sector,
	int line) {
	return (p_level.sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
struct p_sector *
getNextSector(const struct p_line *line,
	const struct p_sector *sec) {
	if(!(line->flags & ML_TWOSIDED))
		return NULL;

	if(line->front_sector == sec)
		return line->back_sector;

	return line->front_sector;
}

//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t
P_FindLowestFloorSurrounding(const struct p_sector *sec) {
	int i;
	const struct p_line *check;
	const struct p_sector *other;
	fixed_t floor = sec->floor_height;

	for(i = 0; i < sec->lines_count; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->floor_height < floor)
			floor = other->floor_height;
	}
	return floor;
}

//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t
P_FindHighestFloorSurrounding(const struct p_sector *sec) {
	int i;
	const struct p_line *check;
	const struct p_sector *other;
	fixed_t floor = -500 * FRACUNIT;

	for(i = 0; i < sec->lines_count; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->floor_height > floor)
			floor = other->floor_height;
	}
	return floor;
}

//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// Note: this should be doable w/o a fixed array.

// 20 adjoining sectors max!
#define MAX_ADJOINING_SECTORS 20

fixed_t
P_FindNextHighestFloor(const struct p_sector *sec,
	int currentheight) {
	int i;
	int h;
	int min;
	const struct p_line *check;
	const struct p_sector *other;
	fixed_t height = currentheight;

	fixed_t heightlist[MAX_ADJOINING_SECTORS];

	for(i = 0, h = 0; i < sec->lines_count; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->floor_height > height)
			heightlist[h++] = other->floor_height;

		// Check for overflow. Exit.
		if(h >= MAX_ADJOINING_SECTORS) {
			fprintf(stderr,
				"Sector with more than 20 adjoining sectors\n");
			break;
		}
	}

	// Find lowest height in list
	if(!h)
		return currentheight;

	min = heightlist[0];

	// Range checking?
	for(i = 1; i < h; i++)
		if(heightlist[i] < min)
			min = heightlist[i];

	return min;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindLowestCeilingSurrounding(const struct p_sector *sec) {
	int i;
	const struct p_line *check;
	const struct p_sector *other;
	fixed_t height = MAXINT;

	for(i = 0; i < sec->lines_count; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->ceiling_height < height)
			height = other->ceiling_height;
	}
	return height;
}

//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindHighestCeilingSurrounding(const struct p_sector *sec) {
	int i;
	const struct p_line *check;
	const struct p_sector *other;
	fixed_t height = 0;

	for(i = 0; i < sec->lines_count; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->ceiling_height > height)
			height = other->ceiling_height;
	}
	return height;
}

//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
int
P_FindSectorFromLineTag(const struct p_line *line,
	int start) {
	int i;

	for(i = start + 1; i < p_level.sectors_count; i++) {
		if(p_level.sectors[i].tag == line->sector_tag) {
			return i;
		}
	}

	return -1;
}

//
// Find minimum light from an adjacent sector
//
int
P_FindMinSurroundingLight(const struct p_sector *sector,
	int max) {
	int i;
	int min;
	const struct p_line *line;
	const struct p_sector *check;

	min = max;
	for(i = 0; i < sector->lines_count; i++) {
		line  = sector->lines[i];
		check = getNextSector(line, sector);

		if(!check)
			continue;

		if(check->lighting < min)
			min = check->lighting;
	}
	return min;
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void
P_CrossSpecialLine(int linenum,
	int side,
	mobj_t *thing) {
	struct p_line *line;
	int ok;

	line = p_level.lines + linenum;

	//	Triggers that other things can activate
	if(!thing->player) {
		// Things that should NOT trigger specials...
		switch(thing->type) {
		case MT_ROCKET:
		case MT_PLASMA:
		case MT_BFG:
		case MT_TROOPSHOT:
		case MT_HEADSHOT:
		case MT_BRUISERSHOT:
			return;
			break;

		default:
			break;
		}

		ok = 0;
		switch(line->special_type) {
		case 39:  // TELEPORT TRIGGER
		case 97:  // TELEPORT RETRIGGER
		case 125: // TELEPORT MONSTERONLY TRIGGER
		case 126: // TELEPORT MONSTERONLY RETRIGGER
		case 4:   // RAISE DOOR
		case 10:  // PLAT DOWN-WAIT-UP-STAY TRIGGER
		case 88:  // PLAT DOWN-WAIT-UP-STAY RETRIGGER
			ok = 1;
			break;
		}
		if(!ok)
			return;
	}

	// Note: could use some const's here.
	switch(line->special_type) {
		// TRIGGERS.
		// All from here to RETRIGGERS.
	case 2:
		// Open Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_OPEN);
		line->special_type = 0;
		break;

	case 3:
		// Close Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_CLOSE);
		line->special_type = 0;
		break;

	case 4:
		// Raise Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_NORMAL);
		line->special_type = 0;
		break;

	case 5:
		// Raise Floor
		EV_DoFloor(line, raiseFloor);
		line->special_type = 0;
		break;

	case 6:
		// Fast Ceiling Crush & Raise
		EV_DoCeiling(line, fastCrushAndRaise);
		line->special_type = 0;
		break;

	case 8:
		// Build Stairs
		EV_BuildStairs(line, build8);
		line->special_type = 0;
		break;

	case 10:
		// PlatDownWaitUp
		EV_DoPlat(line, downWaitUpStay, 0);
		line->special_type = 0;
		break;

	case 12:
		// Light Turn On - brightest near
		EV_LightTurnOn(line, 0);
		line->special_type = 0;
		break;

	case 13:
		// Light Turn On 255
		EV_LightTurnOn(line, 255);
		line->special_type = 0;
		break;

	case 16:
		// Close Door 30
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_CLOSE_30_THEN_OPEN);
		line->special_type = 0;
		break;

	case 17:
		// Start Light Strobing
		EV_StartLightStrobing(line);
		line->special_type = 0;
		break;

	case 19:
		// Lower Floor
		EV_DoFloor(line, lowerFloor);
		line->special_type = 0;
		break;

	case 22:
		// Raise floor to nearest height and change texture
		EV_DoPlat(line, raiseToNearestAndChange, 0);
		line->special_type = 0;
		break;

	case 25:
		// Ceiling Crush and Raise
		EV_DoCeiling(line, crushAndRaise);
		line->special_type = 0;
		break;

	case 30:
		// Raise floor to shortest texture height
		//  on either side of lines.
		EV_DoFloor(line, raiseToTexture);
		line->special_type = 0;
		break;

	case 35:
		// Lights Very Dark
		EV_LightTurnOn(line, 35);
		line->special_type = 0;
		break;

	case 36:
		// Lower Floor (TURBO)
		EV_DoFloor(line, turboLower);
		line->special_type = 0;
		break;

	case 37:
		// LowerAndChange
		EV_DoFloor(line, lowerAndChange);
		line->special_type = 0;
		break;

	case 38:
		// Lower Floor To Lowest
		EV_DoFloor(line, lowerFloorToLowest);
		line->special_type = 0;
		break;

	case 39:
		// TELEPORT!
		EV_Teleport(line, side, thing);
		line->special_type = 0;
		break;

	case 40:
		// RaiseCeilingLowerFloor
		EV_DoCeiling(line, raiseToHighest);
		EV_DoFloor(line, lowerFloorToLowest);
		line->special_type = 0;
		break;

	case 44:
		// Ceiling Crush
		EV_DoCeiling(line, lowerAndCrush);
		line->special_type = 0;
		break;

	case 52:
		// EXIT!
		G_ExitLevel();
		break;

	case 53:
		// Perpetual Platform Raise
		EV_DoPlat(line, perpetualRaise, 0);
		line->special_type = 0;
		break;

	case 54:
		// Platform Stop
		EV_StopPlat(line);
		line->special_type = 0;
		break;

	case 56:
		// Raise Floor Crush
		EV_DoFloor(line, raiseFloorCrush);
		line->special_type = 0;
		break;

	case 57:
		// Ceiling Crush Stop
		EV_CeilingCrushStop(line);
		line->special_type = 0;
		break;

	case 58:
		// Raise Floor 24
		EV_DoFloor(line, raiseFloor24);
		line->special_type = 0;
		break;

	case 59:
		// Raise Floor 24 And Change
		EV_DoFloor(line, raiseFloor24AndChange);
		line->special_type = 0;
		break;

	case 104:
		// Turn lights off in sector(tag)
		EV_TurnTagLightsOff(line);
		line->special_type = 0;
		break;

	case 108:
		// Blazing Door Raise (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_RAISE);
		line->special_type = 0;
		break;

	case 109:
		// Blazing Door Open (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_OPEN);
		line->special_type = 0;
		break;

	case 100:
		// Build Stairs Turbo 16
		EV_BuildStairs(line, turbo16);
		line->special_type = 0;
		break;

	case 110:
		// Blazing Door Close (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_CLOSE);
		line->special_type = 0;
		break;

	case 119:
		// Raise floor to nearest surr. floor
		EV_DoFloor(line, raiseFloorToNearest);
		line->special_type = 0;
		break;

	case 121:
		// Blazing PlatDownWaitUpStay
		EV_DoPlat(line, blazeDWUS, 0);
		line->special_type = 0;
		break;

	case 124:
		// Secret EXIT
		G_SecretExitLevel();
		break;

	case 125:
		// TELEPORT MonsterONLY
		if(!thing->player) {
			EV_Teleport(line, side, thing);
			line->special_type = 0;
		}
		break;

	case 130:
		// Raise Floor Turbo
		EV_DoFloor(line, raiseFloorTurbo);
		line->special_type = 0;
		break;

	case 141:
		// Silent Ceiling Crush & Raise
		EV_DoCeiling(line, silentCrushAndRaise);
		line->special_type = 0;
		break;

		// RETRIGGERS.  All from here till end.
	case 72:
		// Ceiling Crush
		EV_DoCeiling(line, lowerAndCrush);
		break;

	case 73:
		// Ceiling Crush and Raise
		EV_DoCeiling(line, crushAndRaise);
		break;

	case 74:
		// Ceiling Crush Stop
		EV_CeilingCrushStop(line);
		break;

	case 75:
		// Close Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_CLOSE);
		break;

	case 76:
		// Close Door 30
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_CLOSE_30_THEN_OPEN);
		break;

	case 77:
		// Fast Ceiling Crush & Raise
		EV_DoCeiling(line, fastCrushAndRaise);
		break;

	case 79:
		// Lights Very Dark
		EV_LightTurnOn(line, 35);
		break;

	case 80:
		// Light Turn On - brightest near
		EV_LightTurnOn(line, 0);
		break;

	case 81:
		// Light Turn On 255
		EV_LightTurnOn(line, 255);
		break;

	case 82:
		// Lower Floor To Lowest
		EV_DoFloor(line, lowerFloorToLowest);
		break;

	case 83:
		// Lower Floor
		EV_DoFloor(line, lowerFloor);
		break;

	case 84:
		// LowerAndChange
		EV_DoFloor(line, lowerAndChange);
		break;

	case 86:
		// Open Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_OPEN);
		break;

	case 87:
		// Perpetual Platform Raise
		EV_DoPlat(line, perpetualRaise, 0);
		break;

	case 88:
		// PlatDownWaitUp
		EV_DoPlat(line, downWaitUpStay, 0);
		break;

	case 89:
		// Platform Stop
		EV_StopPlat(line);
		break;

	case 90:
		// Raise Door
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_NORMAL);
		break;

	case 91:
		// Raise Floor
		EV_DoFloor(line, raiseFloor);
		break;

	case 92:
		// Raise Floor 24
		EV_DoFloor(line, raiseFloor24);
		break;

	case 93:
		// Raise Floor 24 And Change
		EV_DoFloor(line, raiseFloor24AndChange);
		break;

	case 94:
		// Raise Floor Crush
		EV_DoFloor(line, raiseFloorCrush);
		break;

	case 95:
		// Raise floor to nearest height
		// and change texture.
		EV_DoPlat(line, raiseToNearestAndChange, 0);
		break;

	case 96:
		// Raise floor to shortest texture height
		// on either side of lines.
		EV_DoFloor(line, raiseToTexture);
		break;

	case 97:
		// TELEPORT!
		EV_Teleport(line, side, thing);
		break;

	case 98:
		// Lower Floor (TURBO)
		EV_DoFloor(line, turboLower);
		break;

	case 105:
		// Blazing Door Raise (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_RAISE);
		break;

	case 106:
		// Blazing Door Open (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_OPEN);
		break;

	case 107:
		// Blazing Door Close (faster than TURBO!)
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_BLAZE_CLOSE);
		break;

	case 120:
		// Blazing PlatDownWaitUpStay.
		EV_DoPlat(line, blazeDWUS, 0);
		break;

	case 126:
		// TELEPORT MonsterONLY.
		if(!thing->player)
			EV_Teleport(line, side, thing);
		break;

	case 128:
		// Raise To Nearest Floor
		EV_DoFloor(line, raiseFloorToNearest);
		break;

	case 129:
		// Raise Floor Turbo
		EV_DoFloor(line, raiseFloorTurbo);
		break;
	}
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void
P_ShootSpecialLine(mobj_t *thing,
	struct p_line *line) {
	int ok;

	//	Impacts that other things can activate.
	if(!thing->player) {
		ok = 0;
		switch(line->special_type) {
		case 46:
			// OPEN DOOR IMPACT
			ok = 1;
			break;
		}
		if(!ok)
			return;
	}

	switch(line->special_type) {
	case 24:
		// RAISE FLOOR
		EV_DoFloor(line, raiseFloor);
		P_ChangeSwitchTexture(line, 0);
		break;

	case 46:
		// OPEN DOOR
		EV_DoDoor(line, VERTICAL_DOOR_TYPE_OPEN);
		P_ChangeSwitchTexture(line, 1);
		break;

	case 47:
		// RAISE FLOOR NEAR AND CHANGE
		EV_DoPlat(line, raiseToNearestAndChange, 0);
		P_ChangeSwitchTexture(line, 0);
		break;
	}
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void
P_PlayerInSpecialSector(player_t *player) {
	struct p_sector *sector;

	sector = player->mo->subsector->sector;

	// Falling, not all the way down yet?
	if(player->mo->z != sector->floor_height)
		return;

	// Has hitten ground.
	switch(sector->special_type) {
	case 5:
		// HELLSLIME DAMAGE
		if(!player->powers[pw_ironfeet])
			if(!(leveltime & 0x1f))
				P_DamageMobj(player->mo, NULL, NULL, 10);
		break;

	case 7:
		// NUKAGE DAMAGE
		if(!player->powers[pw_ironfeet])
			if(!(leveltime & 0x1f))
				P_DamageMobj(player->mo, NULL, NULL, 5);
		break;

	case 16:
		// SUPER HELLSLIME DAMAGE
	case 4:
		// STROBE HURT
		if(!player->powers[pw_ironfeet]
			|| (P_Random() < 5)) {
			if(!(leveltime & 0x1f))
				P_DamageMobj(player->mo, NULL, NULL, 20);
		}
		break;

	case 9:
		// SECRET SECTOR
		player->secretcount++;
		sector->special_type = 0;
		break;

	case 11:
		// EXIT SUPER DAMAGE! (for E1M8 finale)
		player->cheats &= ~CF_GODMODE;

		if(!(leveltime & 0x1f))
			P_DamageMobj(player->mo, NULL, NULL, 20);

		if(player->health <= 10)
			G_ExitLevel();
		break;

	default:
		I_Error("P_PlayerInSpecialSector: "
				"unknown special %i",
			sector->special_type);
		break;
	};
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
boolean levelTimer;
int levelTimeCount;

void
P_UpdateSpecials(void) {
	anim_t *anim;
	int pic;
	int i;
	const struct p_line *line;

	//	LEVEL TIMER
	if(levelTimer == true) {
		levelTimeCount--;
		if(!levelTimeCount)
			G_ExitLevel();
	}

	//	ANIMATE FLATS AND TEXTURES GLOBALLY
	for(anim = anims; anim < lastanim; anim++) {
		for(i = anim->basepic; i < anim->basepic + anim->numpics; i++) {
			pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);
			if(anim->istexture)
				texturetranslation[i] = pic;
			else
				flattranslation[i] = pic;
		}
	}

	//	ANIMATE LINE SPECIALS
	for(i = 0; i < numlinespecials; i++) {
		line = linespeciallist[i];
		switch(line->special_type) {
		case 48:
			// EFFECT FIRSTCOL SCROLL +
			p_level.sides[line->sides[0]].offset_x += FRACUNIT;
			break;
		}
	}

	//	DO BUTTONS
	for(i = 0; i < MAXBUTTONS; i++)
		if(buttonlist[i].btimer) {
			buttonlist[i].btimer--;
			if(!buttonlist[i].btimer) {
				switch(buttonlist[i].where) {
				case top:
					p_level.sides[buttonlist[i].line->sides[0]].top_texture = buttonlist[i].btexture;
					break;
				case middle:
					p_level.sides[buttonlist[i].line->sides[0]].middle_texture = buttonlist[i].btexture;
					break;
				case bottom:
					p_level.sides[buttonlist[i].line->sides[0]].bottom_texture = buttonlist[i].btexture;
					break;
				}
				S_StartSound((mobj_t *)&buttonlist[i].soundorg, sfx_swtchn);
				memset(&buttonlist[i], 0, sizeof(button_t));
			}
		}
}

//
// Special Stuff that can not be categorized
//
int
EV_DoDonut(const struct p_line *line) {
	struct p_sector *s1;
	struct p_sector *s2;
	const struct p_sector *s3;
	int secnum;
	int rtn;
	int i;
	floormove_t *floor;

	secnum = -1;
	rtn    = 0;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
		s1 = p_level.sectors + secnum;

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if(s1->special_data)
			continue;

		rtn = 1;
		s2  = getNextSector(s1->lines[0], s1);
		for(i = 0; i < s2->lines_count; i++) {
			if(((!s2->lines[i]->flags) & ML_TWOSIDED) || (s2->lines[i]->back_sector == s1))
				continue;
			s3 = s2->lines[i]->back_sector;

			//	Spawn rising slime
			floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker(&floor->thinker);
			s2->special_data             = floor;
			floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
			floor->type                  = donutRaise;
			floor->crush                 = false;
			floor->direction             = 1;
			floor->sector                = s2;
			floor->speed                 = FLOORSPEED / 2;
			floor->texture               = s3->floor;
			floor->newspecial            = 0;
			floor->floordestheight       = s3->floor_height;

			//	Spawn lowering donut-hole
			floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker(&floor->thinker);
			s1->special_data              = floor;
			floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
			floor->type                  = lowerFloor;
			floor->crush                 = false;
			floor->direction             = -1;
			floor->sector                = s1;
			floor->speed                 = FLOORSPEED / 2;
			floor->floordestheight       = s3->floor_height;
			break;
		}
	}
	return rtn;
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//
short numlinespecials;
const struct p_line *linespeciallist[MAXLINEANIMS];

// Parses command line parameters.
void
P_SpawnSpecials(void) {
	struct p_sector *sector;
	int i;
	int episode;

	episode = 1;
	if(W_FindIdForName("texture2") >= 0)
		episode = 2;

	// See if -TIMER needs to be used.
	levelTimer = false;

	i = M_CheckParm("-avg");
	if(i && deathmatch) {
		levelTimer     = true;
		levelTimeCount = 20 * 60 * 35;
	}

	i = M_CheckParm("-timer");
	if(i && deathmatch) {
		int time;
		time           = atoi(myargv[i + 1]) * 60 * 35;
		levelTimer     = true;
		levelTimeCount = time;
	}

	//	Init special SECTORs.
	sector = p_level.sectors;
	for(i = 0; i < p_level.sectors_count; i++, sector++) {
		if(!sector->special_type)
			continue;

		switch(sector->special_type) {
		case 1:
			// FLICKERING LIGHTS
			P_SpawnLightFlash(sector);
			break;

		case 2:
			// STROBE FAST
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			break;

		case 3:
			// STROBE SLOW
			P_SpawnStrobeFlash(sector, SLOWDARK, 0);
			break;

		case 4:
			// STROBE FAST/DEATH SLIME
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			sector->special_type = 4;
			break;

		case 8:
			// GLOWING LIGHT
			P_SpawnGlowingLight(sector);
			break;
		case 9:
			// SECRET SECTOR
			totalsecret++;
			break;

		case 10:
			// DOOR CLOSE IN 30 SECONDS
			P_SpawnDoorCloseIn30(sector);
			break;

		case 12:
			// SYNC STROBE SLOW
			P_SpawnStrobeFlash(sector, SLOWDARK, 1);
			break;

		case 13:
			// SYNC STROBE FAST
			P_SpawnStrobeFlash(sector, FASTDARK, 1);
			break;

		case 14:
			// DOOR RAISE IN 5 MINUTES
			P_SpawnDoorRaiseIn5Mins(sector, i);
			break;

		case 17:
			P_SpawnFireFlicker(sector);
			break;
		}
	}

	//	Init line EFFECTs
	numlinespecials = 0;
	for(i = 0; i < p_level.lines_count; i++) {
		switch(p_level.lines[i].special_type) {
		case 48:
			// EFFECT FIRSTCOL SCROLL+
			linespeciallist[numlinespecials] = p_level.lines + i;
			numlinespecials++;
			break;
		}
	}

	//	Init other misc stuff
	for(i = 0; i < MAXCEILINGS; i++)
		activeceilings[i] = NULL;

	for(i = 0; i < MAXPLATS; i++)
		activeplats[i] = NULL;

	for(i = 0; i < MAXBUTTONS; i++)
		memset(&buttonlist[i], 0, sizeof(button_t));

	// UNUSED: no horizonal sliders.
	//	P_InitSlidingDoorFrames();
}
