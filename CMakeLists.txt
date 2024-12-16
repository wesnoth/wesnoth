# set minimum version
cmake_minimum_required(VERSION 3.14)

project(wesnoth)

include(CheckCXXCompilerFlag)
include(CTest)

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

#server options
set(SERVER_UID "" CACHE STRING "User id of the user who runs wesnothd")
set(SERVER_GID "" CACHE STRING "Group id of the user who runs wesnothd")
set(FIFO_DIR "/var/run/wesnothd" CACHE STRING "Directory for the wesnothd fifo socket file")

#build options
option(ENABLE_GAME "Enable compilation of the game" ON)
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign(add-ons) server")
option(ENABLE_SERVER "Enable compilation of MP server" ON)
option(ENABLE_MYSQL "Enable building MP/add-ons servers with mysql support" OFF)
option(ENABLE_TESTS "Build unit tests")
option(ENABLE_NLS "Enable building of translations" ${ENABLE_GAME})

set(BOOST_VERSION "1.67")

if(NOT WIN32)
	set(Lua_FIND_VERSION_MAJOR 5)
	set(Lua_FIND_VERSION_MINOR 4)
	option(ENABLE_SYSTEM_LUA "Enable use of system Lua ${Lua_FIND_VERSION_MAJOR}.${Lua_FIND_VERSION_MINOR} (compiled as C++)" OFF)
endif()
if(ENABLE_SYSTEM_LUA)
	set(Lua_FIND_VERSION_EXACT ON)
	set(Lua_FIND_VERSION_COUNT 2)
	include(FindLua)
	if(NOT LUA_FOUND)
		message(FATAL_ERROR "Lua ${Lua_FIND_VERSION_MAJOR}.${Lua_FIND_VERSION_MINOR} C++ not found. Try setting 'LUA_DIR'.")
	endif()
else()
	if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/modules/lua/.git")
		message(FATAL_ERROR "Lua submodule does not exist. You must run 'git submodule update --init --recursive' to initialize it.")
	else()
		message("-- Lua submodule found.")
	endif()
endif()

# set what std version to use
if(NOT CXX_STD)
	set(CXX_STD "17")
endif()
set(CMAKE_CXX_STANDARD ${CXX_STD})
# make sure to force using it
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# forbid defaulting to gnu++NN instead of c++NN
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL 1.0 REQUIRED)

if(APPLE)
	find_library(APPKIT_LIBRARY AppKit REQUIRED)
	find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	find_library(SECURITY_LIBRARY Security REQUIRED)
endif()

find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS iostreams program_options regex system thread random coroutine locale filesystem graph)
find_package(ICU REQUIRED COMPONENTS data i18n uc)

# no, gettext executables are not required when NLS is deactivated
find_package(Gettext)
find_package(Python)

find_package(X11)

if(NOT WIN32 AND NOT ENABLE_SYSTEM_LUA)
	# Use the safer `mkstemp' instead of `tmpnam' on POSIX systems.
	add_definitions(-DLUA_USE_POSIX)
endif()

#check for some compiler/arch specific things and export defines accordingly...
include(SearchForStuff)

# if no build type is specified, it can happen that the game is built without
# optimization (c.f. bug #23445), work around this by enforcing "release" type
# if nothing was selected
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
	message("No build type specified, defaulting to Release")
endif()

