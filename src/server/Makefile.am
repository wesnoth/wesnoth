bin_PROGRAMS = wesnothd

#############################################################################
#    Wesnothd Server                                                        #
#############################################################################

wesnothd_SOURCES = game.cpp \
		   ../game_events.cpp \
		   player.cpp \
		   server.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   ../replay.cpp \
		   ../unit.cpp \
		   game.hpp \
		   ../game_events.hpp \
		   player.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../log.hpp \
		   ../network.hpp \
		   ../replay.hpp \
		   ../unit.hpp

AM_CXXFLAGS = @SDL_CFLAGS@
AM_LDFLAGS = @SDL_LIBS@ @SDL_NET_LIBS@
