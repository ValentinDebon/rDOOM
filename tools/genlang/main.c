#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <err.h>

#include "l_strings.h"

struct genlang_args {
	const char *output;
};

enum genlang_state {
	GENLANG_STATE_WAIT_KEY,
	GENLANG_STATE_PARSE_KEY,
	GENLANG_STATE_WAIT_EQUAL,
	GENLANG_STATE_WAIT_QUOTE,
	GENLANG_STATE_PARSE_VALUE,
	GENLANG_STATE_ESCAPING_VALUE,
};

struct genlang_string {
	size_t capacity, length;
	char *content;
};

struct genlang_lang {
	/* Configuration */
	size_t pagesize;
	char **translations;
	/* Parsing automaton */
	enum genlang_state state;
	struct genlang_string string;
	char **current_translation;
};

static const char *genlang_keys[] = {
	"D_DEVSTR",
	"D_CDROM",
	"M_PRESSKEY",
	"M_PRESSYN",
	"M_QUITMSG",
	"M_QUITDOOM1MSG1",
	"M_QUITDOOM1MSG2",
	"M_QUITDOOM1MSG3",
	"M_QUITDOOM1MSG4",
	"M_QUITDOOM1MSG5",
	"M_QUITDOOM1MSG6",
	"M_QUITDOOM1MSG7",
	"M_QUITDOOM2MSG1",
	"M_QUITDOOM2MSG2",
	"M_QUITDOOM2MSG3",
	"M_QUITDOOM2MSG4",
	"M_QUITDOOM2MSG5",
	"M_QUITDOOM2MSG6",
	"M_QUITDOOM2MSG7",
	"M_QUITFDOOMMSG1",
	"M_QUITFDOOMMSG2",
	"M_QUITFDOOMMSG3",
	"M_QUITFDOOMMSG4",
	"M_QUITFDOOMMSG5",
	"M_QUITFDOOMMSG6",
	"M_QUITFDOOMMSG7",
	"M_LOADNET",
	"M_QLOADNET",
	"M_QSAVESPOT",
	"M_SAVEDEAD",
	"M_QSPROMPT",
	"M_QLPROMPT",
	"M_NEWGAME",
	"M_NIGHTMARE",
	"M_SWSTRING",
	"M_MSGOFF",
	"M_MSGON",
	"M_NETEND",
	"M_ENDGAME",
	"M_DOSY",
	"M_DETAILHI",
	"M_DETAILLO",
	"M_GAMMALVL0",
	"M_GAMMALVL1",
	"M_GAMMALVL2",
	"M_GAMMALVL3",
	"M_GAMMALVL4",
	"M_EMPTYSTRING",
	"P_GOTARMOR",
	"P_GOTMEGA",
	"P_GOTHTHBONUS",
	"P_GOTARMBONUS",
	"P_GOTSTIM",
	"P_GOTMEDINEED",
	"P_GOTMEDIKIT",
	"P_GOTSUPER",
	"P_GOTBLUECARD",
	"P_GOTYELWCARD",
	"P_GOTREDCARD",
	"P_GOTBLUESKUL",
	"P_GOTYELWSKUL",
	"P_GOTREDSKULL",
	"P_GOTINVUL",
	"P_GOTBERSERK",
	"P_GOTINVIS",
	"P_GOTSUIT",
	"P_GOTMAP",
	"P_GOTVISOR",
	"P_GOTMSPHERE",
	"P_GOTCLIP",
	"P_GOTCLIPBOX",
	"P_GOTROCKET",
	"P_GOTROCKBOX",
	"P_GOTCELL",
	"P_GOTCELLBOX",
	"P_GOTSHELLS",
	"P_GOTSHELLBOX",
	"P_GOTBACKPACK",
	"P_GOTBFG9000",
	"P_GOTCHAINGUN",
	"P_GOTCHAINSAW",
	"P_GOTLAUNCHER",
	"P_GOTPLASMA",
	"P_GOTSHOTGUN",
	"P_GOTSHOTGUN2",
	"P_BLUEO",
	"P_REDO",
	"P_YELLOWO",
	"P_BLUEK",
	"P_REDK",
	"P_YELLOWK",
	"G_SAVED",
	"HU_MSGU",
	"HU_E1M1",
	"HU_E1M2",
	"HU_E1M3",
	"HU_E1M4",
	"HU_E1M5",
	"HU_E1M6",
	"HU_E1M7",
	"HU_E1M8",
	"HU_E1M9",
	"HU_E2M1",
	"HU_E2M2",
	"HU_E2M3",
	"HU_E2M4",
	"HU_E2M5",
	"HU_E2M6",
	"HU_E2M7",
	"HU_E2M8",
	"HU_E2M9",
	"HU_E3M1",
	"HU_E3M2",
	"HU_E3M3",
	"HU_E3M4",
	"HU_E3M5",
	"HU_E3M6",
	"HU_E3M7",
	"HU_E3M8",
	"HU_E3M9",
	"HU_E4M1",
	"HU_E4M2",
	"HU_E4M3",
	"HU_E4M4",
	"HU_E4M5",
	"HU_E4M6",
	"HU_E4M7",
	"HU_E4M8",
	"HU_E4M9",
	"HU_DOOM2MAP1",
	"HU_DOOM2MAP2",
	"HU_DOOM2MAP3",
	"HU_DOOM2MAP4",
	"HU_DOOM2MAP5",
	"HU_DOOM2MAP6",
	"HU_DOOM2MAP7",
	"HU_DOOM2MAP8",
	"HU_DOOM2MAP9",
	"HU_DOOM2MAP10",
	"HU_DOOM2MAP11",
	"HU_DOOM2MAP12",
	"HU_DOOM2MAP13",
	"HU_DOOM2MAP14",
	"HU_DOOM2MAP15",
	"HU_DOOM2MAP16",
	"HU_DOOM2MAP17",
	"HU_DOOM2MAP18",
	"HU_DOOM2MAP19",
	"HU_DOOM2MAP20",
	"HU_DOOM2MAP21",
	"HU_DOOM2MAP22",
	"HU_DOOM2MAP23",
	"HU_DOOM2MAP24",
	"HU_DOOM2MAP25",
	"HU_DOOM2MAP26",
	"HU_DOOM2MAP27",
	"HU_DOOM2MAP28",
	"HU_DOOM2MAP29",
	"HU_DOOM2MAP30",
	"HU_DOOM2MAP31",
	"HU_DOOM2MAP32",
	"HU_PLUTONIAMAP1",
	"HU_PLUTONIAMAP2",
	"HU_PLUTONIAMAP3",
	"HU_PLUTONIAMAP4",
	"HU_PLUTONIAMAP5",
	"HU_PLUTONIAMAP6",
	"HU_PLUTONIAMAP7",
	"HU_PLUTONIAMAP8",
	"HU_PLUTONIAMAP9",
	"HU_PLUTONIAMAP10",
	"HU_PLUTONIAMAP11",
	"HU_PLUTONIAMAP12",
	"HU_PLUTONIAMAP13",
	"HU_PLUTONIAMAP14",
	"HU_PLUTONIAMAP15",
	"HU_PLUTONIAMAP16",
	"HU_PLUTONIAMAP17",
	"HU_PLUTONIAMAP18",
	"HU_PLUTONIAMAP19",
	"HU_PLUTONIAMAP20",
	"HU_PLUTONIAMAP21",
	"HU_PLUTONIAMAP22",
	"HU_PLUTONIAMAP23",
	"HU_PLUTONIAMAP24",
	"HU_PLUTONIAMAP25",
	"HU_PLUTONIAMAP26",
	"HU_PLUTONIAMAP27",
	"HU_PLUTONIAMAP28",
	"HU_PLUTONIAMAP29",
	"HU_PLUTONIAMAP30",
	"HU_PLUTONIAMAP31",
	"HU_PLUTONIAMAP32",
	"HU_TNTMAP1",
	"HU_TNTMAP2",
	"HU_TNTMAP3",
	"HU_TNTMAP4",
	"HU_TNTMAP5",
	"HU_TNTMAP6",
	"HU_TNTMAP7",
	"HU_TNTMAP8",
	"HU_TNTMAP9",
	"HU_TNTMAP10",
	"HU_TNTMAP11",
	"HU_TNTMAP12",
	"HU_TNTMAP13",
	"HU_TNTMAP14",
	"HU_TNTMAP15",
	"HU_TNTMAP16",
	"HU_TNTMAP17",
	"HU_TNTMAP18",
	"HU_TNTMAP19",
	"HU_TNTMAP20",
	"HU_TNTMAP21",
	"HU_TNTMAP22",
	"HU_TNTMAP23",
	"HU_TNTMAP24",
	"HU_TNTMAP25",
	"HU_TNTMAP26",
	"HU_TNTMAP27",
	"HU_TNTMAP28",
	"HU_TNTMAP29",
	"HU_TNTMAP30",
	"HU_TNTMAP31",
	"HU_TNTMAP32",
	"HU_CHATMACRO1",
	"HU_CHATMACRO2",
	"HU_CHATMACRO3",
	"HU_CHATMACRO4",
	"HU_CHATMACRO5",
	"HU_CHATMACRO6",
	"HU_CHATMACRO7",
	"HU_CHATMACRO8",
	"HU_CHATMACRO9",
	"HU_CHATMACRO0",
	"HU_TALKTOSELF1",
	"HU_TALKTOSELF2",
	"HU_TALKTOSELF3",
	"HU_TALKTOSELF4",
	"HU_TALKTOSELF5",
	"HU_MESSAGESENT",
	"HU_PLRGREEN",
	"HU_PLRINDIGO",
	"HU_PLRBROWN",
	"HU_PLRRED",
	"HU_KEYGREEN",
	"HU_KEYINDIGO",
	"HU_KEYBROWN",
	"HU_KEYRED",
	"AM_FOLLOWON",
	"AM_FOLLOWOFF",
	"AM_GRIDON",
	"AM_GRIDOFF",
	"AM_MARKEDSPOT",
	"AM_MARKSCLEARED",
	"ST_MUS",
	"ST_NOMUS",
	"ST_DQDON",
	"ST_DQDOFF",
	"ST_KFAADDED",
	"ST_FAADDED",
	"ST_NCON",
	"ST_NCOFF",
	"ST_BEHOLD",
	"ST_BEHOLDX",
	"ST_CHOPPERS",
	"ST_CLEV",
	"F_E1TEXT",
	"F_E2TEXT",
	"F_E3TEXT",
	"F_E4TEXT",
	"F_C1TEXT",
	"F_C2TEXT",
	"F_C3TEXT",
	"F_C4TEXT",
	"F_C5TEXT",
	"F_C6TEXT",
	"F_P1TEXT",
	"F_P2TEXT",
	"F_P3TEXT",
	"F_P4TEXT",
	"F_P5TEXT",
	"F_P6TEXT",
	"F_T1TEXT",
	"F_T2TEXT",
	"F_T3TEXT",
	"F_T4TEXT",
	"F_T5TEXT",
	"F_T6TEXT",
	"F_CC_ZOMBIE",
	"F_CC_SHOTGUN",
	"F_CC_HEAVY",
	"F_CC_IMP",
	"F_CC_DEMON",
	"F_CC_LOST",
	"F_CC_CACO",
	"F_CC_HELL",
	"F_CC_BARON",
	"F_CC_ARACH",
	"F_CC_PAIN",
	"F_CC_REVEN",
	"F_CC_MANCU",
	"F_CC_ARCH",
	"F_CC_SPIDER",
	"F_CC_CYBER",
	"F_CC_HERO",
};

