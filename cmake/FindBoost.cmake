# - Try to find Boost include dirs and libraries
# Usage of this module as follows:
#
#     FIND_PACKAGE( Boost COMPONENTS date_time filesystem iostreams ... )
#
# The Boost_ADDITIONAL_VERSIONS variable can be used to specify a list of
# boost version numbers that should be taken into account when searching
# for the libraries. Unfortunately boost puts the version number into the
# actual filename for the libraries, so this might be needed in the future
# when new boost versions are released.
#
# Currently this module searches for the following version numbers:
# 1.33, 1.33.0, 1.33.1, 1.34, 1.34.0, 1.34.1
#
# The components list needs to be the actual names of boost libraries, that is
# the part of the actual library files that differ on different libraries. So
# its "date_time" for "libboost_date_time...". Anything else will result in
# errors
#
# Variables used by this module, they can change the default behaviour:
#  Boost_USE_NONMULTITHREAD      Can be set to TRUE to use the non-multithreaded
#                                boost libraries.
#  Boost_ADDITIONAL_VERSIONS     A list of version numbers to use for searching
#                                the boost include directory. The default list
#                                of version numbers is:
#                                1.33, 1.33.0, 1.33.1, 1.34, 1.34.0, 1.34.1
#                                If you want to look for an older or newer
#                                version set this variable to a list of
#                                strings, where each string contains a number, i.e.
#                                SET(Boost_ADDITIONAL_VERSIONS "0.99.0" "1.35.0")
#  Boost_ROOT                    Preferred installation prefix for searching for Boost,
#                                set this if the module has problems finding the proper Boost installation
#  Boost_INCLUDEDIR              Set this to the include directory of Boost, if the
#                                module has problems finding the proper Boost installation
#  Boost_LIBRARYDIR              Set this to the lib directory of Boost, if the
#                                module has problems finding the proper Boost installation
#
#  The last three variables are available also as environment variables
#
#
# Variables defined by this module:
#
#  Boost_FOUND                System has Boost, this means the include dir was found,
#                             as well as all the libraries specified in the COMPONENTS list
#  Boost_INCLUDE_DIRS         Boost include directories, not cached
#  Boost_INCLUDE_DIR          This is almost the same as above, but this one is cached and may be
#                             modified by advanced users
#  Boost_LIBRARIES            Link these to use the Boost libraries that you specified, not cached
#  Boost_LIBRARY_DIRS         The path to where the Boost library files are.
#  Boost_VERSION              The version number of the boost libraries that have been found,
#                             same as in version.hpp from Boost
#  Boost_LIB_VERSION          The version number in filename form as its appended to the library filenames
#  Boost_MAJOR_VERSION        major version number of boost
#  Boost_MINOR_VERSION        minor version number of boost
#  Boost_SUBMINOR_VERSION     subminor version number of boost

# For each component you list the following variables are set.
# ATTENTION: The component names need to be in lower case, just as the boost
# library names however the cmake variables use upper case for the component
# part. So you'd get Boost_SERIALIZATION_FOUND for example.
#
#  Boost_${COMPONENT}_FOUND             True IF the Boost library "component" was found.
#  Boost_${COMPONENT}_LIBRARY           The absolute path of the Boost library "component".
#  Boost_${COMPONENT}_LIBRARY_DEBUG     The absolute path of the debug version of the
#                                       Boost library "component".
#  Boost_${COMPONENT}_LIBRARY_RELEASE   The absolute path of the release version of the
#                                       Boost library "component"
#
#  Copyright (c) 2006-2008 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2007      Wengo
#  Copyright (c) 2007      Mike Jackson
#  Copyright (c) 2008      Andreas Pakulat <apaku@gmx.de>
#
#  Redistribution AND use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# MESSAGE(STATUS "Finding Boost libraries.... ")

