
# set minimum version
cmake_minimum_required(VERSION 2.8.5)

project(wesnoth)

include(CheckCXXCompilerFlag)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# use our own version of FindBoost.cmake and other Find* scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# function to remove a flag from a variable
function(RemoveFlag VAR SCOPE FLAG DOCSTRING)
	if(NOT "${${VAR}}" STREQUAL "")
		MESSAGE("Removing ${FLAG} flag from ${VAR}")
		separate_arguments(${VAR})
		list(REMOVE_ITEM ${VAR} ${FLAG})
		string(REPLACE ";" " " ${VAR} "${${VAR}}")

		if("${SCOPE}" STREQUAL "CACHE")
			set(${VAR} "${${VAR}}" CACHE STRING "${DOCSTRING}" FORCE)
		elseif("${SCOPE}" STREQUAL "SCRIPT")
			set(${VAR} "${${VAR}}" PARENT_SCOPE)
		endif()
	endif()
endfunction()

#
# Options
#

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

#Path options
set(DATADIRNAME "wesnoth" CACHE STRING "change the name of the directory for the read-only architecture-independent game data")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}" CACHE STRING "change the dir where binaries are placed right at compile time")
set(LOCALEDIR "translations" CACHE STRING "change the name of the locale data directory to a non-default name")
set(PREFERENCES_DIR "" CACHE STRING "Use a non-default preferences directory (.wesnoth on unix)")
set(DEFAULT_PREFS_FILE "" CACHE STRING "Set system wide preferences file")

#Game options
option(ENABLE_FRIBIDI "Enable FriBIDi support" ON)

#server options
set(SERVER_UID "" CACHE STRING "User id of the user who runs wesnothd")
set(SERVER_GID "" CACHE STRING "Group id of the user who runs wesnothd")
set(FIFO_DIR "/var/run/wesnothd" CACHE STRING "Directory for the wesnothd fifo socket file")

#build options
option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign(add-ons) server")
option(ENABLE_SERVER "Enable compilation of MP server" ON)
option(ENABLE_TESTS "Build unit tests")
option(ENABLE_NLS "Enable building of translations" ENABLE_GAME)
option(ENABLE_OMP "Enables OpenMP, and has additional dependencies" OFF)
option(ENABLE_LIBPNG "Enable support for writing png files (screenshots, images)" ON)
option(ENABLE_HISTORY "Enable using GNU history for history in lua console" ON)

