#############################################################################
#    Castle building helpers                                                #
#############################################################################

if LIBPNG

bin_PROGRAMS = exploder cutter

AM_CXXFLAGS = @SDL_CFLAGS@ @PNG_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" -I../ \
	      -DLOCALEDIR=\"$(LOCALEDIR)\" -I$(srcdir)/..

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


exploder_LDADD = @SDL_IMAGE_LIBS@ @PNG_LIBS@ @SDL_LIBS@ $(LIBZIPIOS)
cutter_LDADD = @SDL_IMAGE_LIBS@ @PNG_LIBS@ @SDL_LIBS@ $(LIBZIPIOS)

endif
