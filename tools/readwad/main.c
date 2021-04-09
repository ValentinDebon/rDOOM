#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>

#include "wad.h"

#define readwad_at(address, offset) ((const void *)((const uint8_t *)(address) + (offset)))

struct readwad_args {
	unsigned show_lumps : 1;
	unsigned show_pnames_lumps : 1;
	unsigned show_texture_lumps : 1;
	unsigned show_texture_patches : 1;
};

struct readwad_mapping {
	void  *address;
	size_t size;
};

struct readwad_lump_type {
	const char name[8];
	void (*show)(const struct readwad_mapping *, const struct readwad_args *, const struct lump_info *);
};

static int
readwad_mapping_init(struct readwad_mapping *mapping, const char *filename) {
	int fd = open(filename, O_RDONLY);
	int retval = 0;

	if(fd >= 0) {
		struct stat st;

		if(fstat(fd, &st) == 0) {
			void * const address = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

			if(address != MAP_FAILED) {
				mapping->address = address;
				mapping->size = st.st_size;
			} else {
				warn("readwad_mapping_init: Unable to mmap %s", filename);
				retval = -1;
			}
		} else {
			warn("readwad_mapping_init: Unable to stat %s", filename);
			retval = -1;
		}

		close(fd);
	} else {
		warn("readwad_mapping_init: Unable to open %s", filename);
		retval = -1;
	}

	return retval;
}

static int
readwad_mapping_deinit(struct readwad_mapping *mapping) {

	munmap(mapping->address, mapping->size);

	return 0;
}

static void
readwad_show_lump_PNAMES(const struct readwad_mapping *mapping, const struct readwad_args *args, const struct lump_info *lump_info) {
	if(!args->show_pnames_lumps) {
		return;
	}

	const struct lump_PNAMES *PNAMES = readwad_at(mapping->address, WAD_LONG(lump_info->filepos));

	printf("- Patch Names lump %.8s, size: %u, nummappatches: %u\n",
		lump_info->name, WAD_LONG(lump_info->size), WAD_LONG(PNAMES->nummappatches));

	const char *current = *PNAMES->name_p,
		* const end = *PNAMES->name_p + WAD_LONG(PNAMES->nummappatches) * sizeof(*PNAMES->name_p);

	while(current != end) {
		printf("\t- %.8s\n", current);
		current += sizeof(*PNAMES->name_p);
	}
}

static void
readwad_show_lump_TEXTUREx(const struct readwad_mapping *mapping, const struct readwad_args *args, const struct lump_info *lump_info) {
	if(!args->show_texture_lumps) {
		return;
	}

	const struct lump_TEXTUREx *TEXTUREx = readwad_at(mapping->address, WAD_LONG(lump_info->filepos));

	printf("- Texture lump %.8s, size: %u, numtextures: %u\n",
		lump_info->name, WAD_LONG(lump_info->size), WAD_LONG(TEXTUREx->numtextures));

	for(uint32_t i = 0; i < WAD_LONG(TEXTUREx->numtextures); i++) {
		const struct lump_TEXTUREx_map_texture *map_texture = readwad_at(TEXTUREx, WAD_LONG(TEXTUREx->offset[i]));

		printf("\t- Texture %.8s, masked: %s, width: %hd, height: %hd, patchcount: %hd\n",
			map_texture->name, map_texture->masked != 0 ? "true" : "false", WAD_SHORT(map_texture->width), WAD_SHORT(map_texture->height), WAD_SHORT(map_texture->patchcount));

		if(args->show_texture_patches) {
			const struct lump_TEXTUREx_map_patch *current = map_texture->patches,
				* const end = map_texture->patches + WAD_SHORT(map_texture->patchcount);

			while(current != end) {
				printf("\t\t- Patch originx: %hd, originy: %hd, patch: %hd\n",
					WAD_SHORT(current->originx), WAD_SHORT(current->originy), WAD_SHORT(current->patch));
				current++;
			}
		}
	}
}

const struct readwad_lump_type 
readwad_lump_types[] = {
	{ "PNAMES"  , readwad_show_lump_PNAMES },
	{ "TEXTURE1", readwad_show_lump_TEXTUREx },
	{ "TEXTURE2", readwad_show_lump_TEXTUREx },
};

static void
readwad_show_lump(const struct readwad_mapping *mapping, const struct readwad_args *args, const struct lump_info *lump_info) {
	const struct readwad_lump_type *current = readwad_lump_types,
		* const end = readwad_lump_types + sizeof(readwad_lump_types) / sizeof(*readwad_lump_types);

	if(args->show_lumps) {
		printf("- Lump %.8s.\n", lump_info->name);
	}

	while(current != end && strncmp(current->name, lump_info->name, sizeof(lump_info->name)) != 0) {
		current++;
	}

	if(current != end) {
		current->show(mapping, args, lump_info);
	}
}

static void
readwad_show(const char *filename, const struct readwad_args *args) {
	struct readwad_mapping mapping;

	if(readwad_mapping_init(&mapping, filename) != 0) {
		return;
	}

	const struct wad_info *wad_info = mapping.address;

	printf("Mapped WAD file %s.\nType: %.4s, Lumps: %u, Info Table at: %u.\n",
		filename, wad_info->identification, WAD_LONG(wad_info->numlumps), WAD_LONG(wad_info->infotableofs));

	const struct lump_info *current = readwad_at(mapping.address, WAD_LONG(wad_info->infotableofs));
	const struct lump_info * const end = current + WAD_LONG(wad_info->numlumps);

	while(current != end) {

		readwad_show_lump(&mapping, args, current);

		current++;
	}

	readwad_mapping_deinit(&mapping);
}

static void
readwad_usage(const char *readwad_name) {
	fprintf(stderr, "usage: %s wadfile...\n", readwad_name);
	exit(EXIT_FAILURE);
}

static const struct readwad_args
readwad_parse_args(int argc, char **argv) {
	struct readwad_args args = {
		.show_lumps = 0,
		.show_pnames_lumps = 0,
		.show_texture_lumps = 0,
		.show_texture_patches = 0,
	};
	int c;

	while((c = getopt(argc, argv, ":lptP")) != -1) {
		switch(c) {
		case 'l':
			args.show_lumps = 1;
			break;
		case 'p':
			args.show_pnames_lumps = 1;
			break;
		case 't':
			args.show_texture_lumps = 1;
			break;
		case 'P':
			args.show_texture_patches = 1;
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

	while(current != end) {
		const char *filename = *current;

		readwad_show(filename, &args);

		current++;
	}

	return EXIT_SUCCESS;
}

