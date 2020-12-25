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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

#include <math.h>

#include "z_zone.h"

#include "m_swap.h"
#include "m_bbox.h"

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "doomstat.h"

void
P_SpawnMapThing(const mapthing_t *mthing);

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int numsides;
side_t *sides;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int bmapwidth;
int bmapheight;  // size in mapblocks
// offsets in blockmap are from here
const short *blockmaplump;
// origin of block map
fixed_t bmaporgx;
fixed_t bmaporgy;
// for thing chains
mobj_t **blocklinks;

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
const byte *rejectmatrix;

// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS 10

mapthing_t deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];

//
// P_LoadVertexes
//
static void
P_LoadVertexes(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const mapvertex_t *ml;
	vertex_t *li;

	// Determine number of lumps:
	//  total lump length / vertex record length.
	numvertexes = lump->size / sizeof(mapvertex_t);

	// Allocate zone memory for buffer.
	vertexes = Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);

	ml = (const mapvertex_t *)lump->data;
	li = vertexes;

	// Copy and convert vertex coordinates,
	// internal representation as fixed.
	for(i = 0; i < numvertexes; i++, li++, ml++) {
		li->x = SHORT(ml->x) << FRACBITS;
		li->y = SHORT(ml->y) << FRACBITS;
	}
}

//
// P_LoadSegs
//
static void
P_LoadSegs(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const mapseg_t *ml;
	seg_t *li;
	line_t *ldef;
	int linedef;
	int side;

	numsegs = lump->size / sizeof(mapseg_t);
	segs    = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
	memset(segs, 0, numsegs * sizeof(seg_t));

	ml = (const mapseg_t *)lump->data;
	li = segs;
	for(i = 0; i < numsegs; i++, li++, ml++) {
		li->v1 = &vertexes[SHORT(ml->v1)];
		li->v2 = &vertexes[SHORT(ml->v2)];

		li->angle       = (SHORT(ml->angle)) << 16;
		li->offset      = (SHORT(ml->offset)) << 16;
		linedef         = SHORT(ml->linedef);
		ldef            = &lines[linedef];
		li->linedef     = ldef;
		side            = SHORT(ml->side);
		li->sidedef     = &sides[ldef->sidenum[side]];
		li->frontsector = sides[ldef->sidenum[side]].sector;
		if(ldef->flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
		else
			li->backsector = 0;
	}
}

//
// P_LoadSubsectors
//
static void
P_LoadSubsectors(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const mapsubsector_t *ms;
	subsector_t *ss;

	numsubsectors = lump->size / sizeof(mapsubsector_t);
	subsectors    = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);

	ms = (const mapsubsector_t *)lump->data;
	memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
	ss = subsectors;

	for(i = 0; i < numsubsectors; i++, ss++, ms++) {
		ss->numlines  = SHORT(ms->numsegs);
		ss->firstline = SHORT(ms->firstseg);
	}
}

//
// P_LoadSectors
//
static void
P_LoadSectors(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const mapsector_t *ms;
	sector_t *ss;

	numsectors = lump->size / sizeof(mapsector_t);
	sectors    = Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, 0);
	memset(sectors, 0, numsectors * sizeof(sector_t));

	ms = (const mapsector_t *)lump->data;
	ss = sectors;
	for(i = 0; i < numsectors; i++, ss++, ms++) {
		ss->floorheight   = SHORT(ms->floorheight) << FRACBITS;
		ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
		ss->floorpic      = R_FlatNumForName(ms->floorpic);
		ss->ceilingpic    = R_FlatNumForName(ms->ceilingpic);
		ss->lightlevel    = SHORT(ms->lightlevel);
		ss->special       = SHORT(ms->special);
		ss->tag           = SHORT(ms->tag);
		ss->thinglist     = NULL;
	}
}

//
// P_LoadNodes
//
static void
P_LoadNodes(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	int j;
	int k;
	const mapnode_t *mn;
	node_t *no;

	numnodes = lump->size / sizeof(mapnode_t);
	nodes    = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);

	mn = (const mapnode_t *)lump->data;
	no = nodes;

	for(i = 0; i < numnodes; i++, no++, mn++) {
		no->x  = SHORT(mn->x) << FRACBITS;
		no->y  = SHORT(mn->y) << FRACBITS;
		no->dx = SHORT(mn->dx) << FRACBITS;
		no->dy = SHORT(mn->dy) << FRACBITS;
		for(j = 0; j < 2; j++) {
			no->children[j] = SHORT(mn->children[j]);
			for(k = 0; k < 4; k++)
				no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
		}
	}
}