if(NOT CMAKE_CROSSCOMPILING)
	try_run(IEEE754_TEST_RETURN_CODE IEEE754_TEST_COMPILED ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src/compile_time_tests/ieee_754.cpp
		CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=11" COMPILE_OUTPUT_VARIABLE IEEE754_TEST_COMPILE_OUTPUT)
	if(NOT IEEE754_TEST_COMPILED)
		message(WARNING "Failed to compile the IEEE 754 test. Skipping it.")
		message(${IEEE754_TEST_COMPILE_OUTPUT})
	elseif(NOT IEEE754_TEST_RETURN_CODE EQUAL 0)
		message(FATAL_ERROR "Your platform does not represent floating point numbers in the IEEE 754 format.")
	endif()
else()
	message(WARNING "You are cross-compiling. Skipping IEEE 754 test.")
endif()

find_package(SDL2 2.0.4 REQUIRED)
find_package(Crypto 1.0 REQUIRED)
find_package(Boost 1.50 REQUIRED COMPONENTS iostreams program_options regex system thread random)

# no, gettext executables are not required when NLS is deactivated
find_package(Gettext)

find_package(X11)

# needed to get some SDL2 defines in... (as of rev31694 -D_GNU_SOURCE=1 is required!)
set(SDL2_CONFIG "sdl2-config" CACHE STRING "Path to sdl2-config script")
exec_program(${SDL2_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE SDL2_CFLAGS)
add_definitions(${SDL2_CFLAGS})

if(NOT WIN32)
	# Use the safer `mkstemp' instead of `tmpnam' on POSIX systems.
	add_definitions(-DLUA_USE_POSIX)
endif(NOT WIN32)

#check for some compiler/arch specific things and export defines accordingly...
include(SearchForStuff)

# if no build type is specified, it can happen that the game is built without
# optimization (c.f. bug #23445), work around this by enforcing "release" type
# if nothing was selected
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release Profile RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT DEFINED ENABLE_DISPLAY_REVISION)
	# Test whether the code is used in a repository if not autorevision will
	# fail and should be disabled by default. If inside a repository enable
	# the display of revision numbers by default.
	execute_process(
		COMMAND
			${CMAKE_SOURCE_DIR}/utils/autorevision.sh
				-t h
				> ${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
		WORKING_DIRECTORY
			${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE
			 ENABLE_DISPLAY_REVISION_TEST_OUTPUT
		ERROR_VARIABLE
			ENABLE_DISPLAY_REVISION_TEST_ERRNO
	)

	if("${ENABLE_DISPLAY_REVISION_TEST_ERRNO}" STREQUAL "")
		set(DEFAULT_ENABLE_DISPLAY_REVISION true)
	else()
		set(DEFAULT_ENABLE_DISPLAY_REVISION false)
	endif()

	unset(ENABLE_DISPLAY_REVISION_TEST_OUTPUT)
	unset(ENABLE_DISPLAY_REVISION_TEST_ERRNO)

endif()

option(
	ENABLE_DISPLAY_REVISION
	"Enable the display of the revision number in the game, only enable it when in a checkout"
	${DEFAULT_ENABLE_DISPLAY_REVISION}
)


# The use of shared libraries makes compilation debug versions faster but
# results in extra shared libraries. For installation this is not practical
# since the libraries need to be given SONAMES and kept in sync. Therefore
# installation is not supported for this option and will probably fail.
set(ENABLE_SHARED_LIBRARIES OFF CACHE BOOL "Enables shared libraries, this option is meant for development only, installation is *NOT* supported")
mark_as_advanced(on ENABLE_SHARED_LIBRARIES)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_DESKTOP_ENTRY "enable installation of desktop entry files" ON)
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

option(ENABLE_STRICT_COMPILATION "Sets the strict compilation mode" OFF)
option(ENABLE_PEDANTIC_COMPILATION "Sets the pedantic compilation mode" OFF)
option(ENABLE_DEBUG_WINDOW_LAYOUT "Add the debug option to allow the generation of debug layout files in dot format" OFF)
option(ENABLE_DESIGN_DOCUMENTS "Enables the generation of design documents, and has additional dependencies" OFF)
option(ENABLE_LTO "Sets Link Time Optimization for Release builds" OFF)
option(GLIBCXX_DEBUG "Whether to define _GLIBCXX_DEBUG and _GLIBCXX_DEBUG_PEDANTIC" OFF)
option(ENABLE_POT_UPDATE_TARGET "Enables the tools to update the pot files and manuals. This target has extra dependencies." OFF)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_NOTIFICATIONS "Enable Window manager notification messages" ON)
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")

#
# Handle options (set paths/definitions/etc...)
#

### Set the environment compiler flags.

if(CONFIGURED)
	# The CONFIGURED flag was replaced when trunk `was' 1.11, before the release of 1.11.0
	message("Builed files depending on 'CONFIGURED' found, please regenerate your build files.")
	set(CXX_FLAGS_USER
		"${CMAKE_CXX_FLAGS}"
		CACHE
		STRING
		"The CXXFLAGS environment variable used for the initial generation."
		FORCE
	)
	unset(CONFIGURED CACHE)
endif(CONFIGURED)

if(NOT DEFINED CXX_FLAGS_USER)

	message(STATUS "Environment compiler flags set to »${CXX_FLAGS_USER}«")
	set(CXX_FLAGS_USER
		"$ENV{CXXFLAGS}"
		CACHE
		STRING
		"The CXXFLAGS environment variable used for the initial generation."
		FORCE
	)

endif(NOT DEFINED CXX_FLAGS_USER)

set(COMPILER_FLAGS "-std=c++11 -Wall -Wextra -Werror=non-virtual-dtor -Wno-unused-local-typedefs -Wno-maybe-uninitialized -Wold-style-cast")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Qunused-arguments -Wno-unknown-warning-option -Wmismatched-tags -Wno-conditional-uninitialized")
endif()

### Set strict compiler flags.

if(ENABLE_STRICT_COMPILATION)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Werror")
endif(ENABLE_STRICT_COMPILATION)

### Set pedantic compiler flags.

if(ENABLE_PEDANTIC_COMPILATION)

	set(CXX_FLAGS_PEDANTIC_COMPILATION "-Wlogical-op -Wmissing-declarations -Wredundant-decls -Wctor-dtor-privacy -Wdouble-promotion -Wuseless-cast -Wnoexcept")

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CXX_FLAGS_PEDANTIC_COMPILATION "${CXX_FLAGS_PEDANTIC_COMPILATION} -Wdocumentation -Wno-documentation-deprecated-sync")
	endif()
	
	set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_PEDANTIC_COMPILATION}")

