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
// DESCRIPTION:  Heads-up displays
//
//-----------------------------------------------------------------------------

#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"

#include "l_strings.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "w_wad.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#define HU_INPUTTOGGLE 't'

/* Chat macros are filled in by m_misc in M_LoadDefaults */
char *chat_macros[10];

char chat_char; // remove later.
static player_t *plr;
const patch_t *hu_font[HU_FONTSIZE];
static hu_textline_t w_title;
bool chat_on;
static hu_itext_t w_chat;
static bool always_off = false;
static char chat_dest[MAXPLAYERS];
static hu_itext_t w_inputbuffer[MAXPLAYERS];

static bool message_on;
bool message_dontfuckwithme;
static bool message_nottobefuckedwith;

static hu_stext_t w_message;
static int message_counter;

extern int showMessages;
extern bool automapactive;

static bool headsupactive = false;

const char *shiftxform;

const char french_shiftxform[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	' ',
	'!',
	'"',
	'#',
	'$',
	'%',
	'&',
	'"', // shift-'
	'(',
	')',
	'*',
	'+',
	'?', // shift-,
	'_', // shift--
	'>', // shift-.
	'?', // shift-/
	'0', // shift-0
	'1', // shift-1
	'2', // shift-2
	'3', // shift-3
	'4', // shift-4
	'5', // shift-5
	'6', // shift-6
	'7', // shift-7
	'8', // shift-8
	'9', // shift-9
	'/',
	'.', // shift-;
	'<',
	'+', // shift-=
	'>',
	'?',
	'@',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'[', // shift-[
	'!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
	']', // shift-]
	'"',
	'_',
	'\'', // shift-`
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'{',
	'|',
	'}',
	'~',
	127

};

const char english_shiftxform[] = {

	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	' ',
	'!',
	'"',
	'#',
	'$',
	'%',
	'&',
	'"', // shift-'
	'(',
	')',
	'*',
	'+',
	'<', // shift-,
	'_', // shift--
	'>', // shift-.
	'?', // shift-/
	')', // shift-0
	'!', // shift-1
	'@', // shift-2
	'#', // shift-3
	'$', // shift-4
	'%', // shift-5
	'^', // shift-6
	'&', // shift-7
	'*', // shift-8
	'(', // shift-9
	':',
	':', // shift-;
	'<',
	'+', // shift-=
	'>',
	'?',
	'@',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'[', // shift-[
	'!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
	']', // shift-]
	'"',
	'_',
	'\'', // shift-`
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'{',
	'|',
	'}',
	'~',
	127
};

char frenchKeyMap[128] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	' ',
	'!',
	'"',
	'#',
	'$',
	'%',
	'&',
	'%',
	'(',
	')',
	'*',
	'+',
	';',
	'-',
	':',
	'!',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	':',
	'M',
	'<',
	'=',
	'>',
	'?',
	'@',
	'Q',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	',',
	'N',
	'O',
	'P',
	'A',
	'R',
	'S',
	'T',
	'U',
	'V',
	'Z',
	'X',
	'Y',
	'W',
	'^',
	'\\',
	'$',
	'^',
	'_',
	'@',
	'Q',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	',',
	'N',
	'O',
	'P',
	'A',
	'R',
	'S',
	'T',
	'U',
	'V',
	'Z',
	'X',
	'Y',
	'W',
	'^',
	'\\',
	'$',
	'^',
	127
};

char
ForeignTranslation(unsigned char ch) {
	return ch < 128 ? frenchKeyMap[ch] : ch;
}

void
HU_Init(void) {

	int i;
	int j;
	char buffer[9];

	if(L_CurrentLanguage() == LANGUAGE_FRENCH)
		shiftxform = french_shiftxform;
	else
		shiftxform = english_shiftxform;

	// load the heads-up font
	j = HU_FONTSTART;
	for(i = 0; i < HU_FONTSIZE; i++) {
		sprintf(buffer, "STCFN%.3d", j++);
		hu_font[i] = W_LumpForName(buffer)->data;
	}
}

void
HU_Stop(void) {
	headsupactive = false;
}

