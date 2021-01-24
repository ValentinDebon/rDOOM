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

#include "p_setup.h"
#include "p_local.h"

#include "doomstat.h"
#include "m_fixed.h"
#include "m_swap.h"
#include "m_bbox.h"
#include "s_sound.h"
#include "g_game.h"
#include "r_data.h"
#include "r_things.h"
#include "z_zone.h"
#include "w_wad.h"

#include <stdbool.h>

void
P_SpawnMapThing(const mapthing_t *mthing);

// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS 10

mapthing_t deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];

static void
P_LoadVertices(lumpId_t id) {
	const struct w_lump * const lump     = W_LumpForId(id);
	const struct w_mapVertex *map_vertex = lump->data;

	/* Determine number of lumps: total lump length / vertex record length. */
	p_level.vertices_count = lump->size / sizeof(*map_vertex);

	/* Allocate zone memory for buffer. */
	p_level.vertices = Z_Malloc(p_level.vertices_count * sizeof(*p_level.vertices), PU_LEVEL, 0);

	/* Copy and convert vertex coordinates,
	 * internal representation as fixed. */
	struct p_vertex *vertex = p_level.vertices;
	for(size_t i = p_level.vertices_count; i != 0; i--, vertex++, map_vertex++) {
		vertex->x = FixedFor(LE_S16(map_vertex->x));
		vertex->y = FixedFor(LE_S16(map_vertex->y));
	}
}

static void
P_LoadSegments(lumpId_t id) {
	const struct w_lump * const lump       = W_LumpForId(id);
	const struct w_mapSegment *map_segment = lump->data;

	p_level.segments_count = lump->size / sizeof(*map_segment);

	p_level.segments = Z_Malloc(p_level.segments_count * sizeof(*p_level.segments), PU_LEVEL, 0);
	memset(p_level.segments, 0, p_level.segments_count * sizeof(*p_level.segments));

	struct p_segment *segment = p_level.segments;
	for(size_t i = p_level.segments_count; i != 0; i--, segment++, map_segment++) {
		struct p_line * const line = p_level.lines + LE_U16(map_segment->line);
		const int side = LE_S16(map_segment->side);

		/* Set segment edges */
		segment->first_vertex = p_level.vertices + LE_U16(map_segment->first_vertex);
		segment->last_vertex  = p_level.vertices + LE_U16(map_segment->last_vertex);

		/* Set segment properties */
		segment->angle  = FixedFor(LE_S16(map_segment->angle));
		segment->offset = FixedFor(LE_S16(map_segment->offset));
		segment->line   = line;
		segment->side   = p_level.sides + line->sides[side];

		/* Depending on single-sided or two sided, set sides */
		segment->front_sector = segment->side->sector;
		segment->back_sector  = (line->flags & ML_TWOSIDED) != 0 ?
			p_level.sides[line->sides[side ^ 1]].sector : 0;
	}
}

static void
P_LoadSubSectors(lumpId_t id) {
	const struct w_lump * const lump            = W_LumpForId(id);
	const struct w_mapSubSector *map_sub_sector = lump->data;

	p_level.sub_sectors_count = lump->size / sizeof(*map_sub_sector);

	p_level.sub_sectors = Z_Malloc(p_level.sub_sectors_count * sizeof(*p_level.sub_sectors), PU_LEVEL, 0);
	memset(p_level.sub_sectors, 0, p_level.sub_sectors_count * sizeof(*p_level.sub_sectors));

	struct p_subSector *sub_sector = p_level.sub_sectors;
	for(size_t i = p_level.sub_sectors_count; i != 0; i--, sub_sector++, map_sub_sector++) {
		sub_sector->lines_count = LE_S16(map_sub_sector->segments_count);
		sub_sector->first_line  = LE_S16(map_sub_sector->first_segment);
	}
}

