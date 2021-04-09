#ifndef READWAD_WAD_H
#define READWAD_WAD_H

#include <stdint.h>

#define WAD __attribute__((packed))

#ifdef __BIG_ENDIAN__
static inline uint16_t
bswap_16(uint16_t value) {
 return (value << 8) | (value >> 8);
}

static inline uint32_t
bswap_32(uint32_t value) {
 return ((uint32_t)bswap_16(value) << 16) | bswap_16(value >> 16);
}
#define WAD_SHORT(x) ((int16_t)bswap_16((x)))
#define WAD_LONG(x) ((int32_t)bswap_32((x)))
#else
#define WAD_SHORT(x) (x)
#define WAD_LONG(x) (x)
#endif

struct WAD wad_info {
	char     identification[4]; /* Either IWAD or PWAD */
	int32_t  numlumps;
	int32_t  infotableofs;
};

struct WAD lump_info {
	int32_t  filepos;
	int32_t  size;
	char     name[8];
};

struct WAD lump_PNAMES {
	int32_t nummappatches;
	char    name_p[][8];
};

struct WAD lump_TEXTUREx {
	int32_t numtextures;
	int32_t offset[];
};

struct WAD lump_TEXTUREx_map_texture {
	char     name[8];
	int32_t  masked; /* boolean */
	int16_t  width;
	int16_t  height;
	int32_t  columndirectory; /* Obsolete */
	int16_t  patchcount;
	struct WAD lump_TEXTUREx_map_patch {
		int16_t originx;
		int16_t originy;
		int16_t patch;
		int16_t stepdir;
		int16_t colormap;
	} patches[];
};

/* READWAD_WAD_H */
#endif
