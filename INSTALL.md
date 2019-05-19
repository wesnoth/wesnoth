Contents
========

  1. Prerequisites
  2. Build Environment
  3. SCons Build
  4. CMake Build
  5. Build Options


## 1. Prerequisites

Wesnoth requires a compiler with sufficient C++14 support such as GCC 5.0 and
later, or Clang 3.8 and later.

You'll need to have these libraries and their development headers installed in
order to build Wesnoth:

 * Boost libraries             >= 1.56.0
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
 * Pango                       >= 1.22.0 (with Cairo backend)
 * Vorbisfile
 * libbz2
 * libz
 * libcrypto (from OpenSSL)

The following libraries are optional dependencies that enable additional
features:

 * D-Bus (libdbus-1):
   Desktop notifications on Linux, *BSD, etc.

 * GNU history (libreadline):
   Command history and history expansion in the built-in Lua console.

 * FriBiDi >= 0.10.9:
   Bidirectional text support for RTL languages (Hebrew, etc.) in some parts
   of the user interface.


## 2. Build Environment

You can obtain the source code tarball for the latest version from
<https://www.wesnoth.org/downloads>.

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
 * CMake >= 2.8.5

You will also need to have a working installation of GNU gettext to build the
translations.

While Wesnoth may be easily installed system-wide using SCons or CMake, it is
also possible to run it directly from the source directory after building. This
may be useful in situations where you don't have root access or need to
rebuild Wesnoth frequently (i.e. for development and testing).

For Windows users, a Visual C++ 2013 solution is included in _projectfiles/VC12_.
For OS X users, an XCode project is included in _projectfiles/XCode_.


## 3. SCons Build

Unlike CMake 