endif(ENABLE_PEDANTIC_COMPILATION)

### Set the final compiler flags.

set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_USER}")

if(NOT "${CMAKE_CXX_FLAGS}" STREQUAL "${COMPILER_FLAGS}")
	message(STATUS "CMake compiler flags set to »${COMPILER_FLAGS}«")
	set(CMAKE_CXX_FLAGS
		"${COMPILER_FLAGS}"
		CACHE
		STRING
		"Global flags used by the CXX compiler during all builds."
		FORCE
	)
endif(NOT "${CMAKE_CXX_FLAGS}" STREQUAL "${COMPILER_FLAGS}")

if(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)
	# Assume the compiler is the clang compiler.
	set(CMAKE_EXE_LINKER_FLAGS "-lstdc++ -lm ${CMAKE_EXE_LINKER_FLAGS}")
endif(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)

if(NOT WIN32)
	add_definitions(-DWESNOTH_PATH="${CMAKE_INSTALL_FULL_DATADIR}/${DATADIRNAME}")
endif(NOT WIN32)

if(X11_FOUND)
	add_definitions(-D_X11)
endif(X11_FOUND)

add_definitions(-DLOCALEDIR="${LOCALEDIR}")

# -DNDEBUG is automatically added to all release build types, so manually remove this define from the related variables
RemoveFlag(CMAKE_CXX_FLAGS_RELWITHDEBINFO CACHE "-DNDEBUG" "Default C++ flags for RelWithDebInfo")
RemoveFlag(CMAKE_C_FLAGS_RELWITHDEBINFO CACHE "-DNDEBUG" "Default C flags for RelWithDebInfo")
RemoveFlag(CMAKE_CXX_FLAGS_MINSIZEREL CACHE "-DNDEBUG" "Default C++ flags for MinSizeRel")
RemoveFlag(CMAKE_C_FLAGS_MINSIZEREL CACHE "-DNDEBUG" "Default C flags for MinSizeRel")

# -rdynamic is automatically added, but we don't need it, and it increases the executable size
RemoveFlag(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS SCRIPT "-rdynamic" "")
RemoveFlag(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS SCRIPT "-rdynamic" "")

# #
# Start determining options for Release build
# #

# reset the base Release build option
MESSAGE("Replacing default flags used for Release build with -O3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}" CACHE STRING "Release build flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}" CACHE STRING "Release build flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE)
# set the arch to use for Release build if provided
if(ARCH)
	MESSAGE("adding -march=${ARCH} to Release build")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=${ARCH}" CACHE STRING "Release build flags" FORCE)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=${ARCH}" CACHE STRING "Release build flags" FORCE)
endif(ARCH)
# add pentiumpro arch for windows builds with -O3 if no other arch provided, otherwise resulting executable may not work
if(WIN32 AND NOT ARCH)
	MESSAGE("WIN32 and no arch provided, defaulting to pentiumpro")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=pentiumpro" CACHE STRING "Release build flags" FORCE)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=pentiumpro" CACHE STRING "Release build flags" FORCE)
endif(WIN32 AND NOT ARCH)

