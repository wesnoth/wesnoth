
bin_PROGRAMS = make_translation merge_translations 

if LIBPNG
bin_PROGRAMS += exploder
endif

#############################################################################
#    Translation Tools                                                      #
#############################################################################

AM_CXXFLAGS = @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" -I../ 
AM_LDFLAGS = @SDL_LIBS@

if LIBPNG
	AM_CXXFLAGS += @PNG_CFLAGS@
endif

#############################################################################
#    Make Translation                                                       #
#############################################################################

make_translation_SOURCES = make_translation.cpp \
			   ../config.cpp \
			   ../filesystem.cpp \
			   ../game_config.cpp \
			   ../log.cpp \
			   ../server/variable.cpp \
			   ../config.hpp \
			   ../filesystem.hpp \
			   ../game_config.hpp \
			   ../log.hpp 


#############################################################################
#    Merge Translations                                                     #
#############################################################################

merge_translations_SOURCES = merge_translations.cpp \
			     ../config.cpp \
			     ../filesystem.cpp \
			     ../game_config.cpp \
			     ../log.cpp \
			     ../server/variable.cpp \
			     ../config.hpp \
			     ../filesystem.hpp \
			     ../game_config.hpp \
			     ../log.hpp 

#############################################################################
#    Exploder tool
#############################################################################

if LIBPNG

exploder_SOURCES = exploder.cpp \
		   exploder_utils.cpp \
		   exploder_cutter.cpp \
		   exploder_composer.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../sdl_utils.cpp \
		   ../log.cpp \
		   ../server/variable.cpp \
		   dummy_video.cpp

exploder_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @PNG_LIBS@

endif

