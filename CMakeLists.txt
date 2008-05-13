# most stuff should be implemented so far, mgoe should know the details (he has done all the work...)
# what is not working so far:
# * fribidi support
# * "uninstall" target
# * Strict compilation (-Werror -Wno-unused -Wno-sign-compare)
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

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

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

#
# Options
#

#Path options
set(BINDIR "bin" CACHE STRING "Where to install binaries")
set(MANDIR "man" CACHE STRING "Where to install manpages")
set(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Sets the root of data directories to a non-default location")
set(DOCDIR "${DATAROOTDIR}/doc/wesnoth" CACHE STRING "Sets the doc directory to a non-default location.")
set(DATADIRNAME "wesnoth" CACHE STRING "change the name of the directory for the read-only architecture-independent game data")
set(LOCALEDIR "translations" CACHE STRING "change the name of the locale data directory to a non-default name")
set(PREFERENCES_DIR "" CACHE STRING "Use a non-default preferences directory (.wesnoth on unix)")

#Common options
set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")
option(ENABLE_PYTHON "Enable in-game python extensions" ON)
option(ENABLE_DUMMY_LOCALES "Create dummy locales")

#Game options
set(GUI "normal" CACHE STRING "Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0) or 320x240 (PDAs) (normal|small|tiny)")

#server options
set(SERVER_UID "" CACHE STRING "User id of the user who runs wesnothd")
set(SERVER_GID "" CACHE STRING "Group id of the user who runs wesnothd")
set(FIFO_DIR "/var/run/wesnothd" CACHE STRING "Directory for the wesnothd fifo socket file")

#build options
option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign server")
option(ENABLE_SERVER "Enable compilation of server" ON)
option(ENABLE_EDITOR "Enable compilation of map editor")
option(ENABLE_TOOLS "Enable building and installation of tools for artists and WML maintainers")
option(ENABLE_TESTS "Build unit tests")

#
# Handle options (set paths/definitions/etc...)
#

# Set default debug flags at first run
if(NOT CONFIGURED)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -ggdb3 -W -Wall -ansi" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
endif(NOT CONFIGURED)

# compose datadir path of datarootdir and datadirname
set(DATADIR ${DATAROOTDIR}/${DATADIRNAME})

if(NOT WIN32)
  add_definitions(-DWESNOTH_PATH='\"${DATADIR}\"')
endif(NOT WIN32)

add_definitions(-DHAS_RELATIVE_LOCALEDIR)
add_definitions(-DLOCALEDIR='\"${LOCALEDIR}\"')
set(LOCALE_INSTALL ${DATADIR}/${LOCALEDIR})

add_definitions(-DFIFODIR='\"${FIFO_DIR}\"')

if(PREFERENCES_DIR)
  add_definitions(-DPREFERENCES_DIR='\"${PREFERENCES_DIR}\"')
endif(PREFERENCES_DIR)

if(GUI STREQUAL "tiny")
  add_definitions(-DUSE_TINY_GUI)
elseif(GUI STREQUAL "small")
  add_definitions(-DUSE_SMALL_GUI)
endif(GUI STREQUAL "tiny")

if(ENABLE_PYTHON AND PYTHON_LIBRARY)
  add_definitions(-DHAVE_PYTHON)
endif(ENABLE_PYTHON AND PYTHON_LIBRARY)


#create dummy-locales
if(ENABLE_DUMMY_LOCALES)
  add_definitions(-DUSE_DUMMYLOCALES)
  set(DUMMY_LOCALE_C_DIR ${CMAKE_SOURCE_DIR}/locales/C)

  add_custom_command(OUTPUT ${DUMMY_LOCALE_C_DIR}
                     COMMAND mkdir -p ${DUMMY_LOCALE_C_DIR} 
                     && echo | localedef --force ${DUMMY_LOCALE_C_DIR} 2> /dev/null || true)

  file(GLOB_RECURSE LANGS RELATIVE ${CMAKE_SOURCE_DIR}/data/languages data/languages/*.cfg)

  set(DUMMY_LOCALES)
  foreach(LANGFILE ${LANGS})
    string(REGEX REPLACE "(.*)\\.cfg" "\\1" LANG ${LANGFILE})
    if(NOT LANG STREQUAL "C")
      set(DUMMY_LOCALE_DIR ${CMAKE_SOURCE_DIR}/locales/${LANG})
      add_custom_command(OUTPUT ${DUMMY_LOCALE_DIR}
                         COMMAND ln -s 
                         ARGS ${DUMMY_LOCALE_C_DIR} ${DUMMY_LOCALE_DIR}
                         DEPENDS ${DUMMY_LOCALE_C_DIR})

      set(DUMMY_LOCALES ${DUMMY_LOCALES} ${DUMMY_LOCALE_DIR})
    endif(NOT LANG STREQUAL "C")

  endforeach(LANGFILE ${LANGS})
  add_custom_target(create-dummy-locales ALL DEPENDS ${DUMMY_LOCALES})

  # this is a workaround for a bug in 2.4-7
  file(MAKE_DIRECTORY locales)
  install(DIRECTORY locales DESTINATION ${DATADIR} )

endif(ENABLE_DUMMY_LOCALES)

# read languages
file(READ po/LINGUAS LINGUAS)
string(REPLACE "\n" "" LINGUAS ${LINGUAS})
separate_arguments(LINGUAS)

add_subdirectory(doc)
add_subdirectory(po)
add_subdirectory(src)

#shrink images
if(GUI STREQUAL "tiny")
  FIND_PACKAGE( ImageMagick REQUIRED )

  if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    set(IMAGE_BUILD_DIR ${CMAKE_SOURCE_DIR}/tiny_images)
  else()
    set(IMAGE_BUILD_DIR ${CMAKE_BINARY_DIR})
  endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

  set(IMAGE_FILES data/*.jpg data/*.png images/*.jpg images/*.png)

  add_custom_target(shrink-images ALL 
                    COMMAND ${CMAKE_COMMAND}
                    ARGS -DIMAGE_FILES="${IMAGE_FILES}" -DIMAGE_BUILD_DIR="${IMAGE_BUILD_DIR}" -DIMAGEMAGICK_IDENTIFY_EXECUTABLE="${IMAGEMAGICK_IDENTIFY_EXECUTABLE}" -DIMAGEMAGICK_CONVERT_EXECUTABLE="${IMAGEMAGICK_CONVERT_EXECUTABLE}" -P "${CMAKE_SOURCE_DIR}/cmake/ShrinkImages.cmake"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Creating resized images for tinygui.")

  ########### install files ###############

  # this is a workaround for a bug in 2.4-7
  file(MAKE_DIRECTORY ${IMAGE_BUILD_DIR}/data ${IMAGE_BUILD_DIR}/images)
  install(DIRECTORY ${IMAGE_BUILD_DIR}/data ${IMAGE_BUILD_DIR}/images DESTINATION ${DATADIR})
  install(DIRECTORY data fonts sounds DESTINATION ${DATADIR} 
          PATTERN "*.png" EXCLUDE
          PATTERN "*.jpg" EXCLUDE
          PATTERN ".svn" EXCLUDE )
else()

  install(DIRECTORY data fonts images sounds DESTINATION ${DATADIR} PATTERN ".svn" EXCLUDE )

endif(GUI STREQUAL "tiny")

if(ENABLE_SERVER AND FIFO_DIR)
  install(CODE "execute_process(COMMAND mkdir ${FIFO_DIR})")
  if(SERVER_UID AND SERVER_GID)
    install(CODE "execute_process(COMMAND chown ${SERVER_UID}:${SERVER_GID} ${FIFO_DIR})")
  endif(SERVER_UID AND SERVER_GID)
endif(ENABLE_SERVER AND FIFO_DIR)

#
# Packaging stuff
#


INCLUDE(CPack)
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

set(CONFIGURED ON CACHE INTERNAL "")