# PGO and LTO for GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(PGO_DATA STREQUAL "generate")
		MESSAGE("Generating PGO data")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-generate=${CMAKE_SOURCE_DIR}/pgo_data/" CACHE STRING "Release build flags generating PGO data" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-generate=${CMAKE_SOURCE_DIR}/pgo_data/" CACHE STRING "Release build flags generating PGO data" FORCE)
	endif()
	
	if(PGO_DATA STREQUAL "use")
		MESSAGE("Using PGO data from previous runs")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-correction -fprofile-use=${CMAKE_SOURCE_DIR}/pgo_data/" CACHE STRING "Release build flags for using PGO data" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-correction -fprofile-use=${CMAKE_SOURCE_DIR}/pgo_data/" CACHE STRING "Release build flags for using PGO data" FORCE)
	endif()
	
	if(ENABLE_LTO)
		if(NOT LTO_JOBS)
			MESSAGE("LTO_JOBS not set, defaulting to 1")
			set(LTO_JOBS "1" CACHE STRING "Number of threads to use for LTO with gcc" FORCE)
		endif(NOT LTO_JOBS)
		
		MESSAGE("added -flto=${LTO_JOBS} to Release build")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=${LTO_JOBS}" CACHE STRING "Release build flags with LTO" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto=${LTO_JOBS}" CACHE STRING "Release build flags with LTO" FORCE)
		
		MESSAGE("Using GCC gold linker")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=gold" CACHE STRING "" FORCE)
	endif(ENABLE_LTO)
endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

# PGO and LTO for Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	if(PGO_DATA STREQUAL "generate")
		MESSAGE("Generating PGO data")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-instr-generate=${CMAKE_SOURCE_DIR}/pgo_data/wesnoth-%p.profraw" CACHE STRING "Release build flags generating PGO data" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-instr-generate=${CMAKE_SOURCE_DIR}/pgo_data/wesnoth-%p.profraw" CACHE STRING "Release build flags generating PGO data" FORCE)
	endif()
	
	if(PGO_DATA STREQUAL "use")
		MESSAGE("Using PGO data from previous runs")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-instr-use=${CMAKE_SOURCE_DIR}/pgo_data/wesnoth.profdata" CACHE STRING "Release build flags for using PGO data" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-instr-use=${CMAKE_SOURCE_DIR}/pgo_data/wesnoth.profdata" CACHE STRING "Release build flags for using PGO data" FORCE)
	endif()
	
	if(ENABLE_LTO)
		MESSAGE("added -flto=thin to Release build")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin" CACHE STRING "Release build flags with LTO" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto=thin" CACHE STRING "Release build flags with LTO" FORCE)
		
		MESSAGE("Using Clang LLD linker")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=lld" CACHE STRING "Linker flag for building with LTO and clang" FORCE)
	endif(ENABLE_LTO)
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

# set CMAKE_AR and CMAKE_RANLIB to use LTO-enabled variants if LTO is enabled
if(ENABLE_LTO)
	MESSAGE("Using gcc-ar and gcc-ranlib")
	find_program(LTO_AR NAMES gcc-ar)
	find_program(LTO_RANLIB NAMES gcc-ranlib)
	set(CMAKE_AR "${LTO_AR}" CACHE STRING "Supports LTO" FORCE)
	set(CMAKE_RANLIB "${LTO_RANLIB}" CACHE STRING "Supports LTO" FORCE)
else()
	MESSAGE("Using ar, ranlib, and default linker")
	find_program(NON_LTO_AR NAMES ar)
	find_program(NON_LTO_RANLIB NAMES ranlib)
	set(CMAKE_AR "${NON_LTO_AR}" CACHE STRING "Does not support LTO" FORCE)
	set(CMAKE_RANLIB "${NON_LTO_RANLIB}" CACHE STRING "Does not support LTO" FORCE)
endif()
MARK_AS_ADVANCED(LTO_AR LTO_RANLIB NON_LTO_AR NON_LTO_RANLIB)

# add in extra flags
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_RELEASE}")

# clean the pgo data
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/pgo_data/")

# #
# End determining options for Release build
# Start setting options for Debug build
# #

# replace the default Debug flag of -g with -O0 -DDEBUG -ggdb3
# this matches the flags of scons' debug build
MESSAGE("Replacing flags used for Debug build -O0 -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}" CACHE STRING "change cmake's Debug flags to match scons' flags" FORCE)
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}" CACHE STRING "change cmake's Debug flags to match scons' flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_DEBUG}" CACHE STRING "" FORCE)

