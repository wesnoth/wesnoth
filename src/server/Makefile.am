bin_PROGRAMS = wesnothd

#############################################################################
#    Wesnothd Server                                                        #
#############################################################################

wesnothd_SOURCES = game.cpp \
		   player.cpp \
		   server.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   game.hpp \
		   player.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../log.hpp \
		   ../network.hpp

AM_CXXFLAGS = @SDL_CFLAGS@
AM_LDFLAGS = @SDL_LIBS@ @SDL_NET_LIBS@