void
HU_Start(void) {

	if(headsupactive) {
		HU_Stop();
	}

	plr                       = &players[consoleplayer];
	message_on                = false;
	message_dontfuckwithme    = false;
	message_nottobefuckedwith = false;
	chat_on                   = false;

	// create the message widget
	HUlib_initSText(&w_message,
		HU_MSGX,
		HU_MSGY,
		HU_MSGHEIGHT,
		hu_font,
		HU_FONTSTART,
		&message_on);

	// create the map title widget
	HUlib_initTextLine(&w_title,
		0,
		167 - LE_U16(hu_font[0]->height),
		hu_font,
		HU_FONTSTART);

	const char *title;
	switch(gamemode) {
	case shareware:
	case registered:
	case retail:
		title = L_String(STRING_HU_E1M1 + (gameepisode - 1) * 9 + gamemap - 1);
		break;
	/* FIXME
	case pack_plut:
		title = L_String(STRING_HU_PLUTONIAMAP1 + gamemap - 1);
		break;
	case pack_tnt:
		title = L_String(STRING_HU_TNTMAP1 + gamemap - 1);
		break;
	*/
	case commercial:
	default:
		title = L_String(STRING_HU_DOOM2MAP1 + gamemap - 1);
		break;
	}

	while(*title != '\0') {
		HUlib_addCharToTextLine(&w_title, *title);
		title++;
	}

	// create the chat widget
	HUlib_initIText(&w_chat,
		HU_MSGX,
		HU_MSGY + HU_MSGHEIGHT * (LE_U16(hu_font[0]->height) + 1),
		hu_font,
		HU_FONTSTART,
		&chat_on);

	// create the inputbuffer widgets
	for(int i = 0; i < MAXPLAYERS; i++) {
		HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);
	}

	headsupactive = true;
}

void
HU_Drawer(void) {

	HUlib_drawSText(&w_message);
	HUlib_drawIText(&w_chat);
	if(automapactive)
		HUlib_drawTextLine(&w_title, false);
}

void
HU_Erase(void) {

	HUlib_eraseSText(&w_message);
	HUlib_eraseIText(&w_chat);
	HUlib_eraseTextLine(&w_title);
}

void
HU_Ticker(void) {

	int i, rc;
	char c;

	// tick down message counter if message is up
	if(message_counter && !--message_counter) {
		message_on                = false;
		message_nottobefuckedwith = false;
	}

	if(showMessages || message_dontfuckwithme) {

		// display message if necessary
		if((plr->message && !message_nottobefuckedwith)
			|| (plr->message && message_dontfuckwithme)) {
			HUlib_addMessageToSText(&w_message, 0, plr->message);
			plr->message              = 0;
			message_on                = true;
			message_counter           = HU_MSGTIMEOUT;
			message_nottobefuckedwith = message_dontfuckwithme;
			message_dontfuckwithme    = 0;
		}

	} // else message_on = false;

	// check for incoming chat characters
	if(netgame) {
		for(i = 0; i < MAXPLAYERS; i++) {
			if(!playeringame[i])
				continue;
			if(i != consoleplayer
				&& (c = players[i].cmd.chatchar)) {
				if(c <= HU_BROADCAST)
					chat_dest[i] = c;
				else {
					if(c >= 'a' && c <= 'z')
						c = (char)shiftxform[(unsigned char)c];
					rc = HUlib_keyInIText(&w_inputbuffer[i], c);
					if(rc && c == KEY_ENTER) {
						if(w_inputbuffer[i].l.len
							&& (chat_dest[i] == consoleplayer + 1
								|| chat_dest[i] == HU_BROADCAST)) {
							HUlib_addMessageToSText(&w_message,
								L_String(STRING_HU_PLRGREEN + i),
								w_inputbuffer[i].l.l);

							message_nottobefuckedwith = true;
							message_on                = true;
							message_counter           = HU_MSGTIMEOUT;
							if(gamemode == commercial)
								S_StartSound(0, sfx_radio);
							else
								S_StartSound(0, sfx_tink);
						}
						HUlib_resetIText(&w_inputbuffer[i]);
					}
				}
				players[i].cmd.chatchar = 0;
			}
		}
	}
}

#define QUEUESIZE 128

static char chatchars[QUEUESIZE];
static int head = 0;
static int tail = 0;

