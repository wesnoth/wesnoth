# Process this file with autoconf to produce a configure script.

#######################################################################
# Initial configuration                                               #
#######################################################################

AC_PREREQ([2.60])

define([WESNOTH_VERSION],[1.9.0-svn])
define([WESNOTH_BUGS],[http://bugs.wesnoth.org])

AC_INIT([Battle for Wesnoth], WESNOTH_VERSION, WESNOTH_BUGS, [wesnoth])

AC_REVISION([$Revision$])

AM_MAINTAINER_MODE([enable])

echo "****************************************"
echo "***                                  ***"
echo "***  At the moment we are also       ***"
echo "***  working on cmake and scons      ***"
echo "***  based build systems which       ***"
echo "***  might replace the autotools     ***"
echo "***  build system. Due to this       ***"
echo "***  autotools might be become       ***"
echo "***  deprecated.                     ***"
echo "***  Please test scons and cmake     ***"
echo "***  and report any problems!        ***"
echo "***                                  ***"
echo "***  To build with scons, type       ***"
echo "***  'scons' in the distribution     ***"
echo "***  directory.                      ***"
echo "***                                  ***"
echo "***  To build with cmake, type       ***"
echo "***  'cmake .' followed by 'make'    ***"
echo "***  in the distribution directory   ***"
echo "***  or create a directory to build  ***"
echo "***  in and type                     ***"
echo "***  'cmake PATH/TO/WESNOTH/SOURCE'  ***"
echo "***  followed by 'make'.             ***"
echo "***                                  ***"
echo "***  See the INSTALL file for more   ***"
echo "***  details.                        ***"
echo "***                                  ***"
echo "****************************************"

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/actions.cpp])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9 tar-ustar foreign])
AM_GNU_GETTEXT([external])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_PROG_RANLIB

#######################################################################
# Require a recent GCC.
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

AC_ARG_ENABLE([profile],
	      AS_HELP_STRING([--enable-profile], [enable profiling in wesnoth]),
	      [profile=$enableval],
	      [profile=no])

AC_ARG_ENABLE([strict-compilation],
	      AS_HELP_STRING([--disable-strict-compilation], [disable the strict compilation rules (warnings are an error in the strict mode)]),
	      [strict=$enableval],
	      [strict=yes])

if test "x$debug" = "xyes"
then
	# Don't use ansi for CFLAGS since it fails to compile.
	CFLAGS="-O0 -DDEBUG -ggdb3 -W -Wall $CFLAGS"
	CXXFLAGS="-O0 -DDEBUG -ggdb3 -W -Wall -ansi $CXXFLAGS"
else
	# Don't use ansi for CFLAGS since it fails to compile.
	CFLAGS="-O2 -W -Wall $CFLAGS"
	CXXFLAGS="-O2 -W -Wall -ansi $CXXFLAGS"
fi

if test "x$profile" = "xyes"
then
	CFLAGS="$CFLAGS -pg"
	CXXFLAGS="$CXXFLAGS -pg"
fi

if test "x$strict" = "xyes"
then
#   Strict compilation for C files is disabled until somebody wants to clean them.
#	CFLAGS="-Werror -Wno-unused -Wno-sign-compare $CFLAGS"

	# The current networking code breaks strict aliasing in g++ 4.5.
	# Once the code is converted to boost::asio it might be reenabled.
	CXXFLAGS="-Werror -Wno-strict-aliasing $CXXFLAGS"
fi

# Make tests default in svn version
svn_in_version=`expr match "$WESNOTH_VERSION" '.*svn'`
test_build=yes
if test $svn_in_version = 0 
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

AC_ARG_ENABLE([python_install],
              AS_HELP_STRING([--enable-python-install], [enable installation of Python developer tools]),
	      [python_install=$enableval],
	      [python_install=no])

AC_ARG_ENABLE([tinygui],
              AS_HELP_STRING([--enable-tinygui], [enable GUI reductions for resolutions down to 320x240 (PDAs), resize images before installing]),
	      [tinygui=$enableval],
	      [tinygui=no])

