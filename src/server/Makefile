SDL_CFLAGS = `sdl-config --cflags` `freetype-config --cflags`
SDL_LIBS = `sdl-config --libs` -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL_net `freetype-config --libs`
OBJS= game.o server.o ../config.o ../filesystem.o ../game_config.o ../log.o ../network.o ../player.o

server: $(OBJS)
	gcc -lstdc++ ${SDL_CFLAGS} -o $@ ${OBJS} ${SDL_LIBS}

.cpp.o:
	gcc ${SDL_CFLAGS} -c $< -o $*.o

clean:
	-rm -f *.o ../*.o editor
