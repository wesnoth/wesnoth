#############################################################################
#    Castle building helpers                                                #
#############################################################################

if LIBPNG

bin_PROGRAMS = exploder cutter

AM_CXXFLAGS = @SDL_CFLAGS@ @PNG_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" -I../ \
	      -DLOCALEDIR=\"$(LOCALEDIR)\" -I$(srcdir)/..
AM_LDFLAGS = @SDL_LIBS@


exploder_SOURCES = exploder.cpp \
		   exploder_utils.cpp \
		   exploder_cutter.cpp \
		   exploder_composer.cpp \
		   ../server/variable.cpp \
		   dummy_video.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../sdl_utils.cpp \
		   ../log.cpp \
		   ../zipios++/xcoll.cpp \
		   exploder_composer.hpp \
		   exploder_cutter.hpp \
		   exploder_utils.hpp \
		   ../zipios++/xcoll.hpp \
		   ../gettext.cpp

cutter_SOURCES = cutter.cpp \
		   exploder_utils.cpp \
		   exploder_cutter.cpp \
		   ../server/variable.cpp \
		   dummy_video.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../sdl_utils.cpp \
		   ../log.cpp \
		   ../zipios++/xcoll.cpp \
		   exploder_composer.hpp \
		   exploder_cutter.hpp \
		   exploder_utils.hpp \
		   ../zipios++/xcoll.hpp \
		   ../gettext.cpp


exploder_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @PNG_LIBS@ $(LIBZIPIOS)
cutter_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @PNG_LIBS@ $(LIBZIPIOS)

endif
