# most stuff should be implemented so far, mgoe should know the details (he has done all the work...)
# what is not working so far:
# * equivalent to make update-po4a
# * (probably) --enable-dummy-locales
# * some other smaller stuff
# * nice INSTALL howto

# set minimum version and make sure that things work with cmake 2.6, too
if(COMMAND cmake_minimum_required)
    cmake_minimum_required(VERSION 2.4)
endif(COMMAND cmake_minimum_required)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

# use our own version of FindBoost.cmake and other Find* scripts
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# find all deps needed
FIND_PACKAGE( SDL 1.2.7 REQUIRED )
FIND_PACKAGE( SDL_image 1.2 REQUIRED )
FIND_PACKAGE( SDL_mixer 1.2 REQUIRED )
FIND_PACKAGE( SDL_net  REQUIRED )
FIND_PACKAGE( SDL_ttf 2.0.8 REQUIRED )
FIND_PACKAGE( Boost 1.33 REQUIRED COMPONENTS iostreams regex )
FIND_PACKAGE( ZLIB REQUIRED )
FIND_PACKAGE( PNG REQUIRED )
FIND_PACKAGE( Gettext REQUIRED )


#Really find  python
FIND_PACKAGE( PythonInterp 2.4 )
if(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} 
                  -c "import distutils.sysconfig; import os.path; print os.path.join(distutils.sysconfig.get_config_var('LIBDIR'), distutils.sysconfig.get_config_var('LDLIBRARY'));"
                  OUTPUT_VARIABLE PYTHON_LIBDIR)
  string(REPLACE "\n" "" PYTHON_LIBDIR ${PYTHON_LIBDIR})

  if(PYTHON_LIBDIR)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} 
                    -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc();"
                    OUTPUT_VARIABLE PYTHON_INC)
    string(REPLACE "\n" "" PYTHON_INC ${PYTHON_INC})

    file(TO_CMAKE_PATH ${PYTHON_LIBDIR} PYTHON_LIBRARIES)
    file(TO_CMAKE_PATH ${PYTHON_INC} PYTHON_INC)

    set(PYTHON_LIBRARY ${PYTHON_LIBRARIES} CACHE FILEPATH "bla")
    set(PYTHON_INCLUDE_PATH ${PYTHON_INC} CACHE PATH "bla")

  else(PYTHON_LIBDIR)
    FIND_PACKAGE( PythonLibs 2.4 )
  endif(PYTHON_LIBDIR)

elseif(NOT PYTHON_EXECUTABLE)
  FIND_PACKAGE( PythonLibs 2.4 )
else(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)
  set(PYTHON_LIBRARIES ${PYTHON_LIBRARY})
endif(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)

#Options

set(BINDIR "bin" CACHE STRING "Where to install binaries")
set(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Sets the root of data directories to a non-default location")
set(DATADIRNAME "wesnoth" CACHE STRING "change the name of the directory for the read-only architecture-independent game data")
set(LOCALEDIR "translations" CACHE STRING "change the name of the locale data directory to a non-default name")
set(PREFERENCES_DIR "" CACHE STRING "Use a non-default preferences directory (.wesnoth on unix)")
set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")

set(GUI "normal" CACHE STRING "Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0) or 320x240 (PDAs) (normal|small|tiny)")

option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign server")
option(ENABLE_SERVER "Enable compilation of server" ON)
option(ENABLE_EDITOR "Enable compilation of map editor")
option(ENABLE_TOOLS "Enable building and installation of tools for artists and WML maintainers")
option(ENABLE_TESTS "Build unit tests")

option(ENABLE_PYTHON "Enable in-game python extensions" ON)

# compose datadir path of datarootdir and datadirname
set(DATADIR ${DATAROOTDIR}/${DATADIRNAME})
# compose localedir path of datarootdir and localedirname
set(LOCALEDIR ${DATAROOTDIR}/${LOCALEDIRNAME})

if(NOT WIN32)
  add_definitions(-DWESNOTH_PATH='\"${DATADIR}\"')
  message("set -DWESNOTH_PATH='\"${DATADIR}\"'")
endif(NOT WIN32)

add_definitions(-DHAS_RELATIVE_LOCALEDIR)
add_definitions(-DLOCALEDIR=${LOCALEDIR})
message("set -DLOCALEDIR='\"${LOCALEDIR}\"'")
set(LOCALE_INSTALL ${DATADIR}/${LOCALEDIR})

if(PREFERENCES_DIR)
  add_definitions(-DPREFERENCES_DIR='\"${PREFERENCES_DIR}\"')
  message("set -DPREFERENCES_DIR='\"${PREFERENCES_DIR}\"'")
endif(PREFERENCES_DIR)

if(GUI STREQUAL "tiny")
  add_definitions(-DUSE_TINY_GUI)
elseif(GUI STREQUAL "small")
  add_definitions(-DUSE_SMALL_GUI)
endif(GUI STREQUAL "tiny")

if(ENABLE_PYTHON AND PYTHON_LIBRARY)
  add_definitions(-DHAVE_PYTHON)
  message("set -DHAVE_PYTHON")
endif(ENABLE_PYTHON AND PYTHON_LIBRARY)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# only one of the subdirs working so far...
add_subdirectory(po)
add_subdirectory(src)
#add_subdirectory(doc)


INCLUDE(CPack)
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

# some stuff from am2cmake

########### install files ###############

install(DIRECTORY data fonts images sounds DESTINATION ${DATADIR} PATTERN ".svn" EXCLUDE )