_Static_assert(sizeof(genlang_keys) / sizeof(*genlang_keys) == STRING_LAST, "Size incompatibility between l_strings.h and genlang_keys");

/******************
 * GenLang string *
 ******************/

static void
genlang_string_init(struct genlang_string *string) {

	string->capacity = 128;
	string->length = 0;
	string->content = calloc(string->capacity, sizeof(*string->content));

	if(string->content == NULL) {
		err(EXIT_FAILURE, "genlang_string_init: calloc %lu", string->capacity * sizeof(*string->content));
	}
}

static void
genlang_string_deinit(struct genlang_string *string) {
	free(string->content);
}

static void
genlang_string_push(struct genlang_string *string, char character) {

	if(string->capacity - string->length == 1) {
		const size_t newcapacity = string->capacity * 2;
		char * const newcontent = realloc(string->content, newcapacity);

		if(newcontent == NULL) {
			err(EXIT_FAILURE, "genlang_string_push: realloc %lu", newcapacity);
		}

		memset(newcontent + string->capacity, 0, newcapacity - string->capacity);

		string->content = newcontent;
		string->capacity = newcapacity;
	}

	string->content[string->length] = character;
	string->length++;
}

static inline void
genlang_string_empty(struct genlang_string *string) {
	memset(string->content, 0, string->length);
	string->length = 0;
}