SET( _boost_TEST_VERSIONS ${Boost_ADDITIONAL_VERSIONS} "1.33" "1.33.0" "1.33.1" "1.34" "1.34.0" "1.34.1" )

############################################
#
# Check the existence of the libraries.
#
############################################
# This macro was taken directly from the FindQt4.cmake file that is included
# with the CMake distribution. This is NOT my work. All work was done by the
# original authors of the FindQt4.cmake file. Only minor modifications were
# made to remove references to Qt and make this file more generally applicable
#########################################################################

MACRO (_Boost_ADJUST_LIB_VARS basename)
  IF (Boost_INCLUDE_DIR )
    #MESSAGE(STATUS "Adjusting ${basename} ")

    IF (Boost_${basename}_LIBRARY_DEBUG AND Boost_${basename}_LIBRARY_RELEASE)
      # if the generator supports configuration types then set
      # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
      IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET(Boost_${basename}_LIBRARY optimized ${Boost_${basename}_LIBRARY_RELEASE} debug ${Boost_${basename}_LIBRARY_DEBUG})
      ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        # if there are no configuration types and CMAKE_BUILD_TYPE has no value
        # then just use the release libraries
        SET(Boost_${basename}_LIBRARY ${Boost_${basename}_LIBRARY_RELEASE} )
      ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      SET(Boost_${basename}_LIBRARIES optimized ${Boost_${basename}_LIBRARY_RELEASE} debug ${Boost_${basename}_LIBRARY_DEBUG})
    ENDIF (Boost_${basename}_LIBRARY_DEBUG AND Boost_${basename}_LIBRARY_RELEASE)

    # if only the release version was found, set the debug variable also to the release version
    IF (Boost_${basename}_LIBRARY_RELEASE AND NOT Boost_${basename}_LIBRARY_DEBUG)
      SET(Boost_${basename}_LIBRARY_DEBUG ${Boost_${basename}_LIBRARY_RELEASE})
      SET(Boost_${basename}_LIBRARY       ${Boost_${basename}_LIBRARY_RELEASE})
      SET(Boost_${basename}_LIBRARIES     ${Boost_${basename}_LIBRARY_RELEASE})
    ENDIF (Boost_${basename}_LIBRARY_RELEASE AND NOT Boost_${basename}_LIBRARY_DEBUG)

    # if only the debug version was found, set the release variable also to the debug version
    IF (Boost_${basename}_LIBRARY_DEBUG AND NOT Boost_${basename}_LIBRARY_RELEASE)
      SET(Boost_${basename}_LIBRARY_RELEASE ${Boost_${basename}_LIBRARY_DEBUG})
      SET(Boost_${basename}_LIBRARY         ${Boost_${basename}_LIBRARY_DEBUG})
      SET(Boost_${basename}_LIBRARIES       ${Boost_${basename}_LIBRARY_DEBUG})
    ENDIF (Boost_${basename}_LIBRARY_DEBUG AND NOT Boost_${basename}_LIBRARY_RELEASE)
    
    IF (Boost_${basename}_LIBRARY)
      SET(Boost_${basename}_LIBRARY ${Boost_${basename}_LIBRARY} CACHE FILEPATH "The Boost ${basename} library")
      GET_FILENAME_COMPONENT(Boost_LIBRARY_DIRS "${Boost_${basename}_LIBRARY}" PATH)
      SET(Boost_${basename}_FOUND 1)
    ENDIF (Boost_${basename}_LIBRARY)

  ENDIF (Boost_INCLUDE_DIR )
  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED(
      Boost_${basename}_LIBRARY
      Boost_${basename}_LIBRARY_RELEASE
      Boost_${basename}_LIBRARY_DEBUG
  )
ENDMACRO (_Boost_ADJUST_LIB_VARS)

#-------------------------------------------------------------------------------


SET( _boost_IN_CACHE TRUE)

