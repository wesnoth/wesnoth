
CC=g++
SDL_CFLAGS = -Wall `sdl-config --cflags` `freetype-config --cflags`
SDL_LIBS = `sdl-config --libs` `freetype-config --libs` -lSDL_mixer -lSDL_ttf -lSDL_image

OBJS=actions.o ai.o ai_attack.o ai_move.o config.o dialogs.o display.o filesystem.o font.o game.o game_config.o game_events.o gamestatus.o hotkeys.o intro.o key.o language.o log.o map.o menu.o multiplayer.o pathfind.o playlevel.o playturn.o preferences.o replay.o sdl_utils.o sound.o team.o terrain.o unit.o unit_types.o video.o widgets/button.o widgets/slider.o widgets/textbox.o

MAKE_TRANS_OBJS=make_translation.o config.o filesystem.o log.o
MERGE_TRANS_OBJS=merge_translations.o config.o filesystem.o log.o

wesnoth: $(OBJS)
	${CC} ${CXXFLAGS} ${SDL_CFLAGS} -o $@ ${OBJS} ${SDL_LIBS} -lstdc++

make_translation: $(MAKE_TRANS_OBJS)
	${CC} ${CXXFLAGS} -o $@ ${MAKE_TRANS_OBJS} ${SDL_LIBS} -lstdc++

merge_translations: $(MERGE_TRANS_OBJS)
	${CC} ${CXXFLAGS} -o $@ ${MERGE_TRANS_OBJS} ${SDL_LIBS} -lstdc++

.cpp.o:
	${CC} ${CXXFLAGS} ${SDL_CFLAGS} -c $< -o $*.o

clean:
	-rm -f *.o wesnoth make_translation merge_translations widgets/*.o

	
