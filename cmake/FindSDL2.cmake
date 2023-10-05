#[=======================================================================[.rst:
FindSDL2
--------

Find the SDL2 includes and libraries.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines the `IMPORTED` targets ``SDL2::SDL2`` and ``SDL2::SDL2main``, if
SDL2 has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

	SDL2_FOUND         - True if SDL2 found.
	SDL2_INCLUDE_DIRS  - Where to find SDL2/SDL.h.
	SDL2_LIBRARIES     - Libraries when using SDL2.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

# first specifically look for the CMake config version of SDL2 (either system or package manager)
# provides two TARGETs SDL2::SDL2 and SDL2::SDL2Main
find_package(SDL2 QUIET NO_MODULE)

# after the CONFIG type `find_package` we get, if available, a SDL2_VERSION var
# if we found the SDL2 cmake package then we are almost done
# we still need to check the version for some edge cases where the -version.cmake file
# is missing
if(SDL2_FOUND)
	find_package_handle_standard_args(SDL2 HANDLE_COMPONENTS CONFIG_MODE)

	# in some cases the target is an ALIAS, we want the original
	get_target_property(_SDL2_ORIGINAL_TARGET "SDL2::SDL2" ALIASED_TARGET)

	# if it's a regular target, just set it to SDL2::SDL2
	if(NOT _SDL2_ORIGINAL_TARGET)
		set(_SDL2_ORIGINAL_TARGET "SDL2::SDL2")
	endif()

	get_target_property(_SDL2_INCLUDE_DIRS_FROM_CONFIG "${_SDL2_ORIGINAL_TARGET}" INTERFACE_INCLUDE_DIRECTORIES)
	
	# in the original sdl2-config.cmake we get only <path>/include/SDL2 and in vcpkg we that + <path>/include/
	if(_SDL2_INCLUDE_DIRS_FROM_CONFIG MATCHES ".*/include/SDL2.*")
		get_filename_component(_SDL2_INCLUDE_DIRS "${_SDL2_INCLUDE_DIRS_FROM_CONFIG}" DIRECTORY)
		list(APPEND _SDL2_INCLUDE_DIRS_FROM_CONFIG "${_SDL2_INCLUDE_DIRS}")
		# we can endup with a duplicate with vcpkg
		list(REMOVE_DUPLICATES _SDL2_INCLUDE_DIRS_FROM_CONFIG)

		set_property(TARGET "${_SDL2_ORIGINAL_TARGET}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${_SDL2_INCLUDE_DIRS_FROM_CONFIG}")
	endif()

	get_target_property(_SDL2_INCLUDE_DIRS_FROM_CONFIG "${_SDL2_ORIGINAL_TARGET}" INTERFACE_INCLUDE_DIRECTORIES)

	mark_as_advanced(SDL2_INCLUDE_DIRS SDL2_LIBRARIES)

	if(NOT TARGET SDL2::SDL2main)
		# In SDL 2.24.0, the CONFIG method doesn't add a target for libSDL2main. Seen on Debian
		# Unstable, and seems to be https://github.com/libsdl-org/SDL/issues/6119 which is reported
		# upstream on macOS. For consistency's sake, let's add a phony library we can link with,
		# which is also the workaround that upstream suggests in the bug report.
		message(STATUS "SDL2 found via CONFIG, but it didn't define SDL2::SDL2main, adding dummy target")
		add_library(SDL2::SDL2main INTERFACE IMPORTED)
	endif()

	return()
endif()

# let's try pkg-config
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_SDL2 REQUIRED sdl2>=${SDL2_FIND_VERSION})
endif()

find_package_handle_standard_args(SDL2 DEFAULT_MSG PC_SDL2_INCLUDE_DIRS PC_SDL2_LIBRARIES)

if(SDL2_FOUND)
	if(NOT TARGET SDL2::SDL2)
		# the .pc file for SDL2 contains `<prefix>/include/SDL2` which is different from the cmake version
		# i.e. `<prefix>/include`
		get_filename_component(SDL2_INCLUDE_DIRS ${PC_SDL2_INCLUDE_DIRS} DIRECTORY)

		set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS} CACHE STRING "SDL2 include directories")
		set(SDL2_LIBRARIES ${PC_SDL2_LIBRARIES} CACHE STRING "SDL2 libraries")
		
		mark_as_advanced(SDL2_INCLUDE_DIRS SDL2_LIBRARIES)

		add_library(SDL2::SDL2 INTERFACE IMPORTED)
		set_target_properties(SDL2::SDL2 PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
			INTERFACE_LINK_DIRECTORIES "${PC_SDL2_LIBDIR}"
			INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}")
	endif()

	if(NOT TARGET SDL2::SDL2main)
		# on *nix it seems that .pc files don't actually include libSDL2main, but for consistency's sake,
		# let's add a phony library we can link with
		add_library(SDL2::SDL2main INTERFACE IMPORTED)
	endif()
endif()
