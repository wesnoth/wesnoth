# Building Wesnoth from Source

## Prerequisites

Wesnoth requires a compiler with sufficient C++17 support such as GCC 11 and
later, or a version of Clang with equivalent support.

You'll need to have these libraries and their development headers installed in
order to build Wesnoth:

 * Boost libraries             >= 1.66.0
     Most headers plus the following binary libs:
   * Filesystem
   * Locale
   * Iostreams
   * Random
   * Regex
   * Program Options
   * System
   * Coroutine
   * Graph
   * Charconv (This requires boost 1.85 or higher and is optional but reccomended especially for clang builds)
 * SDL2 libraries:
   * SDL2                      >= 2.0.18 (macOS: 2.0.22 due to needing https://github.com/libsdl-org/SDL/commit/3bebdaccb7bff8c40438856081d404a7ce3def30)
   * SDL2_image                >= 2.0.2 (with PNG, JPEG, and WEBP support)
   * SDL2_mixer                >= 2.0.0 (with Ogg Vorbis support)
 * Fontconfig                  >= 2.4.1
 * Cairo                       >= 1.10.0
 * Pango                       >= 1.44.0 (with Cairo backend)
 * Vorbisfile aka libvorbis
 * libbz2
 * libz
 * libssl
 * libcrypto (from OpenSSL)
 * libcurl4 (OpenSSL version)

The following libraries are optional dependencies that enable additional
features:

 * D-Bus (libdbus-1):
   Desktop notifications on Linux, *BSD, etc.

 * GNU history (libreadline):
   Command history and history expansion in the built-in Lua console.


## Build Environment

You can obtain the source code tarball for the latest version from
<https://www.wesnoth.org/downloads>.

Before building, make sure to untar the package and change into the newly
created directory:

    $ tar xvjf wesnoth-<version>.tar.bz2
    $ cd wesnoth-<version>

Alternatively, you can clone this git repository. Since Wesnoth uses submodules, when cloning you must add the `--recurse-submodules` option, or if you have already cloned the repository without using that option then you must run the command `git submodule update --init --recursive`.

The following build systems are fully supported for compiling Wesnoth on Linux,
*BSD, and other Unix-like platforms:

 * SCons >= 0.98.3
 * CMake >= 3.14

You will also need to have a working installation of GNU gettext to build the
translations.

While Wesnoth may be easily installed system-wide using SCons or CMake, it is
also possible to run it directly from the source directory after building. This
may be useful in situations where you don't have root access or need to
rebuild Wesnoth frequently (i.e. for development and testing).

### macOS/OS X
See [here](https://github.com/wesnoth/wesnoth/blob/master/projectfiles/Xcode/README.md) for instructions on using Xcode.

### Windows
Wesnoth uses CMake for project configuration and vcpkg for installing dependencies. See [here](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio) for information on using Visual Studio with cmake. The first time it's run, vcpkg will build all the required dependencies which may take over an hour, however it will only need to be done once.

NOTE 1: You will need to run `vcpkg integrate install` on the command line to make Visual Studio aware of vcpkg. If Visual Studio is open when this is executed, then you will need to close and re-open Visual Studio.

## SCons Build

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

    $ scons install

SCons takes a `prefix=` argument that specifies where to install the game and
its resource files. The prefix defaults to `/usr/local`; for production builds,
you may wish to use `/usr` instead:

    $ scons prefix=/usr


## CMake Build

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


## Build Options

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

### Some of the most important options follow.

| SCons                                                                  | CMake                                                              | Description                                                                                                                                                                                                                                                                                        |
|------------------------------------------------------------------------|--------------------------------------------------------------------| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `build=<build type>`                                                   | `CMAKE_BUILD_TYPE=<build type>`                                    | Selects a specific build configuration when compiling. `release`  produces the default, optimized (-O3) build for regular use. `debug`  produces a slower and larger unoptimized (-O0) build with full debug symbols, which is often needed for obtaining detailed backtraces when reporting bugs. |
| `wesnoth`                                                              | `ENABLE_GAME=<boolean>`                                            | Whether to build the game client binary. To disable just don't mention the target for SCons.                                                                                                                                                                                                       |
| `wesnothd`                                                             | `ENABLE_SERVER=<boolean>`                                          | Whether to build the MP server binary. To disable just don't mention the target for SCons.                                                                                                                                                                                                         |
| `prefix=<full path>`                                                   | `CMAKE_INSTALL_PREFIX=<full path>`                                 | Installation prefix for binaries, resources, and documentation files.                                                                                                                                                                                                                              |
| `nls=<boolean>`                                                        | `ENABLE_NLS=<boolean>`                                             | Whether to compile and install translations.                                                                                                                                                                                                                                                       |
| `strict=<boolean>`                                                     | `ENABLE_STRICT_COMPILATION=<boolean>`                              | Whether to treat compiler warnings as errors or not. Primarily intended for developers.                                                                                                                                                                                                            |
| `prefsdir=<directory name>`                                            | `PREFERENCES_DIR=<directory name>`                                 | Hardcoded user preferences and user data directory. The default is to leave this unspecified so that Wesnoth will use separate XDG paths such as .config/wesnoth and .local/share/wesnoth/<version>  for its user preferences and data, respectively.                                              |
| `cxxtool=<program>`                                                    | `CMAKE_CXX_COMPILER=<program>`                                     | Specifies which C++ compiler to use. By default, the system's default C++ compiler will be automatically selected during configuration.                                                                                                                                                            |
| `ccache=<boolean>`                                                     | `CMAKE_CXX_COMPILER_LAUNCHER=ccache`                               | Whether to run the compiler through ccache first. Useful if the compiler executable is not a symbolic link to ccache. Requires ccache to be installed first.  If using CMake, use CMAKE_C_COMPILER and CMAKE_CXX_COMPILER instead.                                                                 |
| `extra_flags_<buildtype>=<flags>` `extra_flags_config=<flags>`         | `CXX_FLAGS_USER=<flags>`                                           | Additional compiler flags to use when compiling a specific build type (SCons-only). To apply the same flags to all builds, use extra_flags_config (SCons) or CXX_FLAGS_USER (CMake) without a build type suffix.  Alternatively, you may specify your flags in the CXXFLAGS environment variable.  |
| `fifodir=<full path>` `server_uid=<UID>` `server_gid=<GID>`            | `FIFO_DIR=<full path>` `SERVER_UID=<UID>` `SERVER_GID=<GID>`       | Directory and owner id for the wesnothd control FIFO file. This is relevant only if you wish to be able to communicate with a local wesnothd instance through a named pipe. You must run wesnothd with the same UID specified at build time for this to work.                                      |
| `enable_lto=<boolean>`                                                 | `ENABLE_LTO=<boolean>`                                             | Controls using Link Time Optimization. Enabling will result in a smaller, faster executable at the cost of increased time to compile and link. For cmake, use LTO_JOBS=N tells how many threads to use during linking.                                                                             |
| `--debug=time`                                                         | `VERBOSE=1` (make option)                                          | Enables some additional output while building.                                                                                                                                                                                                                                                     |
| `jobs=N`                                                               | `-jN` (make option)                                                | Enables compiling with multiple threads, where N is the number of threads to use.                                                                                                                                                                                                                  |
