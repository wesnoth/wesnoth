# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(wesnoth, 0.7, davidnwhite@optusnet.com.au, Battle for Wesnoth)

AC_REVISION($Version: 0.7$)
PACKAGE=wesnoth
MAJOR_VERSION="0"
MINOR_VERSION="6"
MICRO_VERSION="1RC1"

if test "x$MICRO_VERSION" = "x"; then
        VERSION=$MAJOR_VERSION.$MINOR_VERSION
else
        VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION
fi

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AC_CONFIG_SRCDIR([src/actions.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.

AC_PROG_CXX
AC_PROG_CC

AC_PATH_PROGS(SDL_CONFIG, [sdl-config sdl11-config], none)

if test "x$SDL_CONFIG" = "xnone"; then
	
AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])

fi

# Checks for libraries.

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

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS

SDL_CFLAGS=`$SDL_CONFIG --cflags`
CPPFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CFLAGS $SDL_CFLAGS"


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

# Checks for typedefs, structures, and compiler characteristics.
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

AC_ARG_ENABLE(debug,
	      [  --enable-debug        Enable debug in wesnoth],
	      CXXFLAGS="$CXXFLAGS -DDEBUG -ggdb3")

AC_ARG_ENABLE(datadir-name,
	      [  --datadir-name[[=dir]]    Change name of data directory. Default with this option: wesnoth-data], 
	      [case "${enableval}" in
	       yes)
			DATADIR="wesnoth-data"
			;;
		no)
			DATADIR=$PACKAGE
			;;
		*)
			DATADIR="${enableval}"
			;;
		esac])

AC_SUBST(DATADIR)
		
# Default behaviour
server=no 
editor=no
tools=no

AC_ARG_ENABLE(server,
	      [  --enable-server       Enable compilation of server],
	      server=yes)

AC_ARG_ENABLE(editor,
	      [  --enable-editor       Enable compilation editor],
	      editor=yes)

AC_ARG_ENABLE(tools,
	      [  --enable-tools        Enable compilation of translation tools],
	      tools=yes)

AM_CONDITIONAL(SERVER, test x$server = xyes)
AM_CONDITIONAL(EDITOR, test x$editor = xyes)
AM_CONDITIONAL(TOOLS, test x$tools = xyes)

MANUAL_FILES=`ls MANUAL*`
MANUAL_FILES=`echo $MANUAL_FILES`
DATA_FILES=`find data -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
DATA_FILES=`echo $DATA_FILES`
FONT_FILES=`find fonts  -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
FONT_FILES=`echo $FONT_FILES`
IMAGE_FILES=`find images -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
IMAGE_FILES=`echo $IMAGE_FILES`
MUSIC_FILES=`find music -name CVS -prune -o -name ".cvs*" -o -name ".#*" -o -type d -o -print`
MUSIC_FILES=`echo $MUSIC_FILES`
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
		 src/Makefile
		 src/server/Makefile
		 src/tools/Makefile])

AC_OUTPUT