if(NOT DEFINED ENABLE_DISPLAY_REVISION)
	# can't run the shell script on windows
	if(NOT WIN32)
		# Test whether the code is used in a repository if not autorevision will
		# fail and should be disabled by default. If inside a repository enable
		# the display of revision numbers by default.
		execute_process(
			COMMAND ${CMAKE_SOURCE_DIR}/utils/autorevision.sh -t h > ${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
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
	else()
		set(DEFAULT_ENABLE_DISPLAY_REVISION false)
	endif()
endif()

option(
	ENABLE_DISPLAY_REVISION
	"Enable the display of the revision number in the game, only enable it when in a checkout"
	${DEFAULT_ENABLE_DISPLAY_REVISION}
)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_DESKTOP_ENTRY "enable installation of desktop entry files" ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	option(ENABLE_APPDATA_FILE "enable installation of an appdata file for appstream" ON)
endif()

option(HARDEN "Whether to enable options to harden the executables" ON)
option(ENABLE_STRICT_COMPILATION "Sets the strict compilation mode" OFF)
option(ENABLE_PEDANTIC_COMPILATION "Sets the pedantic compilation mode" OFF)
option(ENABLE_DEBUG_WINDOW_LAYOUT "Add the debug option to allow the generation of debug layout files in dot format" OFF)
option(ENABLE_DESIGN_DOCUMENTS "Enables the generation of design documents, and has additional dependencies" OFF)
option(ENABLE_LTO "Sets Link Time Optimization for Release builds" OFF)
option(GLIBCXX_ASSERTIONS "Whether to define _GLIBCXX_ASSERTIONS" OFF)
option(GLIBCXX_DEBUG "Whether to define _GLIBCXX_DEBUG and _GLIBCXX_DEBUG_PEDANTIC. Requires a version of Boost's program_options that's compiled with __GLIBCXX_DEBUG too." OFF)
option(ENABLE_POT_UPDATE_TARGET "Enables the tools to update the pot files and manuals. This target has extra dependencies." OFF)
option(FORCE_COLOR_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
option(CLANG_TIDY "Enable clang-tidy linter checks." OFF)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_NOTIFICATIONS "Enable Window manager notification messages" ON)
endif()

set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")

#
# Handle options (set paths/definitions/etc...)
#

### Set the environment compiler flags.

if(NOT MSVC)
	if(NOT DEFINED CXX_FLAGS_USER)

		MESSAGE(STATUS "Environment compiler flags set to »${CXX_FLAGS_USER}«")
		set(CXX_FLAGS_USER
			"$ENV{CXXFLAGS}"
			CACHE
			STRING
			"The CXXFLAGS environment variable used for the initial generation."
			FORCE
		)

	endif()

	set(COMPILER_FLAGS "-Wall -Wextra -Werror=non-virtual-dtor -Wno-unused-local-typedefs -Wno-maybe-uninitialized -Wold-style-cast -Wtrampolines")

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -Qunused-arguments -Wno-unknown-warning-option -Wmismatched-tags -Wno-conditional-uninitialized -Wno-unused-lambda-capture")
	endif()

	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU"
			AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13
			AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
		# GCC-13 added this new warning, and included it in -Wextra,
		# however in GCC-13 it has a lot of false positives.
		#
		# It's likely to generate false postives with GCC-14 too, but
		# I'm using a narrow version check as GCC-14 is still in dev.
		# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110075
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-dangling-reference")
	endif()

### Set strict compiler flags.

	if(ENABLE_STRICT_COMPILATION)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -Werror")
	endif()

### Set pedantic compiler flags.

	if(ENABLE_PEDANTIC_COMPILATION)

		set(CXX_FLAGS_PEDANTIC_COMPILATION "-Wlogical-op -Wmissing-declarations -Wredundant-decls -Wctor-dtor-privacy -Wdouble-promotion -Wuseless-cast -Wnoexcept")

		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			set(CXX_FLAGS_PEDANTIC_COMPILATION "${CXX_FLAGS_PEDANTIC_COMPILATION} -Wdocumentation -Wno-documentation-deprecated-sync")
		endif()

		set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_PEDANTIC_COMPILATION}")

	endif()

# check for sanitizer options
	if(SANITIZE)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fsanitize=${SANITIZE}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${SANITIZE}")
		# manually disable some optimizations to get better stacktraces if sanitizers are used
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
	endif()

### Force colour output (for example for Ninja, or piped CI)

	if(FORCE_COLOR_OUTPUT)
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
			set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdiagnostics-color=always")
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			set(COMPILER_FLAGS "${COMPILER_FLAGS} -fcolor-diagnostics")
		endif()
	endif()

### Enable clang-tidy linting
	if (CLANG_TIDY)
		set(CMAKE_CXX_CLANG_TIDY "clang-tidy -Wno-unknown-warning-option")
	endif()

### Set the final compiler flags.

	set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_USER}")

	if(NOT "${CMAKE_CXX_FLAGS}" STREQUAL "${COMPILER_FLAGS}")
		MESSAGE(STATUS "CMake compiler flags set to »${COMPILER_FLAGS}«")
		set(CMAKE_CXX_FLAGS
			"${COMPILER_FLAGS}"
			CACHE
			STRING
			"Global flags used by the CXX compiler during all builds."
			FORCE
		)
	endif()

