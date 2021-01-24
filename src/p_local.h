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
//	Play functions, animation, global header.
//
//-----------------------------------------------------------------------------

#ifndef __P_LOCAL__
#define __P_LOCAL__

#include "d_think.h"
#include "d_player.h"
#include "r_defs.h"
#include "m_fixed.h"

#include <stdint.h>

#define FLOATSPEED (FRACUNIT * 4)

#define MAXHEALTH 100
#define VIEWHEIGHT (41 * FRACUNIT)

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS 128
#define MAPBLOCKSIZE (MAPBLOCKUNITS * FRACUNIT)
#define MAPBLOCKSHIFT (FRACBITS + 7)
#define MAPBMASK (MAPBLOCKSIZE - 1)
#define MAPBTOFRAC (MAPBLOCKSHIFT - FRACBITS)

// player radius for movement checking
#define PLAYERRADIUS 16 * FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS 32 * FRACUNIT

#define GRAVITY FRACUNIT
#define MAXMOVE (30 * FRACUNIT)

#define USERANGE (64 * FRACUNIT)
#define MELEERANGE (64 * FRACUNIT)
#define MISSILERANGE (32 * 64 * FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD 100

/**************************
 * Structures definitions *
 **************************/

/* Your plain vanilla vertex.
 * Note: transformed values not buffered locally,
 *  like some DOOM-alikes ("wt", "WebView") did.
 */
struct p_vertex {
	fixed_t x;
	fixed_t y;
};

struct p_segment {
	const struct p_vertex *first_vertex;
	const struct p_vertex *last_vertex;

	fixed_t offset;

	angle_t angle;

	const struct p_side *side;
	struct p_line *line;

	/* Sector references.
	 * Could be retrieved from linedef, too.
	 * backsector is NULL for one sided lines
	 */
	const struct p_sector *front_sector;
	const struct p_sector *back_sector;
};

/* The SECTORS record, at runtime.
 * Stores things/mobjs.
 */
struct p_sector {
	fixed_t floor_height;
	fixed_t ceiling_height;

	short floor;
	short ceiling;

	short lighting;
	short special_type;
	short tag;

	/* 0 = untraversed, 1,2 = sndlines -1 */
	int sound_traversed;

	/* thing that made a sound (or null) */
	mobj_t *sound_target;

	/* mapblock bounding box for height changes */
	int blockbox[4];

	/* Each sector has a degenmobj_t in its center
	 *  for sound origin purposes.
	 * I suppose this does not handle sound from
	 *  moving objects (doppler), because
	 *  position is prolly just buffered, not
	 *  updated.
	 */
	/* origin for any sounds played by the sector */
	struct {
		thinker_t thinker; /* not used for anything */
		fixed_t x;
		fixed_t y;
		fixed_t z;
	} sound_origin;

	/* if == validcount, already checked */
	int valid_count;

	/* list of mobjs in sector */
	mobj_t *thing_list;

	/* thinker_t for reversable actions */
	void *special_data;

	size_t lines_count;
	struct p_line **lines;
};

/* A SubSector.
 * References a Sector.
 * Basically, this is a list of LineSegs,
 *  indicating the visible walls that define
 *  (all or some) sides of a convex BSP leaf.
 */
struct p_subSector {
	struct p_sector *sector;
	short lines_count;
	short first_line;
};

/* BSP node. */
struct p_node {
	/* Partition line. */
	fixed_t x;
	fixed_t y;
	fixed_t dx;
	fixed_t dy;

	/* Bounding box for each child. */
	fixed_t bounding_boxes[2][4];

	/* If NF_SUBSECTOR its a subsector. */
	unsigned short children[2];
};

/* The LineSeg. */
struct p_line {
	/* Vertices, from v1 to v2. */
	const struct p_vertex *first_vertex;
	const struct p_vertex *last_vertex;

	/* Precalculated v2 - v1 for side checking. */
	fixed_t dx;
	fixed_t dy;

	/* Animation related. */
	short flags;
	short special_type;
	short sector_tag;

	/* Visual appearance: SideDefs.
	 *  sidenum[1] will be -1 if one sided
	 */
	short sides[2];

	/* Neat. Another bounding box, for the extent
	 *  of the LineDef.
	 */
	fixed_t bounding_boxes[4];

	/* To aid move clipping. */
	enum p_slopeType {
		SLOPE_TYPE_HORIZONTAL,
		SLOPE_TYPE_VERTICAL,
		SLOPE_TYPE_POSITIVE,
		SLOPE_TYPE_NEGATIVE,
	} slope_type;

	/* Front and back sector.
	 * Note: redundant? Can be retrieved from SideDefs.
	 */
	struct p_sector *front_sector;
	struct p_sector *back_sector;

	/* if == validcount, already checked */
	int valid_count;

	/* thinker_t for reversable actions */
	void *specialdata;
};

/* The SideDef */
struct p_side {
	/* Add this to the calculated texture column. */
	fixed_t offset_x;

	/* Add this to the calculated texture top. */
	fixed_t offset_y;

	/* Texture indices.
	 * We do not maintain names here.
	 */
	short top_texture;
	short bottom_texture;
	short middle_texture;

	/* Sector the SideDef is facing. */
	struct p_sector *sector;
};

/**************************************
 * Functions and locally defined data *
 **************************************/

//
// P_TICK
//

// both the head and tail of the thinker list
extern thinker_t thinkercap;

void
P_InitThinkers(void);
void
P_AddThinker(thinker_t *thinker);
void
P_RemoveThinker(thinker_t *thinker);

