bin_PROGRAMS = wesnoth_zip

#############################################################################
#    Wesnoth Zip                                                            #
#############################################################################

wesnoth_zip_SOURCES = zip.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
           ../gettext.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   ../game_events.hpp \
		   ../config.hpp \
           ../gettext.hpp \
           ../wesconfig.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../log.hpp \
		   ../network.hpp \
		   ../server/variable.cpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DLOCALEDIR=\"$(LOCALEDIR)\" -I../intl \
    -I$(top_srcdir)/intl
AM_LDFLAGS = @SDL_LIBS@ @SDL_NET_LIBS@