if test "x$tinygui" = "xyes"
then
    CPPFLAGS="$CPPFLAGS -DUSE_TINY_GUI"
fi

AM_CONDITIONAL([TINYGUI], [test "x$tinygui" = "xyes"])

AC_ARG_ENABLE([lowmem],
              AS_HELP_STRING([--enable-lowmem], [reduce memory usage by removing extra functionality]),
	      [lowmem=$enableval],
	      [lowmem=no])

if test "x$lowmem" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DLOW_MEM"
fi

AM_CONDITIONAL([LOWMEM], [test "x$lowmem" = "xyes"])

AC_ARG_ENABLE([debug-window-layout],
			  AS_HELP_STRING([--enable-debug-window-layout], [add the debug option to allow the generation of debug layout files in dot format]),
	      [debug_window_layout=$enableval],
	      [debug_window_layout=no])

if test "x$debug_window_layout" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DDEBUG_WINDOW_LAYOUT_GRAPHS"
fi


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
	      [server=yes])

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
            AS_HELP_STRING([--disable-editor], [disable the map editor in the game executable]),
	      [editor=$enableval],
	      [editor=yes])

AC_ARG_ENABLE([tools],
              AS_HELP_STRING([--enable-tools], [enable building and installation of tools for artists and WML maintainers]),
	      [tools=$enableval],
	      [tools=no])

AC_ARG_WITH([fribidi],
            AS_HELP_STRING([--without-fribidi], [disable Bidirectional language support]),
	      [fribidi=$withval],
	      [fribidi=yes])

AC_ARG_ENABLE([ana],
              AS_HELP_STRING([--enable-ana], [use the experimental ANA networking framework (work-in-progress)]),
	      [ana=$enableval],
	      [ana=yes])

AC_ARG_ENABLE([pool-alloc],
              AS_HELP_STRING([--enable-pool-alloc], [enable the pool allocator]),
	      [poolalloc=$enableval],
	      [poolalloc=no])

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

AC_ARG_ENABLE([notifications],
		AS_HELP_STRING([--disable-notifications],
			[disable OS-specific notifications]),
		[notifications=$enableval],
		[notifications=yes])

AC_ARG_ENABLE([dbus],
		AS_HELP_STRING([--disable-dbus],
			[disable dbus support for notifications]),
		[dbus=$enableval],
		[dbus=yes])

#if test "x$game" = "xno"
#then
#	python=no
#	AC_MSG_WARN([*** Game build disabled, suppressing Python support.])
#fi

if test "x$python" = "xno"
then
	python_install=no
	AC_MSG_WARN([*** Python support disabled, suppressing installation of Python tools.])
fi

AC_ARG_ENABLE([raw-sockets],
              AS_HELP_STRING([--enable-raw-sockets], [use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities]),
	      [raw_sockets=$enableval],
	      [raw_sockets=no])

AC_ARG_ENABLE([bandwidth-monitor],
              AS_HELP_STRING([--enable-bandwidth-monitor], [Enable bandwidth monitoring for server]),
	      [bandwidth_monitor=$enableval],
	      [bandwidth_monitor=no])

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
AM_CONDITIONAL([BANDWIDTH_MONITOR], [test x$bandwidth_monitor = xyes])
AM_CONDITIONAL([TOOLS], [test x$tools = xyes])
AM_CONDITIONAL([GCC], [test x$GXX = xyes])
AM_CONDITIONAL([INCLUDEDINTL], [test x$nls_cv_use_gnu_gettext = xyes])
AM_CONDITIONAL([INSTALLDATA], [test [ x$game = xyes || test x$editor = xyes ]])
AM_CONDITIONAL([POOLALLOC], [test x$poolalloc = xyes])
AM_CONDITIONAL([ANA], [test x$ana = xyes])

if test x$editor = xno
then
	CFLAGS="$CFLAGS -DDISABLE_EDITOR"
	CXXFLAGS="$CXXFLAGS -DDISABLE_EDITOR"
fi

