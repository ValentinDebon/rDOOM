# Remote DOOM

This is a port of the original DOOM source code, as released by Id Software.
I wanted to take a clean slate to implement a version which would use modern X11 capabilities to remotely render the game.
It also is a good experience to port an old codebase to recent hardware/softwares.
I released the repository at the first runtime error I encountered (not segfault/linking or compilation error).

# To do list

- [x] Replace Xlib by XCB.
- [x] Update WAD/Lumps parsing, rewrite structures for portability.
- [ ] Update language system.
- [ ] Hunt down awful C hacks.
- [ ] Regroup global variables in "namespaces" structs.
- [ ] Implement Pulseaudio-backed audio.
- [ ] Implement remote rendering system.

