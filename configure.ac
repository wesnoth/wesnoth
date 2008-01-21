# Process this file with autoconf to produce a configure script.

#######################################################################
# Initial configuration                                               #
#######################################################################

AC_PREREQ([2.60])

define([WESNOTH_VERSION],[1.3.14+svn])

AC_INIT([Battle for Wesnoth], WESNOTH_VERSION, [isaac@warp.es], [wesnoth])

dnl
dnl Generate wesconfig.h from the information above
dnl
AC_CONFIG_COMMANDS([src/wesconfig.h],
		   [cat > src/wesconfig.h <<EOF
#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

//! @file wesconfig.h
//! Some defines: VERSION, PACKAGE, MIN_SAVEGAME_VERSION
//!
//! DO NOT MODIFY THIS FILE !!!
//! modify configure.ac otherwise the settings will be overwritten.



#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "]WESNOTH_VERSION["
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif

/**
 * Some older savegames of Wesnoth can't be loaded anymore,
 * this variable defines the minimum required version.
 * It is only to be updated upon changes that break *all* saves/replays
 * (break as in crash wesnoth, not compatibility issues like stat changes)
 */
#define MIN_SAVEGAME_VERSION "1.3.10"

#endif
EOF
])

AC_REVISION([$Revision$])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/actions.cpp])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9 tar-ustar foreign])
AM_GNU_GETTEXT([external])
AC_PROG_RANLIB

#######################################################################
# Require GCC 4.x
#######################################################################

set -- `g++ --version`
GCC_VERSION=$3
parts=`echo $GCC_VERSION | tr '.' ' '`
set $parts
GCC_MAJOR_VERSION=$1
GCC_MINOR_VERSION=$2
if test ${GCC_MAJOR_VERSION}${GCC_MINOR_VERSION} -lt 33 
then
	AC_MSG_ERROR([*** G++ major version $GCC_VERSION is too old.])
fi

#######################################################################
# Configuration options                                               #
#######################################################################

AC_ARG_ENABLE([debug],
	      AS_HELP_STRING([--enable-debug], [enable debug in wesnoth]),
	      [debug=$enableval],
	      [debug=no])

if test "x$debug" = "xyes"
then
	CXXFLAGS="$CXXFLAGS -O0 -DDEBUG -ggdb3 -W -Wall -ansi"
else
	CXXFLAGS="-O2 -W -Wall -ansi $CXXFLAGS"
fi


# Make tests default in svn version
svn_in_version=`expr match "$WESNOTH_VERSION" '.*svn'`
test_build=yes
if test $svn_in_version == 0 
then
	#disabling building tests for release version
	test_build=no
fi

AC_ARG_ENABLE([tests],
       	      AS_HELP_STRING([--enable-tests], [build unit tests]),
	      [tests=$enableval],
	      [tests=no])

AC_ARG_ENABLE([static],
              AS_HELP_STRING([--enable-static], [enable static building of wesnoth]),
	      [static=$enableval],
	      [static=no])

AC_ARG_ENABLE([python],
              AS_HELP_STRING([--disable-python], [disable Python support]),
	      [python=$enableval],
	      [python=yes])

AC_ARG_ENABLE([python_install],
              AS_HELP_STRING([--enable-python-install], [enable installation of Python developer tools]),
	      [python_install=$enableval],
	      [python_install=no])

AC_ARG_ENABLE([lite],
              AS_HELP_STRING([--enable-lite], [enable lite version of wesnoth (without music or large images)]),
	      [lite=$enableval],
	      [lite=no])

AC_ARG_ENABLE([tinygui],
              AS_HELP_STRING([--enable-tinygui], [enable GUI reductions for resolutions down to 320x240 (PDAs), resize images before installing]),
	      [tinygui=$enableval],
	      [tinygui=no])

if test "x$tinygui" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DUSE_TINY_GUI"
fi

AM_CONDITIONAL([TINYGUI], [test "x$tinygui" = "xyes"])

AC_ARG_ENABLE([optipng],
	AS_HELP_STRING([--enable-optipng],
	[run optipng png compression before installing graphics]),
	[optipng=$enableval],
	[optipng=no])