void
HU_queueChatChar(char c) {
	if(((head + 1) & (QUEUESIZE - 1)) == tail) {
		plr->message = L_String(STRING_HU_MSGU);
	} else {
		chatchars[head] = c;
		head            = (head + 1) & (QUEUESIZE - 1);
	}
}

char
HU_dequeueChatChar(void) {
	char c;

	if(head != tail) {
		c    = chatchars[tail];
		tail = (tail + 1) & (QUEUESIZE - 1);
	} else {
		c = 0;
	}

	return c;
}

bool
HU_Responder(event_t *ev) {

	static char lastmessage[HU_MAXLINELENGTH + 1];
	const char *macromessage;
	bool eatkey           = false;
	static bool shiftdown = false;
	static bool altdown   = false;
	unsigned char c;
	int i;
	int numplayers;

	static int num_nobrainers = 0;

	numplayers = 0;
	for(i = 0; i < MAXPLAYERS; i++)
		numplayers += playeringame[i];

	if(ev->data1 == KEY_RSHIFT) {
		shiftdown = ev->type == ev_keydown;
		return false;
	} else if(ev->data1 == KEY_RALT || ev->data1 == KEY_LALT) {
		altdown = ev->type == ev_keydown;
		return false;
	}

	if(ev->type != ev_keydown)
		return false;

	if(!chat_on) {
		if(ev->data1 == HU_MSGREFRESH) {
			message_on      = true;
			message_counter = HU_MSGTIMEOUT;
			eatkey          = true;
		} else if(netgame && ev->data1 == HU_INPUTTOGGLE) {
			eatkey = chat_on = true;
			HUlib_resetIText(&w_chat);
			HU_queueChatChar(HU_BROADCAST);
		} else if(netgame && numplayers > 2) {
			for(i = 0; i < MAXPLAYERS; i++) {
				if(ev->data1 == *L_String(STRING_HU_KEYGREEN + i)) {
					if(playeringame[i] && i != consoleplayer) {
						eatkey = chat_on = true;
						HUlib_resetIText(&w_chat);
						HU_queueChatChar(i + 1);
						break;
					} else if(i == consoleplayer) {
						num_nobrainers++;
						if(num_nobrainers < 3)
							plr->message = L_String(STRING_HU_TALKTOSELF1);
						else if(num_nobrainers < 6)
							plr->message = L_String(STRING_HU_TALKTOSELF2);
						else if(num_nobrainers < 9)
							plr->message = L_String(STRING_HU_TALKTOSELF3);
						else if(num_nobrainers < 32)
							plr->message = L_String(STRING_HU_TALKTOSELF4);
						else
							plr->message = L_String(STRING_HU_TALKTOSELF5);
					}
				}
			}
		}
	} else {
		c = ev->data1;
		// send a macro
		if(altdown) {
			c = c - '0';
			if(c > 9)
				return false;
			// fprintf(stderr, "got here\n");
			macromessage = chat_macros[c];

			// kill last message with a '\n'
			HU_queueChatChar(KEY_ENTER); // DEBUG!!!

			// send the macro message
			while(*macromessage)
				HU_queueChatChar(*macromessage++);
			HU_queueChatChar(KEY_ENTER);

			// leave chat mode and notify that it was sent
			chat_on = false;
			strcpy(lastmessage, chat_macros[c]);
			plr->message = lastmessage;
			eatkey       = true;
		} else {
			if(L_CurrentLanguage() == LANGUAGE_FRENCH)
				c = ForeignTranslation(c);
			if(shiftdown || (c >= 'a' && c <= 'z'))
				c = shiftxform[c];
			eatkey = HUlib_keyInIText(&w_chat, c);
			if(eatkey) {
				// static unsigned char buf[20]; // DEBUG
				HU_queueChatChar(c);

				// sprintf(buf, "KEY: %d => %d", ev->data1, c);
				//      plr->message = buf;
			}
			if(c == KEY_ENTER) {
				chat_on = false;
				if(w_chat.l.len) {
					strcpy(lastmessage, w_chat.l.l);
					plr->message = lastmessage;
				}
			} else if(c == KEY_ESCAPE)
				chat_on = false;
		}
	}

	return eatkey;
}