/******************
 * GenLang lang *
 ******************/

static void
genlang_lang_init(struct genlang_lang *lang) {
	/* Configuration */
	lang->pagesize = getpagesize();
	lang->translations = calloc(STRING_LAST, sizeof(*lang->translations));

	if(lang->translations == NULL) {
		err(EXIT_FAILURE, "genlang_lang_init: calloc %lu", STRING_LAST * sizeof(*lang->translations));
	}

	/* Parsing automaton */
	lang->state = GENLANG_STATE_WAIT_KEY;
	genlang_string_init(&lang->string);
	/* no requirement for lang->current_translation */
}

static inline void
genlang_lang_deinit(struct genlang_lang *lang) {
	genlang_string_deinit(&lang->string);
	for(int i = 0; i < STRING_LAST; i++) {
		free(lang->translations[i]);
	}
	free(lang->translations);
}

static void
genlang_lang_currently_translating(struct genlang_lang *lang) {
	const char **current = genlang_keys, ** const end = genlang_keys + sizeof(genlang_keys) / sizeof(*genlang_keys);

	while(current != end && strcmp(lang->string.content, *current) != 0) {
		current++;
	}

	const unsigned int index = current - genlang_keys;

	if(index == STRING_LAST) {
		warnx("Invalid key for translation: %s", lang->string.content);
		lang->current_translation = NULL;
	} else {
		lang->current_translation = lang->translations + index;
	}
}

