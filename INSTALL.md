Contents
========

  1. Prerequisites
  2. Build Environment
  3. SCons Build
  4. CMake Build
  5. Build Options


## 1. Prerequisites

Wesnoth requires a compiler with sufficient C++11 support such as GCC 4.8 and
later, or Clang 3.3 and later.

You'll need to have these libraries and their development headers installed in
order to build Wesnoth:

 * Boost libraries             >= 1.48.0
     Most headers plus the following binary libs:
   * Filesystem
   * Locale
   * Iostreams
   * Random
   * Regex
   * Program Options
   * System
   * Thread
 * SDL2 libraries:
   * SDL2                      >= 2.0.4
   * SDL2_image                >= 2.0.0 (with PNG and JPEG support)
   * SDL2_mixer                >= 2.0.0 (with Ogg Vorbis support)
   * SDL2_ttf                  >= 2.0.12
 * Fontconfig                  >= 2.4.1
 * Cairo                       >= 1.10.0
 * Pango                       >= 1.21.3 (with Cairo backend)
 * Vorbisfile
 * libbz2
 * libz
 * libcrypto (from OpenSSL)

The following libraries are optional dependencies that enable additional
features:

 * libpng:
   PNG screenshots, otherwise only BMP is supported.

 * D-Bus (libdbus-1):
   Desktop notifications on Linux, *BSD, etc.

 * GNU history (libreadline):
   Command history and history expansion in the built-in Lua console.

 * FriBiDi >= 0.10.9:
   Bidirectional text support for RTL languages (Hebrew, etc.) in some parts
   of the user interface.

 * Growl
   Desktop notifications on OS X, particularly on 10.7.

Although not recommended, you may use libintl on platforms other than Windows
instead of Boost.Locale. For scons, set the `libintl` option to `true`.


## 2. Build Environment

You can obtain the source code tarball for the latest version from
<http://www.wesnoth.org/downloads>.

Before building, make sure to untar the package and change into the newly
created directory:

    $ tar xvjf wesnoth-<version>.tar.bz2
    $ cd wesnoth-<version>

Or:

    $ tar xvzf wesnoth-<version>.tar.gz
    $ cd wesnoth-<version>

The following build systems are fully supported for compiling Wesnoth on Linux,
*BSD, and other Unix-like platforms:

 * SCons >= 0.98.3
 * CMake >= 2.6.0

You will also need to have a working installation of GNU gettext to build the
translations.

While Wesnoth may be easily installed system-wide using SCons or CMake, it is
also possible to run it directly from the source directory after building. This
may be useful in situations where you don't have root access or need to
rebuild Wesnoth frequently (i.e. for development and testing).

For Windows users, a Visual C++ 2013 solution is included in _projectfiles/VC12_.
For OS X users, an XCode project is included in _projectfiles/XCode_.


## 3. SCons Build

Unlike CMake or the classic "autotools" build-system (configure && make),
configuration and building are done in the same step with SCons.

Simply type `scons` in the top-level directory to build the game client and
MP server:

    $ scons

It is possible to select individual targets to build by naming them in the
command line separated by spaces.

To build the game client only:

    $ scons wesnoth

Building the MP server only:

    $ scons wesnothd

The `install` target will install any binaries that were previously compiled
(use su or sudo if necessary to write files into the installation prefix):

    # scons install

SCons takes a `prefix=` argument that specifies where to install the game and
its resource files. The prefix defaults to `/usr/local`; for production builds,
you may wish to use `/usr` instead:

    $ scons prefix=/usr


## 4. CMake Build

Unlike SCons, CMake has separate configuration and build steps. Configuration
is done using CMake itself, and the actual build is done using `make`.

There are two ways to build Wesnoth with CMake: inside the source tree or
outside of it. Out-of-source builds have the advantage that you can have
multiple builds with different options from one source directory.

To build Wesnoth out of source:

    $ mkdir build && cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release
    $ make

To build Wesnoth in the source directory:

    $ cmake . -DCMAKE_BUILD_TYPE=Release
    $ make