# #
# Determine optimization level
# #

	if(NOT OPT)
		if(PROFILER STREQUAL "perf")
			set(CMAKE_CXX_FLAGS_RELEASE "-Og")
			set(CMAKE_C_FLAGS_RELEASE "-Og")
		else()
			set(CMAKE_CXX_FLAGS_RELEASE "-O3")
			set(CMAKE_C_FLAGS_RELEASE "-O3")
		endif()
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${OPT}")
		set(CMAKE_C_FLAGS_RELEASE "${OPT}")

		set(CMAKE_CXX_FLAGS_DEBUG "${OPT}")
		set(CMAKE_C_FLAGS_DEBUG "${OPT}")
	endif()

# check for hardening options
	if(HARDEN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -fstack-protector-strong")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE -fstack-protector-strong")

		if(APPLE)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -Wl,-pie")
		elseif(WIN32 AND MINGW)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")
		else()
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie -Wl,-z,relro,-z,now")
		endif()

		if(NOT CMAKE_CXX_FLAGS_DEBUG STREQUAL "-O0")
			add_definitions(-D_FORTIFY_SOURCE=2)
		endif()
	endif()

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CMAKE_EXE_LINKER_FLAGS "-lstdc++ -lm ${CMAKE_EXE_LINKER_FLAGS}")
	endif()

	add_definitions(-DWESNOTH_PATH="${CMAKE_INSTALL_FULL_DATADIR}/${DATADIRNAME}")

	if(X11_FOUND)
		add_definitions(-D_X11)
	endif()

	add_definitions(-DLOCALEDIR="${LOCALEDIR}")

# -rdynamic is automatically added, but we don't need it, and it increases the executable size
	RemoveFlag(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS SCRIPT "-rdynamic" "")
	RemoveFlag(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS SCRIPT "-rdynamic" "")

# -DNDEBUG is automatically added to all release build types, so manually remove this define from the related variables
	RemoveFlag(CMAKE_CXX_FLAGS_RELWITHDEBINFO CACHE "-DNDEBUG" "Default C++ flags for RelWithDebInfo")
	RemoveFlag(CMAKE_C_FLAGS_RELWITHDEBINFO CACHE "-DNDEBUG" "Default C flags for RelWithDebInfo")
	RemoveFlag(CMAKE_CXX_FLAGS_MINSIZEREL CACHE "-DNDEBUG" "Default C++ flags for MinSizeRel")
	RemoveFlag(CMAKE_C_FLAGS_MINSIZEREL CACHE "-DNDEBUG" "Default C flags for MinSizeRel")

# #
# Start determining options for Release build
# #

# reset the base Release build option
	MESSAGE("Replacing default flags used for Release build with ${OPT} ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}" CACHE STRING "Release build flags" FORCE)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_RELEASE}" CACHE STRING "Release build flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE)
# set the arch to use for Release build if provided
	if(ARCH)
		MESSAGE("adding -march=${ARCH} to Release build")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=${ARCH}" CACHE STRING "Release build flags" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=${ARCH}" CACHE STRING "Release build flags" FORCE)
	endif()

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
			endif()

			MESSAGE("added -flto=${LTO_JOBS} to Release build")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=${LTO_JOBS}" CACHE STRING "Release build flags with LTO" FORCE)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto=${LTO_JOBS}" CACHE STRING "Release build flags with LTO" FORCE)

			MESSAGE("Using GCC gold linker")
			set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=gold -Wno-stringop-overflow" CACHE STRING "" FORCE)
		endif()
	endif()

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
		endif()
	endif()

