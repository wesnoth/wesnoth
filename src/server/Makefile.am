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
		   ../gettext.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   ../network_worker.cpp \
		   ../thread.cpp \
		   ../serialization/preprocessor.cpp \
		   ../serialization/string_utils.cpp \
		   ../zipios++/xcoll.cpp \
		   game.hpp \
		   ../game_events.hpp \
		   input_stream.hpp \
		   metrics.hpp \
		   player.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../gettext.hpp \
		   ../log.hpp \
		   ../network.hpp \
		   ../network_worker.hpp \
		   ../thread.hpp \
		   ../serialization/preprocessor.hpp \
		   ../serialization/string_utils.hpp \
		   ../zipios++/xcoll.hpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DLOCALEDIR=\"$(LOCALEDIR)\" -I$(srcdir)/..
LDADD = @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBZIPIOS)
