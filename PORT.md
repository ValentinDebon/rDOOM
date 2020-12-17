
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
I also have to implement mouse grabing and fullscreen. Surely, must internal work is to be done to avoid spurious segfaults etc...

## Porting to macOS
- Removed `-DLINUX` in `Makefile` when non Linux.
- Missing `<values.h>` on the system.
- Missing references to `<stdarg.h>` in `i_system.c`, `<alloca.h>` in `r_data.c`.
- Replaced `<malloc.h>` by `<stdlib.h>` in `w_wad.c`.

## Port continuation
I replaced my .wad by the doom1.wad shareware version 1.9, I had one segfault/zone id error when loading a level,
turned out it was *once again* a bad allocation size, `p_setup.c`, a linebuffer was allocating an hardcoded 4, but it was a `line_t **`.