# set CMAKE_AR and CMAKE_RANLIB to use LTO-enabled variants if LTO is enabled
	if(ENABLE_LTO)
		MESSAGE("Using gcc-ar and gcc-ranlib")
		find_program(LTO_AR NAMES gcc-ar)
		find_program(LTO_RANLIB NAMES gcc-ranlib)
		set(CMAKE_AR "${LTO_AR}" CACHE STRING "Supports LTO" FORCE)
		set(CMAKE_RANLIB "${LTO_RANLIB}" CACHE STRING "Supports LTO" FORCE)
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
	MESSAGE("Replacing flags used for Debug build ${OPT} -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}" CACHE STRING "change cmake's Debug flags to match scons' flags" FORCE)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -ggdb3 ${EXTRA_FLAGS_CONFIG} ${EXTRA_FLAGS_DEBUG}" CACHE STRING "change cmake's Debug flags to match scons' flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LINK_EXTRA_FLAGS_CONFIG} ${LINK_EXTRA_FLAGS_DEBUG}" CACHE STRING "" FORCE)

# Enabling GLIBCXX_ASSERTIONS puts bounds-checks on std::vector::operator[], etc
	if(GLIBCXX_ASSERTIONS)
		MESSAGE("Defining _GLIBCXX_ASSERTIONS")
		add_definitions(-D_GLIBCXX_ASSERTIONS)
	endif()

# GLIBCXX_DEBUG enables more checks that GLIBCXX_ASSERTIONS, but changes the ABI of Boost's program_options library.
# When _GLIBCXX_DEBUG is defined, _GLIBCXX_ASSERTIONS is automatically implied (Gnu's c++config.h will define it).
	if(GLIBCXX_DEBUG)
		MESSAGE("Defining _GLIBCXX_DEBUG and _GLIBCXX_DEBUG_PEDANTIC")
		add_definitions(-D_GLIBCXX_DEBUG)
		add_definitions(-D_GLIBCXX_DEBUG_PEDANTIC)
	endif()

# #
# Setup profiler build options
# #
	set(PROFILER "" CACHE STRING "Enable performance-measuring tools (and choose which tool to use)")

	if(PROFILER STREQUAL "gprof")
		MESSAGE("Profiler is gprof")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pg ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with gprof" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pg ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with gprof" FORCE)
	endif()

	if(PROFILER STREQUAL "gcov")
		MESSAGE("Profiler is gcov")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with gcov" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with gcov" FORCE)
	endif()

	if(PROFILER STREQUAL "gperftools")
		MESSAGE("Profiler is gperftools")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--no-as-needed,-lprofiler ${LINK_EXTRA_FLAGS_CONFIG}" CACHE STRING "" FORCE)
	endif()

	if(PROFILER STREQUAL "perf")
		MESSAGE("Profiler is perf")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ggdb -fno-omit-frame-pointer ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with perf" FORCE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ggdb -fno-omit-frame-pointer ${EXTRA_FLAGS_CONFIG}" CACHE STRING "Flags for profiling with perf" FORCE)
	endif()
# #
# End setting profiler build options
# #
else()
	set(CMAKE_CXX_FLAGS "/W3 /WX /wd4503 /wd4351 /wd4250 /wd4244 /wd4267 /we4239 /wd4275 /EHsc /utf-8 /Zc:__cplusplus" CACHE STRING "Global flags used by the CXX compiler during all builds." FORCE)
	set(CMAKE_C_FLAGS "/WX" CACHE STRING "Global flags used by the C compiler during all builds." FORCE)
	add_definitions(-D_WIN32_WINNT=0x0A00 -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -DNOMINMAX)

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_LUA")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")

# -DNDEBUG is automatically added to all release build types, so manually remove this define from the related variables
	RemoveFlag(CMAKE_CXX_FLAGS_RELEASE CACHE "/DNDEBUG" "Default C++ flags for RELEASE")
	RemoveFlag(CMAKE_C_FLAGS_RELEASE CACHE "/DNDEBUG" "Default C flags for RELEASE")
	RemoveFlag(CMAKE_CXX_FLAGS_RELWITHDEBINFO CACHE "/DNDEBUG" "Default C++ flags for RelWithDebInfo")
	RemoveFlag(CMAKE_C_FLAGS_RELWITHDEBINFO CACHE "/DNDEBUG" "Default C flags for RelWithDebInfo")
	RemoveFlag(CMAKE_CXX_FLAGS_MINSIZEREL CACHE "/DNDEBUG" "Default C++ flags for MinSizeRel")
	RemoveFlag(CMAKE_C_FLAGS_MINSIZEREL CACHE "/DNDEBUG" "Default C flags for MinSizeRel")
endif()

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
endif()


