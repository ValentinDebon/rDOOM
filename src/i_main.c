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
//	Main program, UNIX argument parsing, calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

#include "m_param.h"
#include "d_main.h"

#include <stdio.h>
#include <getopt.h>

static const struct option longopts[] = {
	/* Arguments */
	{ "config", required_argument },
	{ "episode", required_argument },
	{ "loadgame", required_argument },
	{ "maxdemo", required_argument },
	{ "net", required_argument },
	{ "playdemo", required_argument },
	{ "port", required_argument },
	{ "record", required_argument },
	{ "scale", required_argument },
	{ "skill", required_argument },
	{ "timedemo", required_argument },
	{ "timer", required_argument },
	{ "turbo", required_argument },
	{ "warp", required_argument },
	/* No arguments */
	{ "altdeath", no_argument },
	{ "avg", no_argument },
	{ "comdev", no_argument },
	{ "deathmatch", no_argument },
	{ "debugfile", no_argument },
	{ "devparm", no_argument },
	{ "dup", no_argument },
	{ "extratic", no_argument },
	{ "fast", no_argument },
	{ "grabmouse", no_argument },
	{ "noblit", no_argument },
	{ "nodraw", no_argument },
	{ "nomonsters", no_argument },
	{ "regdev", no_argument },
	{ "respawn", no_argument },
	{ "shdev", no_argument },
	{ },
};

int
main(int argc, char **argv) {
	int longindex, c;

	while(c = getopt_long_only(argc, argv, ":", longopts, &longindex), c != -1) {
		switch(c) {
		case 0:
			M_AppendParam(longopts[longindex].name, optarg);
			break;
		case '?':
			fprintf(stderr, "%s: Invalid option '%s'\n", *argv, argv[optind - 1]);
			return 1;
		case ':':
			fprintf(stderr, "%s: Missing option argument for '%s'\n", *argv, argv[optind - 1]);
			return 1;
		default:
			break;
		}
	}

	D_DoomMain(argv + optind, argv + argc);
}
