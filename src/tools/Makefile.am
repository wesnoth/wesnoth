
bin_PROGRAMS = make_translation merge_translations 

if LIBPNG
bin_PROGRAMS += exploder cutter
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
#    Castle building helpers                                                #
#############################################################################

if LIBPNG

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
		   ../log.cpp

 cutter_SOURCES = cutter.cpp \
		   exploder_utils.cpp \
		   exploder_cutter.cpp \
		   ../server/variable.cpp \
		   dummy_video.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../sdl_utils.cpp \
		   ../log.cpp


exploder_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @PNG_LIBS@
cutter_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @PNG_LIBS@

endif

