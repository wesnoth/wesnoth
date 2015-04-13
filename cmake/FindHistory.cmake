# Locale GNU history library

find_path(HISTORY_INCLUDE_DIR readline/history.h)

find_library(HISTORY_LIBRARY history)

# handle the QUIETLY and REQUIRED arguments and set XXX_FOUND to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HISTORY DEFAULT_MSG HISTORY_LIBRARY HISTORY_INCLUDE_DIR)

mark_as_advanced(HISTORY_INCLUDE_DIR HISTORY_LIBRARY)
