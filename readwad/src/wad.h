#ifndef READWAD_WAD_H
#define READWAD_WAD_H

#include <stdint.h>

#define PACKED __attribute__((packed))

struct PACKED wad_info {
	char     identification[4]; /* Either IWAD or PWAD */
	int32_t  numlumps;
	int32_t  infotableofs;
};

struct PACKED wad_lump {
	int32_t  filepos;
	int32_t  size;
	char     name[8];
};

struct PACKED wad_lump_patch_names {
	int32_t nummappatches;
	char    name_p[][8];
};

struct PACKED wad_lump_texture_info {
	int32_t numtextures;
	int32_t offset[];
};

struct PACKED wad_lump_texture {
	char     name[8];
	int32_t  masked; /* boolean */
	int16_t  width;
	int16_t  height;
	uint32_t columndirectory; /* Obsolete */
	int16_t  patchcount;
	struct PACKED wad_lump_texture_patch {
		int16_t originx;
		int16_t originy;
		int16_t patch;
		int16_t stepdir;
		int16_t colormap;
	} patches[];
};

/* READWAD_WAD_H */
#endif
