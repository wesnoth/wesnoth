bin_PROGRAMS = wesnothd

#############################################################################
#    Wesnothd Server                                                        #
#############################################################################

wesnothd_SOURCES = game.cpp \
		   input_stream.cpp \
		   metrics.cpp \
		   player.cpp \
		   server.cpp \
		   variable.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   game.hpp \
		   ../game_events.hpp \
		   input_stream.hpp \
		   metrics.hpp \
		   player.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../log.hpp \
		   ../network.hpp

AM_CXXFLAGS = @SDL_CFLAGS@
AM_LDFLAGS = @SDL_LIBS@ @SDL_NET_LIBS@
