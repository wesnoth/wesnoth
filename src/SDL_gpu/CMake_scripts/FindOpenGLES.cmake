#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGLES
#  OPENGLES_INCLUDE_DIR  - the GL include directory
#  OPENGLES_LIBRARIES    - Link these to use OpenGLES

include(FindPkgMacros)

IF (WIN32)

	IF(0) # Disabled, untill further testing
	  IF (CYGWIN)

		FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h )

		FIND_LIBRARY(OPENGLES_gl_LIBRARY libgles_cm )

	  ELSE (CYGWIN)

		IF(BORLAND)
		  SET (OPENGLES_gl_LIBRARY import32 CACHE STRING "OpenGL library for win32")
		ELSE(BORLAND)
		  #MS compiler - todo - fix the following line:
		  SET (OPENGLES_gl_LIBRARY ${OGRE_SOURCE_DIR}/Dependencies/lib/release/libgles_cm.lib CACHE STRING "OpenGL library for win32")
		ENDIF(BORLAND)

	  ENDIF (CYGWIN)
	ENDIF()

ELSE (WIN32)

  IF (APPLE)

	create_search_paths(/Developer/Platforms)
	findpkg_framework(OpenGLES)
    set(OPENGLES_gl_LIBRARY "-framework OpenGLES")

  ELSE(APPLE)

	IF(0) # Disabled, untill further testing


		FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h
		  /usr/openwin/share/include
		  /opt/graphics/OpenGL/include /usr/X11R6/include
		  /usr/include
		)


		FIND_LIBRARY(OPENGLES_gl_LIBRARY
		  NAMES GLES_CM
		  PATHS /opt/graphics/OpenGL/lib
				/usr/openwin/lib
				/usr/shlib /usr/X11R6/lib
				/usr/lib
		)

		# On Unix OpenGL most certainly always requires X11.
		# Feel free to tighten up these conditions if you don't 
		# think this is always true.
		# It's not true on OSX.

		IF (OPENGLES_gl_LIBRARY)
		  IF(NOT X11_FOUND)
			INCLUDE(FindX11)
		  ENDIF(NOT X11_FOUND)
		  IF (X11_FOUND)
			IF (NOT APPLE)
			  SET (OPENGLES_LIBRARIES ${X11_LIBRARIES})
			ENDIF (NOT APPLE)
		  ENDIF (X11_FOUND)
		ENDIF (OPENGLES_gl_LIBRARY)
	ENDIF()

  ENDIF(APPLE)
ENDIF (WIN32)

SET( OPENGLES_FOUND "NO" )
IF(OPENGLES_gl_LIBRARY)

    SET( OPENGLES_LIBRARIES ${OPENGLES_gl_LIBRARY} ${OPENGLES_LIBRARIES})

    SET( OPENGLES_FOUND "YES" )

ENDIF(OPENGLES_gl_LIBRARY)

MARK_AS_ADVANCED(
  OPENGLES_INCLUDE_DIR
  OPENGLES_gl_LIBRARY
)