# adds GLIBCXX_DEBUG definitions
if(GLIBCXX_DEBUG)
	MESSAGE("Defining _GLIBCXX_DEBUG and _GLIBCXX_DEBUG_PEDANTIC")
	add_definitions(-D_GLIBCXX_DEBUG)
	add_definitions(-D_GLIBCXX_DEBUG_PEDANTIC)
endif(GLIBCXX_DEBUG)

# #
# End setting options for Debug build
# Start setting options for Profile build
# #

if(PROFILER STREQUAL "gprof" OR NOT PROFILER)
	MESSAGE("Profiler is gprof")
	set(CMAKE_CXX_FLAGS_PROFILE "-pg ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gprof" FORCE)
	set(CMAKE_C_FLAGS_PROFILE "-pg ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gprof" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_PROFILE}" CACHE STRING "" FORCE)
endif()

if(PROFILER STREQUAL "gcov")
	MESSAGE("Profiler is gcov")
	set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gcov" FORCE)
	set(CMAKE_C_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gcov" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_PROFILE}" CACHE STRING "" FORCE)
endif()

if(PROFILER STREQUAL "gperftools")
	MESSAGE("Profiler is gperftools")
	set(CMAKE_CXX_FLAGS_PROFILE "${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gperftools" FORCE)
	set(CMAKE_C_FLAGS_PROFILE "${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with gperftools" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-Wl,--no-as-needed,-lprofiler ${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_PROFILE}" CACHE STRING "" FORCE)
endif()

if(PROFILER STREQUAL "perf")
	MESSAGE("Profiler is perf")
	set(CMAKE_CXX_FLAGS_PROFILE "-ggdb -Og ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with perf" FORCE)
	set(CMAKE_C_FLAGS_PROFILE "-ggdb -Og ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_PROFILE}" CACHE STRING "Flags for profiling with perf" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_PROFILE}" CACHE STRING "" FORCE)
endif()

# #
# End setting options for Profile build
# #

# When the path starts with a / on a Unix system it's an absolute path.
# This means that on Windows the path used is always relative.
if(IS_ABSOLUTE "${LOCALEDIR}")
	add_definitions(-DHAS_RELATIVE_LOCALEDIR=0)
	set(LOCALE_INSTALL ${LOCALEDIR})
else()
	add_definitions(-DHAS_RELATIVE_LOCALEDIR=1)
	set(LOCALE_INSTALL ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME}/${LOCALEDIR})
endif()

add_definitions(-DFIFODIR="${FIFO_DIR}")

if(PREFERENCES_DIR)
	add_definitions(-DPREFERENCES_DIR="${PREFERENCES_DIR}")
endif(PREFERENCES_DIR)


if(DEFAULT_PREFS_FILE)
	add_definitions(-DDEFAULT_PREFS_PATH="${DEFAULT_PREFS_FILE}")

	if(NOT DEFAULT_PREFS_FILE MATCHES "^/")
		add_definitions(-DHAS_RELATIVE_DEFPREF)
	endif(NOT DEFAULT_PREFS_FILE MATCHES "^/")
endif(DEFAULT_PREFS_FILE)

if(ENABLE_OMP)
	find_package(OpenMP REQUIRED)
	set(CMAKE_C_FLAGS "${OpenMP_C_FLAGS} ${CMAKE_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${OpenMP_CXX_FLAGS} ${CMAKE_C_FLAGS}")
endif(ENABLE_OMP)

if(ENABLE_DEBUG_WINDOW_LAYOUT)
	add_definitions(-DDEBUG_WINDOW_LAYOUT_GRAPHS)
endif(ENABLE_DEBUG_WINDOW_LAYOUT)

#
# Libraries that are only required by some targets
#

if(ENABLE_GAME OR ENABLE_TESTS)
	find_package( SDL2_image 2.0.0 REQUIRED )
endif(ENABLE_GAME OR ENABLE_TESTS)

if(ENABLE_GAME OR ENABLE_TESTS)
	find_package( SDL2_mixer 2.0.0 REQUIRED )
	find_package( SDL2_ttf 2.0.12 REQUIRED )
	find_package(VorbisFile REQUIRED)
	find_package( PkgConfig REQUIRED )
	pkg_check_modules( CAIRO REQUIRED cairo>=1.10 )
	pkg_check_modules( PANGOCAIRO REQUIRED pangocairo>=1.21.3 )
	pkg_check_modules( FONTCONFIG REQUIRED fontconfig>=2.4.1 )
	pkg_check_modules( SYSTEMD systemd )
