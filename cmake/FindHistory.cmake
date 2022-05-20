#[=======================================================================[.rst:
FindHistory
-----------

Find the GNU History includes and library.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines the `IMPORTED` target ``History::History``, if
History has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

	HISTORY_FOUND         - True if history found.
	HISTORY_INCLUDE_DIR   - Where to find readline/history.h.
	HISTORY_LIBRARY       - Library when using history.

#]=======================================================================]

find_path(HISTORY_INCLUDE_DIR readline/history.h)
find_library(HISTORY_LIBRARY history)

# handle the QUIETLY and REQUIRED arguments and set HISTORY_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(History DEFAULT_MSG HISTORY_LIBRARY HISTORY_INCLUDE_DIR)

mark_as_advanced(HISTORY_INCLUDE_DIR HISTORY_LIBRARY)

if(HISTORY_FOUND AND (NOT TARGET History::History))
	add_library(History::History UNKNOWN IMPORTED)
	set_target_properties(History::History PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${HISTORY_INCLUDE_DIR}")
	set_target_properties(History::History PROPERTIES IMPORTED_LOCATION "${HISTORY_LIBRARY}")
endif()