To install Wesnoth after building (as root using su or sudo if necessary):

    # make install

To change build options, you can either pass the options on the command line:

    $ cmake .. -DOPTION_NAME=option_value

Or use either the `ccmake` or `cmake-gui` front-ends, which display all options
and their cached values on a console and graphical UI, respectively.

    $ ccmake ..
    $ cmake-gui ..


## 5. Build Options

A full list of options supported by SCons along with their descriptions and
defaults is available by running `scons --help` from the Wesnoth source. For
CMake, you may either run the `ccmake` or `cmake-gui` front-ends, or run
`cmake` and open the generated CMakeCache.txt from the build directory in a
text editor.

    $ scons option_name1=option_value1 [option_name2=option_value2 [...]]
    $ cmake -DOPTION_NAME1=option_value1 [-DOPTION_NAME2=option_value2 [...]]

With SCons, boolean options take `yes` or `true` for a true value, and `no` or
`false` for a false value. CMake uses `ON` for a true value, and `OFF` for a
false value.

Some of the most important options follow.

 * build=<build type>                          (SCons)
   CMAKE_BUILD_TYPE=<build type>               (CMake)

   Selects a specific build configuration when compiling. `release` produces
   the default, optimized (-O2) build for regular use. `debug` produces a
   slower and larger unoptimized (-O0) build with full debug symbols, which is
   often needed for obtaining detailed backtraces when reporting bugs.

   NOTE: By default, CMake will produce `debug` builds unless a different
   configuration option is passed in the command line.

 * ENABLE_GAME=<boolean>                       (CMake)

   Whether to build the game client binary. Use command line target selection
   selection with SCons instead.

 * ENABLE_SERVER=<boolean>                     (CMake)

   Whether to build the MP server binary. Use command line target selection
   selection with SCons instead.

 * prefix=<full path>                          (SCons)
   CMAKE_INSTALL_PREFIX=<full path>            (CMake)

   Installation prefix for binaries, resources, and documentation files.

 * nls=<boolean>                               (SCons)
   ENABLE_NLS=<boolean>                        (CMake)

   Whether to compile and install translations.

 * strict=<boolean>                            (SCons)
   ENABLE_STRICT_COMPILATION=<boolean>         (CMake)

   Whether to treat compiler warnings as errors or not. Primarily intended for
   developers.

 * prefsdir=<directory name>                   (SCons)
   PREFERENCES_DIR=<directory name>            (CMake)

   Hardcoded user preferences and user data directory. The default is to leave
   this unspecified so that Wesnoth will use separate XDG paths such as
   .config/wesnoth and .local/share/wesnoth/<version> for its user preferences
   and data, respectively.

 * cxxtool=<program>                           (SCons)
   CMAKE_CXX_COMPILER=<program>                (CMake)

   Specifies which C++ compiler to use. By default, the system's default C++
   compiler will be automatically selected during configuration.

 * ccache=<boolean>                            (SCons)

   Whether to run the compiler through ccache first. Useful if the compiler
   executable is not a symbolic link to ccache. Requires ccache to be
   installed first.

   If using CMake, use CMAKE_CXX_COMPILER instead.

 * extra_flags_<buildtype>=<flags>             (SCons)
   extra_flags_config=<flags>                  (SCons)
   CXX_FLAGS_USER=<flags>                      (CMake)

   Additional compiler flags to use when compiling a specific build type
   (SCons-only). To apply the same flags to all builds, use extra_flags_config
   (SCons) or CXX_FLAGS_USER (CMake) without a build type suffix.

   Alternatively, you may specify your flags in the CXXFLAGS environment
   variable.

 * fifodir=<full path>                         (SCons)
   FIFO_DIR=<full path>                        (CMake)

   server_uid=<UID> server_gid=<GID>           (SCons)
   SERVER_UID=<UID> SERVER_GID=<GID>           (CMake)

   Directory and owner id for the wesnothd control FIFO file. This is relevant
   only if you wish to be able to communicate with a local wesnothd instance
   through a named pipe. You must run wesnothd with the same UID specified at
   build time for this to work.