IF(Boost_INCLUDE_DIR)
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} COMPONENT)
    IF(NOT Boost_${COMPONENT}_FOUND)
      SET( _boost_IN_CACHE FALSE)
    ENDIF(NOT Boost_${COMPONENT}_FOUND)
  ENDFOREACH(COMPONENT)
ELSE(Boost_INCLUDE_DIR)
  SET( _boost_IN_CACHE FALSE)
ENDIF(Boost_INCLUDE_DIR)

IF (_boost_IN_CACHE)
  # in cache already
  SET(Boost_FOUND TRUE)
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} COMPONENT)
    _Boost_ADJUST_LIB_VARS( ${COMPONENT} )
  FOREACH(COMPONENT)
  SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
ELSE (_boost_IN_CACHE)
  # Need to search for boost

  SET(_boost_INCLUDE_SEARCH_DIRS
    C:/boost/include
    "C:/Program Files/boost/boost_${Boost_REQUIRED_VERSION}"
    # D: is very often the cdrom drive, IF you don't have a
    # cdrom inserted it will popup a very annoying dialog
    #D:/boost/include
    /sw/local/include
  )

  SET(_boost_LIBRARIES_SEARCH_DIRS
    C:/boost/lib
    "C:/Program Files/boost/boost_${Boost_REQUIRED_VERSION}/lib"
    /sw/local/lib
  )

  IF( NOT $ENV{Boost_ROOT} STREQUAL "" )
    SET(_boost_INCLUDE_SEARCH_DIRS $ENV{Boost_ROOT}/include ${_boost_INCLUDE_SEARCH_DIRS})
    SET(_boost_LIBRARIES_SEARCH_DIRS $ENV{Boost_ROOT}/lib ${_boost_INCLUDE_SEARCH_DIRS})
  ENDIF( NOT $ENV{Boost_ROOT} STREQUAL "" )

  IF( NOT $ENV{Boost_INCLUDEDIR} STREQUAL "" )
    SET(_boost_INCLUDE_SEARCH_DIRS $ENV{Boost_INCLUDEDIR} ${_boost_INCLUDE_SEARCH_DIRS})
  ENDIF( NOT $ENV{Boost_INCLUDEDIR} STREQUAL "" )

  IF( NOT $ENV{Boost_LIBRARYDIR} STREQUAL "" )
    SET(_boost_LIBRARIES_SEARCH_DIRS $ENV{Boost_LIBRARYDIR} ${_boost_INCLUDE_SEARCH_DIRS})
  ENDIF( NOT $ENV{Boost_LIBRARYDIR} STREQUAL "" )

  IF( Boost_ROOT )
    SET(_boost_INCLUDE_SEARCH_DIRS ${Boost_ROOT}/include ${_boost_INCLUDE_SEARCH_DIRS})
    SET(_boost_LIBRARIES_SEARCH_DIRS ${Boost_ROOT}/lib ${_boost_LIBRARIES_SEARCH_DIRS})
  ENDIF( Boost_ROOT )

  IF( Boost_INCLUDEDIR )
    SET(_boost_INCLUDE_SEARCH_DIRS ${Boost_INCLUDEDIR}/include ${_boost_INCLUDE_SEARCH_DIRS})
  ENDIF( Boost_INCLUDEDIR )

  IF( Boost_LIBRARYDIR )
    SET(_boost_LIBRARIES_SEARCH_DIRS ${Boost_LIBRARYDIR}/include ${_boost_LIBRARIES_SEARCH_DIRS})
  ENDIF( Boost_LIBRARYDIR )

  FOREACH(_boost_VER ${_boost_TEST_VERSIONS})
    IF( NOT Boost_INCLUDE_DIR )

      # Add in a path suffix, based on the required version, ideally we could
      # read this from version.hpp, but for that to work we'd need to know the include
      # dir already
      SET(_boost_PATH_SUFFIX
        boost-${_boost_VER}
      )
      STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1_\\2_\\3" _boost_PATH_SUFFIX ${_boost_PATH_SUFFIX})


      FIND_PATH(Boost_INCLUDE_DIR
          NAMES         boost/config.hpp
          PATHS         ${_boost_INCLUDE_SEARCH_DIRS}
          PATH_SUFFIXES ${_boost_PATH_SUFFIX}
      )

      # Extract Boost_VERSION and Boost_LIB_VERSION from version.hpp
      # Read the whole file:
      #
      SET(BOOST_VERSION 0)
      SET(BOOST_LIB_VERSION "")
      FILE(READ "${Boost_INCLUDE_DIR}/boost/version.hpp" _boost_VERSION_HPP_CONTENTS)

      STRING(REGEX REPLACE ".*#define BOOST_VERSION ([0-9]+).*" "\\1" Boost_VERSION "${_boost_VERSION_HPP_CONTENTS}")
      STRING(REGEX REPLACE ".*#define BOOST_LIB_VERSION \"([0-9_]+)\".*" "\\1" Boost_LIB_VERSION "${_boost_VERSION_HPP_CONTENTS}")

      SET(Boost_LIB_VERSION ${Boost_LIB_VERSION} CACHE STRING "The library version string for boost libraries")
      SET(Boost_VERSION ${Boost_VERSION} CACHE STRING "The version number for boost libraries")
      
      IF(NOT "${Boost_VERSION}" STREQUAL "0")
        MATH(EXPR Boost_MAJOR_VERSION "${Boost_VERSION} / 100000")
        MATH(EXPR Boost_MINOR_VERSION "${Boost_VERSION} / 100 % 1000")
        MATH(EXPR Boost_SUBMINOR_VERSION "${Boost_VERSION} % 100")
      ENDIF(NOT "${Boost_VERSION}" STREQUAL "0")


    ENDIF( NOT Boost_INCLUDE_DIR )
  ENDFOREACH(_boost_VER)

  #Setting some more suffixes for the library
  SET (Boost_LIB_PREFIX "")
  IF ( WIN32 )
    SET (Boost_LIB_PREFIX "lib")
  ENDIF ( WIN32 )
  SET (_boost_COMPILER "-gcc")
  IF (MSVC71)
    SET (_boost_COMPILER "-vc71")
  ENDIF(MSVC71)
   IF (MSVC80)
    SET (_boost_COMPILER "-vc80")
  ENDIF(MSVC80)
  IF (MINGW)
    SET (_boost_COMPILER "-mgw")
  ENDIF(MINGW)
  IF (CYGWIN)
    SET (_boost_COMPILER "-gcc")
  ENDIF (CYGWIN)
  IF (UNIX)
    IF (APPLE)
        SET (_boost_COMPILER "")
    ELSE (APPLE)
      IF (NOT CMAKE_COMPILER_IS_GNUCC)
        # This is for the intel compiler
        SET (_boost_COMPILER "-il")
      ELSE (NOT CMAKE_COMPILER_IS_GNUCC)
        #find out the version of gcc being used.
        EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
            ARGS --version
            OUTPUT_VARIABLE _boost_COMPILER_VERSION
        )
        STRING(REGEX REPLACE ".* ([0-9])\\.([0-9])\\.[0-9] .*" "\\1\\2"
               _boost_COMPILER_VERSION ${_boost_COMPILER_VERSION})
        SET (_boost_COMPILER "-gcc${_boost_COMPILER_VERSION}")
      ENDIF (NOT CMAKE_COMPILER_IS_GNUCC)
    ENDIF (APPLE)
  ENDIF(UNIX)

  SET (_boost_MULTITHREADED "-mt")

  IF( Boost_USE_NONMULTITHREADED )
    SET (_boost_MULTITHREADED "")
  ENDIF( Boost_USE_NONMULTITHREADED )

  SET( _boost_STATIC_TAG "")
  IF (WIN32)
    SET (_boost_ABI_TAG "g")
    SET( _boost_STATIC_TAG "-s")
  ENDIF(WIN32)
  SET (_boost_ABI_TAG "${_boost_ABI_TAG}d")

  # ------------------------------------------------------------------------
  #  Begin finding boost libraries
  # ------------------------------------------------------------------------
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} UPPERCOMPONENT)
    SET( Boost_{UPPERCOMPONENT}_LIBRARY FALSE)
    SET( Boost_{UPPERCOMPONENT}_LIBRARY_RELEASE FALSE)
    SET( Boost_{UPPERCOMPONENT}_LIBRARY_DEBUG FALSE)
    FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE
        NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}
        PATHS  ${_boost_LIBRARIES_SEARCH_DIRS}
	NO_DEFAULT_PATH
    )

    IF( NOT ${Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE} )
      FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE
          NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${Boost_LIB_VERSION}
                 ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}-${Boost_LIB_VERSION}
                 ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}
                 ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}
                 ${Boost_LIB_PREFIX}boost_${COMPONENT}
      )
    ENDIF( NOT ${Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE} )

    FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG
        NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}-${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}-${_boost_ABI_TAG}
        PATHS  ${_boost_LIBRARIES_SEARCH_DIRS}
	NO_DEFAULT_PATH
    )

    IF( NOT ${Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG} )
      FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG
          NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}-${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}-${_boost_ABI_TAG}
      )
    ENDIF( NOT ${Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG} )
    _Boost_ADJUST_LIB_VARS(${UPPERCOMPONENT})
  ENDFOREACH(COMPONENT)
  # ------------------------------------------------------------------------
  #  End finding boost libraries
  # ------------------------------------------------------------------------

  SET(Boost_INCLUDE_DIRS
    ${Boost_INCLUDE_DIR}
  )

  # MESSAGE(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
  # MESSAGE(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

  SET(Boost_FOUND FALSE)
  IF(Boost_INCLUDE_DIR)
    SET( Boost_FOUND TRUE )
    FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
      STRING(TOUPPER ${COMPONENT} COMPONENT)
      IF(NOT Boost_${COMPONENT}_FOUND)
        SET( Boost_FOUND FALSE)
      ENDIF(NOT Boost_${COMPONENT}_FOUND)
    ENDFOREACH(COMPONENT)
  ELSE(Boost_INCLUDE_DIR)
    SET( Boost_FOUND FALSE)
  ENDIF(Boost_INCLUDE_DIR)

  IF (Boost_FOUND)
      IF (NOT Boost_FIND_QUIETLY)
        MESSAGE(STATUS "Found The Following Boost Libraries:")
        FOREACH ( COMPONENT  ${Boost_FIND_COMPONENTS} )
          STRING( TOUPPER ${COMPONENT} UPPERCOMPONENT )
          IF ( Boost_${UPPERCOMPONENT}_FOUND )
            MESSAGE (STATUS "  ${COMPONENT}")
	    SET(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_${UPPERCOMPONENT}_LIBRARY})
          ENDIF ( Boost_${UPPERCOMPONENT}_FOUND )
        ENDFOREACH(COMPONENT)
	MESSAGE(STATUS "Boost Version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
      ENDIF(NOT Boost_FIND_QUIETLY)
  ELSE (Boost_FOUND)
      IF (Boost_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Please install the Boost libraries AND development packages")
      ENDIF(Boost_FIND_REQUIRED)
  ENDIF(Boost_FOUND)

  # Under Windows, automatic linking is performed, so no need to specify the libraries.
  IF (WIN32)
      SET(Boost_LIBRARIES "")
  ENDIF(WIN32)

  # show the Boost_INCLUDE_DIRS AND Boost_LIBRARIES variables only in the advanced view
  MARK_AS_ADVANCED(Boost_INCLUDE_DIRS
      Boost_LIBRARIES
      Boost_LIBRARY_DIRS
  )
ENDIF(_boost_IN_CACHE)

