# Process this file with autoconf to produce a configure script.

#######################################################################
# Initial configuration                                               #
#######################################################################

AC_PREREQ([2.57])
AC_INIT([Battle for Wesnoth], [0.7.8], [davidnwhite@optusnet.com.au], [wesnoth])

AC_REVISION([$Revision$])

AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign 1.5])
AC_CONFIG_SRCDIR([src/actions.cpp])
AC_CONFIG_HEADER([config.h])


#######################################################################
# Checks for programs.                                                #
#######################################################################

AC_PROG_CXX
AC_PROG_CC

# SDL_CONFIG

AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])

if test "x$SDL_CONFIG" = "xnone"; then
	
	AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])

fi


# GNOME_CONFIG

AC_PATH_PROGS([GNOME_CONFIG], [gnome-config], [none])

if test "x$GNOME_CONFIG" = "xnone"; then
	gnome1found=no
else
	GNOME1_DESKTOP=`$GNOME_CONFIG --datadir`/gnome/apps/Games
	gnome1found=yes
fi

AC_SUBST([GNOME1_DESKTOP])


# KDE_CONFIG

AC_PATH_PROGS([KDE_CONFIG], [kde-config], [none])

if test "x$KDE_CONFIG" = "xnone"; then
	kdefound=no
else
	KDE_DESKTOP=`$KDE_CONFIG --expandvars --install apps`/Games/TacticStrategy
	KDE_ICON=`$KDE_CONFIG --expandvars --install icon`
	kdefound=yes
fi

AC_SUBST([KDE_DESKTOP])
AC_SUBST([KDE_ICON])


#######################################################################
# Checks for types.                                                   #
#######################################################################

# No need to chech anymore    
#AC_LANG([C++])
#AC_MSG_CHECKING([if char is signed])

#AC_COMPILE_IFELSE(
#	[[static int test_array [1 - 2 * !(((char) -1) < 0)];]],
#	[AC_MSG_RESULT(yes)],
#	[AC_MSG_RESULT(no)] 
#	[AC_MSG_ERROR([*** char is unsigned. Wesnoth cannot work with unsigned 
#chars. Try with another compiler (specify it using CXX environment variable.)
#If you use GCC, you can force signed chars with CXXFLAGS=-fsigned-char])])

#######################################################################
# Checks for libraries.                                               #
#######################################################################

SDL_LIBS=`$SDL_CONFIG --libs`
OLD_LIBS=$LIBS
LIBS="$LIBS $SDL_LIBS"

# There's no need for this, $SDL_CONFIG comes with libsdl and
# it doesn't find it in FreeBSD
# AC_CHECK_LIB([SDL], [SDL_Init])