static void
P_LoadSectors(lumpId_t id) {
	const struct w_lump * const lump     = W_LumpForId(id);
	const struct w_mapSector *map_sector = lump->data;

	p_level.sectors_count = lump->size / sizeof(*map_sector);

	p_level.sectors = Z_Malloc(p_level.sectors_count * sizeof(*p_level.sectors), PU_LEVEL, 0);
	memset(p_level.sectors, 0, p_level.sectors_count * sizeof(*p_level.sectors));

	struct p_sector *sector = p_level.sectors;
	for(size_t i = p_level.sectors_count; i != 0; i--, sector++, map_sector++) {
		sector->floor_height   = FixedFor(LE_S16(map_sector->floor_height));
		sector->ceiling_height = FixedFor(LE_S16(map_sector->ceiling_height));
		sector->floor          = R_FlatIdForName(map_sector->floor);
		sector->ceiling        = R_FlatIdForName(map_sector->ceiling);
		sector->lighting       = LE_S16(map_sector->lighting);
		sector->special_type   = LE_S16(map_sector->special_type);
		sector->tag            = LE_S16(map_sector->tag);
		sector->thing_list     = NULL;
	}
}

static void
P_LoadNodes(lumpId_t id) {
	const struct w_lump * const lump = W_LumpForId(id);
	const struct w_mapNode *map_node = lump->data;

	p_level.nodes_count = lump->size / sizeof(*map_node);

	p_level.nodes = Z_Malloc(p_level.nodes_count * sizeof(*p_level.nodes), PU_LEVEL, 0);
	memset(p_level.nodes, 0, p_level.nodes_count * sizeof(*p_level.nodes));

	struct p_node *node = p_level.nodes;
	for(size_t i = p_level.nodes_count; i != 0; i--, node++, map_node++) {
		node->x  = FixedFor(LE_S16(map_node->x));
		node->y  = FixedFor(LE_S16(map_node->y));
		node->dx = FixedFor(LE_S16(map_node->dx));
		node->dy = FixedFor(LE_S16(map_node->dy));

		for(int j = 0; j < 2; j++) {

			node->children[j] = LE_S16(map_node->children[j]);

			for(int k = 0; k < 4; k++) {
				node->bounding_boxes[j][k] = FixedFor(LE_S16(map_node->bounding_boxes[j][k]));
			}
		}
	}
}