if(DEFAULT_PREFS_FILE)
	add_definitions(-DDEFAULT_PREFS_PATH="${DEFAULT_PREFS_FILE}")

	if(NOT DEFAULT_PREFS_FILE MATCHES "^/")
		add_definitions(-DHAS_RELATIVE_DEFPREF)
	endif()
endif()

if(ENABLE_DEBUG_WINDOW_LAYOUT)
	add_definitions(-DDEBUG_WINDOW_LAYOUT_GRAPHS)
endif()

#
# Libraries that are only required by some targets
#

if(ENABLE_GAME OR ENABLE_TESTS)
	find_package(CURL REQUIRED)
	find_package(VorbisFile REQUIRED)
	find_package(PkgConfig REQUIRED)
	find_package(Fontconfig REQUIRED)
	find_package(SDL2 2.0.18 REQUIRED)
	if(NOT MSVC)
		# for everything else, use pkgconfig
		# SDL2_image and SDL2_mixer don't seem to have any cmake configuration available at all
		pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image>=2.0.2)
		pkg_check_modules(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	else()
		# for MSVC, vcpkg builds and provides custom SDL2-related modules for cmake to use, so use those
		# this also fixes the issue with our previous FindSDL2* scripts incorrectly using the Release version of these libs instead of the Debug version
		find_package(SDL2_image CONFIG REQUIRED)
		find_package(SDL2_mixer CONFIG REQUIRED)
	endif()
	pkg_check_modules(CAIRO REQUIRED cairo>=1.10)
	pkg_check_modules(PANGOCAIRO REQUIRED pangocairo>=1.44.0)
	pkg_check_modules(PANGO REQUIRED pango>=1.44.0)
	pkg_check_modules(LIBREADLINE readline)
endif()

if(ENABLE_TESTS)
	find_package( Boost ${BOOST_VERSION} REQUIRED COMPONENTS unit_test_framework )
endif()

if(ENABLE_GAME)
	if(ENABLE_NOTIFICATIONS)
		pkg_check_modules(LIBDBUS dbus-1)
		if(LIBDBUS_FOUND)
			add_definitions(-DHAVE_LIBDBUS)
		else()
			message("Could not find dbus-1, Disabling notification support.")
		endif()
	else()
		unset(LIBDBUS_FOUND CACHE)
	endif()

	find_package(History)
	if(HISTORY_FOUND)
		add_definitions(-DHAVE_HISTORY)
	endif()
endif()

if(ENABLE_POT_UPDATE_TARGET)
	find_package(TranslationTools REQUIRED)
endif()

# get languages
if(ENABLE_NLS)
	file(READ po/LINGUAS LINGUAS)
	string(REPLACE "\n" "" LINGUAS ${LINGUAS})
	separate_arguments(LINGUAS)
endif()

#
# Include subdirectories
#

add_subdirectory(doc)

if(GETTEXT_FOUND AND Python_FOUND AND ENABLE_NLS)
	add_subdirectory(po)
endif()

add_subdirectory(src)

#
# Install files
#
if(ENABLE_GAME)
	install(DIRECTORY data fonts images sounds DESTINATION ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME} USE_SOURCE_PERMISSIONS PATTERN ".git" EXCLUDE )
endif()

# install file for add-ons server
if(ENABLE_CAMPAIGN_SERVER AND NOT ENABLE_GAME)
	install(FILES data/COPYING.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/${DATADIRNAME}/data)
endif()

#
# Install desktop file so wesnoth appears in the application start menu with an icon
#
if(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)
	install(FILES packaging/org.wesnoth.Wesnoth.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )
	install(DIRECTORY packaging/icons/hicolor packaging/icons/HighContrast DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons )
endif()

if(ENABLE_APPDATA_FILE AND ENABLE_GAME)
	install(FILES packaging/org.wesnoth.Wesnoth.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo )
endif()

if(ENABLE_SERVER AND FIFO_DIR)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{DESTDIR}/${FIFO_DIR})")
	if(SERVER_UID AND SERVER_GID)
		install(CODE "execute_process(COMMAND chown ${SERVER_UID}:${SERVER_GID} \$ENV{DESTDIR}/${FIFO_DIR})")
	endif()
endif()

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
