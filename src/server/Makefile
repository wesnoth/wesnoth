SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs` -lSDL_net
OBJS= game.o player.o server.o ../config.o ../filesystem.o ../game_config.o ../log.o ../network.o 

server: $(OBJS)
	gcc -lstdc++ ${SDL_CFLAGS} -o $@ ${OBJS} ${SDL_LIBS}

.cpp.o:
	gcc ${SDL_CFLAGS} -c $< -o $*.o

clean:
	-rm -f *.o ../*.o editor
