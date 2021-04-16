# Remote DOOM

This is a port of the original DOOM source code, as released by Id Software.
I wanted to take a clean slate to implement a version which would use modern X11 capabilities to remotely render the game.
It also is a good experience to port an old codebase to recent hardware/softwares.
I released the repository at the first runtime error I encountered (not segfault/linking or compilation error).

# To do list

- [x] Replace Xlib by XCB.
- [x] Update WAD/Lumps parsing, rewrite structures for portability.
- [x] Update language system.
- [ ] Remove arbitrary limitations.
- [ ] Hunt down awful C hacks.
- [ ] Regroup global variables in "namespaces" structs.
- [ ] Implement Pulseaudio-backed audio.
- [ ] Implement remote rendering system.

# Tools

To assist the port and build, several utilities are planned to introspect/port wad contents and generate build files.

- [x] readwad: Parse and list WAD content
- [ ] wad2png: Extract textures from WAD files to PNG.
- [x] genlang: Create source file from a translation listing.

