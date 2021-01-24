CC=clang

ifeq ($(shell uname -s),Linux)
CFLAGS=-g -Wall -DNORMALUNIX -DLINUX
else
CFLAGS=-g -Wall -DNORMALUNIX
endif

LD=$(CC)
LDFLAGS=-lxcb -lxcb-render

BUILD=build
DOOM=rdoom

OBJECTS= \
	$(BUILD)/am_map.o \
	$(BUILD)/d_items.o \
	$(BUILD)/d_main.o \
	$(BUILD)/d_net.o \
	$(BUILD)/doomdef.o \
	$(BUILD)/doomstat.o \
	$(BUILD)/dstrings.o \
	$(BUILD)/f_finale.o \
	$(BUILD)/f_wipe.o \
	$(BUILD)/g_game.o \
	$(BUILD)/hu_lib.o \
	$(BUILD)/hu_stuff.o \
	$(BUILD)/i_main.o \
	$(BUILD)/i_net.o \
	$(BUILD)/info.o \
	$(BUILD)/i_sound.o \
	$(BUILD)/i_system.o \
	$(BUILD)/i_video.o \
	$(BUILD)/i_xcb.o \
	$(BUILD)/m_argv.o \
	$(BUILD)/m_bbox.o \
	$(BUILD)/m_cheat.o \
	$(BUILD)/m_fixed.o \
	$(BUILD)/m_menu.o \
	$(BUILD)/m_misc.o \
	$(BUILD)/m_random.o \
	$(BUILD)/m_swap.o \
	$(BUILD)/p_ceilng.o \
	$(BUILD)/p_doors.o \
	$(BUILD)/p_enemy.o \
	$(BUILD)/p_floor.o \
	$(BUILD)/p_inter.o \
	$(BUILD)/p_lights.o \
	$(BUILD)/p_local.o \
	$(BUILD)/p_map.o \
	$(BUILD)/p_maputl.o \
	$(BUILD)/p_mobj.o \
	$(BUILD)/p_plats.o \
	$(BUILD)/p_pspr.o \
	$(BUILD)/p_saveg.o \
	$(BUILD)/p_setup.o \
	$(BUILD)/p_sight.o \
	$(BUILD)/p_spec.o \
	$(BUILD)/p_switch.o \
	$(BUILD)/p_telept.o \
	$(BUILD)/p_tick.o \
	$(BUILD)/p_user.o \
	$(BUILD)/r_bsp.o \
	$(BUILD)/r_data.o \
	$(BUILD)/r_draw.o \
	$(BUILD)/r_main.o \
	$(BUILD)/r_plane.o \
	$(BUILD)/r_segs.o \
	$(BUILD)/r_sky.o \
	$(BUILD)/r_things.o \
	$(BUILD)/sounds.o \
	$(BUILD)/s_sound.o \
	$(BUILD)/st_lib.o \
	$(BUILD)/st_stuff.o \
	$(BUILD)/tables.o \
	$(BUILD)/v_video.o \
	$(BUILD)/wi_stuff.o \
	$(BUILD)/w_wad.o \
	$(BUILD)/z_zone.o \

.PHONY: all clean

all: $(BUILD)/$(DOOM)

clean:
	rm -f $(BUILD)/*

$(BUILD)/$(DOOM): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

