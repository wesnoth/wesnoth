## $Id$

CC=g++
ifndef CXXFLAGS
CXXFLAGS=-O2 -Wall
endif
SDL_CFLAGS=`sdl-config --cflags` `freetype-config --cflags`
SDL_LIBS=`sdl-config --libs` `freetype-config --libs` -lSDL_mixer -lSDL_ttf -lSDL_image -lSDL_net
LIBS=${SDL_LIBS} -lstdc++
INCLUDES=-I. -Isrc -Isrc/tools -Isrc/widgets
OBJS=src/actions.o src/ai.o src/ai_attack.o src/ai_move.o src/config.o src/dialogs.o src/display.o src/events.o src/filesystem.o src/font.o src/game.o src/game_config.o src/game_events.o src/gamestatus.o src/hotkeys.o src/image.o src/intro.o src/key.o src/language.o src/log.o src/map.o src/mouse.o src/multiplayer.o src/multiplayer_client.o src/multiplayer_lobby.o src/network.o src/pathfind.o src/playlevel.o src/playturn.o src/preferences.o src/replay.o src/sdl_utils.o src/show_dialog.o src/sound.o src/team.o src/terrain.o src/tooltips.o src/unit.o src/unit_types.o src/video.o src/widgets/button.o src/widgets/menu.o src/widgets/slider.o src/widgets/textbox.o

MAKE_TRANS_OBJS=src/tools/make_translation.o src/config.o src/filesystem.o src/game_config.o src/log.o
MERGE_TRANS_OBJS=src/tools/merge_translations.o src/config.o src/filesystem.o src/game_config.o src/log.o

wesnoth: $(OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} ${SDL_CFLAGS} -o $@ ${OBJS} ${LIBS}

make_translation: $(MAKE_TRANS_OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} -o $@ ${MAKE_TRANS_OBJS} ${LIBS}

merge_translations: $(MERGE_TRANS_OBJS)
	${CC} ${CXXFLAGS} ${INCLUDES} -o $@ ${MERGE_TRANS_OBJS} ${LIBS}

.cpp.o:
	${CC} ${CXXFLAGS} ${INCLUDES} ${SDL_CFLAGS} -c $< -o $*.o

.PHONY: clean
clean:
	-rm -f ${OBJS} ${MAKE_TRANS_OBJS} ${MERGE_TRANS_OBJS} wesnoth make_translation merge_translations
