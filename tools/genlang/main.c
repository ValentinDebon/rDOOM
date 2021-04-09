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
	"BLUEO",
	"REDO",
	"YELLOWO",
	"BLUEK",
	"REDK",
	"YELLOWK",
	"SAVED",
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
	"HU_1",
	"HU_2",
	"HU_3",
	"HU_4",
	"HU_5",
	"HU_6",
	"HU_7",
	"HU_8",
	"HU_9",
	"HU_10",
	"HU_11",
	"HU_12",
	"HU_13",
	"HU_14",
	"HU_15",
	"HU_16",
	"HU_17",
	"HU_18",
	"HU_19",
	"HU_20",
	"HU_21",
	"HU_22",
	"HU_23",
	"HU_24",
	"HU_25",
	"HU_26",
	"HU_27",
	"HU_28",
	"HU_29",
	"HU_30",
	"HU_31",
	"HU_32",
	"PHU_1",
	"PHU_2",
	"PHU_3",
	"PHU_4",
	"PHU_5",
	"PHU_6",
	"PHU_7",
	"PHU_8",
	"PHU_9",
	"PHU_10",
	"PHU_11",
	"PHU_12",
	"PHU_13",
	"PHU_14",
	"PHU_15",
	"PHU_16",
	"PHU_17",
	"PHU_18",
	"PHU_19",
	"PHU_20",
	"PHU_21",
	"PHU_22",
	"PHU_23",
	"PHU_24",
	"PHU_25",
	"PHU_26",
	"PHU_27",
	"PHU_28",
	"PHU_29",
	"PHU_30",
	"PHU_31",
	"PHU_32",
	"THU_1",
	"THU_2",
	"THU_3",
	"THU_4",
	"THU_5",
	"THU_6",
	"THU_7",
	"THU_8",
	"THU_9",
	"THU_10",
	"THU_11",
	"THU_12",
	"THU_13",
	"THU_14",
	"THU_15",
	"THU_16",
	"THU_17",
	"THU_18",
	"THU_19",
	"THU_20",
	"THU_21",
	"THU_22",
	"THU_23",
	"THU_24",
	"THU_25",
	"THU_26",
	"THU_27",
	"THU_28",
	"THU_29",
	"THU_30",
	"THU_31",
	"THU_32",
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
