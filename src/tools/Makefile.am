bin_PROGRAMS = make_translation merge_translations

#############################################################################
#    Translation Tools                                                      #
#############################################################################

AM_CXXFLAGS = @SDL_CFLAGS@ -I../
AM_LDFLAGS = @SDL_LIBS@

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