static void
genlang_lang_feed(struct genlang_lang *lang, const char *buffer, size_t count) {
	while(count != 0) {
		switch(lang->state) {
		case GENLANG_STATE_WAIT_KEY:
			while(count != 0 && isspace(*buffer)) {
				buffer++; count--;
			}
			if(count != 0) {
				lang->state = GENLANG_STATE_PARSE_KEY;
			}
			break;
		case GENLANG_STATE_PARSE_KEY:
			while(count != 0 && (isalnum(*buffer) || *buffer == '_')) {
				genlang_string_push(&lang->string, *buffer);
				buffer++; count--;
			}
			if(count != 0) {
				lang->state = GENLANG_STATE_WAIT_EQUAL;
				genlang_lang_currently_translating(lang);
				genlang_string_empty(&lang->string);
			}
			break;
		case GENLANG_STATE_WAIT_EQUAL:
			while(count != 0 && isspace(*buffer)) {
				buffer++; count--;
			}
			if(count != 0) {
				if(*buffer != '=') {
					errx(EXIT_FAILURE, "Invalid character after key, expected '=', found '%c'\n", *buffer);
				}
				lang->state = GENLANG_STATE_WAIT_QUOTE;
				buffer++; count--;
			}
			break;
		case GENLANG_STATE_WAIT_QUOTE:
			while(count != 0 && isspace(*buffer)) {
				buffer++; count--;
			}
			if(count != 0) {
				if(*buffer != '"') {
					errx(EXIT_FAILURE, "Invalid character after equal, expected '\"', found '%c'\n", *buffer);
				}
				lang->state = GENLANG_STATE_PARSE_VALUE;
				buffer++; count--;
			}
			break;
		case GENLANG_STATE_PARSE_VALUE:
			while(count != 0 && (*buffer != '\\' && *buffer != '"')) {
				genlang_string_push(&lang->string, *buffer);
				buffer++; count--;
			}
			if(count != 0) {
				if(*buffer == '"') {
					if(lang->current_translation != NULL) {
						free(*lang->current_translation);
						*lang->current_translation = strdup(lang->string.content);
					}
					lang->state = GENLANG_STATE_WAIT_KEY;
					genlang_string_empty(&lang->string);
				} else { /* *buffer == '\\' */
					lang->state = GENLANG_STATE_ESCAPING_VALUE;
				}
				buffer++; count--;
			}
			break;
		case GENLANG_STATE_ESCAPING_VALUE: {
			char pushed;
			switch(*buffer) {
			case 'n':
				pushed = '\n';
				break;
			default:
				pushed = *buffer;
				break;
			}
			genlang_string_push(&lang->string, pushed);
			lang->state = GENLANG_STATE_PARSE_VALUE;
			buffer++; count--;
		}	break;
		}
	}
}