if test x$poolalloc = xno
then
	CFLAGS="$CFLAGS -DDISABLE_POOL_ALLOC"
	CXXFLAGS="$CXXFLAGS -DDISABLE_POOL_ALLOC"
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

# SDL_CONFIG

AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])

if test "x$SDL_CONFIG" = "xnone"; then

	AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])

fi

# fribidi-config

AS_IF([test "x$fribidi" != "xno"],
	[PKG_CHECK_MODULES([FRIBIDI2], [fribidi >= 0.19.0],
		[
			fribidifound=yes
		],
		[
			AC_MSG_RESULT(no)
			PKG_CHECK_MODULES([FRIBIDI], [fribidi],
				[
					fribidifound=yes
					oldfribidi=yes
				],
				[
					fribidifound=no
					AC_MSG_RESULT(no)
				])
		])])
AM_CONDITIONAL([FRIBIDI], [test "x$fribidifound" = xyes])
AM_CONDITIONAL([OLD_FRIBIDI], [test "x$oldfribidi" = xyes])

# python
AC_PATH_PROG(PYTHON, python, none)
AC_SUBST(pkgpythondir)
if test "x$python_install" = "xyes"; then
	pkgpythondir=$PYTHON_PREFIX"/lib/python"$PYTHON_VERSION"/site-packages/wesnoth"
fi

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
min_sdl_ttf_version=2.0.8
AC_MSG_CHECKING(for SDL - version >= $min_sdl_version and SDL_ttf - version >= $min_sdl_ttf_version)

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_LIBS=`$SDL_CONFIG --libs`
sdl_major_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
sdl_minor_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
sdl_micro_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

sdl_ttf_major_version=`echo $min_sdl_ttf_version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
sdl_ttf_minor_version=`echo $min_sdl_ttf_version| \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
sdl_ttf_micro_version=`echo $min_sdl_ttf_version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`


if test "x$enable_sdltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $SDL_CFLAGS"
    LIBS="$LIBS $SDL_LIBS"

    # Now check if the installed SDL is sufficiently new. (Also sanity
    # checks the results of sdl-config to some extent)

    rm -f conf.sdltest
    AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

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
        SDL_version compile_version, *link_version;
        TTF_VERSION(&compile_version);
        if ((compile_version.major > $sdl_ttf_major_version) ||
           ((compile_version.major == $sdl_ttf_major_version) 
               && (compile_version.minor > $sdl_ttf_minor_version)) ||
           ((compile_version.major == $sdl_ttf_major_version) 
               && (compile_version.minor == $sdl_ttf_minor_version) 
               && (compile_version.patch >= $sdl_ttf_micro_version)))
        {
            return 0;
        } else {
            printf("\n*** TTF_VERSION returned %d.%d.%d, but the minimum version\n",compile_version.major, compile_version.minor, compile_version.patch);
            printf("*** required version is %d.%d.%d. Please update your library.\n",$sdl_ttf_major_version,$sdl_ttf_minor_version,$sdl_ttf_micro_version);
            return 1;
        }
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
    [AC_MSG_ERROR([*** Please upgrade your SDL and/or SDL_ttf version])],
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

AC_CHECK_LIB([SDL_ttf],
	     [TTF_Init],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_ttf.la
then SDL_TTF_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_ttf.la
else SDL_TTF_LIBS=-lSDL_ttf
fi],
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
AC_CHECK_LIB(m, floor)
AC_CHECK_FUNCS([round socket strtoul])
AC_CHECK_HEADER([sys/sendfile.h], [AC_CHECK_FUNCS([sendfile])], [])


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
	char *testimage = "${srcdir}/images/buttons/button-pressed.png";

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

#######################################################################
# Check for ZLib                                                      #
#######################################################################

AM_PATH_ZLIB

CPPFLAGS="$CPPFLAGS $ZLIB_CFLAGS"
CFLAGS="$CFLAGS $ZLIB_CFLAGS"
LIBS="$LIBS $ZLIB_LIBS"

#######################################################################
# Check for Lua                                                       #
#######################################################################