AM_CONDITIONAL([OPTIPNG], [test "x$optipng" = "xyes"])

if test "x$optipng" = "xyes"
then
	AC_PATH_PROGS([OPTIPNG_BIN], [optipng], [none])
	if test \( "x$OPTIPNG_BIN" = "xnone" \) -a \( "x$optipng" = xyes \); then
		AC_MSG_ERROR([*** You must install the optipng utility before building with --optipng.])
	else
		echo "****************************************"
		echo "*** optipng support enabled          ***"
		echo "*** please keep in mind that optipng ***"
		echo "*** - may take a long time to run    ***"
		echo "*** - makes graphics unusable for    ***"
		echo "***   artists to work on.            ***"
		echo "***                                  ***"
		echo "*** It is intented to optimize the   ***"
		echo "*** file size for packaging.         ***"
		echo "****************************************"
	fi
fi

AC_ARG_ENABLE([lowmem],
              AS_HELP_STRING([--enable-lowmem], [reduce memory usage by removing extra functionality]),
	      [lowmem=$enableval],
	      [lowmem=no])

if test "x$lowmem" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DLOW_MEM"
fi

AM_CONDITIONAL([LOWMEM], [test "x$lowmem" = "xyes"])

DATADIR=$PACKAGE
AC_ARG_WITH([datadir-name],
            AS_HELP_STRING([--with-datadir-name@<:@=DIR@:>@], [change name of data directory @<:@wesnoth@:>@]),
	      [case "${withval}" in
	       yes)
			DATADIR="wesnoth"
			;;
		no)
			;;
		*)
			DATADIR="${withval}"
			;;
		esac])
AC_SUBST([DATADIR])

#LOCALEDIR="$datadir/locale"
LOCALEDIR=translations
AC_ARG_WITH([localedir],
            AS_HELP_STRING([--with-localedir@<:@=DIR@:>@], [install locale data under dir @<:@translations@:>@]),
	      [case "${withval}" in
	       yes)
			LOCALEDIR="translations"
			;;
		no)
			;;
		*)
			LOCALEDIR="${withval}"
			;;
		esac])
AC_SUBST([LOCALEDIR])

