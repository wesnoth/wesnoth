#[=======================================================================[.rst:
FindVorbisFile
--------------

Find the VorbisFile includes and library.

Environment
^^^^^^^^^^^

$ENV{VORBISDIR} is an environment variable that would correspond to
the `./configure --prefix=$VORBISDIR` used in building Vorbis.


IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines the `IMPORTED` target ``VorbisFile::VorbisFile``, if
VorbisFile has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

	VORBISFILE_FOUND         - True if VorbisFile found.
	VORBISFILE_INCLUDE_DIR   - Where to find vorbis/vorbisfile.h.
	VORBISFILE_LIBRARIES     - Libraries when using VorbisFile.

Inspiration
^^^^^^^^^^^

http://code.google.com/p/osgaudio/source/browse/trunk/CMakeModules/FindVorbisFile.cmake

#]=======================================================================]

set(VORBISFILE_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_path(VORBISFILE_INCLUDE_DIR
	NAMES vorbis/vorbisfile.h
	HINTS
		$ENV{VORBISFILEDIR}
		$ENV{VORBISFILE_PATH}
		$ENV{VORBISDIR}
		$ENV{VORBIS_PATH}
	PATH_SUFFIXES include
	PATHS ${VORBISFILE_SEARCH_PATHS}
)

find_library(VORBISFILE_LIBRARY
	NAMES vorbisfile libvorbisfile
	HINTS
		$ENV{VORBISFILEDIR}
		$ENV{VORBISFILE_PATH}
		$ENV{VORBISDIR}
		$ENV{VORBIS_PATH}
	PATH_SUFFIXES
		lib
		lib64
	PATHS ${VORBISFILE_SEARCH_PATHS}
)

# First search for d-suffixed libs
find_library(VORBISFILE_LIBRARY_DEBUG
	NAMES vorbisfiled vorbisfile_d libvorbisfiled libvorbisfile_d
	HINTS
		$ENV{VORBISFILEDIR}
		$ENV{VORBISFILE_PATH}
		$ENV{VORBISDIR}
		$ENV{VORBIS_PATH}
	PATH_SUFFIXES
		lib
		lib64
	PATHS ${VORBISFILE_SEARCH_PATHS}
)

if(NOT VORBISFILE_LIBRARY_DEBUG)
	# Then search for non suffixed libs if necessary, but only in debug dirs
	find_library(VORBISFILE_LIBRARY_DEBUG
		NAMES vorbisfile libvorbisfile
		HINTS
			$ENV{VORBISFILEDIR}
			$ENV{VORBISFILE_PATH}
			$ENV{VORBISDIR}
			$ENV{VORBIS_PATH}
		PATHS ${VORBISFILE_SEARCH_PATHS}
	)
endif()

if(VORBISFILE_LIBRARY)
	if(VORBISFILE_LIBRARY_DEBUG)
		set(VORBISFILE_LIBRARIES optimized "${VORBISFILE_LIBRARY}" debug "${VORBISFILE_LIBRARY_DEBUG}")
	else()
		set(VORBISFILE_LIBRARIES "${VORBISFILE_LIBRARY}") # Could add "general" keyword, but it is optional
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set VORBISFILE_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VorbisFile DEFAULT_MSG VORBISFILE_LIBRARIES VORBISFILE_INCLUDE_DIR)

mark_as_advanced(VORBISFILE_INCLUDE_DIR VORBISFILE_LIBRARIES VORBISFILE_LIBRARY VORBISFILE_LIBRARY_DEBUG)

if(VORBISFILE_FOUND AND (NOT TARGET VorbisFile::VorbisFile))
	add_library(VorbisFile::VorbisFile UNKNOWN IMPORTED)
	set_target_properties(VorbisFile::VorbisFile PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${VORBISFILE_INCLUDE_DIR}")
	if(VORBISFILE_LIBRARY_DEBUG)
		set_target_properties(VorbisFile::VorbisFile PROPERTIES IMPORTED_LOCATION_DEBUG "${VORBISFILE_LIBRARY_DEBUG}")
		set_target_properties(VorbisFile::VorbisFile PROPERTIES IMPORTED_LOCATION_RELEASE "${VORBISFILE_LIBRARY}")
	endif()
	# for the rest of build types
	set_target_properties(VorbisFile::VorbisFile PROPERTIES IMPORTED_LOCATION "${VORBISFILE_LIBRARY}")
endif()
