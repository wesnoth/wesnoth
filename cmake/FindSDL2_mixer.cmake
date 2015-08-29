#.rst:
# FindSDL2_mixer
# -------------
#
# Locate SDL2_mixer library
#
# This module defines:
#
# ::
#
#   SDL2_MIXER_LIBRARIES, the name of the library to link against
#   SDL2_MIXER_INCLUDE_DIRS, where to find the headers
#   SDL2_MIXER_FOUND, if false, do not try to link against
#   SDL2_MIXER_VERSION_STRING - human-readable string containing the
#                              version of SDL2_mixer
#
#
#
# For backward compatiblity the following variables are also set:
#
# ::
#
#   SDL2MIXER_LIBRARY (same value as SDL2_MIXER_LIBRARIES)
#   SDL2MIXER_INCLUDE_DIR (same value as SDL2_MIXER_INCLUDE_DIRS)
#   SDL2MIXER_FOUND (same value as SDL2_MIXER_FOUND)
#
#
#
# $SDL2DIR is an environment variable that would correspond to the
# ./configure --prefix=$SDL2DIR used in building SDL2.
#
# Created by Andreas Löf.  This was influenced by the FindSDL_mixer.cmake
# module, but with modifications to use SDL2.
#
#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
# Copyright 2015 Andreas Löf
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#=============================================================================

if(NOT SDL2_MIXER_INCLUDE_DIR AND SDL2MIXER_INCLUDE_DIR)
  set(SDL2_MIXER_INCLUDE_DIR ${SDL2MIXER_INCLUDE_DIR} CACHE PATH "directory cache
entry initialized from old variable name")
endif()
find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
  HINTS
    ENV SDL2MIXERDIR
    ENV SDL2DIR
  PATH_SUFFIXES SDL2
                # path suffixes to search inside ENV{SDL2DIR}
                include/SDL2
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

if(NOT SDL2_MIXER_LIBRARY AND SDL2MIXER_LIBRARY)
  set(SDL2_MIXER_LIBRARY ${SDL2MIXER_LIBRARY} CACHE FILEPATH "file cache entry
initialized from old variable name")
endif()
find_library(SDL2_MIXER_LIBRARY
  NAMES SDL2_mixer
  HINTS
    ENV SDL2MIXERDIR
    ENV SDL2DIR
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)

if(SDL2_MIXER_INCLUDE_DIR AND EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MAJOR "${SDL_MIXER_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MINOR "${SDL_MIXER_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_PATCH "${SDL_MIXER_VERSION_PATCH_LINE}")
  set(SDL2_MIXER_VERSION_STRING ${SDL_MIXER_VERSION_MAJOR}.${SDL_MIXER_VERSION_MINOR}.${SDL_MIXER_VERSION_PATCH})
  unset(SDL_MIXER_VERSION_MAJOR_LINE)
  unset(SDL_MIXER_VERSION_MINOR_LINE)
  unset(SDL_MIXER_VERSION_PATCH_LINE)
  unset(SDL_MIXER_VERSION_MAJOR)
  unset(SDL_MIXER_VERSION_MINOR)
  unset(SDL_MIXER_VERSION_PATCH)
endif()

set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_mixer
                                  REQUIRED_VARS SDL2_MIXER_LIBRARIES SDL2_MIXER_INCLUDE_DIRS
                                  VERSION_VAR SDL2_MIXER_VERSION_STRING)

# for backward compatiblity
set(SDL2MIXER_LIBRARY ${SDL2_MIXER_LIBRARIES})
set(SDL2MIXER_INCLUDE_DIR ${SDL2_MIXER_INCLUDE_DIRS})
set(SDL2MIXER_FOUND ${SDL2_MIXER_FOUND})

mark_as_advanced(SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)