PKG_CHECK_MODULES(LUA, [lua >= 5.1], , [
	AC_MSG_RESULT([no])
	PKG_CHECK_MODULES(LUA, [lua5.1 >= 5.1], , [
		AC_MSG_RESULT([no])
		PKG_CHECK_MODULES(LUA, [lua-5.1 >= 5.1])
	])])
CPPFLAGS="$CPPFLAGS $LUA_CFLAGS"
LIBS="$LIBS $LUA_LIBS"

#######################################################################
# Check for different notifications systems                           #
#######################################################################
found_notifications=no
if test "x$notifications" = "xyes"; then
	if test "x$dbus" = "xyes"; then
		PKG_CHECK_MODULES([LIBDBUS], [dbus-1], [
			CPPFLAGS="$CPPFLAGS $LIBDBUS_CFLAGS"
			LIBS="$LIBS $LIBDBUS_LIBS"
			found_notifications=yes
			AC_DEFINE([HAVE_LIBDBUS],,[Define if you have libdbus.])
		], [AC_MSG_RESULT([no])])
	fi
	# tests for other notification systems
fi
AC_MSG_CHECKING([for notifications])
AC_MSG_RESULT([$found_notifications])


#######################################################################
# Check for boost iostreams                                           #
#######################################################################

BOOST_REQUIRE([1.35])
BOOST_IOSTREAMS
BOOST_REGEX

if test "x$ana" = "xyes"; then
	BOOST_ASIO
	BOOST_THREADS
fi

if test "x$tests" = "xyes"; then
	BOOST_TEST

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

        CPPFLAGS=$OLD_CPPFLAGS
        CFLAGS=$OLD_CFLAGS
        LIBS=$OLD_LIBS

fi

m4_pattern_allow([^BOOST_TEST_DYN_LINK$])
	
AM_CONDITIONAL([BOOST_TEST_DYN_LINK], [test x"$boost_test_dyn_link" = xyes])

#######################################################################
# Pango support                                                       #
#######################################################################

if test "x$game" = "xyes" || test "x$tests" = "xyes"; then

	PKG_CHECK_MODULES(PANGO, pangocairo >= 1.14.8)
	CPPFLAGS="$CPPFLAGS $PANGO_CFLAGS"

	PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.4.1)
	CPPFLAGS="$CPPFLAGS $FONTCONFIG_CFLAGS"
fi

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

AC_SUBST([DATA_FILES])
AC_SUBST([FONT_FILES])
AC_SUBST([IMAGE_FILES])
AC_SUBST([MUSIC_FILES])
AC_SUBST([SOUND_FILES])

AC_CONFIG_FILES([Makefile
		 po/Makefile
		 po/wesnoth/Makefile.in
		 po/wesnoth-editor/Makefile.in
		 po/wesnoth-test/Makefile.in
		 po/wesnoth-lib/Makefile.in
		 po/wesnoth-units/Makefile.in
		 po/wesnoth-multiplayer/Makefile.in
		 po/wesnoth-anl/Makefile.in
		 po/wesnoth-tutorial/Makefile.in
		 po/wesnoth-aoi/Makefile.in
		 po/wesnoth-did/Makefile.in
		 po/wesnoth-dm/Makefile.in
		 po/wesnoth-dw/Makefile.in
		 po/wesnoth-ei/Makefile.in
		 po/wesnoth-httt/Makefile.in
		 po/wesnoth-l/Makefile.in
		 po/wesnoth-low/Makefile.in
		 po/wesnoth-nr/Makefile.in
		 po/wesnoth-sof/Makefile.in
		 po/wesnoth-sotbe/Makefile.in
		 po/wesnoth-tb/Makefile.in
		 po/wesnoth-thot/Makefile.in
		 po/wesnoth-trow/Makefile.in
		 po/wesnoth-tsg/Makefile.in
		 po/wesnoth-utbs/Makefile.in
		 m4/Makefile
		 icons/Makefile
		 src/Makefile
		 doc/Makefile
		 doc/man/Makefile
		 doc/manual/Makefile])

AC_OUTPUT
