# - Find Gettext run-time library and tools.
# This module finds the GNU gettext run-time library (LGPL), include paths and 
# associated tools (GPL). This code sets the following variables:
#  GETTEXT_INCLUDE_DIR         = path(s) to gettext's include files
#  GETTEXT_LIBRARIES           = the libraries to link against to use gettext
#  GETTEXT_INTL_LIBRARY        = path to gettext's intl library
#  GETTEXT_RUNTIME_FOUND       = true if runtime libs were found (intl)
#  GETTEXT_INFO_MSG            = information string about gettext
#  GETTEXT_XGETTEXT_EXECUTABLE = xgettext tool
#  GETTEXT_MSGINIT_EXECUTABLE  = msginit tool
#  GETTEXT_MSGCAT_EXECUTABLE   = msgcat tool
#  GETTEXT_MSGCONV_EXECUTABLE  = msgconv tool
#  GETTEXT_TOOLS_FOUND         = true if all the tools were found
#  GETTEXT_FOUND               = true if both runtime and tools were found
# As a convenience, the following variables can be set before including
# this module to make its life easier:
#  GETTEXT_SEARCH_PATH         = list of path to search gettext components for
# --------------------------------------------------------------------------
# As a convenience, try to find everything as soon as we set any one of
# the cache variables.

macro(GETTEXT_FIND_POTENTIAL_DIRS)

  set(potential_bin_dirs)
  set(potential_lib_dirs)
  set(potential_include_dirs)
  foreach(filepath 
      "${GETTEXT_INTL_LIBRARY}"
      "${GETTEXT_XGETTEXT_EXECUTABLE}"
      "${GETTEXT_MSGINIT_EXECUTABLE}"
      "${GETTEXT_MSGCAT_EXECUTABLE}"
      "${GETTEXT_MSGCONV_EXECUTABLE}"
      )
    get_filename_component(path "${filepath}" PATH)
    set(potential_bin_dirs ${potential_bin_dirs} "${path}/../bin")
    set(potential_lib_dirs ${potential_lib_dirs} "${path}/../lib")
    set(potential_include_dirs ${potential_include_dirs} "${path}/../include")
  endforeach(filepath)

  foreach(path 
      "${GETTEXT_INCLUDE_DIR}"
      "${GETTEXT_SEARCH_PATH}"
      )
    set(potential_bin_dirs ${potential_bin_dirs} "${path}/../bin")
    set(potential_lib_dirs ${potential_lib_dirs} "${path}/../lib")
    set(potential_include_dirs ${potential_include_dirs} "${path}/../include")
  endforeach(path)

endmacro(GETTEXT_FIND_POTENTIAL_DIRS)

# --------------------------------------------------------------------------
# Find the runtime lib

macro(GETTEXT_FIND_RUNTIME_LIBRARY)

  set(GETTEXT_RUNTIME_FOUND 1)

  # The gettext intl include dir (libintl.h)
  
  find_path(GETTEXT_INCLUDE_DIR 
    libintl.h 
    ${potential_include_dirs}
    DOC "Path to gettext include directory (where libintl.h can be found)")
  mark_as_advanced(GETTEXT_INCLUDE_DIR)
  if(NOT GETTEXT_INCLUDE_DIR)
    set(GETTEXT_RUNTIME_FOUND 0)
  endif(NOT GETTEXT_INCLUDE_DIR)

  set(GETTEXT_LIBRARIES)

  # The gettext intl library
  # Some Unix system (like Linux) have gettext right into libc

  if(WIN32)
    set(HAVE_GETTEXT 0)
  else(WIN32)
    include(CheckFunctionExists)
    check_function_exists(gettext HAVE_GETTEXT)
  endif(WIN32)

  if(HAVE_GETTEXT)
    # Even if we have a system one, let the user provide another one
    # eventually (i.e., more recent, or GNU).
    set(GETTEXT_INTL_LIBRARY "" CACHE FILEPATH
      "Path to gettext intl library (leave it empty to use the system one)")
  else(HAVE_GETTEXT)
    find_library(GETTEXT_INTL_LIBRARY 
      NAMES intl 
      PATHS ${potential_lib_dirs}
      DOC "Path to gettext intl library")
    if(NOT GETTEXT_INTL_LIBRARY)
      set(GETTEXT_RUNTIME_FOUND 0)
    endif(NOT GETTEXT_INTL_LIBRARY)
  endif(HAVE_GETTEXT)

  mark_as_advanced(GETTEXT_INTL_LIBRARY)
  if(GETTEXT_INTL_LIBRARY)
    set(GETTEXT_LIBRARIES ${GETTEXT_LIBRARIES} ${GETTEXT_INTL_LIBRARY})
  endif(GETTEXT_INTL_LIBRARY)

  # The gettext asprintf library
  # Actually not useful as it does not seem to exist on Unix

  #   IF(WIN32)
  #     FIND_LIBRARY(GETTEXT_ASPRINTF_LIBRARY 
  #       NAMES asprintf 
  #       PATHS ${potential_lib_dirs}
  #       DOC "Gettext asprintf library")
  #     MARK_AS_ADVANCED(GETTEXT_ASPRINTF_LIBRARY)
  #     IF(NOT GETTEXT_ASPRINTF_LIBRARY)
  #       SET(GETTEXT_RUNTIME_FOUND 0)
  #     ELSE(NOT GETTEXT_ASPRINTF_LIBRARY)
  #       SET(GETTEXT_LIBRARIES ${GETTEXT_LIBRARIES} ${GETTEXT_ASPRINTF_LIBRARY})
  #     ENDIF(NOT GETTEXT_ASPRINTF_LIBRARY)
  #   ENDIF(WIN32)

