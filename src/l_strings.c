//-----------------------------------------------------------------------------
//
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
//	Localized strings.
//
//-----------------------------------------------------------------------------

#include "l_strings.h"

#include <assert.h>

static struct {
	const char * const localized[LANGUAGE_LAST][STRING_LAST];
	const char * const *current;
} l_localization = {
	.localized = {
		{ /* LANGUAGE_ENGLISH */

			/* d_main.c */
			"Development mode ON.\n",
			"CD-ROM Version: default.cfg from c:\\doomdata\n",

			/* m_menu.c */
			"press a key.",
			"press y or n.",
			"are you sure you want to\nquit this great game?",
			"you can't do load while in a net game!\n\npress a key.",
			"you can't quickload during a netgame!\n\npress a key.",
			"you haven't picked a quicksave slot yet!\n\npress a key.",
			"you can't save if you aren't playing!\n\npress a key.",
			"quicksave over your game named\n\n'%s'?\n\npress y or n.",
			"do you want to quickload the game named\n\n'%s'?\n\npress y or n.",
			"you can't start a new game\nwhile in a network game.\n\npress a key.",
			"are you sure? this skill level\nisn't even remotely fair.\n\npress y or n.",
			"this is the shareware version of doom.\n\nyou need to order the entire trilogy.\n\npress a key.",
			"Messages OFF",
			"Messages ON",
			"you can't end a netgame!\n\npress a key.",
			"are you sure you want to end the game?\n\npress a key.",
			"(press y to quit)",
			"High detail",
			"Low detail",
			"Gamma correction OFF",
			"Gamma correction level 1",
			"Gamma correction level 2",
			"Gamma correction level 3",
			"Gamma correction level 4",
			"empty slot",

			/* p_inter.c */
			"Picked up the armor.",
			"Picked up the MegaArmor!",
			"Picked up a health bonus.",
			"Picked up an armor bonus.",
			"Picked up a stimpack.",
			"Picked up a medikit that you REALLY need!",
			"Picked up a medikit.",
			"Supercharge!",
			"Picked up a blue keycard.",
			"Picked up a yellow keycard.",
			"Picked up a red keycard.",
			"Picked up a blue skull key.",
			"Picked up a yellow skull key.",
			"Picked up a red skull key.",
			"Invulnerability!",
			"Berserk!",
			"Partial Invisibility",
			"Radiation Shielding Suit",
			"Computer Area Map",
			"Light Amplification Visor",
			"MegaSphere!",
			"Picked up a clip.",
			"Picked up a box of bullets.",
			"Picked up a rocket.",
			"Picked up a box of rockets.",
			"Picked up an energy cell.",
			"Picked up an energy cell pack.",
			"Picked up 4 shotgun shells.",
			"Picked up a box of shotgun shells.",
			"Picked up a backpack full of ammo!",
			"You got the BFG9000!  Oh, yes.",
			"You got the chaingun!",
			"A chainsaw!  Find some meat!",
			"You got the rocket launcher!",
			"You got the plasma gun!",
			"You got the shotgun!",
			"You got the super shotgun!",

			/* p_doors.c */
			"You need a blue key to activate this object",
			"You need a red key to activate this object",
			"You need a yellow key to activate this object",
			"You need a blue key to open this door",
			"You need a red key to open this door",
			"You need a yellow key to open this door",

			/* g_game.c */
			"game saved.",

			/* hu_stuff.c */
			"[Message unsent]",
			"E1M1: Hangar",
			"E1M2: Nuclear Plant",
			"E1M3: Toxin Refinery",
			"E1M4: Command Control",
			"E1M5: Phobos Lab",
			"E1M6: Central Processing",
			"E1M7: Computer Station",
			"E1M8: Phobos Anomaly",
			"E1M9: Military Base",
			"E2M1: Deimos Anomaly",
			"E2M2: Containment Area",
			"E2M3: Refinery",
			"E2M4: Deimos Lab",
			"E2M5: Command Center",
			"E2M6: Halls of the Damned",
			"E2M7: Spawning Vats",
			"E2M8: Tower of Babel",
			"E2M9: Fortress of Mystery",
			"E3M1: Hell Keep",
			"E3M2: Slough of Despair",
			"E3M3: Pandemonium",
			"E3M4: House of Pain",
			"E3M5: Unholy Cathedral",
			"E3M6: Mt. Erebus",
			"E3M7: Limbo",
			"E3M8: Dis",
			"E3M9: Warrens",
			"E4M1: Hell Beneath",
			"E4M2: Perfect Hatred",
			"E4M3: Sever The Wicked",
			"E4M4: Unruly Evil",
			"E4M5: They Will Repent",
			"E4M6: Against Thee Wickedly",
			"E4M7: And Hell Followed",
			"E4M8: Unto The Cruel",
			"E4M9: Fear",
			"level 1: entryway",
			"level 2: underhalls",
			"level 3: the gantlet",
			"level 4: the focus",
			"level 5: the waste tunnels",
			"level 6: the crusher",
			"level 7: dead simple",
			"level 8: tricks and traps",
			"level 9: the pit",
			"level 10: refueling base",
			"level 11: 'o' of destruction!",
			"level 12: the factory",
			"level 13: downtown",
			"level 14: the inmost dens",
			"level 15: industrial zone",
			"level 16: suburbs",
			"level 17: tenements",
			"level 18: the courtyard",
			"level 19: the citadel",
			"level 20: gotcha!",
			"level 21: nirvana",
			"level 22: the catacombs",
			"level 23: barrels o' fun",
			"level 24: the chasm",
			"level 25: bloodfalls",
			"level 26: the abandoned mines",
			"level 27: monster condo",
			"level 28: the spirit world",
			"level 29: the living end",
			"level 30: icon of sin",
			"level 31: wolfenstein",
			"level 32: grosse",
			"level 1: congo",
			"level 2: well of souls",
			"level 3: aztec",
			"level 4: caged",
			"level 5: ghost town",
			"level 6: baron's lair",
			"level 7: caughtyard",
			"level 8: realm",
			"level 9: abattoire",
			"level 10: onslaught",
			"level 11: hunted",
			"level 12: speed",
			"level 13: the crypt",
			"level 14: genesis",
			"level 15: the twilight",
			"level 16: the omen",
			"level 17: compound",
			"level 18: neurosphere",
			"level 19: nme",
			"level 20: the death domain",
			"level 21: slayer",
			"level 22: impossible mission",
			"level 23: tombstone",
			"level 24: the final frontier",
			"level 25: the temple of darkness",
			"level 26: bunker",
			"level 27: anti-christ",
			"level 28: the sewers",
			"level 29: odyssey of noises",
			"level 30: the gateway of hell",
			"level 31: cyberden",
			"level 32: go 2 it",
			"level 1: system control",
			"level 2: human bbq",
			"level 3: power control",
			"level 4: wormhole",
			"level 5: hanger",
			"level 6: open season",
			"level 7: prison",
			"level 8: metal",
			"level 9: stronghold",
			"level 10: redemption",
			"level 11: storage facility",
			"level 12: crater",
			"level 13: nukage processing",
			"level 14: steel works",
			"level 15: dead zone",
			"level 16: deepest reaches",
			"level 17: processing area",
			"level 18: mill",
			"level 19: shipping/respawning",
			"level 20: central processing",
			"level 21: administration center",
			"level 22: habitat",
			"level 23: lunar mining project",
			"level 24: quarry",
			"level 25: baron's den",
			"level 26: ballistyx",
			"level 27: mount pain",
			"level 28: heck",
			"level 29: river styx",
			"level 30: last call",
			"level 31: pharaoh",
			"level 32: caribbean",
			"I'm ready to kick butt!",
			"I'm OK.",
			"I'm not looking too good!",
			"Help!",
			"You suck!",
			"Next time, scumbag...",
			"Come here!",
			"I'll take care of it.",
			"Yes",
			"No",
			"You mumble to yourself",
			"Who's there?",
			"You scare yourself",
			"You start to rave",
			"You've lost it...",
			"[Message Sent]",
			"Green: ",
			"Indigo: ",
			"Brown: ",
			"Red: ",
			"g",
			"i",
			"b",
			"r",

			/* am_map.c */
			"Follow Mode ON",
			"Follow Mode OFF",
			"Grid ON",
			"Grid OFF",
			"Marked Spot",
			"All Marks Cleared",

			/* st_stuff.c */
			"Music Change",
			"IMPOSSIBLE SELECTION",
			"Degreelessness Mode On",
			"Degreelessness Mode Off",
			"Very Happy Ammo Added",
			"Ammo (no keys) Added",
			"No Clipping Mode ON",
			"No Clipping Mode OFF",
			"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp",
			"Power-up Toggled",
			"... doesn't suck - GM",
			"Changing Level...",

			/* f_finale.c */
			"Once you beat the big badasses and\n"
			"clean out the moon base you're supposed\n"
			"to win, aren't you? Aren't you? Where's\n"
			"your fat reward and ticket home? What\n"
			"the hell is this? It's not supposed to\n"
			"end this way!\n"
			"\n"
			"It stinks like rotten meat, but looks\n"
			"like the lost Deimos base.  Looks like\n"
			"you're stuck on The Shores of Hell.\n"
			"The only way out is through.\n"
			"\n"
			"To continue the DOOM experience, play\n"
			"The Shores of Hell and its amazing\n"
			"sequel, Inferno!\n",

			"You've done it! The hideous cyber-\n"
			"demon lord that ruled the lost Deimos\n"
			"moon base has been slain and you\n"
			"are triumphant! But ... where are\n"
			"you? You clamber to the edge of the\n"
			"moon and look down to see the awful\n"
			"truth.\n"
			"\n"
			"Deimos floats above Hell itself!\n"
			"You've never heard of anyone escaping\n"
			"from Hell, but you'll make the bastards\n"
			"sorry they ever heard of you! Quickly,\n"
			"you rappel down to  the surface of\n"
			"Hell.\n"
			"\n"
			"Now, it's on to the final chapter of\n"
			"DOOM! -- Inferno.",

			"The loathsome spiderdemon that\n"
			"masterminded the invasion of the moon\n"
			"bases and caused so much death has had\n"
			"its ass kicked for all time.\n"
			"\n"
			"A hidden doorway opens and you enter.\n"
			"You've proven too tough for Hell to\n"
			"contain, and now Hell at last plays\n"
			"fair -- for you emerge from the door\n"
			"to see the green fields of Earth!\n"
			"Home at last.\n"
			"\n"
			"You wonder what's been happening on\n"
			"Earth while you were battling evil\n"
			"unleashed. It's good that no Hell-\n"
			"spawn could have come through that\n"
			"door with you ...",

			"the spider mastermind must have sent forth\n"
			"its legions of hellspawn before your\n"
			"final confrontation with that terrible\n"
			"beast from hell.  but you stepped forward\n"
			"and brought forth eternal damnation and\n"
			"suffering upon the horde as a true hero\n"
			"would in the face of something so evil.\n"
			"\n"
			"besides, someone was gonna pay for what\n"
			"happened to daisy, your pet rabbit.\n"
			"\n"
			"but now, you see spread before you more\n"
			"potential pain and gibbitude as a nation\n"
			"of demons run amok among our cities.\n"
			"\n"
			"next stop, hell on earth!",

			"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n"
			"STARPORT. BUT SOMETHING IS WRONG. THE\n"
			"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n"
			"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n"
			"IS BEING SUBVERTED BY THEIR PRESENCE.\n"
			"\n"
			"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n"
			"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n"
			"YOU CAN PENETRATE INTO THE HAUNTED HEART\n"
			"OF THE STARBASE AND FIND THE CONTROLLING\n"
			"SWITCH WHICH HOLDS EARTH'S POPULATION\n"
			"HOSTAGE.",

			"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n"
			"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"
			"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"
			"HUMAN LEFT ON THE FACE OF THE PLANET.\n"
			"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"
			"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"
			"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"
			"THAT YOU HAVE SAVED YOUR SPECIES.\n"
			"\n"
			"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"
			"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"
			"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"
			"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"
			"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"
			"YOUR OWN HOME CITY, NOT FAR FROM THE\n"
			"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"
			"UP AND RETURN TO THE FRAY.",

			"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
			"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
			"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
			"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
			"TEETH AND PLUNGE THROUGH IT.\n"
			"\n"
			"THERE MUST BE A WAY TO CLOSE IT ON THE\n"
			"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"
			"GOT TO GO THROUGH HELL TO GET TO IT?",

			"THE HORRENDOUS VISAGE OF THE BIGGEST\n"
			"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"
			"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"
			"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"
			"UP AND DIES, ITS THRASHING LIMBS\n"
			"DEVASTATING UNTOLD MILES OF HELL'S\n"
			"SURFACE.\n"
			"\n"
			"YOU'VE DONE IT. THE INVASION IS OVER.\n"
			"EARTH IS SAVED. HELL IS A WRECK. YOU\n"
			"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"
			"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"
			"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"
			"HOME. REBUILDING EARTH OUGHT TO BE A\n"
			"LOT MORE FUN THAN RUINING IT WAS.\n",

			"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
			"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"
			"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"
			"WHO THE INMATES OF THIS CORNER OF HELL\n"
			"WILL BE.",

			"CONGRATULATIONS, YOU'VE FOUND THE\n"
			"SUPER SECRET LEVEL!  YOU'D BETTER\n"
			"BLAZE THROUGH THIS ONE!\n",

			"You gloat over the steaming carcass of the\n"
			"Guardian.  With its death, you've wrested\n"
			"the Accelerator from the stinking claws\n"
			"of Hell.  You relax and glance around the\n"
			"room.  Damn!  There was supposed to be at\n"
			"least one working prototype, but you can't\n"
			"see it. The demons must have taken it.\n"
			"\n"
			"You must find the prototype, or all your\n"
			"struggles will have been wasted. Keep\n"
			"moving, keep fighting, keep killing.\n"
			"Oh yes, keep living, too.",

			"Even the deadly Arch-Vile labyrinth could\n"
			"not stop you, and you've gotten to the\n"
			"prototype Accelerator which is soon\n"
			"efficiently and permanently deactivated.\n"
			"\n"
			"You're good at that kind of thing.",

			"You've bashed and battered your way into\n"
			"the heart of the devil-hive.  Time for a\n"
			"Search-and-Destroy mission, aimed at the\n"
			"Gatekeeper, whose foul offspring is\n"
			"cascading to Earth.  Yeah, he's bad. But\n"
			"you know who's worse!\n"
			"\n"
			"Grinning evilly, you check your gear, and\n"
			"get ready to give the bastard a little Hell\n"
			"of your own making!",

			"The Gatekeeper's evil face is splattered\n"
			"all over the place.  As its tattered corpse\n"
			"collapses, an inverted Gate forms and\n"
			"sucks down the shards of the last\n"
			"prototype Accelerator, not to mention the\n"
			"few remaining demons.  You're done. Hell\n"
			"has gone back to pounding bad dead folks \n"
			"instead of good live ones.  Remember to\n"
			"tell your grandkids to put a rocket\n"
			"launcher in your coffin. If you go to Hell\n"
			"when you die, you'll need it for some\n"
			"final cleaning-up ...",

			"You've found the second-hardest level we\n"
			"got. Hope you have a saved game a level or\n"
			"two previous.  If not, be prepared to die\n"
			"aplenty. For master marines only.",

			"Betcha wondered just what WAS the hardest\n"
			"level we had ready for ya?  Now you know.\n"
			"No one gets out alive.",

			"You've fought your way out of the infested\n"
			"experimental labs.   It seems that UAC has\n"
			"once again gulped it down.  With their\n"
			"high turnover, it must be hard for poor\n"
			"old UAC to buy corporate health insurance\n"
			"nowadays..\n"
			"\n"
			"Ahead lies the military complex, now\n"
			"swarming with diseased horrors hot to get\n"
			"their teeth into you. With luck, the\n"
			"complex still has some warlike ordnance\n"
			"laying around.",

			"You hear the grinding of heavy machinery\n"
			"ahead.  You sure hope they're not stamping\n"
			"out new hellspawn, but you're ready to\n"
			"ream out a whole herd if you have to.\n"
			"They might be planning a blood feast, but\n"
			"you feel about as mean as two thousand\n"
			"maniacs packed into one mad killer.\n"
			"\n"
			"You don't plan to go down easy.",

			"The vista opening ahead looks real damn\n"
			"familiar. Smells familiar, too -- like\n"
			"fried excrement. You didn't like this\n"
			"place before, and you sure as hell ain't\n"
			"planning to like it now. The more you\n"
			"brood on it, the madder you get.\n"
			"Hefting your gun, an evil grin trickles\n"
			"onto your face. Time to take some names.",

			"Suddenly, all is silent, from one horizon\n"
			"to the other. The agonizing echo of Hell\n"
			"fades away, the nightmare sky turns to\n"
			"blue, the heaps of monster corpses start \n"
			"to evaporate along with the evil stench \n"
			"that filled the air. Jeeze, maybe you've\n"
			"done it. Have you really won?\n"
			"\n"
			"Something rumbles in the distance.\n"
			"A blue light begins to glow inside the\n"
			"ruined skull of the demon-spitter.",

			"What now? Looks totally different. Kind\n"
			"of like King Tut's condo. Well,\n"
			"whatever's here can't be any worse\n"
			"than usual. Can it?  Or maybe it's best\n"
			"to let sleeping gods lie..",

			"Time for a vacation. You've burst the\n"
			"bowels of hell and by golly you're ready\n"
			"for a break. You mutter to yourself,\n"
			"Maybe someone else can kick Hell's ass\n"
			"next time around. Ahead lies a quiet town,\n"
			"with peaceful flowing water, quaint\n"
			"buildings, and presumably no Hellspawn.\n"
			"\n"
			"As you step off the transport, you hear\n"
			"the stomp of a cyberdemon's iron shoe.",

			"ZOMBIEMAN",
			"SHOTGUN GUY",
			"HEAVY WEAPON DUDE",
			"IMP",
			"DEMON",
			"LOST SOUL",
			"CACODEMON",
			"HELL KNIGHT",
			"BARON OF HELL",
			"ARACHNOTRON",
			"PAIN ELEMENTAL",
			"REVENANT",
			"MANCUBUS",
			"ARCH-VILE",
			"THE SPIDER MASTERMIND",
			"THE CYBERDEMON",
			"OUR HERO",
		},
		{ /* LANGUAGE_FRENCH */

			/* d_main.c */
			"MODE DEVELOPPEMENT ON.\n",
			"VERSION CD-ROM: DEFAULT.CFG DANS C:\\DOOMDATA\n",

			/* m_menu.c */
			"APPUYEZ SUR UNE TOUCHE.",
			"APPUYEZ SUR Y OU N",
			"VOUS VOULEZ VRAIMENT\nQUITTER CE SUPER JEU?",
			"VOUS NE POUVEZ PAS CHARGER\nUN JEU EN RESEAU!\n\nAPPUYEZ SUR UNE TOUCHE.",
			"CHARGEMENT RAPIDE INTERDIT EN RESEAU!\n\nAPPUYEZ SUR UNE TOUCHE.",
			"VOUS N'AVEZ PAS CHOISI UN EMPLACEMENT!\n\nAPPUYEZ SUR UNE TOUCHE.",
			"VOUS NE POUVEZ PAS SAUVER SI VOUS NE JOUEZ PAS!\n\nAPPUYEZ SUR UNE TOUCHE.",
			"SAUVEGARDE RAPIDE DANS LE FICHIER \n\n'%s'?\n\nAPPUYEZ SUR Y OU N",
			"VOULEZ-VOUS CHARGER LA SAUVEGARDE\n\n'%s'?\n\nAPPUYEZ SUR Y OU N",
			"VOUS NE POUVEZ PAS LANCER\nUN NOUVEAU JEU SUR RESEAU.\n\nAPPUYEZ SUR UNE TOUCHE.",
			"VOUS CONFIRMEZ? CE NIVEAU EST\nVRAIMENT IMPITOYABLE!\n\nAPPUYEZ SUR Y OU N",
			"CECI EST UNE VERSION SHAREWARE DE DOOM.\n\nVOUS DEVRIEZ COMMANDER LA TRILOGIE COMPLETE.\n\nAPPUYEZ SUR UNE TOUCHE.",
			"MESSAGES OFF",
			"MESSAGES ON",
			"VOUS NE POUVEZ PAS METTRE FIN A UN JEU SUR RESEAU!\n\nAPPUYEZ SUR UNE TOUCHE.",
			"VOUS VOULEZ VRAIMENT METTRE FIN AU JEU?\n\nAPPUYEZ SUR Y OU N",
			"(APPUYEZ SUR Y POUR REVENIR AU OS.)",
			"GRAPHISMES MAXIMUM ",
			"GRAPHISMES MINIMUM ",
			"CORRECTION GAMMA OFF",
			"CORRECTION GAMMA NIVEAU 1",
			"CORRECTION GAMMA NIVEAU 2",
			"CORRECTION GAMMA NIVEAU 3",
			"CORRECTION GAMMA NIVEAU 4",
			"EMPLACEMENT VIDE",

			/* p_inter.c */
			"ARMURE RECUPEREE.",
			"MEGA-ARMURE RECUPEREE!",
			"BONUS DE SANTE RECUPERE.",
			"BONUS D'ARMURE RECUPERE.",
			"STIMPACK RECUPERE.",
			"MEDIKIT RECUPERE. VOUS EN AVEZ VRAIMENT BESOIN!",
			"MEDIKIT RECUPERE.",
			"SUPERCHARGE!",
			"CARTE MAGNETIQUE BLEUE RECUPEREE.",
			"CARTE MAGNETIQUE JAUNE RECUPEREE.",
			"CARTE MAGNETIQUE ROUGE RECUPEREE.",
			"CLEF CRANE BLEUE RECUPEREE.",
			"CLEF CRANE JAUNE RECUPEREE.",
			"CLEF CRANE ROUGE RECUPEREE.",
			"INVULNERABILITE!",
			"BERSERK!",
			"INVISIBILITE PARTIELLE ",
			"COMBINAISON ANTI-RADIATIONS ",
			"CARTE INFORMATIQUE ",
			"VISEUR A AMPLIFICATION DE LUMIERE ",
			"MEGASPHERE!",
			"CHARGEUR RECUPERE.",
			"BOITE DE BALLES RECUPEREE.",
			"ROQUETTE RECUPEREE.",
			"CAISSE DE ROQUETTES RECUPEREE.",
			"CELLULE D'ENERGIE RECUPEREE.",
			"PACK DE CELLULES D'ENERGIE RECUPERE.",
			"4 CARTOUCHES RECUPEREES.",
			"BOITE DE CARTOUCHES RECUPEREE.",
			"SAC PLEIN DE MUNITIONS RECUPERE!",
			"VOUS AVEZ UN BFG9000!  OH, OUI!",
			"VOUS AVEZ LA MITRAILLEUSE!",
			"UNE TRONCONNEUSE!",
			"VOUS AVEZ UN LANCE-ROQUETTES!",
			"VOUS AVEZ UN FUSIL A PLASMA!",
			"VOUS AVEZ UN FUSIL!",
			"VOUS AVEZ UN SUPER FUSIL!",

			/* p_doors.c */
			"IL VOUS FAUT UNE CLEF BLEUE",
			"IL VOUS FAUT UNE CLEF ROUGE",
			"IL VOUS FAUT UNE CLEF JAUNE",
			"IL VOUS FAUT UNE CLEF BLEUE",
			"IL VOUS FAUT UNE CLEF ROUGE",
			"IL VOUS FAUT UNE CLEF JAUNE",

			/* g_game.c */
			"JEU SAUVEGARDE.",

			/* hu_stuff.c */
			"[MESSAGE NON ENVOYE]",
			"E1M1: HANGAR",
			"E1M2: USINE NUCLEAIRE ",
			"E1M3: RAFFINERIE DE TOXINES ",
			"E1M4: CENTRE DE CONTROLE ",
			"E1M5: LABORATOIRE PHOBOS ",
			"E1M6: TRAITEMENT CENTRAL ",
			"E1M7: CENTRE INFORMATIQUE ",
			"E1M8: ANOMALIE PHOBOS ",
			"E1M9: BASE MILITAIRE ",
			"E2M1: ANOMALIE DEIMOS ",
			"E2M2: ZONE DE CONFINEMENT ",
			"E2M3: RAFFINERIE",
			"E2M4: LABORATOIRE DEIMOS ",
			"E2M5: CENTRE DE CONTROLE ",
			"E2M6: HALLS DES DAMNES ",
			"E2M7: CUVES DE REPRODUCTION ",
			"E2M8: TOUR DE BABEL ",
			"E2M9: FORTERESSE DU MYSTERE ",
			"E3M1: DONJON DE L'ENFER ",
			"E3M2: BOURBIER DU DESESPOIR ",
			"E3M3: PANDEMONIUM",
			"E3M4: MAISON DE LA DOULEUR ",
			"E3M5: CATHEDRALE PROFANE ",
			"E3M6: MONT EREBUS",
			"E3M7: LIMBES",
			"E3M8: DIS",
			"E3M9: CLAPIERS",
			"E4M1: Hell Beneath",
			"E4M2: Perfect Hatred",
			"E4M3: Sever The Wicked",
			"E4M4: Unruly Evil",
			"E4M5: They Will Repent",
			"E4M6: Against Thee Wickedly",
			"E4M7: And Hell Followed",
			"E4M8: Unto The Cruel",
			"E4M9: Fear",
			"NIVEAU 1: ENTREE ",
			"NIVEAU 2: HALLS SOUTERRAINS ",
			"NIVEAU 3: LE FEU NOURRI ",
			"NIVEAU 4: LE FOYER ",
			"NIVEAU 5: LES EGOUTS ",
			"NIVEAU 6: LE BROYEUR ",
			"NIVEAU 7: L'HERBE DE LA MORT",
			"NIVEAU 8: RUSES ET PIEGES ",
			"NIVEAU 9: LE PUITS ",
			"NIVEAU 10: BASE DE RAVITAILLEMENT ",
			"NIVEAU 11: LE CERCLE DE LA MORT!",
			"NIVEAU 12: L'USINE ",
			"NIVEAU 13: LE CENTRE VILLE",
			"NIVEAU 14: LES ANTRES PROFONDES ",
			"NIVEAU 15: LA ZONE INDUSTRIELLE ",
			"NIVEAU 16: LA BANLIEUE",
			"NIVEAU 17: LES IMMEUBLES",
			"NIVEAU 18: LA COUR ",
			"NIVEAU 19: LA CITADELLE ",
			"NIVEAU 20: JE T'AI EU!",
			"NIVEAU 21: LE NIRVANA",
			"NIVEAU 22: LES CATACOMBES ",
			"NIVEAU 23: LA GRANDE FETE ",
			"NIVEAU 24: LE GOUFFRE ",
			"NIVEAU 25: LES CHUTES DE SANG",
			"NIVEAU 26: LES MINES ABANDONNEES ",
			"NIVEAU 27: CHEZ LES MONSTRES ",
			"NIVEAU 28: LE MONDE DE L'ESPRIT ",
			"NIVEAU 29: LA LIMITE ",
			"NIVEAU 30: L'ICONE DU PECHE ",
			"NIVEAU 31: WOLFENSTEIN",
			"NIVEAU 32: LE MASSACRE",
			"level 1: congo",
			"level 2: well of souls",
			"level 3: aztec",
			"level 4: caged",
			"level 5: ghost town",
			"level 6: baron's lair",
			"level 7: caughtyard",
			"level 8: realm",
			"level 9: abattoire",
			"level 10: onslaught",
			"level 11: hunted",
			"level 12: speed",
			"level 13: the crypt",
			"level 14: genesis",
			"level 15: the twilight",
			"level 16: the omen",
			"level 17: compound",
			"level 18: neurosphere",
			"level 19: nme",
			"level 20: the death domain",
			"level 21: slayer",
			"level 22: impossible mission",
			"level 23: tombstone",
			"level 24: the final frontier",
			"level 25: the temple of darkness",
			"level 26: bunker",
			"level 27: anti-christ",
			"level 28: the sewers",
			"level 29: odyssey of noises",
			"level 30: the gateway of hell",
			"level 31: cyberden",
			"level 32: go 2 it",
			"level 1: system control",
			"level 2: human bbq",
			"level 3: power control",
			"level 4: wormhole",
			"level 5: hanger",
			"level 6: open season",
			"level 7: prison",
			"level 8: metal",
			"level 9: stronghold",
			"level 10: redemption",
			"level 11: storage facility",
			"level 12: crater",
			"level 13: nukage processing",
			"level 14: steel works",
			"level 15: dead zone",
			"level 16: deepest reaches",
			"level 17: processing area",
			"level 18: mill",
			"level 19: shipping/respawning",
			"level 20: central processing",
			"level 21: administration center",
			"level 22: habitat",
			"level 23: lunar mining project",
			"level 24: quarry",
			"level 25: baron's den",
			"level 26: ballistyx",
			"level 27: mount pain",
			"level 28: heck",
			"level 29: river styx",
			"level 30: last call",
			"level 31: pharaoh",
			"level 32: caribbean",
			"JE SUIS PRET A LEUR EN FAIRE BAVER!",
			"JE VAIS BIEN.",
			"JE N'AI PAS L'AIR EN FORME!",
			"AU SECOURS!",
			"TU CRAINS!",
			"LA PROCHAINE FOIS, MINABLE...",
			"VIENS ICI!",
			"JE VAIS M'EN OCCUPER.",
			"OUI",
			"NON",
			"VOUS PARLEZ TOUT SEUL ",
			"QUI EST LA?",
			"VOUS VOUS FAITES PEUR ",
			"VOUS COMMENCEZ A DELIRER ",
			"VOUS ETES LARGUE...",
			"[MESSAGE ENVOYE]",
			"VERT: ",
			"INDIGO: ",
			"BRUN: ",
			"ROUGE: ",
			"g",
			"i",
			"b",
			"r",

			/* am_map.c */
			"MODE POURSUITE ON",
			"MODE POURSUITE OFF",
			"GRILLE ON",
			"GRILLE OFF",
			"REPERE MARQUE ",
			"REPERES EFFACES ",

			/* st_stuff.c */
			"CHANGEMENT DE MUSIQUE ",
			"IMPOSSIBLE SELECTION",
			"INVULNERABILITE ON ",
			"INVULNERABILITE OFF",
			"ARMEMENT MAXIMUM! ",
			"ARMES (SAUF CLEFS) AJOUTEES",
			"BARRIERES ON",
			"BARRIERES OFF",
			" inVuln, Str, Inviso, Rad, Allmap, or Lite-amp",
			"AMELIORATION ACTIVEE",
			"... DOESN'T SUCK - GM",
			"CHANGEMENT DE NIVEAU...",

			/* f_finale.c */
			"APRES AVOIR VAINCU LES GROS MECHANTS\n"
			"ET NETTOYE LA BASE LUNAIRE, VOUS AVEZ\n"
			"GAGNE, NON? PAS VRAI? OU EST DONC VOTRE\n"
			" RECOMPENSE ET VOTRE BILLET DE\n"
			"RETOUR? QU'EST-QUE CA VEUT DIRE?CE"
			"N'EST PAS LA FIN ESPEREE!\n"
			"\n"
			"CA SENT LA VIANDE PUTREFIEE, MAIS\n"
			"ON DIRAIT LA BASE DEIMOS. VOUS ETES\n"
			"APPAREMMENT BLOQUE AUX PORTES DE L'ENFER.\n"
			"LA SEULE ISSUE EST DE L'AUTRE COTE.\n"
			"\n"
			"POUR VIVRE LA SUITE DE DOOM, JOUEZ\n"
			"A 'AUX PORTES DE L'ENFER' ET A\n"
			"L'EPISODE SUIVANT, 'L'ENFER'!\n",
			
			"VOUS AVEZ REUSSI. L'INFAME DEMON\n"
			"QUI CONTROLAIT LA BASE LUNAIRE DE\n"
			"DEIMOS EST MORT, ET VOUS AVEZ\n"
			"TRIOMPHE! MAIS... OU ETES-VOUS?\n"
			"VOUS GRIMPEZ JUSQU'AU BORD DE LA\n"
			"LUNE ET VOUS DECOUVREZ L'ATROCE\n"
			"VERITE.\n"
			"\n"
			"DEIMOS EST AU-DESSUS DE L'ENFER!\n"
			"VOUS SAVEZ QUE PERSONNE NE S'EN\n"
			"EST JAMAIS ECHAPPE, MAIS CES FUMIERS\n"
			"VONT REGRETTER DE VOUS AVOIR CONNU!\n"
			"VOUS REDESCENDEZ RAPIDEMENT VERS\n"
			"LA SURFACE DE L'ENFER.\n"
			"\n"
			"VOICI MAINTENANT LE CHAPITRE FINAL DE\n"
			"DOOM! -- L'ENFER.",
			
			"LE DEMON ARACHNEEN ET REPUGNANT\n"
			"QUI A DIRIGE L'INVASION DES BASES\n"
			"LUNAIRES ET SEME LA MORT VIENT DE SE\n"
			"FAIRE PULVERISER UNE FOIS POUR TOUTES.\n"
			"\n"
			"UNE PORTE SECRETE S'OUVRE. VOUS ENTREZ.\n"
			"VOUS AVEZ PROUVE QUE VOUS POUVIEZ\n"
			"RESISTER AUX HORREURS DE L'ENFER.\n"
			"IL SAIT ETRE BEAU JOUEUR, ET LORSQUE\n"
			"VOUS SORTEZ, VOUS REVOYEZ LES VERTES\n"
			"PRAIRIES DE LA TERRE, VOTRE PLANETE.\n"
			"\n"
			"VOUS VOUS DEMANDEZ CE QUI S'EST PASSE\n"
			"SUR TERRE PENDANT QUE VOUS AVEZ\n"
			"COMBATTU LE DEMON. HEUREUSEMENT,\n"
			"AUCUN GERME DU MAL N'A FRANCHI\n"
			"CETTE PORTE AVEC VOUS...",
			
			"VOUS ETES AU PLUS PROFOND DE L'ASTROPORT\n"
			"INFESTE DE MONSTRES, MAIS QUELQUE CHOSE\n"
			"NE VA PAS. ILS ONT APPORTE LEUR PROPRE\n"
			"REALITE, ET LA TECHNOLOGIE DE L'ASTROPORT\n"
			"EST AFFECTEE PAR LEUR PRESENCE.\n"
			"\n"
			"DEVANT VOUS, VOUS VOYEZ UN POSTE AVANCE\n"
			"DE L'ENFER, UNE ZONE FORTIFIEE. SI VOUS\n"
			"POUVEZ PASSER, VOUS POURREZ PENETRER AU\n"
			"COEUR DE LA BASE HANTEE ET TROUVER \n"
			"L'INTERRUPTEUR DE CONTROLE QUI GARDE LA \n"
			"POPULATION DE LA TERRE EN OTAGE.",
			
			"VOUS AVEZ GAGNE! VOTRE VICTOIRE A PERMIS\n"
			"A L'HUMANITE D'EVACUER LA TERRE ET \n"
			"D'ECHAPPER AU CAUCHEMAR. VOUS ETES \n"
			"MAINTENANT LE DERNIER HUMAIN A LA SURFACE \n"
			"DE LA PLANETE. VOUS ETES ENTOURE DE \n"
			"MUTANTS CANNIBALES, D'EXTRATERRESTRES \n"
			"CARNIVORES ET D'ESPRITS DU MAL. VOUS \n"
			"ATTENDEZ CALMEMENT LA MORT, HEUREUX \n"
			"D'AVOIR PU SAUVER VOTRE RACE.\n"
			"MAIS UN MESSAGE VOUS PARVIENT SOUDAIN\n"
			"DE L'ESPACE: \"NOS CAPTEURS ONT LOCALISE\n"
			"LA SOURCE DE L'INVASION EXTRATERRESTRE.\n"
			"SI VOUS Y ALLEZ, VOUS POURREZ PEUT-ETRE\n"
			"LES ARRETER. LEUR BASE EST SITUEE AU COEUR\n"
			"DE VOTRE VILLE NATALE, PRES DE L'ASTROPORT.\n"
			"VOUS VOUS RELEVEZ LENTEMENT ET PENIBLEMENT\n"
			"ET VOUS REPARTEZ POUR LE FRONT.",
			
			"VOUS ETES AU COEUR DE LA CITE CORROMPUE,\n"
			"ENTOURE PAR LES CADAVRES DE VOS ENNEMIS.\n"
			"VOUS NE VOYEZ PAS COMMENT DETRUIRE LA PORTE\n"
			"DES CREATURES DE CE COTE. VOUS SERREZ\n"
			"LES DENTS ET PLONGEZ DANS L'OUVERTURE.\n"
			"\n"
			"IL DOIT Y AVOIR UN MOYEN DE LA FERMER\n"
			"DE L'AUTRE COTE. VOUS ACCEPTEZ DE\n"
			"TRAVERSER L'ENFER POUR LE FAIRE?",
			
			"LE VISAGE HORRIBLE D'UN DEMON D'UNE\n"
			"TAILLE INCROYABLE S'EFFONDRE DEVANT\n"
			"VOUS LORSQUE VOUS TIREZ UNE SALVE DE\n"
			"ROQUETTES DANS SON CERVEAU. LE MONSTRE\n"
			"SE RATATINE, SES MEMBRES DECHIQUETES\n"
			"SE REPANDANT SUR DES CENTAINES DE\n"
			"KILOMETRES A LA SURFACE DE L'ENFER.\n"
			"\n"
			"VOUS AVEZ REUSSI. L'INVASION N'AURA.\n"
			"PAS LIEU. LA TERRE EST SAUVEE. L'ENFER\n"
			"EST ANEANTI. EN VOUS DEMANDANT OU IRONT\n"
			"MAINTENANT LES DAMNES, VOUS ESSUYEZ\n"
			"VOTRE FRONT COUVERT DE SUEUR ET REPARTEZ\n"
			"VERS LA TERRE. SA RECONSTRUCTION SERA\n"
			"BEAUCOUP PLUS DROLE QUE SA DESTRUCTION.\n",
			
			"FELICITATIONS! VOUS AVEZ TROUVE LE\n"
			"NIVEAU SECRET! IL SEMBLE AVOIR ETE\n"
			"CONSTRUIT PAR LES HUMAINS. VOUS VOUS\n"
			"DEMANDEZ QUELS PEUVENT ETRE LES\n"
			"HABITANTS DE CE COIN PERDU DE L'ENFER.",
			
			"FELICITATIONS! VOUS AVEZ DECOUVERT\n"
			"LE NIVEAU SUPER SECRET! VOUS FERIEZ\n"
			"MIEUX DE FONCER DANS CELUI-LA!\n",

			"You gloat over the steaming carcass of the\n"
			"Guardian.  With its death, you've wrested\n"
			"the Accelerator from the stinking claws\n"
			"of Hell.  You relax and glance around the\n"
			"room.  Damn!  There was supposed to be at\n"
			"least one working prototype, but you can't\n"
			"see it. The demons must have taken it.\n"
			"\n"
			"You must find the prototype, or all your\n"
			"struggles will have been wasted. Keep\n"
			"moving, keep fighting, keep killing.\n"
			"Oh yes, keep living, too.",

			"Even the deadly Arch-Vile labyrinth could\n"
			"not stop you, and you've gotten to the\n"
			"prototype Accelerator which is soon\n"
			"efficiently and permanently deactivated.\n"
			"\n"
			"You're good at that kind of thing.",

			"You've bashed and battered your way into\n"
			"the heart of the devil-hive.  Time for a\n"
			"Search-and-Destroy mission, aimed at the\n"
			"Gatekeeper, whose foul offspring is\n"
			"cascading to Earth.  Yeah, he's bad. But\n"
			"you know who's worse!\n"
			"\n"
			"Grinning evilly, you check your gear, and\n"
			"get ready to give the bastard a little Hell\n"
			"of your own making!",

			"The Gatekeeper's evil face is splattered\n"
			"all over the place.  As its tattered corpse\n"
			"collapses, an inverted Gate forms and\n"
			"sucks down the shards of the last\n"
			"prototype Accelerator, not to mention the\n"
			"few remaining demons.  You're done. Hell\n"
			"has gone back to pounding bad dead folks \n"
			"instead of good live ones.  Remember to\n"
			"tell your grandkids to put a rocket\n"
			"launcher in your coffin. If you go to Hell\n"
			"when you die, you'll need it for some\n"
			"final cleaning-up ...",

			"You've found the second-hardest level we\n"
			"got. Hope you have a saved game a level or\n"
			"two previous.  If not, be prepared to die\n"
			"aplenty. For master marines only.",

			"Betcha wondered just what WAS the hardest\n"
			"level we had ready for ya?  Now you know.\n"
			"No one gets out alive.",

			"You've fought your way out of the infested\n"
			"experimental labs.   It seems that UAC has\n"
			"once again gulped it down.  With their\n"
			"high turnover, it must be hard for poor\n"
			"old UAC to buy corporate health insurance\n"
			"nowadays..\n"
			"\n"
			"Ahead lies the military complex, now\n"
			"swarming with diseased horrors hot to get\n"
			"their teeth into you. With luck, the\n"
			"complex still has some warlike ordnance\n"
			"laying around.",

			"You hear the grinding of heavy machinery\n"
			"ahead.  You sure hope they're not stamping\n"
			"out new hellspawn, but you're ready to\n"
			"ream out a whole herd if you have to.\n"
			"They might be planning a blood feast, but\n"
			"you feel about as mean as two thousand\n"
			"maniacs packed into one mad killer.\n"
			"\n"
			"You don't plan to go down easy.",

			"The vista opening ahead looks real damn\n"
			"familiar. Smells familiar, too -- like\n"
			"fried excrement. You didn't like this\n"
			"place before, and you sure as hell ain't\n"
			"planning to like it now. The more you\n"
			"brood on it, the madder you get.\n"
			"Hefting your gun, an evil grin trickles\n"
			"onto your face. Time to take some names.",

			"Suddenly, all is silent, from one horizon\n"
			"to the other. The agonizing echo of Hell\n"
			"fades away, the nightmare sky turns to\n"
			"blue, the heaps of monster corpses start \n"
			"to evaporate along with the evil stench \n"
			"that filled the air. Jeeze, maybe you've\n"
			"done it. Have you really won?\n"
			"\n"
			"Something rumbles in the distance.\n"
			"A blue light begins to glow inside the\n"
			"ruined skull of the demon-spitter.",

			"What now? Looks totally different. Kind\n"
			"of like King Tut's condo. Well,\n"
			"whatever's here can't be any worse\n"
			"than usual. Can it?  Or maybe it's best\n"
			"to let sleeping gods lie..",

			"Time for a vacation. You've burst the\n"
			"bowels of hell and by golly you're ready\n"
			"for a break. You mutter to yourself,\n"
			"Maybe someone else can kick Hell's ass\n"
			"next time around. Ahead lies a quiet town,\n"
			"with peaceful flowing water, quaint\n"
			"buildings, and presumably no Hellspawn.\n"
			"\n"
			"As you step off the transport, you hear\n"
			"the stomp of a cyberdemon's iron shoe.",

			"ZOMBIE",
			"TYPE AU FUSIL",
			"MEC SUPER-ARME",
			"DIABLOTIN",
			"DEMON",
			"AME PERDUE",
			"CACODEMON",
			"CHEVALIER DE L'ENFER",
			"BARON DE L'ENFER",
			"ARACHNOTRON",
			"ELEMENTAIRE DE LA DOULEUR",
			"REVENANT",
			"MANCUBUS",
			"ARCHI-INFAME",
			"L'ARAIGNEE CERVEAU",
			"LE CYBERDEMON",
			"NOTRE HEROS",
		},
	},
	.current = l_localization.localized[LANGUAGE_ENGLISH],
};

enum l_language
L_CurrentLanguage(void) {
	return l_localization.current - l_localization.localized[0];
}

void
L_SetCurrentLanguage(enum l_language language) {
	assert(language >= 0 && language < LANGUAGE_LAST);
	l_localization.current = l_localization.localized[language];
}

const char *
L_String(enum l_string identifier) {
	assert(identifier >= 0 && identifier < STRING_LAST);
	return l_localization.current[identifier];
}