//
// P_LoadThings
//
static void
P_LoadThings(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	const mapthing_t *mt = lump->data;
	const mapthing_t *mtend = mt + lump->size / sizeof(*mt);

	while(mt != mtend) {
		boolean spawn = true;

		// Do not spawn cool, new monsters if !commercial
		if(gamemode != commercial) {
			switch(mt->type) {
			case 68: // Arachnotron
			case 64: // Archvile
			case 88: // Boss Brain
			case 89: // Boss Shooter
			case 69: // Hell Knight
			case 67: // Mancubus
			case 71: // Pain Elemental
			case 65: // Former Human Commando
			case 66: // Revenant
			case 84: // Wolf SS
				spawn = false;
				break;
			}
		}
		if(spawn == false)
			break;

		// Do spawn all other stuff.
		const mapthing_t localmt = {
			.x       = SHORT(mt->x),
			.y       = SHORT(mt->y),
			.angle   = SHORT(mt->angle),
			.type    = SHORT(mt->type),
			.options = SHORT(mt->options),
		};

		P_SpawnMapThing(&localmt);

		mt++;
	}
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
static void
P_LoadLineDefs(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const maplinedef_t *mld;
	line_t *ld;
	vertex_t *v1;
	vertex_t *v2;

	numlines = lump->size / sizeof(maplinedef_t);
	lines    = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines * sizeof(line_t));

	mld = (const maplinedef_t *)lump->data;
	ld  = lines;
	for(i = 0; i < numlines; i++, mld++, ld++) {
		ld->flags   = SHORT(mld->flags);
		ld->special = SHORT(mld->special);
		ld->tag     = SHORT(mld->tag);
		v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
		ld->dx      = v2->x - v1->x;
		ld->dy      = v2->y - v1->y;

		if(!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if(!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else {
			if(FixedDiv(ld->dy, ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if(v1->x < v2->x) {
			ld->bbox[BOXLEFT]  = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		} else {
			ld->bbox[BOXLEFT]  = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}

		if(v1->y < v2->y) {
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP]    = v2->y;
		} else {
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP]    = v1->y;
		}

		ld->sidenum[0] = SHORT(mld->sidenum[0]);
		ld->sidenum[1] = SHORT(mld->sidenum[1]);

		if(ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;

		if(ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;
	}
}

//
// P_LoadSideDefs
//
static void
P_LoadSideDefs(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int i;
	const mapsidedef_t *msd;
	side_t *sd;

	numsides = lump->size / sizeof(mapsidedef_t);
	sides    = Z_Malloc(numsides * sizeof(side_t), PU_LEVEL, 0);
	memset(sides, 0, numsides * sizeof(side_t));

	msd = (const mapsidedef_t *)lump->data;
	sd  = sides;
	for(i = 0; i < numsides; i++, msd++, sd++) {
		sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
		sd->rowoffset     = SHORT(msd->rowoffset) << FRACBITS;
		sd->toptexture    = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		sd->midtexture    = R_TextureNumForName(msd->midtexture);
		sd->sector        = &sectors[SHORT(msd->sector)];
	}
}

//
// P_LoadBlockMap
//
static void
P_LoadBlockMap(lumpId_t id) {
	const struct w_lump *lump = W_LumpForId(id);
	int count;

	blockmaplump = lump->data;
	count        = lump->size / 2;

	bmaporgx   = SHORT(blockmaplump[0]) << FRACBITS;
	bmaporgy   = SHORT(blockmaplump[1]) << FRACBITS;
	bmapwidth  = SHORT(blockmaplump[2]);
	bmapheight = SHORT(blockmaplump[3]);

	// clear out mobj chains
	count      = sizeof(*blocklinks) * bmapwidth * bmapheight;
	blocklinks = Z_Malloc(count, PU_LEVEL, 0);
	memset(blocklinks, 0, count);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void
P_GroupLines(void) {
	line_t **linebuffer;
	int i;
	int j;
	int total;
	line_t *li;
	sector_t *sector;
	subsector_t *ss;
	seg_t *seg;
	fixed_t bbox[4];
	int block;

	// look up sector number for each subsector
	ss = subsectors;
	for(i = 0; i < numsubsectors; i++, ss++) {
		seg        = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
	}

	// count number of lines in each sector
	li    = lines;
	total = 0;
	for(i = 0; i < numlines; i++, li++) {
		total++;
		li->frontsector->linecount++;

		if(li->backsector && li->backsector != li->frontsector) {
			li->backsector->linecount++;
			total++;
		}
	}

	// build line tables for each sector
	linebuffer = Z_Malloc(total * sizeof(*linebuffer), PU_LEVEL, 0);
	sector     = sectors;
	for(i = 0; i < numsectors; i++, sector++) {
		M_ClearBox(bbox);
		sector->lines = linebuffer;
		li            = lines;
		for(j = 0; j < numlines; j++, li++) {
			if(li->frontsector == sector || li->backsector == sector) {
				*linebuffer++ = li;
				M_AddToBox(bbox, li->v1->x, li->v1->y);
				M_AddToBox(bbox, li->v2->x, li->v2->y);
			}
		}
		if(linebuffer - sector->lines != sector->linecount)
			I_Error("P_GroupLines: miscounted");

		// set the degenmobj_t to the middle of the bounding box
		sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
		sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

		// adjust bounding box to map blocks
		block                    = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
		block                    = block >= bmapheight ? bmapheight - 1 : block;
		sector->blockbox[BOXTOP] = block;

		block                       = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
		block                       = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM] = block;

		block                      = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
		block                      = block >= bmapwidth ? bmapwidth - 1 : block;
		sector->blockbox[BOXRIGHT] = block;

		block                     = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
		block                     = block < 0 ? 0 : block;
		sector->blockbox[BOXLEFT] = block;
	}
}

//
// P_SetupLevel
//
void
P_SetupLevel(int episode,
	int map,
	int playermask,
	skill_t skill) {
	int i;
	char lumpname[9];
	int lumpnum;

	totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
	wminfo.partime                                          = 180;
	for(i = 0; i < MAXPLAYERS; i++) {
		players[i].killcount       = players[i].secretcount
			= players[i].itemcount = 0;
	}

	// Initial height of PointOfView
	// will be set by player think.
	players[consoleplayer].viewz = 1;

	// Make sure all sounds are stopped before Z_FreeTags.
	S_Start();

#if 0 // UNUSED
    if (debugfile)
    {
	Z_FreeTags (PU_LEVEL, MAXINT);
	Z_FileDumpHeap (debugfile);
    }
    else
#endif
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

	// UNUSED W_Profile ();
	P_InitThinkers();

	// find map name
	if(gamemode == commercial) {
		if(map < 10)
			sprintf(lumpname, "map0%i", map);
		else
			sprintf(lumpname, "map%i", map);
	} else {
		lumpname[0] = 'E';
		lumpname[1] = '0' + episode;
		lumpname[2] = 'M';
		lumpname[3] = '0' + map;
		lumpname[4] = 0;
	}

	lumpnum = W_GetIdForName(lumpname);

	leveltime = 0;

	// note: most of this ordering is important
	P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
	P_LoadVertexes(lumpnum + ML_VERTEXES);
	P_LoadSectors(lumpnum + ML_SECTORS);
	P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

	P_LoadLineDefs(lumpnum + ML_LINEDEFS);
	P_LoadSubsectors(lumpnum + ML_SSECTORS);
	P_LoadNodes(lumpnum + ML_NODES);
	P_LoadSegs(lumpnum + ML_SEGS);

	rejectmatrix = W_LumpForId(lumpnum + ML_REJECT)->data;
	P_GroupLines();

	bodyqueslot  = 0;
	deathmatch_p = deathmatchstarts;
	P_LoadThings(lumpnum + ML_THINGS);

	// if deathmatch, randomly spawn the active players
	if(deathmatch) {
		for(i = 0; i < MAXPLAYERS; i++)
			if(playeringame[i]) {
				players[i].mo = NULL;
				G_DeathMatchSpawnPlayer(i);
			}
	}

	// clear special respawning que
	iquehead = iquetail = 0;

	// set up world state
	P_SpawnSpecials();

	// build subsector connect matrix
	//	UNUSED P_ConnectSubsectors ();

	// preload graphics
	if(precache)
		R_PrecacheLevel();

	//printf ("free memory: 0x%x\n", Z_FreeMemory());
}

//
// P_Init
//
void
P_Init(void) {
	P_InitSwitchList();
	P_InitPicAnims();
	R_InitSprites(sprnames);
}
