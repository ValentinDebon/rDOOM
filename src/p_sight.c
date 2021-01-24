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
//	LineOfSight/Visibility checks, uses REJECT Lookup Table.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"

#include "i_system.h"
#include "p_local.h"

// State.
#include "r_state.h"

#include "r_main.h"

//
// P_CheckSight
//
fixed_t sightzstart; // eye z of looker
fixed_t topslope;
fixed_t bottomslope; // slopes to top and bottom of target

divline_t strace; // from t1 to t2
fixed_t t2x;
fixed_t t2y;

int sightcounts[2];

//
// P_DivlineSide
// Returns side 0 (front), 1 (back), or 2 (on).
//
int
P_DivlineSide(fixed_t x,
	fixed_t y,
	divline_t *node) {
	fixed_t dx;
	fixed_t dy;
	fixed_t left;
	fixed_t right;

	if(!node->dx) {
		if(x == node->x)
			return 2;

		if(x <= node->x)
			return node->dy > 0;

		return node->dy < 0;
	}

	if(!node->dy) {
		if(x == node->y)
			return 2;

		if(y <= node->y)
			return node->dx < 0;

		return node->dx > 0;
	}

	dx = (x - node->x);
	dy = (y - node->y);

	left  = (node->dy >> FRACBITS) * (dx >> FRACBITS);
	right = (dy >> FRACBITS) * (node->dx >> FRACBITS);

	if(right < left)
		return 0; // front side

	if(left == right)
		return 2;
	return 1; // back side
}

//
// P_InterceptVector2
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings and addlines traversers.
//
fixed_t
P_InterceptVector2(divline_t *v2,
	divline_t *v1) {
	fixed_t frac;
	fixed_t num;
	fixed_t den;

	den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy);

	if(den == 0)
		return 0;
	//	I_Error ("P_InterceptVector: parallel");

	num  = FixedMul((v1->x - v2->x) >> 8, v1->dy) + FixedMul((v2->y - v1->y) >> 8, v1->dx);
	frac = FixedDiv(num, den);

	return frac;
}

//
// P_CrossSubsector
// Returns true
//  if strace crosses the given subsector successfully.
//
boolean
P_CrossSubsector(int num) {
	const struct p_segment *seg;
	struct p_line *line;
	int s1;
	int s2;
	int count;
	const struct p_subSector *sub;
	const struct p_sector *front;
	const struct p_sector *back;
	fixed_t opentop;
	fixed_t openbottom;
	divline_t divl;
	const struct p_vertex *v1;
	const struct p_vertex *v2;
	fixed_t frac;
	fixed_t slope;

#ifdef RANGECHECK
	if(num >= p_level.sub_sectors_count)
		I_Error("P_CrossSubsector: ss %i with numss = %i",
			num,
			p_level.sub_sectors_count);
#endif

	sub = p_level.sub_sectors + num;

	// check lines
	count = sub->lines_count;
	seg   = p_level.segments + sub->first_line;

	for(; count; seg++, count--) {
		line = seg->line;

		// already checked other side?
		if(line->valid_count == validcount)
			continue;

		line->valid_count = validcount;

		v1 = line->first_vertex;
		v2 = line->last_vertex;
		s1 = P_DivlineSide(v1->x, v1->y, &strace);
		s2 = P_DivlineSide(v2->x, v2->y, &strace);

		// line isn't crossed?
		if(s1 == s2)
			continue;

		divl.x  = v1->x;
		divl.y  = v1->y;
		divl.dx = v2->x - v1->x;
		divl.dy = v2->y - v1->y;
		s1      = P_DivlineSide(strace.x, strace.y, &divl);
		s2      = P_DivlineSide(t2x, t2y, &divl);

		// line isn't crossed?
		if(s1 == s2)
			continue;

		// stop because it is not two sided anyway
		// might do this after updating validcount?
		if(!(line->flags & ML_TWOSIDED))
			return false;

		// crosses a two sided line
		front = seg->front_sector;
		back  = seg->back_sector;

		// no wall to block sight with?
		if(front->floor_height == back->floor_height
			&& front->ceiling_height == back->ceiling_height)
			continue;

		// possible occluder
		// because of ceiling height differences
		if(front->ceiling_height < back->ceiling_height)
			opentop = front->ceiling_height;
		else
			opentop = back->ceiling_height;

		// because of ceiling height differences
		if(front->floor_height > back->floor_height)
			openbottom = front->floor_height;
		else
			openbottom = back->floor_height;

		// quick test for totally closed doors
		if(openbottom >= opentop)
			return false; // stop

		frac = P_InterceptVector2(&strace, &divl);

		if(front->floor_height != back->floor_height) {
			slope = FixedDiv(openbottom - sightzstart, frac);
			if(slope > bottomslope)
				bottomslope = slope;
		}

		if(front->ceiling_height != back->ceiling_height) {
			slope = FixedDiv(opentop - sightzstart, frac);
			if(slope < topslope)
				topslope = slope;
		}

		if(topslope <= bottomslope)
			return false; // stop
	}
	// passed the subsector ok
	return true;
}

//
// P_CrossBSPNode
// Returns true
//  if strace crosses the given node successfully.
//
boolean
P_CrossBSPNode(int bspnum) {
	const struct p_node *bsp;
	int side;

	if(bspnum & NF_SUBSECTOR) {
		if(bspnum == -1)
			return P_CrossSubsector(0);
		else
			return P_CrossSubsector(bspnum & (~NF_SUBSECTOR));
	}

	bsp = p_level.nodes + bspnum;

	// decide which side the start point is on
	side = P_DivlineSide(strace.x, strace.y, (divline_t *)bsp);
	if(side == 2)
		side = 0; // an "on" should cross both sides

	// cross the starting side
	if(!P_CrossBSPNode(bsp->children[side]))
		return false;

	// the partition plane is crossed here
	if(side == P_DivlineSide(t2x, t2y, (divline_t *)bsp)) {
		// the line doesn't touch the other side
		return true;
	}

	// cross the ending side
	return P_CrossBSPNode(bsp->children[side ^ 1]);
}

//
// P_CheckSight
// Returns true
//  if a straight line between t1 and t2 is unobstructed.
// Uses REJECT.
//
boolean
P_CheckSight(mobj_t *t1,
	mobj_t *t2) {
	int s1;
	int s2;
	int pnum;
	int bytenum;
	int bitnum;

	// First check for trivial rejection.

	// Determine subsector entries in REJECT table.
	s1      = (t1->subsector->sector - p_level.sectors);
	s2      = (t2->subsector->sector - p_level.sectors);
	pnum    = s1 * p_level.sectors_count + s2;
	bytenum = pnum >> 3;
	bitnum  = 1 << (pnum & 7);

	// Check in REJECT table.
	if(p_level.reject_matrix[bytenum] & bitnum) {
		sightcounts[0]++;

		// can't possibly be connected
		return false;
	}

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	sightcounts[1]++;

	validcount++;

	sightzstart = t1->z + t1->height - (t1->height >> 2);
	topslope    = (t2->z + t2->height) - sightzstart;
	bottomslope = (t2->z) - sightzstart;

	strace.x  = t1->x;
	strace.y  = t1->y;
	t2x       = t2->x;
	t2y       = t2->y;
	strace.dx = t2->x - t1->x;
	strace.dy = t2->y - t1->y;

	// the head node is the last node output
	return P_CrossBSPNode(p_level.nodes_count - 1);
}
