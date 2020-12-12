
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
files raw memory pointers management. Some structures weren't right and ought to be modified and packed. All used strucutres are
all around the code and it IS difficult to debug things when you don't known what's in the wad file.
So, I created a `readwad` utlity to help me determine what composed the WAD.
- A reference used an `extern` definition for `errno` which isn't right anymore because errno is supposed to be thread local.
- The `r_data.c` initialization was a pain. I finally figured out a lot of segfaults came from the `Z_Malloc` not allocating a right size (usually hardcoded 4 as a pointer size, oof).
- Argh, 8 bit PseudoColor Display not supported on my X11. No worry, will come back later.
- While thinking for this PseudoColor Display thing, I changed the `m_misc.c` source to handle string defaults more properly.