case "`eval echo \"$LOCALEDIR\"`" in
/*) FULLLOCALEDIR="$LOCALEDIR"; HAS_RELATIVE_LOCALEDIR=0 ;;
*)  FULLLOCALEDIR='${datadir}/${DATADIR}/${LOCALEDIR}'; HAS_RELATIVE_LOCALEDIR=1 ;;
esac
AC_SUBST([FULLLOCALEDIR])
AC_SUBST([HAS_RELATIVE_LOCALEDIR])


AC_ARG_ENABLE([game],
              AS_HELP_STRING([--disable-game], [disable compilation of game]),
	      [game=$enableval],
	      [game=yes])

AC_ARG_ENABLE([server],
              AS_HELP_STRING([--enable-server], [enable compilation of server]),
	      [server=$enableval],
	      [server=no])

AC_ARG_WITH([fifodir],
	         AS_HELP_STRING([--with-fifodir], [directory for the wesnothd fifo socket file]),
	      [fifodir=$withval],
	      [fifodir=$localstatedir/run/wesnothd])
AC_SUBST([fifodir])

AC_ARG_WITH([server-uid],
	          AS_HELP_STRING([--with-server-uid], [user id of the user who runs wesnothd]),
	      [serveruid=$withval],
	      [serveruid=""])
AC_SUBST([serveruid])

AC_ARG_WITH([server-gid],
	          AS_HELP_STRING([--with-server-gid], [group id of the user who runs wesnothd]),
	      [servergid=$withval],
	      [servergid=""])
AC_SUBST([servergid])

AC_ARG_ENABLE([campaign_server],
              AS_HELP_STRING([--enable-campaign-server], [enable compilation of campaign server]),
	      [campaignserver=$enableval],
	      [campaignserver=no])

AC_ARG_ENABLE([editor],
              AS_HELP_STRING([--enable-editor], [enable compilation of map editor]),
	      [editor=$enableval],
	      [editor=no])

AC_ARG_ENABLE([tools],
              AS_HELP_STRING([--enable-tools], [enable building and installation of tools for artists and WML maintainers]),
	      [tools=$enableval],
	      [tools=no])

AC_ARG_WITH([fribidi],
            AS_HELP_STRING([--without-fribidi], [disable Bidirectional language support]),
	      [fribidi=$withval],
	      [fribidi=yes])

AC_ARG_ENABLE([dummy-locales],
              AS_HELP_STRING([--enable-dummy-locales], [enable installation of Wesnoth own private locales]),
              [dummylocales=$enableval],
              [dummylocales=no])

AC_ARG_WITH([preferences-dir],
              AS_HELP_STRING([--with-preferences-dir], [use a non-default preferences directory (.wesnoth on unix)]),
              [prefsdir=$withval
               AC_SUBST([prefsdir])])
AM_CONDITIONAL([PREFSDIR], [test x$prefsdir != x])


AC_ARG_ENABLE([internal-data],
		AS_HELP_STRING([--enable-internal-data],
			[put data inside application: Mac OS X only]),
		[internaldata=$enableval],
		[internaldata=no])

AC_ARG_ENABLE([display-revision],
              AS_HELP_STRING([--enable-display-revision], [enable svn revision display]),
	      [svnrev=$enableval],
	      [svnrev=no])


if test "x$game" = "xno"
then
	python=no
	AC_MSG_WARN([*** Game build disabled, suppressing Python support.])
fi

if test "x$python" = "xno"
then
	python_install=no
	AC_MSG_WARN([*** Python support disabled, suppressing installation of Python tools.])
fi

AC_ARG_ENABLE([raw-sockets],
              AS_HELP_STRING([--enable-raw-sockets], [use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities]),
	      [raw_sockets=$enableval],
	      [raw_sockets=no])

if test "x$raw_sockets" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DNETWORK_USE_RAW_SOCKETS"
fi

AM_CONDITIONAL([STATIC], [test x$static = xyes])
AM_CONDITIONAL([PYTHON_INSTALL], [test x$python_install = xyes])
AM_CONDITIONAL([GAME], [test x$game = xyes])
AM_CONDITIONAL([SERVER], [test x$server = xyes])
AM_CONDITIONAL([CAMPAIGNSERVER], [test x$campaignserver = xyes])
AM_CONDITIONAL([TESTS], [test x$tests = xyes])
AM_CONDITIONAL([EDITOR], [test x$editor = xyes])
AM_CONDITIONAL([USESVN], [test x$svnrev = xyes])
AM_CONDITIONAL([TOOLS], [test x$tools = xyes])
AM_CONDITIONAL([GCC], [test x$GXX = xyes])
AM_CONDITIONAL([INCLUDEDINTL], [test x$nls_cv_use_gnu_gettext = xyes])
AM_CONDITIONAL([INSTALLDATA], [test x$game = xyes || x$editor = xyes])
AM_CONDITIONAL([DUMMYLOCALES], [test x$dummylocales = xyes])

if test x$dummylocales = xyes; then
  AC_DEFINE([USE_DUMMYLOCALES],,[Define if the game should not use system locales])
fi

if test x$internaldata = xyes; then
  AC_DEFINE([USE_INTERNAL_DATA],[],[Define if translations should be placed inside app, for Mac OS X])
fi

AC_ARG_ENABLE([desktop-entry],
              AS_HELP_STRING([--disable-desktop-entry], [disable installation of desktop entry files]),
	      [desktopentry=$enableval],
	      [desktopentry=yes])

# Allow user to override default icondir and desktopdir paths
AC_ARG_WITH([icondir],
            AS_HELP_STRING([--with-icondir@<:@=DIR@:>@], [change icon directory for desktop entry]),
		  [APP_ICON="${with_icondir}"],
		  [APP_ICON="${datadir}"/icons])

AC_ARG_WITH([desktopdir],
            AS_HELP_STRING([--with-desktopdir@<:@=DIR@:>@], [change desktop file directory for desktop entry]),
		   [APP_ENTRY="${with_desktopdir}"],
		   [APP_ENTRY="${datadir}"/applications])

AC_SUBST([APP_ENTRY])
AC_SUBST([APP_ICON])

AM_CONDITIONAL(GAME_DESKTOP_ENTRY, [test x$desktopentry = xyes && \
	test x$game = xyes])

#######################################################################
# Checks for programs.                                                #
#######################################################################

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

have_libx11='no'
if test "$with_x" != 'no'; then

dnl Locate X include files and libraries
   AC_PATH_XTRA
   NEW_LIBS="$X_LIBS -lX11"

   AC_CHECK_LIB(X11, XOpenDisplay, have_libx11='yes',have_libx11='no',$X_LIBS)
   if test "$have_libx11" != 'no'; then
     AC_DEFINE([HAVE_LIBX11],,[Define if you have X11 libraries])
     X_LIBS="$NEW_LIBS"
     CPPFLAGS="$X_CFLAGS $CPPFLAGS"
     LIBS="$X_LIBS $LIBS"
   fi
fi

AM_CONDITIONAL([X11], [test "$have_libx11" = 'yes'])

# We need pngmeta for the tinygui option

AC_PATH_PROGS([PNGMETA], [pngmeta], [none])

if test \( "x$PNGMETA" = "xnone" \) -a \( "x$TINYGUI" = xyes \); then
	AC_MSG_ERROR([*** You must install the pngmeta utility before building with -- tinygui.])
fi

# SDL_CONFIG

AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])

if test "x$SDL_CONFIG" = "xnone"; then

	AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])

fi

# fribidi-config

AC_PATH_PROGS([FRIBIDI_CONFIG], [fribidi-config], [none])

if test "x$FRIBIDI_CONFIG" = "xnone"; then
    fribidifound=no
    AC_MSG_WARN([*** FRIBIDI not found.])
else
    fribidifound=yes
    FRIBIDI_CFLAGS=`$FRIBIDI_CONFIG --cflags`
    FRIBIDI_LIBS=`$FRIBIDI_CONFIG --libs`
fi

AC_SUBST([FRIBIDI_CFLAGS])
AC_SUBST([FRIBIDI_LIBS])
AM_CONDITIONAL([FRIBIDI], [test "x$fribidifound" = xyes -a "x$fribidi" = xyes ])

# python
if test "x$python" = "xyes"; then
	pythonfound=yes
	if test "x$PYTHON" = "x"; then
		AC_PATH_PROG(PYTHON, python, none)
	fi

	if test "x$PYTHON" = "xnone"; then
    	AC_MSG_WARN([*** Python interpreter not found, Python support disabled.])
		pythonfound=no
	fi

	if test "x$pythonfound" = "xyes"; then
		AC_MSG_CHECKING(Python version and location)
		PYTHON_PREFIX=`$PYTHON -c "import sys; print sys.prefix"`
		PYTHON_VERSION_MAJOR=[`$PYTHON -c "import sys; print '%d' % (sys.version_info[0]);"`]
		PYTHON_VERSION_MINOR=[`$PYTHON -c "import sys; print '%d' % (sys.version_info[1]);"`]
		PYTHON_VERSION="${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}"
		AC_MSG_RESULT([$PYTHON, $PYTHON_VERSION, $PYTHON_PREFIX])

		AC_MSG_CHECKING(whether Python is at least 2.4)
		if test $PYTHON_VERSION_MAJOR -lt 2 -o $PYTHON_VERSION_MAJOR -eq 2 -a $PYTHON_VERSION_MINOR -lt 4; then
			AC_MSG_RESULT(no)
			AC_MSG_WARN([*** Wesnoth requires at least Python 2.4, Python support disabled.])
			pythonfound=no
		fi
		if test "x$pythonfound" = "xyes"; then
			AC_MSG_RESULT(yes)

			PYTHON_CFLAGS="-DHAVE_PYTHON -I$PYTHON_PREFIX/include/python$PYTHON_VERSION"

			OLD_CPPFLAGS="$CPPFLAGS"
			OLD_CXXFLAGS="$CXXFLAGS"
			CPPFLAGS="$CPPFLAGS $PYTHON_CFLAGS"
			CXXFLAGS="$CXXFLAGS $PYTHON_CFLAGS"

			AC_CHECK_HEADER([Python.h],
					[],
					[AC_MSG_WARN([*** Python include files not found! You should install Python development package. Python support disabled]); pythonfound=no])
			CPPFLAGS="$OLD_CPPFLAGS"
			CXXFLAGS="$OLD_CXXFLAGS"

			if test "x$pythonfound" = "xyes"; then
				AC_SUBST([PYTHON_CFLAGS])

				pythonfound=no
				for pylibpath in '/usr/lib' $PYTHON_PREFIX/lib $PYTHON_PREFIX/lib/python$PYTHON_VERSION/config; do
					eval `echo unset ac_cv_lib_python$PYTHON_VERSION'___'Py_Finalize | tr '.' '_'`

					save_LIBS=$LIBS
					LIBS="$LIBS -L$pylibpath"
					AC_CHECK_LIB(python$PYTHON_VERSION, Py_Finalize, PYTHON_LIBS="-L$pylibpath -lpython$PYTHON_VERSION $PYTHON_DEPS"; pythonfound=yes,,$PYTHON_DEPS)
					LIBS=$save_LIBS
					if test "x$pythonfound" = "xyes"; then
						break
					fi
				done

				if test "x$pythonfound" != "xyes"; then
					AC_MSG_WARN(*** Python development libraries required, Python support disabled)
				fi
				AC_SUBST([PYTHON_LIBS])

				AC_SUBST(pkgpythondir)
				if test "x$python_install" = "xyes"; then
					pkgpythondir=$PYTHON_PREFIX"/lib/python"$PYTHON_VERSION"/site-packages/wesnoth"
				fi

			fi
		fi
	fi
fi
AM_CONDITIONAL([PYTHON], [test "x$pythonfound" = xyes -a "x$python" = xyes ])

# libpng-config

AC_PATH_PROGS([PNG_CONFIG], [libpng-config libpng12-config], [none])

if test "x$PNG_CONFIG" = "xnone"; then
    AC_PATH_PROG([PNG_CONFIG], [pkg-config], [none])
    if test "x$PNG_CONFIG" = "xnone"; then
	pngfound=no
	AC_MSG_WARN([*** LIBPNG not found.])
    else
	pngfound=yes
	PNG_CFLAGS=`$PNG_CONFIG --cflags libpng12`
	PNG_LIBS=`$PNG_CONFIG --libs libpng12`
    fi
else
    pngfound=yes
    PNG_CFLAGS=`$PNG_CONFIG --cflags`
    PNG_LIBS=`$PNG_CONFIG --libs`
fi

AC_SUBST([PNG_CFLAGS])
AC_SUBST([PNG_LIBS])
AM_CONDITIONAL([LIBPNG], [test x$pngfound = xyes])

# Check for SDL version. Taken from sdl.m4

AC_ARG_ENABLE([sdltest],
              AS_HELP_STRING([--disable-sdltest], [do not try to compile and run a test SDL program]),
	      ,
              [enable_sdltest=yes])

min_sdl_version=1.2.7
AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_LIBS=`$SDL_CONFIG --libs`
sdl_major_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
sdl_minor_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
sdl_micro_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

if test "x$enable_sdltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $SDL_CFLAGS"
    LIBS="$LIBS $SDL_LIBS"

    # Now check if the installed SDL is sufficiently new. (Also sanity
    # checks the results of sdl-config to some extent

    rm -f conf.sdltest
    AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;

  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;

  return new_str;
}

int main (int argc, char **argv)
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}
    ])],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)]
    [AC_MSG_ERROR([*** Please upgrade your SDL version])],
    [AC_MSG_RESULT([not tested in cross-compiling])])
    rm -f conf.sdltest

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"
fi

# po4a

AC_PATH_PROGS([PO4A], [po4a], [none])

if test "x$PO4A" = "xnone"; then
    po4afound=no
else
    po4afound=yes
fi

AM_CONDITIONAL([PO4AUPDATE], [test "x$po4afound" = "xyes"])

# manual: asciidoc, dos2unix, xsltproc

AC_PATH_PROGS([ASCIIDOC], [asciidoc], [none])
AC_PATH_PROGS([DOS2UNIX], [dos2unix], [none])
AC_PATH_PROGS([XSLTPROC], [xsltproc], [none])

if test "x$PO4A" = "xnone" || test "x$ASCIIDOC" = "xnone" ||
   test "x$DOS2UNIX" = "xnone" || test "x$XSLTPROC" = "xnone" ; then
    manualdeps=no
else
    manualdeps=yes
fi

AM_CONDITIONAL([MANUALUPDATE], [test "x$manualdeps" = "xyes"])

#######################################################################
# Checks for libraries.                                               #
#######################################################################

# Use a modified version of ac_link so that libtool gets called
# this seems pretty broken on most systems
AC_PATH_PROG([LTOOL], [libtool], [])
if test "$static" = "yes" -a -n "$LTOOL"
then
	LDPREFIX="$LTOOL --mode=link --tag=CXX"
else
	LDPREFIX=""
fi
AC_SUBST([LDPREFIX])

#

if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL.la
then SDL_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL.la
else SDL_LIBS=`$SDL_CONFIG --libs`
fi
case $host_os in
	darwin*)
		SDL_LIBS="-framework Carbon $SDL_LIBS"
esac
case $host_os in
	mingw32*)
		SDL_LIBS="-lunicows $SDL_LIBS"
esac
OLD_LIBS=$LIBS
LIBS="$LIBS $SDL_LIBS"

# There's no need for this, $SDL_CONFIG comes with libsdl and
# it doesn't find it in FreeBSD
# AC_CHECK_LIB([SDL], [SDL_Init])
# unfortunately, sdl_config is not shipped with the Mac OS X packages...
# so recommend using fink sdl packages as a workaround

ac_link="$LDPREFIX $ac_link"
AC_CHECK_LIB([SDL_image],
	     [IMG_Load],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_image.la
then SDL_IMAGE_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_image.la
else SDL_IMAGE_LIBS=-lSDL_image
fi],
	     [AC_MSG_ERROR([*** SDL_image lib not found! Get SDL_image from
http://www.libsdl.org/projects/SDL_image/index.html])])

AC_CHECK_LIB([SDL_mixer],
	     [Mix_OpenAudio],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
then SDL_MIXER_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
else SDL_MIXER_LIBS=-lSDL_mixer
fi],
	     [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])

AC_CHECK_LIB([SDL_net],
	     [SDLNet_Init],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_net.la
then SDL_NET_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_net.la
else SDL_NET_LIBS=-lSDL_net
fi],
	     [AC_MSG_ERROR([*** SDL_net lib not found! Get SDL_net from
http://www.libsdl.org/projects/SDL_net/index.html])])

LIBS=$OLD_LIBS

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_IMAGE_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_NET_LIBS])

# Checking for the freetype library. This was copied from the
# config script of SDL_ttf, modified for CPPFLAGS

dnl Get the cflags and libraries from the freetype-config script

AC_ARG_WITH([freetype-prefix],
            AS_HELP_STRING([--with-freetype-prefix=PFX], [Prefix where FREETYPE is installed (optional)]),
            [freetype_prefix="$withval"],
            [freetype_prefix=""])

AC_ARG_WITH([freetype-exec-prefix],
            AS_HELP_STRING([--with-freetype-exec-prefix=PFX], [Exec prefix where FREETYPE is installed (optional)]),
            [freetype_exec_prefix="$withval"],
            [freetype_exec_prefix=""])

if test x$freetype_exec_prefix != x ; then
     freetype_args="$freetype_args --exec-prefix=$freetype_exec_prefix"
     if test x${FREETYPE_CONFIG+set} != xset ; then
        FREETYPE_CONFIG=$freetype_exec_prefix/bin/freetype-config
     fi
fi
if test x$freetype_prefix != x ; then
     freetype_args="$freetype_args --prefix=$freetype_prefix"
     if test x${FREETYPE_CONFIG+set} != xset ; then
        FREETYPE_CONFIG=$freetype_prefix/bin/freetype-config
     fi
fi
AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
no_freetype=""
if test "$FREETYPE_CONFIG" = "no" ; then
    AC_MSG_ERROR([
*** Unable to find FreeType2 library (http://www.freetype.org/)
])
else
    CPPFLAGS="$CPPFLAGS `$FREETYPE_CONFIG $freetypeconf_args --cflags`"
    FREETYPE_LIBS=`$FREETYPE_CONFIG $freetypeconf_args --libs`
fi

AC_SUBST([FREETYPE_LIBS])

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

CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS

AC_SUBST([SDL_CFLAGS])

AC_CHECK_HEADERS([stdlib.h unistd.h poll.h sys/poll.h sys/select.h])


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
# Check for PNG support in SDL_image                                  #
#######################################################################

AC_LANG([C])
AC_MSG_CHECKING([for PNG support in SDL_image])

OLD_CPPFLAGS=$CPPFLAGS
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS $SDL_IMAGE_LIBS -lz"

ac_link="$LDPREFIX $ac_link"
AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <SDL_image.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	SDL_RWops *src;
	char *testimage = "images/buttons/button-pressed.png";

	src = SDL_RWFromFile(testimage, "rb");
	if (src == NULL) {
		exit(2);
	}
	exit(!IMG_isPNG(src));
}
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)] 
	[AC_MSG_ERROR([*** Either your test image has vanished, or SDL_image has no PNG support!])],
	[AC_MSG_RESULT([not tested in cross-compiling])])

CPPFLAGS=$OLD_CPPFLAGS
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS


#######################################################################
# Check for OGG support in SDL_mixer                                  #
#######################################################################

if test "x$lite" = "xno"; then
    if test -e "data/core/music/main_menu.ogg" ; then
        AC_LANG([C])
	AC_MSG_CHECKING([for OGG support in SDL_mixer])

        OLD_CPPFLAGS=$CPPFLAGS
	OLD_CFLAGS=$CFLAGS
        OLD_LIBS=$LIBS


	CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
        CFLAGS="$CFLAGS $SDL_CFLAGS"
	LIBS="$LIBS $SDL_LIBS $SDL_MIXER_LIBS"

	ac_link="$LDPREFIX $ac_link"
        AC_RUN_IFELSE([AC_LANG_SOURCE([
	#include <SDL_mixer.h>
        #include <stdlib.h>

	int main(int argc, char **argv)
        {
	    Mix_Music* music = Mix_LoadMUS("data/core/music/main_menu.ogg");
	    if (music == NULL)
		exit(1);
	    exit(0);
        }
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)]
	[AC_MSG_ERROR([*** SDL_mixer has no OGG support! You need SDL_mixer with OGG support])],
	[AC_MSG_RESULT([not tested in cross-compiling])])


        CPPFLAGS=$OLD_CPPFLAGS
        CFLAGS=$OLD_CFLAGS
        LIBS=$OLD_LIBS
    fi
fi

#######################################################################
# Check for boost iostreams                                           #
#######################################################################

BOOST_REQUIRE([1.33])
BOOST_IOSTREAMS
CPPFLAGS="$BOOST_CPPFLAGS $CPPFLAGS"
LDFLAGS="$BOOST_IOSTREAMS_LDFLAGS $LDFLAGS"
LIBS="$BOOST_IOSTREAMS_LIBS $LIBS"


if test "x$tests" = "xyes"; then
		
	BOOST_TEST
	# workaround for broken 1.33.1 debian packet
	if test x"$BOOST_UNIT_TEST_FRAMEWORK_LIBS" = x; then
		BOOST_UNIT_TEST_FRAMEWORK_LIBS="-lboost_unit_test_framework"
#		BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS="-L/usr/lib -R/usr/lib"
	fi

	AC_LANG([C++])
	AC_MSG_CHECKING([for dynamic linked boost test])

        OLD_CPPFLAGS=$CPPFLAGS
	OLD_CFLAGS=$CFLAGS
        OLD_LIBS=$LIBS


	CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        CFLAGS="$CFLAGS $BOOST_CPPFLAGS"
	LIBS="$LIBS $BOOST_UNIT_TEST_FRAMEWORK_LIBS"

	ac_link="$LDPREFIX $ac_link"
        AC_LINK_IFELSE([AC_LANG_SOURCE([
	#define BOOST_TEST_DYN_LINK
	#define BOOST_TEST_MAIN
        #include <boost/test/unit_test.hpp>

	])],
	[AC_MSG_RESULT(yes)]
	[boost_test_dyn_link=yes],
	[AC_MSG_RESULT(no)]
	[boost_test_dyn_link=no])

	AC_MSG_CHECKING([for boost auto test not in core])

	m4_pattern_allow([^BOOST_AUTO_TEST_CASE$])

	AC_LINK_IFELSE([AC_LANG_SOURCE([
	#define BOOST_AUTO_TEST_MAIN
        #include <boost/test/auto_unit_test.hpp>

	])],
	[AC_MSG_RESULT(yes)]
	[boost_auto_test=yes],
	[AC_MSG_RESULT(no)]
	[boost_auto_test=no])


        CPPFLAGS=$OLD_CPPFLAGS
        CFLAGS=$OLD_CFLAGS
        LIBS=$OLD_LIBS

fi

m4_pattern_allow([^BOOST_TEST_DYN_LINK$])
m4_pattern_allow([^BOOST_AUTO_TEST$])
	
AM_CONDITIONAL([BOOST_TEST_DYN_LINK], [test x"$boost_test_dyn_link" = xyes])
AM_CONDITIONAL([BOOST_AUTO_TEST], [test x"$boost_auto_test" = xyes])

#######################################################################
# Tune gettext stuff for our needs                                    #
#######################################################################

case $srcdir in
/*) topdir=$srcdir ;;
*)  topdir=`pwd`/$srcdir ;;
esac
for domain in `grep ^SUBDIRS $srcdir/po/Makefile.am | cut -d= -f2`
do
    # Symlinks to the single copy of Makefile.in.in
    echo "creating po/$domain/Makefile.in.in"
    mkdir -p po/$domain
    rm -f po/$domain/Makefile.in.in
    ln -s "$topdir/po/Makefile.in.in" "po/$domain/Makefile.in.in"
done

AC_CONFIG_COMMANDS([translations],
[rm -rf translations
case $srcdir in
/*) topdir=$srcdir ;;
*)  topdir=`pwd`/$srcdir ;;
esac
for domain in `grep ^SUBDIRS $srcdir/po/Makefile.am | cut -d= -f2`
do
    # Symlinks that allow message catalogs to be used from build tree
    if test -w $srcdir; then
	for lang in `cat $srcdir/po/$domain/LINGUAS`
	do
	    mkdir -p $srcdir/translations/$lang/LC_MESSAGES
	    rm -f $srcdir/translations/$lang/LC_MESSAGES/$domain.mo
	    ln -s $topdir/po/$domain/$lang.gmo $srcdir/translations/$lang/LC_MESSAGES/$domain.mo
	done
    fi
done])

#######################################################################
# Data file substitution.                                             #
#######################################################################

AM_CONDITIONAL([LITE], [test "x$lite" = "xyes"])
if test "x$lite" = "xyes"; then
    PACKAGE=$PACKAGE-lite
fi
AC_SUBST([DATA_FILES])
AC_SUBST([FONT_FILES])
AC_SUBST([IMAGE_FILES])
AC_SUBST([MUSIC_FILES])
AC_SUBST([SOUND_FILES])

AC_CONFIG_FILES([Makefile
		 po/Makefile
		 po/wesnoth/Makefile.in
		 po/wesnoth-editor/Makefile.in
		 po/wesnoth-lib/Makefile.in
		 po/wesnoth-units/Makefile.in
		 po/wesnoth-multiplayer/Makefile.in
		 po/wesnoth-tutorial/Makefile.in
		 po/wesnoth-did/Makefile.in
		 po/wesnoth-ei/Makefile.in
		 po/wesnoth-httt/Makefile.in
		 po/wesnoth-l/Makefile.in
		 po/wesnoth-nr/Makefile.in
		 po/wesnoth-sof/Makefile.in
		 po/wesnoth-sotbe/Makefile.in
		 po/wesnoth-tb/Makefile.in
		 po/wesnoth-thot/Makefile.in
		 po/wesnoth-trow/Makefile.in
		 po/wesnoth-tsg/Makefile.in
		 po/wesnoth-utbs/Makefile.in
po/wesnoth-aoi/Makefile.in
		 m4/Makefile
		 icons/Makefile
		 src/Makefile
		 doc/Makefile
		 doc/man/Makefile
		 doc/manual/Makefile])

AC_OUTPUT
