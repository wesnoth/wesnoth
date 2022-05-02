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

# Map MSVC version to the installation folder name
set(MSVC_YEAR_NAME)
if(MSVC_VERSION GREATER 1599)		# >= 1600
	set(MSVC_YEAR_NAME VS2010)
elseif(MSVC_VERSION GREATER 1499)	# >= 1500
	set(MSVC_YEAR_NAME VS2008)
elseif(MSVC_VERSION GREATER 1399)	# >= 1400
	set(MSVC_YEAR_NAME VS2005)
elseif(MSVC_VERSION GREATER 1299)	# >= 1300
	set(MSVC_YEAR_NAME VS2003)
elseif(MSVC_VERSION GREATER 1199)	# >= 1200
	set(MSVC_YEAR_NAME VS6)
endif()

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
		win32/VorbisFile_Dynamic_Release
		"Win32/${MSVC_YEAR_NAME}/x64/Release"
		"Win32/${MSVC_YEAR_NAME}/Win32/Release"
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
		win32/VorbisFile_Dynamic_Debug
		"Win32/${MSVC_YEAR_NAME}/x64/Debug"
		"Win32/${MSVC_YEAR_NAME}/Win32/Debug"
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
		PATH_SUFFIXES
			win32/VorbisFile_Dynamic_Debug
			"Win32/${MSVC_YEAR_NAME}/x64/Debug"
			"Win32/${MSVC_YEAR_NAME}/Win32/Debug"
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
	else()
		set_target_properties(VorbisFile::VorbisFile PROPERTIES IMPORTED_LOCATION "${VORBISFILE_LIBRARY}")
	endif()
endif()