//
// P_PSPR
//
void
P_SetupPsprites(player_t *curplayer);
void
P_MovePsprites(player_t *curplayer);
void
P_DropWeapon(player_t *player);

//
// P_USER
//
void
P_PlayerThink(player_t *player);

//
// P_MOBJ
//
#define ONFLOORZ MININT
#define ONCEILINGZ MAXINT

// Time interval for item respawning.
#define ITEMQUESIZE 128

extern mapthing_t itemrespawnque[ITEMQUESIZE];
extern int itemrespawntime[ITEMQUESIZE];
extern int iquehead;
extern int iquetail;

void
P_RespawnSpecials(void);

mobj_t *
P_SpawnMobj(fixed_t x,
	fixed_t y,
	fixed_t z,
	mobjtype_t type);

void
P_RemoveMobj(mobj_t *th);
boolean
P_SetMobjState(mobj_t *mobj, statenum_t state);
void
P_MobjThinker(mobj_t *mobj);

void
P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void
P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage);
mobj_t *
P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
void
P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);

//
// P_ENEMY
//
void
P_NoiseAlert(mobj_t *target, mobj_t *emmiter);

//
// P_MAPUTL
//
typedef struct
{
	fixed_t x;
	fixed_t y;
	fixed_t dx;
	fixed_t dy;

} divline_t;

typedef struct
{
	fixed_t frac; // along trace line
	boolean isaline;
	union {
		mobj_t *thing;
		struct p_line *line;
	} d;
} intercept_t;

#define MAXINTERCEPTS 128

extern intercept_t intercepts[MAXINTERCEPTS];
extern intercept_t *intercept_p;

typedef boolean (*traverser_t)(intercept_t *in);

fixed_t
P_AproxDistance(fixed_t dx, fixed_t dy);
int
P_PointOnLineSide(fixed_t x, fixed_t y, const struct p_line *line);
int
P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t *line);
void
P_MakeDivline(const struct p_line *li, divline_t *dl);
fixed_t
P_InterceptVector(divline_t *v2, divline_t *v1);
int
P_BoxOnLineSide(fixed_t *tmbox, const struct p_line *ld);

extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;

void
P_LineOpening(const struct p_line *linedef);

boolean
P_BlockLinesIterator(int x, int y, boolean (*func)(struct p_line *));
boolean
P_BlockThingsIterator(int x, int y, boolean (*func)(mobj_t *));

#define PT_ADDLINES 1
#define PT_ADDTHINGS 2
#define PT_EARLYOUT 4

extern divline_t trace;

boolean
P_PathTraverse(fixed_t x1,
	fixed_t y1,
	fixed_t x2,
	fixed_t y2,
	int flags,
	boolean (*trav)(intercept_t *));

void
P_UnsetThingPosition(mobj_t *thing);
void
P_SetThingPosition(mobj_t *thing);

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;

extern const struct p_line *ceilingline;

boolean
P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
boolean
P_TryMove(mobj_t *thing, fixed_t x, fixed_t y);
boolean
P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y);
void
P_SlideMove(mobj_t *mo);
boolean
P_CheckSight(mobj_t *t1, mobj_t *t2);
void
P_UseLines(player_t *player);

boolean
P_ChangeSector(const struct p_sector *sector, boolean crunch);

extern mobj_t *linetarget; // who got hit (or NULL)

fixed_t
P_AimLineAttack(mobj_t *t1,
	angle_t angle,
	fixed_t distance);

void
P_LineAttack(mobj_t *t1,
	angle_t angle,
	fixed_t distance,
	fixed_t slope,
	int damage);

void
P_RadiusAttack(mobj_t *spot,
	mobj_t *source,
	int damage);

/***********
 * P_SETUP *
 ***********/

extern struct p_level {
	/*
	 * MAP related Lookup tables.
	 * Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
	 */
	size_t vertices_count;
	struct p_vertex *vertices;

	size_t segments_count;
	struct p_segment *segments;

	size_t sectors_count;
	struct p_sector *sectors;

	size_t sub_sectors_count;
	struct p_subSector *sub_sectors;

	size_t nodes_count;
	struct p_node *nodes;

	size_t lines_count;
	struct p_line *lines;

	size_t sides_count;
	struct p_side *sides;

	/* BLOCKMAP
	 * Created from axis aligned bounding box
	 * of the map, a rectangular array of
	 * blocks of size ...
	 * Used to speed up collision detection
	 * by spatial subdivision in 2D.
	 *
	 * Blockmap size.
	 */
	unsigned blockmap_width;
	unsigned blockmap_height; /* Size in map blocks */
	fixed_t blockmap_origin_x;
	fixed_t blockmap_origin_y; /* Origin of block map */
	const short *blockmap_lump; /* Offsets in blockmap are from here */
	mobj_t **block_links;

	/* REJECT
	 * For fast sight rejection.
	 * Speeds up enemy AI by skipping detailed
	 *  LineOf Sight calculation.
	 * Without special effect, this could be
	 *  used as a PVS lookup as well.
	 */
	const uint8_t *reject_matrix;
} p_level;

//
// P_INTER
//
extern int maxammo[NUMAMMO];
extern int clipammo[NUMAMMO];

void
P_TouchSpecialThing(mobj_t *special,
	mobj_t *toucher);

void
P_DamageMobj(mobj_t *target,
	mobj_t *inflictor,
	mobj_t *source,
	int damage);

//
// P_SPEC
//
#include "p_spec.h"

#endif // __P_LOCAL__
