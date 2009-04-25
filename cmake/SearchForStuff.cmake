#check for some platform specific things and export defines accordingly...
#done basically the same was as AC_CHECK_HEADERS and AC_CHECK_FUNCS in configure.ac
#the file is basically built upon the info available at
#http://www.vtk.org/Wiki/CMake_HowToDoPlatformChecks
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)

#the two includes below seem to not be required, those headers are checked for
#anyway, including them here, too...
CHECK_INCLUDE_FILES(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

#sendfile should be in one of the headers checked for here, so first check if
#one of the headers is available and then check for 'sendfile'
CHECK_INCLUDE_FILES(poll.h HAVE_POLL_H)
CHECK_INCLUDE_FILES(sys/poll.h HAVE_SYS_POLL_H)
CHECK_INCLUDE_FILES(sys/select.h HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILES(sys/sendfile.h HAVE_SYS_SENDFILE_H)
if(HAVE_SYS_SENDFILE_H)
  CHECK_FUNCTION_EXISTS(sendfile HAVE_SENDFILE)
endif(HAVE_SYS_SENDFILE_H)

#in configure.ac it is not explicitly tested, if it is in 'm', instead the first
#test is if "floor" is available in m and later on it is checked if round,
#sendfile and others do exist (with the 'm' lib linked), regarding our sources
#we *only* want round from 'm'
CHECK_LIBRARY_EXISTS(m round "" HAVE_ROUND)

#use config.h.cmake to create a list of #defines comparable to the one configure
#does, this file is created in the dir where cmake is run from
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)
#to make the compiler actually use the generated config.h
add_definitions(-DHAVE_CONFIG_H)
