#check for some platform specific things and export defines accordingly...
#done basically the same was as AC_CHECK_HEADERS and AC_CHECK_FUNCS in configure.ac
#the file is basically built upon the info available at
#http://www.vtk.org/Wiki/CMake_HowToDoPlatformChecks
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)

#sendfile should be in one of the headers checked for here, so first check if
#one of the headers is available and then check for 'sendfile'
CHECK_INCLUDE_FILES(sys/sendfile.h HAVE_SYS_SENDFILE_H)
if(HAVE_SYS_SENDFILE_H)
  CHECK_FUNCTION_EXISTS(sendfile HAVE_SENDFILE)
endif(HAVE_SYS_SENDFILE_H)

#use config.h.cmake to create a list of #defines comparable to the one configure
#does, this file is created in the dir where cmake is run from
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)
#to make the compiler actually use the generated config.h
add_definitions(-DHAVE_CONFIG_H)
