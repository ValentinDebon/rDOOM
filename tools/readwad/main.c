#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "w_wad.h"
#include "m_swap.h"

#define readwad_at(address, offset) ((const void *)((const uint8_t *)(address) + (offset)))

struct readwad_args {
	const char *wadfile;
	unsigned verbosity;
};

struct readwad_lump_type {
	const char name[8];
	void (*show)(const struct w_lump *, const struct readwad_args *);
};

static void
readwad_show_lump_PNAMES(const struct w_lump *lump, const struct readwad_args *args) {
	const struct w_lumpPNAMES * const PNAMES = lump->data;
	const uint32_t nummappatches = LE_U32(PNAMES->patches_count);

	printf("- Patch Names lump %.8s, size: %lu, nummappatches: %u\n",
		lump->name, lump->size, nummappatches);

	const char *current = *PNAMES->names,
		* const end = *PNAMES->names + nummappatches * sizeof(*PNAMES->names);

	while(current != end) {
		printf("\t%.8s\n", current);
		current += sizeof(*PNAMES->names);
	}
}

static void
readwad_show_lump_TEXTUREx(const struct w_lump *lump, const struct readwad_args *args) {
	const struct w_lumpTEXTUREx * const TEXTUREx = lump->data;
	const uint32_t numtextures = LE_U32(TEXTUREx->textures_count);

	printf("- Texture lump %.8s, size: %lu, numtextures: %u\n",
		lump->name, lump->size, numtextures);

	for(uint32_t i = 0; i < numtextures; i++) {
		const uint32_t offset = LE_U32(TEXTUREx->offsets[i]);
		const struct w_mapTexture * const map_texture = readwad_at(TEXTUREx, offset);
		const uint16_t patchcount = LE_U16(map_texture->patches_count);

		printf("\tTexture %.8s, masked: %s, width: %hu, height: %hu, patchcount: %hu\n",
			map_texture->name, map_texture->masked != 0 ? "true" : "false",
			LE_U16(map_texture->width), LE_U16(map_texture->height), patchcount);

		if(args->verbosity > 0) {
			const struct w_mapPatch *current = map_texture->patches,
				* const end = map_texture->patches + patchcount;

			while(current != end) {
				printf("\t\tPatch originx: %hd, originy: %hd, patch: %hd\n",
					LE_S16(current->offset_x), LE_S16(current->offset_y), LE_S16(current->patch));
				current++;
			}
		}
	}
}

const struct readwad_lump_type 
readwad_lump_types[] = {
	{ "PNAMES"  , readwad_show_lump_PNAMES   },
	{ "TEXTURE1", readwad_show_lump_TEXTUREx },
	{ "TEXTURE2", readwad_show_lump_TEXTUREx },
};

static void
readwad_show_lump(const char *name, const struct readwad_args *args) {
	const struct readwad_lump_type *current = readwad_lump_types,
		* const end = readwad_lump_types + sizeof(readwad_lump_types) / sizeof(*readwad_lump_types);

	while(current != end && strncmp(current->name, name, sizeof(current->name)) != 0) {
		current++;
	}

	if(current != end) {
		const struct w_lump *lump = W_LumpForName(name);

		current->show(lump, args);

	} else {
		fprintf(stderr, "Unable to a suitable way to show lump named %s in WAD\n", name);
	}
}

static void
readwad_usage(const char *readwad_name) {
	fprintf(stderr, "usage: %s wadfile -i <wadfile> <lump name>...\n", readwad_name);
	exit(EXIT_FAILURE);
}

static const struct readwad_args
readwad_parse_args(int argc, char **argv) {
	struct readwad_args args = {
		.wadfile = "doom1.wad",
		.verbosity = 0,
	};
	int c;

	while((c = getopt(argc, argv, ":i:v")) != -1) {
		switch(c) {
		case 'i':
			args.wadfile = optarg;
			break;
		case 'v':
			args.verbosity++;
			break;
		case ':':
			warnx("-%c: Missing argument", optopt);
			readwad_usage(*argv);
		default:
			warnx("Unknown argument -%c", optopt);
			readwad_usage(*argv);
		}
	}

	if(argc - optind == 0) {
		readwad_usage(*argv);
	}

	return args;
}

int
main(int argc, char **argv) {
	const struct readwad_args args = readwad_parse_args(argc, argv);
	char **current = argv + optind, ** const end = argv + argc;
	const char * const wadfiles[] = { args.wadfile, NULL };

	puts("Read WAD");
	W_Init(wadfiles);

	while(current != end) {
		const char *name = *current;

		readwad_show_lump(name, &args);

		current++;
	}

	return EXIT_SUCCESS;
}