static void
genlang_lang_parse_file(struct genlang_lang *lang, const char *filename) {
	const int fd = open(filename, O_RDONLY);

	if(fd < 0) {
		err(EXIT_FAILURE, "genlang_lang_parse_file: open %s", filename);
	}

	ssize_t readval;
	char buffer[lang->pagesize];
	while(readval = read(fd, buffer, sizeof(buffer)), readval > 0) {
		genlang_lang_feed(lang, buffer, readval);
	}

	if(readval != 0) {
		err(EXIT_FAILURE, "genlang_lang_parse_file: read %s", filename);
	}

	close(fd);
}

static void
genlang_lang_generate(const struct genlang_lang *lang, const char *filename) {
	FILE *output = fopen(filename, "w");

	for(unsigned i = 0; i < STRING_LAST; i++) {
		const char *translation = lang->translations[i];

		if(translation == NULL) {
			translation = genlang_keys[i];
			warnx("Missing translation for key '%s'", translation);
		}

		fputc('"', output);

		while(*translation != '\0') {
			switch(*translation) {
			case '\n':
				fputs("\\n", output);
				break;
			case '\\':
				/* fallthrough */
			case '"':
				fputc('\\', output);
				/* fallthrough */
			default:
				fputc(*translation, output);
				break;
			}
			translation++;
		}

		fputs("\",\n", output);
	}

	fclose(output);
}


static void
genlang_usage(const char *genlang_name) {
	fprintf(stderr, "usage: %s -o <output file> <lang file>...\n", genlang_name);
	exit(EXIT_FAILURE);
}

static const struct genlang_args
genlang_parse_args(int argc, char **argv) {
	struct genlang_args args = {
		.output = NULL,
	};
	int c;

	while((c = getopt(argc, argv, ":o:")) != -1) {
		switch(c) {
		case 'o':
			args.output = optarg;
			break;
		case ':':
			warnx("-%c: Missing argument", optopt);
			genlang_usage(*argv);
		default:
			warnx("Unknown argument -%c", optopt);
			genlang_usage(*argv);
		}
	}

	if(args.output == NULL) {
		warnx("Missing output file");
		genlang_usage(*argv);
	}

	if(argc - optind == 0) {
		genlang_usage(*argv);
	}

	return args;
}

int
main(int argc, char **argv) {
	const struct genlang_args args = genlang_parse_args(argc, argv);
	char **current = argv + optind, ** const end = argv + argc;
	struct genlang_lang lang;

	genlang_lang_init(&lang);

	while(current != end) {
		genlang_lang_parse_file(&lang, *current);
		current++;
	}

	genlang_lang_generate(&lang, args.output);

	genlang_lang_deinit(&lang);

	return EXIT_SUCCESS;
}