AC_CHECK_LIB([SDL_image],
	     [IMG_Load],
	     [SDL_IMAGE_LIBS=-lSDL_image],
	     [AC_MSG_ERROR([*** SDL_image lib not found! Get SDL_image from
http://www.libsdl.org/projects/SDL_image/index.html])])

AC_CHECK_LIB([SDL_mixer],
	     [Mix_OpenAudio],
	     [SDL_MIXER_LIBS=-lSDL_mixer],
	     [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])
	     
AC_CHECK_LIB([SDL_net],
	     [SDLNet_Init],
	     [SDL_NET_LIBS=-lSDL_net],
	     [AC_MSG_ERROR([*** SDL_net lib not found! Get SDL_net from
http://www.libsdl.org/projects/SDL_net/index.html])])

AC_CHECK_LIB([SDL_ttf],
	     [TTF_OpenFont],
	     [SDL_TTF_LIBS=-lSDL_ttf],
	     [AC_MSG_ERROR([*** SDL_ttf lib not found! Get SDL_ttf from
http://www.libsdl.org/projects/SDL_ttf/index.html])])

LIBS=$OLD_LIBS

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_IMAGE_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_NET_LIBS])
AC_SUBST([SDL_TTF_LIBS])


#######################################################################
# Checks for header files.                                            #
#######################################################################

AC_HEADER_DIRENT
AC_HEADER_STDC

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_CFLAGS="$SDL_CFLAGS"
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"


AC_CHECK_HEADER([SDL.h],
		[],
		[AC_MSG_ERROR([*** SDL include files not found!
You should install SDL development package.])])

AC_CHECK_HEADER([SDL_image.h],
		[],
		[AC_MSG_ERROR([*** SDL_image include files not found!
You should install development package.])])
		
AC_CHECK_HEADER([SDL_mixer.h],
		[],
		[AC_MSG_ERROR([*** SDL_mixer include files not found!
You should install development package.])])
		
AC_CHECK_HEADER([SDL_net.h],
		[],
		[AC_MSG_ERROR([*** SDL_net include files not found!
You should install development package.])])
		
AC_CHECK_HEADER([SDL_ttf.h],
		[],
		[AC_MSG_ERROR([*** SDL_ttf include files not found!
You should install development package.])])
		
CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS

AC_SUBST([SDL_CFLAGS])

AC_CHECK_HEADERS([stdlib.h unistd.h])


#######################################################################
# Checks for typedefs, structures, and compiler characteristics.      #
#######################################################################

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([floor socket strtoul])


#######################################################################
# Configuration options                                               #
#######################################################################

AC_ARG_ENABLE([debug],
	      [  --enable-debug        Enable debug in wesnoth],
	      [CXXFLAGS="$CXXFLAGS -DDEBUG -ggdb3"])

AC_ARG_ENABLE([static],
              [  --enable-static       Enable static building of wesnoth],
	      [static=$enableval],
	      [static=no])

AC_ARG_ENABLE([lite],
	      [  --enable-lite         Enable lite version of wesnoth (without music)],
	      [lite=$enableval],
	      [lite=no])

DATADIR=$PACKAGE
AC_ARG_ENABLE([datadir-name],
	      [  --datadir-name[[=dir]] Change name of data directory. Default with this option: wesnoth-data], 
	      [case "${enableval}" in
	       yes)
			DATADIR="wesnoth-data"
			;;
		no)
			;;
		*)
			DATADIR="${enableval}"
			;;
		esac])

AC_SUBST([DATADIR])
		
AC_ARG_ENABLE([server],
	      [  --enable-server       Enable compilation of server],
	      [server=$enableval],
	      [server=no])

AC_ARG_ENABLE([editor],
	      [  --enable-editor       Enable compilation of map editor],
	      [editor=$enableval],
	      [editor=no])

AC_ARG_ENABLE([tools],
	      [  --enable-tools        Enable compilation of translation tools],
	      [tools=$enableval],
	      [tools=no])

AC_ARG_ENABLE([kde],
	      [  --enable-kde          Enable installation of icon and KDE menu entry],
	      [kde=$enableval],
	      [kde=no])

AC_ARG_ENABLE([gnome],
	      [  --enable-gnome        Enable installation of icon and GNOME menu entry],
	      [gnome=$enableval],
	      [gnome=no])

AC_ARG_ENABLE([wzip],
	      [  --enable-wzip         Enable compilation of wesnoth_zip program],
	      [wzip=$enableval],
	      [wzip=no])

AM_CONDITIONAL([STATIC], [test x$static = xyes])
AM_CONDITIONAL([SERVER], [test x$server = xyes])
AM_CONDITIONAL([EDITOR], [test x$editor = xyes])
AM_CONDITIONAL([TOOLS], [test x$tools = xyes])
AM_CONDITIONAL([KDE], [test x$kde = xyes && test x$kdefound = xyes])
AM_CONDITIONAL([GNOME1], [test x$gnome = xyes && test x$gnome1found = xyes])
AM_CONDITIONAL([GNOME2], [test x$gnome = xyes && test x$gnome1found = xno])
AM_CONDITIONAL([GCC], [test x$GXX = xyes])
AM_CONDITIONAL([WZIP], [test x$wzip = xyes])

#######################################################################
# Check for PNG support in SDL_image                                  #
#######################################################################

AC_LANG([C])
AC_MSG_CHECKING([for PNG support in SDL_image])

OLD_CPPFLAGS=$CPPFLAGS
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS $SDL_IMAGE_LIBS"

AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <SDL_image.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	SDL_RWops *src;

	src = SDL_RWFromFile("images/mage.png", "rb");
	if (src == NULL)
		exit(0);
	exit(!IMG_isPNG(src));
}
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)] 
	[AC_MSG_ERROR([*** SDL_image has no PNG support! You need SDL_image with PNG support])],
	[AC_MSG_RESULT[not tested in cross-compiling]])

CPPFLAGS=$OLD_CPPFLAGS
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS


#######################################################################
# Check for OGG support in SDL_mixer                                  #
#######################################################################

if test "x$lite" = "xno"; then
    if test -e "music/wesnoth-1.ogg" ; then
        AC_LANG([C])
	AC_MSG_CHECKING([for OGG support in SDL_mixer])

        OLD_CPPFLAGS=$CPPFLAGS
	OLD_CFLAGS=$CFLAGS
        OLD_LIBS=$LIBS


	CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
        CFLAGS="$CFLAGS $SDL_CFLAGS"
	LIBS="$LIBS $SDL_LIBS $SDL_MIXER_LIBS"

        AC_RUN_IFELSE([AC_LANG_SOURCE([
	#include <SDL_mixer.h>
        #include <stdlib.h>

	int main(int argc, char **argv)
        {
	    char *music;

	    music = (char *) OGG_new("music/wesnoth-1.ogg");
	    if (music == NULL)
		exit(1);
	    exit(0);
        }
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)]
	[AC_MSG_ERROR([*** SDL_mixer has no OGG support! You need SDL_mixer with OGG support])],
	[AC_MSG_RESULT[not tested in cross-compiling]])


        CPPFLAGS=$OLD_CPPFLAGS
	CFLAGS=$OLD_CFLAGS
        LIBS=$OLD_LIBS
    fi
fi


#######################################################################
# Data file substitution.                                             #
#######################################################################

MANUAL_FILES=`ls MANUAL*`
MANUAL_FILES=`echo $MANUAL_FILES`
DATA_FILES=`find data -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
DATA_FILES=`echo $DATA_FILES`
FONT_FILES=`find fonts  -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
FONT_FILES=`echo $FONT_FILES`
if test "x$lite" = "xyes"; then
    PACKAGE=$PACKAGE-lite
    IMAGE_FILES=`find images -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -path "images/misc/tutorial*" -o -path "images/misc/story*" -o -type d -o -print`
    MUSIC_FILES=""
else
    IMAGE_FILES=`find images -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
    MUSIC_FILES=`find music -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
    MUSIC_FILES=`echo $MUSIC_FILES`
fi
IMAGE_FILES=`echo $IMAGE_FILES`
SOUND_FILES=`find sounds -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
SOUND_FILES=`echo $SOUND_FILES`
UTIL_FILES=`find utils -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
UTIL_FILES=`echo $UTIL_FILES`

AC_SUBST([MANUAL_FILES])
AC_SUBST([DATA_FILES])
AC_SUBST([FONT_FILES])
AC_SUBST([IMAGE_FILES])
AC_SUBST([MUSIC_FILES])
AC_SUBST([SOUND_FILES])
AC_SUBST([UTIL_FILES])


AC_CONFIG_FILES([Makefile
		 icons/Makefile
		 src/Makefile
		 src/server/Makefile
		 src/tools/Makefile
		 src/zip/Makefile])

AC_OUTPUT
