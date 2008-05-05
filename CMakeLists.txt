# most stuff should be implemented so far, mgoe should know the details (he has done all the work...)
# what is not working so far:
# * equivalent to make update-po4a
# * (probably) --enable-dummy-locales
# * some other smaller stuff
# * nice INSTALL howto

# use our own version of FindBoost.cmake
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# find all deps needed
FIND_PACKAGE( SDL 1.2.7 REQUIRED )
FIND_PACKAGE( SDL_image 1.2 REQUIRED )
FIND_PACKAGE( SDL_mixer 1.2 REQUIRED )
FIND_PACKAGE( SDL_net  REQUIRED )
FIND_PACKAGE( SDL_ttf 2.0.8 REQUIRED )
# setting python to a "must have" for the moment, can later on be changed to be optional
FIND_PACKAGE( PythonInterp 2.4 )
FIND_PACKAGE( PythonLibs 2.4 )
FIND_PACKAGE( Boost 1.33 REQUIRED COMPONENTS iostreams regex )
FIND_PACKAGE( ZLIB REQUIRED )
FIND_PACKAGE( PNG REQUIRED )
FIND_PACKAGE( Gettext REQUIRED )

#Options

set(BINDIR "bin" CACHE STRING "Where to install binaries")
set(DATAROOTDIR "share" CACHE STRING "Sets the root of data directories to a non-default location")
set(DATADIR "${DATAROOTDIR}/wesnoth" CACHE STRING "Read-only architecture-independent game data")
set(LOCALEDIR "translations" CACHE STRING "sets the locale data directory to a non-default location")
set(PREFERENCES_DIR "" CACHE STRING "Use a non-default preferences directory (.wesnoth on unix)")

set(GUI "normal" CACHE STRING "Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0) or 320x240 (PDAs) (normal|small|tiny)")

if(NOT WIN32)
  if(IS_ABSOLUTE ${DATADIR})
    add_definitions(-DWESNOTH_PATH='\"${DATADIR}\"')
    message("set -DWESNOTH_PATH='\"${DATADIR}\"'")
  else(IS_ABSOLUTE ${DATADIR})
    add_definitions(-DWESNOTH_PATH='\"${CMAKE_INSTALL_PREFIX}/${DATADIR}\"')
    message("set -DWESNOTH_PATH='\"${CMAKE_INSTALL_PREFIX}/${DATADIR}\"'")
  endif(IS_ABSOLUTE ${DATADIR})
endif(NOT WIN32)

add_definitions(-DLOCALEDIR=='\"${LOCALEDIR}\"')
message("set -DLOCALEDIR='\"${LOCALEDIR}\"'")

if(NOT IS_ABSOLUTE ${LOCALEDIR})
  add_definitions(-DHAS_RELATIVE_LOCALEDIR)
  message("set -DHAS_RELATIVE_LOCALEDIR")
  set(LOCALE_INSTALL ${DATADIR}/${LOCALEDIR})
else(NOT IS_ABSOLUTE ${LOCALEDIR})
  set(LOCALE_INSTALL ${LOCALEDIR})
endif(NOT IS_ABSOLUTE ${LOCALEDIR})

if(PREFERENCES_DIR)
  add_definitions(-DPREFERENCES_DIR='\"${PREFERENCES_DIR}\"')
  message("set -DPREFERENCES_DIR='\"${PREFERENCES_DIR}\"'")
endif(PREFERENCES_DIR)


if(GUI STREQUAL "tiny")
  add_definitions(-DUSE_TINY_GUI)
elseif(GUI STREQUAL "small")
  add_definitions(-DUSE_SMALL_GUI)
endif(GUI STREQUAL "tiny")


option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign server")
option(ENABLE_SERVER "Enable compilation of server" ON)
option(ENABLE_EDITOR "Enable compilation of map editor")
option(ENABLE_TOOLS "Enable building and installation of tools for artists and WML maintainers")
option(ENABLE_TESTS "Build unit tests")
option(ENABLE_PYTHON  "Enable in-game python extensions" ON)

if(ENABLE_PYTHON AND PYTHON_LIBRARY)
  add_definitions(-DHAVE_PYTHON)
  message("set -DHAVE_PYTHON")
endif(ENABLE_PYTHON AND PYTHON_LIBRARY)


#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# only one of the subdirs working so far...
add_subdirectory(po)
add_subdirectory(src)
#add_subdirectory(doc)

########### install files ###############

install(DIRECTORY data fonts images sounds DESTINATION ${DATADIR} PATTERN ".svn" EXCLUDE )

