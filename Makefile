## $Id$

# default installation directories
DESTDIR=/usr/local
DATADIR=/usr/local
SERVDIR=/usr/local

CC=g++
CXXFLAGS=-g -O2 -Wall -DWESNOTH_PATH=\"$(DESTDIR)/share/games/wesnoth-data\"
SDL_CFLAGS=`sdl-config --cflags` `freetype-config --cflags`
SDL_LIBS=`sdl-config --libs` `freetype-config --libs` -lSDL_mixer -lSDL_ttf -lSDL_image -lSDL_net
LIBS=${SDL_LIBS} -lstdc++
INCLUDES=-I. -Isrc -Isrc/tools -Isrc/widgets
OBJS=src/actions.o src/ai.o src/ai_attack.o src/ai_move.o src/config.o src/dialogs.o src/display.o src/events.o src/filesystem.o src/font.o src/game.o src/game_config.o src/game_events.o src/gamestatus.o src/hotkeys.o src/image.o src/intro.o src/key.o src/language.o src/log.o src/map.o src/mouse.o src/multiplayer.o src/multiplayer_client.o src/multiplayer_lobby.o src/network.o src/pathfind.o src/playlevel.o src/playturn.o src/preferences.o src/replay.o src/sdl_utils.o src/show_dialog.o src/sound.o src/team.o src/terrain.o src/tooltips.o src/unit.o src/unit_types.o src/video.o src/widgets/button.o src/widgets/menu.o src/widgets/slider.o src/widgets/textbox.o src/widgets/combo.o

MAKE_TRANS_OBJS=src/tools/make_translation.o src/config.o src/filesystem.o src/game_config.o src/log.o
MERGE_TRANS_OBJS=src/tools/merge_translations.o src/config.o src/filesystem.o src/game_config.o src/log.o

all: wesnoth src/server/wesnothd;

wesnoth: $(OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} ${SDL_CFLAGS} -o $@ ${OBJS} ${LIBS}

src/server/wesnothd:;
	$(MAKE) -C $(@D)

make_translation: $(MAKE_TRANS_OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} -o $@ ${MAKE_TRANS_OBJS} ${LIBS}

merge_translations: $(MERGE_TRANS_OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} -o $@ ${MERGE_TRANS_OBJS} ${LIBS}

.cpp.o:
	${CC} ${CXXFLAGS} ${INCLUDES} ${SDL_CFLAGS} -c $< -o $*.o

.PHONY: clean
clean:
	-rm -f ${OBJS} ${MAKE_TRANS_OBJS} ${MERGE_TRANS_OBJS} wesnoth make_translation merge_translations
	$(MAKE) -C src/server clean

install:
# main
	mkdir -p $(DESTDIR)/games
	cp -p wesnoth $(DESTDIR)/games
#	mkdir -p $(DESTDIR)/share/man/man6
#	cp -p doc/man/wesnoth.6 $(DESTDIR)/share/man/man6
	mkdir -p $(DESTDIR)/share/pixmaps
# data
	mkdir -p $(DATADIR)/share/games/wesnoth-data
	cp -pr data fonts images music sounds $(DATADIR)/share/games/wesnoth-data
# server
	mkdir -p $(SERVDIR)/games
	cp -p src/server/wesnothd $(SERVDIR)/games
	mkdir -p $(SERVDIR)/share/man/man6
	cp -p doc/man/wesnothd.6 $(SERVDIR)/share/man/man6
