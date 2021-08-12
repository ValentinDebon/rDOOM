
# X11 Remote Doom

## Fetching the source

Fetch the source code from [Id repository](https://github.com/id-Software/DOOM).

## Build

First step was to compile the source code. Used the original `Makefile` at first, then modified the hierarchy a bit.
Just trying to compile it gave me a huge amount of errors. I guess because it wasn't made for a 64-bit architecture.
- The `m_misc.c` `default_t` uses an hugly hack to give strings as pointers, doesn't work anymore and found an ugly workaround with unions.
- The `eventtail = (++eventtail)&(MAXEVENTS-1)` construct wasn't right anymore, used comma operator to fix it.
- An errno header `#include <errnos.h>` was wrong.
- Fixed a shi\*t ton of raw pointers to int casts using `uintptr_t` and `intptr_t`.
- Removed the `rcsid` everywhere in the code using an ugly `awk` script. Took the opportunity to modify the licenses.

## Runtime errors

Building was easy. But immediately running it quickly gave segfaults. One of the main points was certainly the WAD
files raw memory pointers management. Some structures weren't right and ought to be modified and packed. All used structures are
all around the code and it IS difficult to debug things when you don't known what's in the wad file.
So, I created a `readwad` utlity to help me determine what composed the WAD.
- A reference used an `extern` definition for `errno` which isn't right anymore because errno is supposed to be thread local.
- The `r_data.c` initialization was a pain. I finally figured out a lot of segfaults came from the `Z_Malloc` not allocating a right size (usually hardcoded 4 as a pointer size, oof).
- Argh, 8 bit PseudoColor Display not supported on my X11. No worry, will come back later.
- While thinking for this PseudoColor Display thing, I changed the `m_misc.c` source to handle string defaults more properly.

## Port to XCB
As pseudocolor wasn't supported in my X11 server, I decided to modify the source code to implement XCB, and purge the natives I\_\* implementations.
During the process, some headers broke, and some symbols collided (who calls a variable `time`?).
I decided to add a file/header handling XCB stuffs: `i_xcb.h` and `i_xcb.c`. To make it clearer, I regrouped its global variables in a `struct i_xcb` named `i_xcb`.
It might be a good idea to do that for the whole codebase one day, it's one hell of global variables mess.
While adding event handling, I also added `const` qualifiers on the `D_PostEvent` argument, I will surely do this throughout the code too.

Finally, I got a game and I can resize the window at will.
However the game crashes soon, demo not compatible and missing lump "HELP2", I guess recent wads are too much incompatible with the old ones.
Purging the sound system, I now also have sound caching error, before implementing pulseaudio, I might add an OpenAL rough implementation.
I also have to implement mouse grabing and fullscreen. Surely, more internal work is to be done to avoid spurious segfaults etc...

## Porting to macOS
- Removed `-DLINUX` in `Makefile` when non Linux.
- Missing `<values.h>` on the system.
- Missing references to `<stdarg.h>` in `i_system.c`, `<alloca.h>` in `r_data.c`.
- Replaced `<malloc.h>` by `<stdlib.h>` in `w_wad.c`.

## Port continuation
I replaced my .wad by the doom1.wad shareware version 1.9, I had one segfault/zone id error when loading a level,
turned out it was *once again* a bad allocation size, `p_setup.c`, a linebuffer was allocating an hardcoded 4, but it was a `line_t **`.
Added grabing, hidden cursor, resize, and center to screen (should be window manager agnostic).
Finished E1M1! Game crashed, it was that `time` variable, I missed one in a `Z_ChangeTag` and the game crashed.
Looked at DOOM music/sound handling, big mess. Won't implement an OpenAL, go directly to pulseaudio. But I'd need a MUS to PCM converter.
Also, I commented this annoying warning from `s_sound.c` regarding 16 bit not being pre-cached (we are not DOS anyway).
Another warning concerning "Demo is from a different game version" was here, not playing demos on title screen.
I modified the condition from different to greater than engine one. Didn't seem to break a thing, allows them to run.
Removed `htonl` and `htons` macros, redundant with `<arpa/inet.h>` ones.

## Build system and utilities
Implementing runtime localization, I thought it would be more interesting to create the source files from
translation dictionaries (like strings files in macOS' Xcode). I also wanted to sync the readwad with my new wad support.
Managing portable Makefiles will be a pain. I'll use CMake to port the build system (and maybe add a generic Makefile just for rDOOM).
These new ideas will be implemented in the `tools/` directory. And use rDOOM's headers and C files to stay coherent. Will also create a utility
to extract textures from a WAD file if libpng is available.

## Localization
Localization is now available! Lang files are parsed by `genlang` and preprocessely included into `l_strings.c`.
It ensures a warning for every non translated key. However, you must manually ensure the `enum l_string` has every key declared in `genlang`.
While upgrading the source code to the localization module, I found out lots of strings manipulations are done in random buffers here and there.
It may be time to add some new files in `M_*` to remove the engine's limitations and have a dedicated memory space for string formatting/manipulations.

## Templates?
Just created a template system for arrays. Testing it in `W_*`, it's pretty neat, and the machine code duplication should be negligible in memory space.
It might be the solution for all limitations removals. And even if the header code is pretty unreadable, it inserts nicely in the C source files.
It might also be a solution to easily regroup all allocations in the `Z_*` module, effectively limiting memory usage explicitly.

## Portability
Once again, portability issues while working on macOS. Removed `NORMALUNIX` and `LINUX`. `NORMALUNIX` shouldn't be need as we only target these systems.
Turns out `LINUX` only existed to extend min/max definitions of `doomtype.h`, whose purpose could be overseeded by the use of `limits.h`.

## Main loop revision?
Networking becoming a critical part of rDOOM (handling rendering, netgame, and in the future, sound), the main loop may be revised
to optimize network polling. The net code seems pretty broken from there, a big dive will be necessary to understands its deep internals.
Will also need heavy profiling for the engine's resources usage. Must do this before heavy changes in the engine's logic.

## Argument parsing
The argument parsing was pretty hacky, and some options may be considered deprecated. To avoid more internal bleeding through
weird argument parsing hacks, it was revised to use `getopt_long_only` in `i_main.c` and forward declarations in a new `m_param.c` interface.
Also eases numerical argument parsing. As net options are not parsed the same way anymore, Net games are deactivated until main loop revision.
Also turns out savegame name is duplicated everywhere, will create a function to get it according to a slot and system settings (XDG?).
Took advantage of the occasion to update defaults parsing once and for all, only a review will be necessary once all globals are regrouped.
Found a bug! Left a `Z_ChangeTag` for demo playback, effectively crashing at each demo end. Doing an unsafe copy of the demo lump's data pointer for now.
Must create two separate `demo_p` when refactoring `g_game.c`.
