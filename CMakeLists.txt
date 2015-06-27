# most stuff should be implemented so far, mgoe should know the details (he has done all the work...)
# what is not working so far:
# * some other smaller stuff
# * nice INSTALL howto
# * Detect Python and install Python tools
# * Rewrite the pot-update
# * Test everything
# * Fix tiny gui with processing the data directory
# * Make translations work without installation
# * install .desktop files taking binary prefix and suffix into account

# set minimum version
cmake_minimum_required(VERSION 2.6.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# use our own version of FindBoost.cmake and other Find* scripts
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package( SDL 1.2.7 REQUIRED )
find_package( Boost 1.35 REQUIRED COMPONENTS iostreams regex )
# yes, gettext is *required* even when NLS is deactivated (this is to compile
# src/gettext.cpp since it includes libintl.h)
find_package( Gettext REQUIRED )

find_package( X11 )

if(NOT MSVC)
	#needed to get some SDL defines in... (as of rev31694 -D_GNU_SOURCE=1 is required!)
	set(SDL_CONFIG "sdl-config" CACHE STRING "Path to sdl-config script")
	exec_program(${SDL_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE SDL_CFLAGS)
	add_definitions(${SDL_CFLAGS})
endif(NOT MSVC)


#check for some compiler/arch specific things and export defines accordingly...
INCLUDE (SearchForStuff)

#
# Options
#

#Path options
set(BINDIR "bin" CACHE STRING "Where to install binaries")
set(MANDIR "man" CACHE STRING "Where to install manpages")
set(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Sets the root of data directories to a non-default location")
set(DOCDIR "${DATAROOTDIR}/doc/wesnoth" CACHE STRING "Sets the doc directory to a non-default location.")
set(DATADIRNAME "wesnoth" CACHE STRING "change the name of the directory for the read-only architecture-independent game data")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}" CACHE STRING "change the dir where binaries are placed right at compile time")
set(LOCALEDIR "translations" CACHE STRING "change the name of the locale data directory to a non-default name")
set(PREFERENCES_DIR "" CACHE STRING "Use a non-default preferences directory (.wesnoth on unix)")

#Game options
option(ENABLE_FRIBIDI "Enable FriBIDi support" ON)
set(GUI "normal" CACHE STRING "Set for GUI reductions for resolutions down to 320x240 (PDAs) (normal|tiny)")

#server options
set(SERVER_UID "" CACHE STRING "User id of the user who runs wesnothd")
set(SERVER_GID "" CACHE STRING "Group id of the user who runs wesnothd")
set(FIFO_DIR "/var/run/wesnothd" CACHE STRING "Directory for the wesnothd fifo socket file")

#build options
option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign server")
option(ENABLE_SERVER "Enable compilation of server" ON)
option(ENABLE_EDITOR "Enable compilation of the new map editor into the game executable" ON)
option(ENABLE_TOOLS "Enable building and installation of tools for artists and WML maintainers")
option(ENABLE_TESTS "Build unit tests")
option(ENABLE_NLS "Enable building of translations" ON)
option(ENABLE_POOL_ALLOC "Enable custom pool malloc" OFF)
option(ENABLE_LOW_MEM "Reduce memory usage by removing extra functionality" OFF)
if (UNIX AND NOT APPLE AND NOT CYGWIN)
  option(ENABLE_DESKTOP_ENTRY "enable installation of desktop entry files" ON)
  set(ICONDIR "${DATAROOTDIR}/pixmaps" CACHE STRING "Sets the icon directory for desktop entry to a non-default location.")
  set(DESKTOPDIR "${DATAROOTDIR}/applications/" CACHE STRING "Sets the desktop file directory for desktop entry to a non-default location.")
endif(UNIX AND NOT APPLE AND NOT CYGWIN)
option(ENABLE_STRICT_COMPILATION "Sets the strict compilation mode" OFF)
option(ENABLE_DEBUG_WINDOW_LAYOUT "Add the debug option to allow the generation of debug layout files in dot format" OFF)

#misc options
if(NOT MSVC)
	# Pot updates don't work at Windows so no reason to even try
    option(ENABLE_POT_UPDATE_TARGET "Enables the tools to update the pot files and manuals. This target has extra dependencies." OFF)
endif(NOT MSVC)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_NOTIFICATIONS "Enable Window manager notification messages" ON)
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")

#
# Handle options (set paths/definitions/etc...)
#



if(CMAKE_COMPILER_IS_GNUCXX)
	# Set our own default flags at first run.
	if(NOT CONFIGURED)

		if(ENABLE_STRICT_COMPILATION)
			# The current networking code breaks strict aliasing in g++ 4.5.
			set(STRICT_FLAGS "-Werror -Wno-unused-parameter -Wno-strict-aliasing")
		else(ENABLE_STRICT_COMPILATION)
			set(STRICT_FLAGS "")
		endif(ENABLE_STRICT_COMPILATION)


		# Strict compilation for C files is disabled until somebody wants to clean them.
		set(CMAKE_C_FLAGS "-O2 -W -Wall -ansi $ENV{CFLAGS}" 
				CACHE STRING "Flags used by the C compiler during normal builds." FORCE)
		set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -ggdb3 -W -Wall -ansi $ENV{CFLAGS}"
				CACHE STRING "Flags used by the C compiler during debug builds." FORCE)

		set(CMAKE_CXX_FLAGS "-O2 -W -Wall -ansi ${STRICT_FLAGS} $ENV{CXXFLAGS}"
				CACHE STRING "Flags used by the CXX compiler during normal builds." FORCE)
		set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -ggdb3 -W -Wall -ansi ${STRICT_FLAGS} $ENV{CXXFLAGS}"
				CACHE STRING "Flags used by the CXX compiler during debug builds." FORCE)

	endif(NOT CONFIGURED)

