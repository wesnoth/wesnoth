# Copyright (c) 2008, OpenCog.org (http://opencog.org)
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying FindUnixODBC.LICENSE file.

# - Try to find the UnixODBC library; Once done this will define
#
# UnixODBC_FOUND - system has the UnixODBC library
# UnixODBC_INCLUDE_DIRS - the UnixODBC include directory
# UnixODBC_LIBRARIES - The libraries needed to use UnixODBC

# Look for the header file
FIND_PATH(UnixODBC_INCLUDE_DIR uodbc_stats.h /usr/include /usr/local/include /usr/include/odbc /usr/local/include/odbc /usr/include/libodbc /usr/local/include/libodbc)

# Look for the library
FIND_LIBRARY(UnixODBC_LIBRARY NAMES odbc PATH /usr/lib /usr/local/lib)

# Copy the results to the output variables.
IF (UnixODBC_INCLUDE_DIR AND UnixODBC_LIBRARY)
SET(UnixODBC_FOUND 1)
SET(UnixODBC_LIBRARIES ${UnixODBC_LIBRARY})
SET(UnixODBC_INCLUDE_DIRS ${UnixODBC_INCLUDE_DIR})
ELSE (UnixODBC_INCLUDE_DIR AND UnixODBC_LIBRARY)
SET(UnixODBC_FOUND 0)
SET(UnixODBC_LIBRARIES)
SET(UnixODBC_INCLUDE_DIRS)
ENDIF (UnixODBC_INCLUDE_DIR AND UnixODBC_LIBRARY)

# Report the results.
IF (NOT UnixODBC_FOUND)
SET(UnixODBC_DIR_MESSAGE "UnixODBC was not found. Make sure UnixODBC_LIBRARY and UnixODBC_INCLUDE_DIR are set.")
IF (NOT UnixODBC_FIND_QUIETLY)
MESSAGE(STATUS "${UnixODBC_DIR_MESSAGE}")
ELSE (NOT UnixODBC_FIND_QUIETLY)
IF (UnixODBC_FIND_REQUIRED)
MESSAGE(FATAL_ERROR "${UnixODBC_DIR_MESSAGE}")
ENDIF (UnixODBC_FIND_REQUIRED)
ENDIF (NOT UnixODBC_FIND_QUIETLY)
ENDIF (NOT UnixODBC_FOUND)

MARK_AS_ADVANCED(UnixODBC_INCLUDE_DIRS)
MARK_AS_ADVANCED(UnixODBC_LIBRARIES)