endif(ENABLE_GAME OR ENABLE_TESTS)

if(ENABLE_TESTS)
	find_package( Boost 1.36 REQUIRED COMPONENTS unit_test_framework )
endif(ENABLE_TESTS)

if(ENABLE_GAME)
	if(ENABLE_FRIBIDI)
		PKG_CHECK_MODULES(FRIBIDI fribidi>=0.10.9)
		if(FRIBIDI_FOUND)
			add_definitions(-DHAVE_FRIBIDI)
		elseif(NOT FRIBIDI_FOUND)
			message("Could not find FriBiDi. Disabling FriBiDi support.")
		endif(FRIBIDI_FOUND)
	endif(ENABLE_FRIBIDI)

	if(ENABLE_NOTIFICATIONS)
		pkg_check_modules(LIBDBUS dbus-1)
		if(LIBDBUS_FOUND)
			add_definitions(-DHAVE_LIBDBUS)
		else(LIBDBUS_FOUND)
			message("Could not find dbus-1, Disabling notification support.")
		endif(LIBDBUS_FOUND)
	else()
		unset(LIBDBUS_FOUND CACHE)
	endif()

	find_package( PNG )
	if(ENABLE_LIBPNG AND PNG_FOUND)
		add_definitions(-DHAVE_LIBPNG)
	else(ENABLE_LIBPNG AND PNG_FOUND)
		message("Could not find lib PNG. Disabling support for writing PNG images.")	
	endif(ENABLE_LIBPNG AND PNG_FOUND)

	find_package( History )
	if(ENABLE_HISTORY AND HISTORY_FOUND)
		add_definitions(-DHAVE_HISTORY)
	else(ENABLE_HISTORY AND HISTORY_FOUND)
		message("Could not find GNU history. Disabling support for command history in lua console.")
	endif(ENABLE_HISTORY AND HISTORY_FOUND)
endif(ENABLE_GAME)

find_package(Boost 1.50 REQUIRED COMPONENTS filesystem)
find_package(Boost 1.50 REQUIRED COMPONENTS locale)

if(ENABLE_POT_UPDATE_TARGET)
	find_package(TranslationTools REQUIRED)
endif(ENABLE_POT_UPDATE_TARGET)

# get languages
file(READ po/LINGUAS LINGUAS)
string(REPLACE "\n" "" LINGUAS ${LINGUAS})
separate_arguments(LINGUAS)

#
# Include subdirectories
#

add_subdirectory(doc)

if(GETTEXT_FOUND AND ENABLE_NLS)
	add_subdirectory(po)
endif (GETTEXT_FOUND AND ENABLE_NLS)

add_subdirectory(src)

#
# Install files
#
if(ENABLE_GAME)
	install(DIRECTORY data fonts images sounds DESTINATION ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME} USE_SOURCE_PERMISSIONS PATTERN ".git" EXCLUDE )
endif(ENABLE_GAME)

# install file for add-ons server
if(ENABLE_CAMPAIGN_SERVER AND NOT ENABLE_GAME)
	install(FILES data/COPYING.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME}/data)
endif(ENABLE_CAMPAIGN_SERVER AND NOT ENABLE_GAME)

#
# Install desktop file so wesnoth appears in the application start menu with an icon
#
if(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)
	install(FILES packaging/wesnoth.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )
	install(DIRECTORY packaging/icons/hicolor DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons )
endif(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)


if(ENABLE_SERVER AND FIFO_DIR)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{DESTDIR}/${FIFO_DIR})")
	if(SERVER_UID AND SERVER_GID)
		install(CODE "execute_process(COMMAND chown ${SERVER_UID}:${SERVER_GID} \$ENV{DESTDIR}/${FIFO_DIR})")
	endif()
endif()

# Index for checking states of localized images at runtime.
if(ENABLE_GAME)
	install(FILES l10n-track DESTINATION ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME})
endif(ENABLE_GAME)

#
# uninstall
#

configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
)

#
# Packaging stuff
#

include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