endif(CMAKE_COMPILER_IS_GNUCXX)

# compose datadir path of datarootdir and datadirname
set(DATADIR ${DATAROOTDIR}/${DATADIRNAME})

if(NOT WIN32)
  add_definitions(-DWESNOTH_PATH=\\\"${DATADIR}\\\")
endif(NOT WIN32)

if(NOT ENABLE_POOL_ALLOC OR WIN32)
  add_definitions(-DDISABLE_POOL_ALLOC)
endif(NOT ENABLE_POOL_ALLOC OR WIN32)

if(NOT ENABLE_EDITOR)
  add_definitions(-DDISABLE_EDITOR)
endif(NOT ENABLE_EDITOR)

if(X11_FOUND)
  add_definitions(-D_X11)
endif(X11_FOUND)

add_definitions(-DHAS_RELATIVE_LOCALEDIR=1)
add_definitions(-DLOCALEDIR=\\\"${LOCALEDIR}\\\")
set(LOCALE_INSTALL ${DATADIR}/${LOCALEDIR})

add_definitions(-DFIFODIR=\\\"${FIFO_DIR}\\\")

if(PREFERENCES_DIR)
  add_definitions(-DPREFERENCES_DIR=\\\"${PREFERENCES_DIR}\\\")
endif(PREFERENCES_DIR)

if(GUI STREQUAL "tiny")
  add_definitions(-DUSE_TINY_GUI)
endif(GUI STREQUAL "tiny")

if(ENABLE_LOW_MEM)
  add_definitions(-DLOW_MEM)
endif(ENABLE_LOW_MEM)

if(ENABLE_DEBUG_WINDOW_LAYOUT)
	add_definitions(-DDEBUG_WINDOW_LAYOUT_GRAPHS)
endif(ENABLE_DEBUG_WINDOW_LAYOUT)

#
# Libraries that are only required by some targets
#

if(ENABLE_TOOLS OR ENABLE_GAME OR ENABLE_TESTS)
  find_package( SDL_image 1.2 REQUIRED )
endif(ENABLE_TOOLS OR ENABLE_GAME OR ENABLE_TESTS)
if(ENABLE_GAME OR ENABLE_TESTS)
  find_package( SDL_mixer 1.2 REQUIRED )
  find_package( SDL_ttf 2.0.8 REQUIRED )
endif(ENABLE_GAME OR ENABLE_TESTS)
if(ENABLE_GAME OR ENABLE_SERVER OR ENABLE_CAMPAIGN_SERVER OR ENABLE_TESTS)
  find_package( SDL_net REQUIRED )
endif(ENABLE_GAME OR ENABLE_SERVER OR ENABLE_CAMPAIGN_SERVER OR ENABLE_TESTS)
if(ENABLE_TOOLS)
  find_package( ZLIB REQUIRED )
  find_package( PNG REQUIRED )
endif(ENABLE_TOOLS)
if(ENABLE_TESTS)
	find_package( Boost 1.35 REQUIRED COMPONENTS unit_test_framework )
endif(ENABLE_TESTS)
if(ENABLE_GAME)
	find_package( Lua51 REQUIRED)

	if(NOT MSVC)
		find_package( PkgConfig REQUIRED )
		pkg_check_modules( PANGOCAIRO REQUIRED pangocairo>=1.14.8 )
		pkg_check_modules( FONTCONFIG REQUIRED fontconfig>=2.4.1 )
	endif(NOT MSVC)

	find_package( FriBiDi )
	if(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)
	  add_definitions(-DHAVE_FRIBIDI)
	elseif(ENABLE_FRIBIDI AND NOT FRIBIDI_LIBRARIES)
	  message("Could not find FriBiDi. Disabling FriBiDi support.")
	endif()

	if(ENABLE_NOTIFICATIONS)
		pkg_check_modules(LIBDBUS dbus-1)
		if(LIBDBUS_FOUND)
		  	add_definitions(-DHAVE_LIBDBUS)
		else(LIBDBUS_FOUND)
			message("Could not find dbus-1, Disabling notification support.")
		endif(LIBDBUS_FOUND)
	endif(ENABLE_NOTIFICATIONS)

endif(ENABLE_GAME)

# get languages
file(READ po/LINGUAS LINGUAS)
string(REPLACE "\n" "" LINGUAS ${LINGUAS})
separate_arguments(LINGUAS)

#
# Include subdirectories
#

add_subdirectory(doc)

if (GETTEXT_FOUND AND ENABLE_NLS)
  add_subdirectory(po)
endif (GETTEXT_FOUND AND ENABLE_NLS)
add_subdirectory(src)

#
# shrink images for tinygui
#

if(GUI STREQUAL "tiny")
  FIND_PACKAGE( ImageMagick REQUIRED )

  if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    set(IMAGE_BUILD_DIR ${CMAKE_SOURCE_DIR}/tiny_images)
  else()
    set(IMAGE_BUILD_DIR ${CMAKE_BINARY_DIR})
  endif()

  set(IMAGE_FILES data/*.jpg data/*.png images/*.jpg images/*.png)

  add_custom_target(shrink-images ALL 
                    COMMAND ${CMAKE_COMMAND}
                    ARGS -DIMAGE_FILES="${IMAGE_FILES}" -DIMAGE_BUILD_DIR="${IMAGE_BUILD_DIR}" -DIMAGEMAGICK_IDENTIFY_EXECUTABLE="${IMAGEMAGICK_IDENTIFY_EXECUTABLE}" -DIMAGEMAGICK_CONVERT_EXECUTABLE="${IMAGEMAGICK_CONVERT_EXECUTABLE}" -P "${CMAKE_MODULE_PATH}/ShrinkImages.cmake"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Creating resized images for tinygui.")

  #
  # Install files (exclude images from data-dir, we install them from our build dir)
  #

  # this is a workaround for a bug in 2.4-7
  file(MAKE_DIRECTORY ${IMAGE_BUILD_DIR}/data ${IMAGE_BUILD_DIR}/images)
  install(DIRECTORY ${IMAGE_BUILD_DIR}/data ${IMAGE_BUILD_DIR}/images DESTINATION ${DATADIR} USE_SOURCE_PERMISSIONS)
  install(DIRECTORY data fonts sounds DESTINATION ${DATADIR} USE_SOURCE_PERMISSIONS
          PATTERN "*.png" EXCLUDE
          PATTERN "*.jpg" EXCLUDE
          PATTERN ".svn" EXCLUDE )
else()

  #
  # Install files
  #
  install(DIRECTORY data fonts images sounds DESTINATION ${DATADIR} USE_SOURCE_PERMISSIONS PATTERN ".svn" EXCLUDE )

endif()

#
# Install desktop file so wesnoth appears in the application start menu with an icon
# TODO: string replacement to have the real binary name in does not work atm, because in the Categories= block semicolons are lost...
#
if(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)
  # do some crude string replacing to have the real binary name in the .desktop file (read in original .desktop file, replace the Exec= line with the correct value and output the generated file)
#  file(READ icons/wesnoth.desktop wesnoth-desktop-orig)
  #string(REGEX REPLACE "(\nName.*=.*)\n" "\\1 (${BINARY_SUFFIX})\n" wesnoth-desktop-modified ${wesnoth-desktop-orig} )
#  string(REPLACE "Exec=wesnoth" "Exec=${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}" wesnoth-desktop-modified ${wesnoth-desktop-orig} )
#  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop ${wesnoth-desktop-modified} )
  
  #execute_process(COMMAND sed "-i" "'s/^\(Name.*=.*\)$/\1TEST/g'" ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop )
  #exec_program(sed ARGS "-i" "'s/^\(Name.*=.*\)$/\1TEST/g'" ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop )
  # install the generated .desktop file
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop DESTINATION ${DESKTOPDIR} )
  install(FILES icons/wesnoth.desktop DESTINATION ${DESKTOPDIR} )
  install(FILES icons/wesnoth-icon.png DESTINATION ${ICONDIR} )
  
  if(ENABLE_EDITOR)
    # do some crude string replacing to have the real binary name in the .desktop file (read in original .desktop file, replace the Exec= line with the correct value and output the generated file)
#    file(READ icons/wesnoth_editor.desktop wesnoth-editor-desktop-orig)
#    string(REPLACE "Exec=wesnoth -e" "Exec=${BINARY_PREFIX}wesnoth${BINARY_SUFFIX} -e" wesnoth-editor-desktop-modified ${wesnoth-editor-desktop-orig} )
#    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX}.desktop ${wesnoth-editor-desktop-modified} )
    # install the generated .desktop file
#    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX}.desktop DESTINATION ${DESKTOPDIR} )
    install(FILES icons/wesnoth_editor.desktop DESTINATION ${DESKTOPDIR} )
    install(FILES icons/wesnoth_editor-icon.png DESTINATION ${ICONDIR} )
  endif(ENABLE_EDITOR)
endif(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)


if(ENABLE_SERVER AND FIFO_DIR)
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{DESTDIR}/${FIFO_DIR})")
  if(SERVER_UID AND SERVER_GID)
    install(CODE "execute_process(COMMAND chown ${SERVER_UID}:${SERVER_GID} \$ENV{DESTDIR}/${FIFO_DIR})")
  endif()
endif()

# Index for checking states of localized images at runtime.
install(FILES l10n-track DESTINATION ${DATADIR})

#
# uninstall
#

configure_file(
  "${CMAKE_MODULE_PATH}/uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake")

#
# Packaging stuff
#

include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")

set(CONFIGURED ON CACHE INTERNAL "")
