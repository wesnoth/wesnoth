# - Find the native FriBiDI includes and library
#

# This module defines
#  FRIBIDI_INCLUDE_DIR, where to find fribidi.h, etc.
#  FRIBIDI_LIBRARIES, the libraries to link against to use FriBiDi.
#  PNG_DEFINITIONS - You should ADD_DEFINITONS(${PNG_DEFINITIONS}) before compiling code that includes png library files.
#  FRIBIDI_FOUND, If false, do not try to use PNG.
# also defined, but not for general use are
#  FRIBIDI_LIBRARY, where to find the FriBiDi library.

SET(FRIBIDI_FOUND "NO")

FIND_PATH(FRIBIDI_INCLUDE_DIR fribidi/fribidi.h
  /usr/local/include
  /usr/include
  )

SET(FRIBIDI_NAMES ${FRIBIDI_NAMES} fribidi libfribidi)
FIND_LIBRARY(FRIBIDI_LIBRARY
  NAMES ${FRIBIDI_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (FRIBIDI_LIBRARY AND FRIBIDI_INCLUDE_DIR)
  SET(FRIBIDI_LIBRARIES ${FRIBIDI_LIBRARY})
  SET(FRIBIDI_FOUND "YES")
ENDIF (FRIBIDI_LIBRARY AND _FRIBIDI_INCLUDE_DIR)

IF (FRIBIDI_FOUND)
  IF (NOT FRIBIDI_FIND_QUIETLY)
    MESSAGE(STATUS "Found FriBiDi: ${FRIBIDI_LIBRARY}")
  ENDIF (NOT FRIBIDI_FIND_QUIETLY)
ELSE (FRIBIDI_FOUND)
  IF (FRIBIDI_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find FriBiDi library")
  ENDIF (FRIBIDI_FIND_REQUIRED)
ENDIF (FRIBIDI_FOUND)