endmacro(GETTEXT_FIND_RUNTIME_LIBRARY)

# --------------------------------------------------------------------------
# Find the tools

macro(GETTEXT_FIND_TOOLS)
  set(GETTEXT_TOOLS_FOUND 1)
  foreach(tool
      xgettext
      msginit
      msgmerge
      msgcat
      msgconv
      msgfmt
      )
    string(TOUPPER ${tool} tool_upper)
    find_program(GETTEXT_${tool_upper}_EXECUTABLE
      NAMES ${tool} 
      PATHS ${potential_bin_dirs}
      DOC "Path to gettext ${tool} tool")
    mark_as_advanced(GETTEXT_${tool_upper}_EXECUTABLE)
    if(NOT GETTEXT_${tool_upper}_EXECUTABLE)
      set(GETTEXT_TOOLS_FOUND 0)
    endif(NOT GETTEXT_${tool_upper}_EXECUTABLE)
  endforeach(tool)
endmacro(GETTEXT_FIND_TOOLS)

# --------------------------------------------------------------------------
# Some convenient info about gettext, where to get it, etc.

set(GETTEXT_INFO_MSG "More information about gettext can be found at http://directory.fsf.org/gettext.html.")
if(WIN32)
  set(GETTEXT_INFO_MSG "${GETTEXT_INFO_MSG} Windows users can download gettext-runtime-0.13.1.bin.woe32.zip (LGPL), gettext-tools-0.13.1.bin.woe32.zip (GPL) as well as libiconv-1.9.1.bin.woe32.zip (LGPL) from any GNU mirror (say, http://mirrors.kernel.org/gnu/gettext/ and http://mirrors.kernel.org/gnu/libiconv/), unpack the archives in the same directory, then set GETTEXT_INTL_LIBRARY to 'lib/intl.lib' in and GETTEXT_INCLUDE_DIR to 'include' in that directory.\n\nWarning: if you are using ActiveTcl, the ActiveState binary distribution for Tcl, make sure you overwrite the iconv.dll file found in both the Tcl bin/ and lib/ directories with the iconv.dll file found in your gettext bin/ directory.")
endif(WIN32)

# --------------------------------------------------------------------------
# Found ?

gettext_find_potential_dirs()
gettext_find_runtime_library()
gettext_find_tools()

# Try again with new potential dirs now that we may have found the runtime
# or the tools

gettext_find_potential_dirs()
if(NOT GETTEXT_RUNTIME_FOUND)
  gettext_find_runtime_library()
endif(NOT GETTEXT_RUNTIME_FOUND)
if(NOT GETTEXT_TOOLS_FOUND)
  gettext_find_tools()
endif(NOT GETTEXT_TOOLS_FOUND)

if(GETTEXT_RUNTIME_FOUND AND GETTEXT_TOOLS_FOUND)
  set(GETTEXT_FOUND 1)
else(GETTEXT_RUNTIME_FOUND AND GETTEXT_TOOLS_FOUND)
  set(GETTEXT_FOUND 0)
endif(GETTEXT_RUNTIME_FOUND AND GETTEXT_TOOLS_FOUND)

if(NOT GETTEXT_FOUND AND NOT Gettext_FIND_QUIETLY AND Gettext_FIND_REQUIRED)
  message(FATAL_ERROR "Could not find gettext runtime library and tools for internationalization purposes.\n\n${GETTEXT_INFO_MSG}")
endif(NOT GETTEXT_FOUND AND NOT Gettext_FIND_QUIETLY AND Gettext_FIND_REQUIRED)