static void
P_LoadThings(lumpId_t id) {
	const struct w_lump *lump          = W_LumpForId(id);
	const struct w_mapThing *map_thing = lump->data;

	for(size_t i = lump->size / sizeof(*map_thing); i != 0; i--, map_thing++) {
		bool spawn = true;

		/* Do not spawn cool, new monsters if not commercial */
		if(gamemode != commercial) {
			switch(map_thing->type) {
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

		if(!spawn)
			break;

		/* Do spawn all other stuff. */
		const mapthing_t host_map_thing = {
			.x       = LE_S16(map_thing->x),
			.y       = LE_S16(map_thing->y),
			.angle   = LE_S16(map_thing->angle),
			.type    = LE_S16(map_thing->type),
			.options = LE_S16(map_thing->flags),
		};

		P_SpawnMapThing(&host_map_thing);
	}
}

/* P_LoadLines
 * Also counts secret lines for intermissions.
 */
static void
P_LoadLines(lumpId_t id) {
	const struct w_lump * const lump   = W_LumpForId(id);
	const struct w_mapLine *map_line = lump->data;

	p_level.lines_count = lump->size / sizeof(*map_line);

	p_level.lines = Z_Malloc(p_level.lines_count * sizeof(*p_level.lines), PU_LEVEL, 0);
	memset(p_level.lines, 0, p_level.lines_count * sizeof(*p_level.lines));

	struct p_line *line = p_level.lines;
	for(size_t i = p_level.lines_count; i != 0; i--, line++, map_line++) {
		const struct p_vertex * const first_vertex = p_level.vertices + LE_U16(map_line->first_vertex),
			* const last_vertex = p_level.vertices + LE_U16(map_line->last_vertex);

		line->flags        = LE_S16(map_line->flags);
		line->special_type = LE_S16(map_line->special_type);
		line->sector_tag   = LE_S16(map_line->sector_tag);

		line->first_vertex = first_vertex;
		line->last_vertex  = last_vertex;

		line->dx = last_vertex->x - first_vertex->x;
		line->dy = last_vertex->y - first_vertex->y;

		if(line->dx == 0) {
			line->slope_type = SLOPE_TYPE_VERTICAL;
		} else if(line->dy == 0) {
			line->slope_type = SLOPE_TYPE_HORIZONTAL;
		} else {
			if(FixedDiv(line->dy, line->dx) > 0) {
				line->slope_type = SLOPE_TYPE_POSITIVE;
			} else {
				line->slope_type = SLOPE_TYPE_NEGATIVE;
			}
		}

		if(first_vertex->x < last_vertex->x) {
			line->bounding_boxes[BOXLEFT]  = first_vertex->x;
			line->bounding_boxes[BOXRIGHT] = last_vertex->x;
		} else {
			line->bounding_boxes[BOXLEFT]  = last_vertex->x;
			line->bounding_boxes[BOXRIGHT] = first_vertex->x;
		}

		if(first_vertex->y < last_vertex->y) {
			line->bounding_boxes[BOXBOTTOM]  = first_vertex->y;
			line->bounding_boxes[BOXTOP]     = last_vertex->y;
		} else {
			line->bounding_boxes[BOXBOTTOM]  = last_vertex->y;
			line->bounding_boxes[BOXTOP]     = first_vertex->y;
		}

		line->sides[0] = LE_S16(map_line->sides[0]);
		line->sides[1] = LE_S16(map_line->sides[1]);

		line->front_sector = line->sides[0] != -1 ?
			p_level.sides[line->sides[0]].sector : NULL;

		line->back_sector = line->sides[1] != -1 ?
			p_level.sides[line->sides[1]].sector : NULL;
	}
}

static void
P_LoadSides(lumpId_t id) {
	const struct w_lump * const lump   = W_LumpForId(id);
	const struct w_mapSide *map_side = lump->data;

	p_level.sides_count = lump->size / sizeof(*map_side);

	p_level.sides = Z_Malloc(p_level.sides_count * sizeof(*p_level.sides), PU_LEVEL, 0);
	memset(p_level.sides, 0, p_level.sides_count * sizeof(*p_level.sides));

	struct p_side *side = p_level.sides;
	for(size_t i = p_level.sides_count; i != 0; i--, side++, map_side++) {
		side->offset_x = FixedFor(LE_S16(map_side->offset_x));
		side->offset_y = FixedFor(LE_S16(map_side->offset_y));

		side->top_texture    = R_TextureIdForName(map_side->top_texture);
		side->middle_texture = R_TextureIdForName(map_side->middle_texture);
		side->bottom_texture = R_TextureIdForName(map_side->bottom_texture);

		side->sector = p_level.sectors + LE_U16(map_side->faced_sector);
	}
}

static void
P_LoadBlockMap(lumpId_t id) {
	const struct w_lump * const lump = W_LumpForId(id);

	p_level.blockmap_lump     = lump->data;
	p_level.blockmap_origin_x = FixedFor(LE_S16(p_level.blockmap_lump[0]));
	p_level.blockmap_origin_y = FixedFor(LE_S16(p_level.blockmap_lump[1]));
	p_level.blockmap_width    = LE_S16(p_level.blockmap_lump[2]);
	p_level.blockmap_height   = LE_S16(p_level.blockmap_lump[3]);

	const size_t block_links_size = p_level.blockmap_width * p_level.blockmap_height * sizeof(*p_level.block_links);
	p_level.block_links = Z_Malloc(block_links_size, PU_LEVEL, 0);
	memset(p_level.block_links, 0, block_links_size);
}

/*
 * P_GroupLines
 * Builds sector line lists and subsector sector numbers.
 * Finds block bounding boxes for sectors.
 */
void
P_GroupLines(void) {
	// look up sector number for each subsector
	struct p_subSector *sub_sector = p_level.sub_sectors;
	for(size_t i = p_level.sub_sectors_count; i != 0; i--, sub_sector++) {
		const struct p_segment * const segment = p_level.segments + sub_sector->first_line;
		sub_sector->sector = segment->side->sector;
	}

	// count number of lines in each sector
	struct p_line *line = p_level.lines;
	size_t total = 0;
	for(size_t i = p_level.lines_count; i != 0; i--, line++) {
		total++;

		line->front_sector->lines_count++;

		if(line->back_sector != NULL && line->back_sector != line->front_sector) {
			line->back_sector->lines_count++;
			total++;
		}
	}

	// build line tables for each sector
	struct p_line **lines_buffer = Z_Malloc(total * sizeof(*lines_buffer), PU_LEVEL, 0);
	struct p_sector *sector = p_level.sectors;
	for(size_t i = p_level.sectors_count; i != 0; i--, sector++) {
		fixed_t bbox[4];

		M_ClearBox(bbox);

		sector->lines = lines_buffer;

		struct p_line *line = p_level.lines;
		for(size_t j = p_level.lines_count; j != 0; j--, line++) {
			if(line->front_sector == sector || line->back_sector == sector) {
				*lines_buffer++ = line;
				M_AddToBox(bbox, line->first_vertex->x, line->first_vertex->y);
				M_AddToBox(bbox, line->last_vertex->x, line->last_vertex->y);
			}
		}

		if(lines_buffer - sector->lines != sector->lines_count) {
			I_Error("P_GroupLines: miscounted");
		}

		// set the degenmobj_t to the middle of the bounding box
		sector->sound_origin.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
		sector->sound_origin.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

		// adjust bounding box to map blocks
		int block;

		block                    = (bbox[BOXTOP] - p_level.blockmap_origin_y + MAXRADIUS) >> MAPBLOCKSHIFT;
		block                    = block >= p_level.blockmap_height ? p_level.blockmap_height - 1 : block;
		sector->blockbox[BOXTOP] = block;

		block                       = (bbox[BOXBOTTOM] - p_level.blockmap_origin_y - MAXRADIUS) >> MAPBLOCKSHIFT;
		block                       = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM] = block;

		block                      = (bbox[BOXRIGHT] - p_level.blockmap_origin_x + MAXRADIUS) >> MAPBLOCKSHIFT;
		block                      = block >= p_level.blockmap_width ? p_level.blockmap_width - 1 : block;
		sector->blockbox[BOXRIGHT] = block;

		block                     = (bbox[BOXLEFT] - p_level.blockmap_origin_x - MAXRADIUS) >> MAPBLOCKSHIFT;
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

	totalkills  = 0;
	totalitems  = 0;
	totalsecret = 0;

	wminfo.maxfrags = 0;
	wminfo.partime  = 180;

	for(unsigned i = 0; i < MAXPLAYERS; i++) {
		players[i].killcount   = 0;
		players[i].secretcount = 0;
		players[i].itemcount   = 0;
	}

	// Initial height of PointOfView
	// will be set by player think.
	players[consoleplayer].viewz = 1;

	// Make sure all sounds are stopped before Z_FreeTags.
	S_Start();

	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

	P_InitThinkers();

	// find map name
	char lump[8] = {};
	if(gamemode == commercial) {
		snprintf(lump, sizeof(lump), "map%.2d", map);
	} else {
		lump[0] = 'E';
		lump[1] = '0' + episode;
		lump[2] = 'M';
		lump[3] = '0' + map;
	}

	const lumpId_t id = W_GetIdForName(lump);

	leveltime = 0;

	// note: most of this ordering is important
	P_LoadBlockMap(id + ML_BLOCKMAP);
	P_LoadVertices(id + ML_VERTEXES);
	P_LoadSectors(id + ML_SECTORS);
	P_LoadSides(id + ML_SIDEDEFS);

	P_LoadLines(id + ML_LINEDEFS);
	P_LoadSubSectors(id + ML_SSECTORS);
	P_LoadNodes(id + ML_NODES);
	P_LoadSegments(id + ML_SEGS);

	p_level.reject_matrix = W_LumpForId(id + ML_REJECT)->data;
	P_GroupLines();

	bodyqueslot  = 0;
	deathmatch_p = deathmatchstarts;
	P_LoadThings(id + ML_THINGS);

	// if deathmatch, randomly spawn the active players
	if(deathmatch) {
		for(unsigned i = 0; i < MAXPLAYERS; i++) {
			if(playeringame[i]) {
				players[i].mo = NULL;
				G_DeathMatchSpawnPlayer(i);
			}
		}
	}

	// clear special respawning que
	iquehead = iquetail = 0;

	// set up world state
	P_SpawnSpecials();

	// preload graphics
	if(precache)
		R_PrecacheLevel();
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
