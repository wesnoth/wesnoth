SDL_CFLAGS = `sdl-config --cflags` `freetype-config --cflags`
SDL_LIBS = `sdl-config --libs` -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL_net `freetype-config --libs`
OBJS= editor.o ../actions.o ../ai.o ../ai_attack.o ../ai_move.o ../config.o ../dialogs.o ../display.o ../events.o ../filesystem.o ../font.o ../game_config.o ../game_events.o ../gamestatus.o ../hotkeys.o ../image.o ../intro.o ../key.o ../language.o ../log.o ../map.o ../show_dialog.o ../mouse.o ../network.o ../pathfind.o ../playlevel.o ../playturn.o ../preferences.o ../replay.o ../sdl_utils.o ../sound.o ../team.o ../terrain.o ../tooltips.o ../unit.o ../unit_types.o ../video.o ../widgets/button.o ../widgets/menu.o ../widgets/textbox.o ../widgets/slider.o

editor: $(OBJS)
	gcc -lstdc++ ${SDL_CFLAGS} -o $@ ${OBJS} ${SDL_LIBS}

.cpp.o:
	gcc ${SDL_CFLAGS} -c $< -o $*.o

clean:
	-rm -f *.o ../*.o editor
