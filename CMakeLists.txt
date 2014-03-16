# most stuff should be implemented so far, what is not working so far:
# * some other smaller stuff
# * nice INSTALL howto
# * Detect Python and install Python tools
# * Rewrite the po4a-update
# * Test everything
# * install .desktop files taking binary prefix and suffix into account

# set minimum version
cmake_minimum_required(VERSION 2.6.0)

include(CheckCXXCompilerFlag)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# use our own version of FindBoost.cmake and other Find* scripts
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDL 1.2.7 REQUIRED)
find_package(Boost 1.36 REQUIRED COMPONENTS iostreams program_options regex system)

# no, gettext executables are not required when NLS is deactivated
find_package(Gettext)
# yes, libintl is *required* even when NLS is deactivated (this is to compile
# src/gettext.cpp since it includes libintl.h)
find_package(Libintl REQUIRED)

find_package(X11)

if(NOT MSVC)
	#needed to get some SDL defines in... (as of rev31694 -D_GNU_SOURCE=1 is required!)
	set(SDL_CONFIG "sdl-config" CACHE STRING "Path to sdl-config script")
	exec_program(${SDL_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE SDL_CFLAGS)
	add_definitions(${SDL_CFLAGS})

	# Use the safer `mkstemp' instead of `tmpnam' on POSIX systems.
	add_definitions(-DLUA_USE_POSIX)
endif(NOT MSVC)

if(MSVC)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DNOMINMAX)
endif(MSVC)

#check for some compiler/arch specific things and export defines accordingly...
include(SearchForStuff)

#
# Options
#

#Path options
set(BINDIR "bin" CACHE STRING "Where to install binaries")
set(DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Sets the root of data directories to a non-default location")
set(DOCDIR "${DATAROOTDIR}/doc/wesnoth" CACHE STRING "Sets the doc directory to a non-default location.")
set(MANDIR "${DATAROOTDIR}/man" CACHE STRING "Where to install manpages")
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
option(ENABLE_CAMPAIGN_SERVER "Enable compilation of campaign server")
option(ENABLE_SERVER "Enable compilation of server" ON)
option(ENABLE_TOOLS "Enable building and installation of tools for artists and WML maintainers")
option(ENABLE_SDL2_TOOLS "Enable building and installation of tools for testing with SDL2" OFF)
option(ENABLE_TESTS "Build unit tests")
option(ENABLE_NLS "Enable building of translations" ON)
option(ENABLE_LOW_MEM "Reduce memory usage by removing extra functionality" OFF)
option(ENABLE_OMP "Enables OpenMP, and has additional dependencies" OFF)
option(ENABLE_PANDORA "Add support for the OpenPandora by deactivating libvorbis support" OFF)

if(NOT DEFINED ENABLE_DISPLAY_REVISION)
	# Test whether the code is used in a repository if not autorevision will
	# fail and should be disabled by default. If inside a repository enable
	# the display of revision numbers by default.
	execute_process(
		COMMAND
			${CMAKE_SOURCE_DIR}/utils/autorevision
				-th
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
	set(ICONDIR "${DATAROOTDIR}/pixmaps" CACHE STRING "Sets the icon directory for desktop entry to a non-default location.")
	set(DESKTOPDIR "${DATAROOTDIR}/applications/" CACHE STRING "Sets the desktop file directory for desktop entry to a non-default location.")
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

option(ENABLE_STRICT_COMPILATION "Sets the strict compilation mode" ON)
option(ENABLE_PEDANTIC_COMPILATION "Sets the pedantic compilation mode" OFF)
option(ENABLE_DEBUG_WINDOW_LAYOUT "Add the debug option to allow the generation of debug layout files in dot format" OFF)
option ( USE_ANA_NETWORK "Compile network module using ana" OFF )
option(ENABLE_DESIGN_DOCUMENTS "Enables the generation of design documents, and has additional dependencies" OFF)

#misc options
if(NOT MSVC)
	# Pot updates don't work at Windows so no reason to even try
	option(ENABLE_POT_UPDATE_TARGET "Enables the tools to update the pot files and manuals. This target has extra dependencies." OFF)
endif(NOT MSVC)

if(UNIX AND NOT APPLE AND NOT CYGWIN)
	option(ENABLE_NOTIFICATIONS "Enable Window manager notification messages" ON)
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

set(BINARY_SUFFIX "" CACHE STRING "Suffix behind all binaries")
set(BINARY_PREFIX "" CACHE STRING "Prefix in front of all binaries")

#
# Handle options (set paths/definitions/etc...)
#

##### Set the compiler flags.

# This macro checks whether a compiler supports a compiler flag.
#
# If the flag is supported the flag will be added to the target compiler flags.
# GCC seems to be quite happy to accept flags it does not support when there is
# a `no' in it e.g. -Wno-not_supported_flag but will fail to compile with
# -Wnot_supported_flag. For that case all not-named parameters will be added to
# the target instead.
#
# param target                    The variable to add the compiler flag to.
# param flag                      The compiler flag to test.
# param variable                  The test macro needs a variable to store the
#                                 result of the test, this paramter holds that
#                                 variable.
# param ...                       If this variable is set it will be added to
#                                 target instead of flag when the compiler
#                                 supports flag.
macro(check_compiler_has_flag target flag variable)
	check_cxx_compiler_flag(${flag} ${variable})
	if(${variable})
		if(${ARGC} GREATER 3)
			set(${target} "${${target}} ${ARGN}")
		else(${ARGC} GREATER 3)
			set(${target} "${${target}} ${flag}")
		endif(${ARGC} GREATER 3)
	endif(${variable})
endmacro(check_compiler_has_flag)


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


### Set default Wesnoth project compiler flags
if(MSVC AND NOT DEFINED CXX_FLAGS_MSVC)
	# MSVC sets some initial compilation flags, these need to
	# be preserved.
	set(CXX_FLAGS_MSVC
		"${CMAKE_CXX_FLAGS_INIT}"
		CACHE
		STRING
		"The initial CXX flags for MSVC"
		FORCE
	)
endif(MSVC AND NOT DEFINED CXX_FLAGS_MSVC)

set(CXX_FLAGS_PROJECT)
check_compiler_has_flag(CXX_FLAGS_PROJECT "-std=c++98" HAS_COMPILER_FLAG_STD)
check_compiler_has_flag(CXX_FLAGS_PROJECT "-W" HAS_COMPILER_FLAG_W)
# MSVC's -Wall is not like gcc's, it really enables *all* warnings which include zillions for system headers and doesn't make sense.
if(NOT MSVC)
check_compiler_has_flag(CXX_FLAGS_PROJECT "-Wall" HAS_COMPILER_FLAG_WALL)
endif(NOT MSVC)


### Set strict compiler flags.

set(CXX_FLAGS_STRICT_COMPILATION)
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Werror"
	HAS_COMPILER_FLAG_WERROR
)

# The current unit test code breaks strict aliasing with g++ 4.4.
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wstrict-aliasing"
	HAS_COMPILER_FLAG_WERROR_STRICT_ALIASING
	"-Wno-strict-aliasing"
)

# This flag is/will be added in gcc-4.8 and fails with BOOST_STATIC_ASSERT
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wunused-local-typedefs"
	HAS_COMPILER_FLAG_WUNUSED_LOCAL_TYPEDEFS
	"-Wno-unused-local-typedefs"
)

# This flag is/will be added in gcc-4.8 and fails with png in C++11 mode
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wliteral-suffix"
	HAS_COMPILER_FLAG_WLITERAL_SUFFIX
	"-Wno-literal-suffix"
)

# This removes a lot of warnings from Clang regarding unused -I arguments
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Qunused-arguments"
	HAS_COMPILER_FLAG_QUNUSED_ARGUMENTS
)

# Silences Clang warnings about declaring a class a class first and
# a struct later.
check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wmismatched-tags"
	HAS_COMPILER_FLAG_WMISMATCHED_TAGS
	"-Wno-mismatched-tags"
)

check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wnull-conversion"
	HAS_COMPILER_FLAG_WNULL_CONVERSION
	"-Wno-null-conversion"
)

if(NOT CMAKE_COMPILER_IS_GNUCXX)
	# Silences warnings about overloaded virtuals.
	# (GCC doesn't complain Clang 3.2 does. Clang 3.4 no longer does.)
	check_compiler_has_flag(
		CXX_FLAGS_STRICT_COMPILATION
		"-Woverloaded-virtual"
		HAS_COMPILER_FLAG_WOVERLOADED_VIRTUAL
		"-Wno-overloaded-virtual"
	)
endif(NOT CMAKE_COMPILER_IS_GNUCXX)

check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wold-style-cast"
	HAS_COMPILER_FLAG_WOLD_STYLE_CAST
)

check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wdeprecated-register"
	HAS_COMPILER_FLAG_WDEPRECATED_REGISTER
	"-Wno-deprecated-register"
)


### Set pedantic compiler flags.

set(CXX_FLAGS_PEDANTIC_COMPILATION)
check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wextra"
	HAS_COMPILER_FLAG_WEXTRA
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Winit-self"
	HAS_COMPILER_FLAG_WINIT_SELF
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wlogical-op"
	HAS_COMPILER_FLAG_WLOGICAL_OP
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wmissing-declarations"
	HAS_COMPILER_FLAG_WMISSING_DECLARATIONS
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wredundant-decls"
	HAS_COMPILER_FLAG_WREDUNDANT_DECLS
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wctor-dtor-privacy"
	HAS_COMPILER_FLAG_WCTOR_DTOR_PRIVACY
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wnon-virtual-dtor"
	HAS_COMPILER_FLAG_WNON_VIRTUAL_DTOR
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wdouble-promotion"
	HAS_COMPILER_FLAG_WDOUBLE_PROMOTION
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wmismatched-tags"
	HAS_COMPILER_FLAG_WMISMATCHED_TAGS
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wuseless-cast"
	HAS_COMPILER_FLAG_WUSELESS_CAST
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wc++11-compat"
	HAS_COMPILER_FLAG_WCXX_11_COMPAT
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wnoexcept"
	HAS_COMPILER_FLAG_WNOEXCEPT
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wdocumentation"
	HAS_COMPILER_FLAG_WDOCUMENTATION
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wdocumentation-deprecated-sync"
	HAS_COMPILER_FLAG_WDOCUMENTATION
	"-Wno-documentation-deprecated-sync"
)

check_compiler_has_flag(
	CXX_FLAGS_PEDANTIC_COMPILATION
	"-Wextra-semi"
	HAS_COMPILER_FLAG_WEXTRA_SEMI
)

check_compiler_has_flag(
	CXX_FLAGS_STRICT_COMPILATION
	"-Wconditional-uninitialized"
	HAS_COMPILER_FLAG_WCONDITIONAL_INITIALIZED
)

### Set the final compiler flags.

set(COMPILER_FLAGS "${CXX_FLAGS_PROJECT}")
if(MSVC)
	set(COMPILER_FLAGS "${CXX_FLAGS_MSVC} ${COMPILER_FLAGS}")
endif(MSVC)
if(ENABLE_STRICT_COMPILATION)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_STRICT_COMPILATION}")
endif(ENABLE_STRICT_COMPILATION)
if(ENABLE_PEDANTIC_COMPILATION)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CXX_FLAGS_PEDANTIC_COMPILATION}")
endif(ENABLE_PEDANTIC_COMPILATION)
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

# compose datadir path of datarootdir and datadirname
set(DATADIR ${DATAROOTDIR}/${DATADIRNAME})

if(NOT WIN32)
	add_definitions(-DWESNOTH_PATH=\\\"${DATADIR}\\\")
endif(NOT WIN32)

if(X11_FOUND)
	add_definitions(-D_X11)
endif(X11_FOUND)

add_definitions(-DLOCALEDIR=\\\"${LOCALEDIR}\\\")

# -NDEBUG is automatically added to all release build types, so manually remove
# this define from the related variables
MESSAGE ("removing NDEBUG flag from CMAKE_CXX_FLAGS_RELEASE")
separate_arguments(CMAKE_CXX_FLAGS_RELEASE)
list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "removed NDEBUG flag" FORCE)
MESSAGE ("removing NDEBUG flag from CMAKE_C_FLAGS_RELEASE")
separate_arguments(CMAKE_C_FLAGS_RELEASE)
list(REMOVE_ITEM CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
string(REPLACE ";" " " CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "removed NDEBUG flag" FORCE)
MESSAGE ("removing NDEBUG flag from CMAKE_CXX_FLAGS_RELWITHDEBINFO")
separate_arguments(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DNDEBUG")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "removed NDEBUG flag" FORCE)
MESSAGE ("removing NDEBUG flag from CMAKE_C_FLAGS_RELWITHDEBINFO")
separate_arguments(CMAKE_C_FLAGS_RELWITHDEBINFO)
list(REMOVE_ITEM CMAKE_C_FLAGS_RELWITHDEBINFO "-DNDEBUG")
string(REPLACE ";" " " CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING "removed NDEBUG flag" FORCE)
MESSAGE ("removing NDEBUG flag from CMAKE_CXX_FLAGS_MINSIZEREL")
separate_arguments(CMAKE_CXX_FLAGS_MINSIZEREL)
list(REMOVE_ITEM CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}" CACHE STRING "removed NDEBUG flag" FORCE)
MESSAGE ("removing NDEBUG flag from CMAKE_C_FLAGS_MINSIZEREL")
separate_arguments(CMAKE_C_FLAGS_MINSIZEREL)
list(REMOVE_ITEM CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG")
string(REPLACE ";" " " CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}" CACHE STRING "removed NDEBUG flag" FORCE)


# When the path starts with a / on a Unix system it's an absolute path.
# This means that on Windows the path used is always relative.
if(LOCALEDIR MATCHES "^/")
	add_definitions(-DHAS_RELATIVE_LOCALEDIR=0)
	set(LOCALE_INSTALL ${LOCALEDIR})
else(LOCALEDIR MATCHES "^/")
	add_definitions(-DHAS_RELATIVE_LOCALEDIR=1)
	set(LOCALE_INSTALL ${DATADIR}/${LOCALEDIR})
endif(LOCALEDIR MATCHES "^/")

add_definitions(-DFIFODIR=\\\"${FIFO_DIR}\\\")

if(PREFERENCES_DIR)
	add_definitions(-DPREFERENCES_DIR=\\\"${PREFERENCES_DIR}\\\")
endif(PREFERENCES_DIR)


if(DEFAULT_PREFS_FILE)
	add_definitions(-DDEFAULT_PREFS_PATH=\\\"${DEFAULT_PREFS_FILE}\\\")

	if(NOT DEFAULT_PREFS_FILE MATCHES "^/")
		add_definitions(-DHAS_RELATIVE_DEFPREF)
	endif(NOT DEFAULT_PREFS_FILE MATCHES "^/")
endif(DEFAULT_PREFS_FILE)

if(ENABLE_LOW_MEM)
	add_definitions(-DLOW_MEM)
endif(ENABLE_LOW_MEM)

if(ENABLE_PANDORA)
	add_definitions(-DPANDORA)
endif(ENABLE_PANDORA)

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

if(ENABLE_TOOLS OR ENABLE_GAME OR ENABLE_TESTS)
	find_package( SDL_image 1.2 REQUIRED )
endif(ENABLE_TOOLS OR ENABLE_GAME OR ENABLE_TESTS)
if(ENABLE_GAME OR ENABLE_TESTS)
	find_package( SDL_mixer 1.2 REQUIRED )
	find_package( SDL_ttf 2.0.8 REQUIRED )

	if(NOT MSVC)
		find_package(VorbisFile REQUIRED)
		find_package( PkgConfig REQUIRED )
		pkg_check_modules( PANGOCAIRO REQUIRED pangocairo>=1.21.3 )
		pkg_check_modules( FONTCONFIG REQUIRED fontconfig>=2.4.1 )
        pkg_check_modules( SYSTEMD systemd )
	endif(NOT MSVC)

endif(ENABLE_GAME OR ENABLE_TESTS)
if(ENABLE_GAME OR ENABLE_SERVER OR ENABLE_CAMPAIGN_SERVER OR ENABLE_TESTS)
	find_package( SDL_net REQUIRED )
endif(ENABLE_GAME OR ENABLE_SERVER OR ENABLE_CAMPAIGN_SERVER OR ENABLE_TESTS)
if(ENABLE_TOOLS)
	find_package( ZLIB REQUIRED )
	find_package( PNG REQUIRED )
endif(ENABLE_TOOLS)
if(ENABLE_TESTS)
	find_package( Boost 1.36 REQUIRED COMPONENTS unit_test_framework )
endif(ENABLE_TESTS)
if(ENABLE_GAME)
	find_package( FriBiDi )
	if(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)
		add_definitions(-DHAVE_FRIBIDI)
	elseif(ENABLE_FRIBIDI AND NOT FRIBIDI_LIBRARIES)
		message("Could not find FriBiDi. Disabling FriBiDi support.")
	endif()

	if(ENABLE_NOTIFICATIONS)
		pkg_check_modules(LIBDBUS dbus-1)
		if(LIBDBUS_FOUND)
			add_definitions(-DHAVE_LIBDBUS)
		else(LIBDBUS_FOUND)
			message("Could not find dbus-1, Disabling notification support.")
		endif(LIBDBUS_FOUND)
	endif(ENABLE_NOTIFICATIONS)

endif(ENABLE_GAME)

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
install(DIRECTORY data fonts images sounds DESTINATION ${DATADIR} USE_SOURCE_PERMISSIONS PATTERN ".git" EXCLUDE )


#
# Install desktop file so wesnoth appears in the application start menu with an icon
# TODO: string replacement to have the real binary name in does not work atm, because in the Categories= block semicolons are lost...
#
if(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)
	# do some crude string replacing to have the real binary name in the .desktop file (read in original .desktop file, replace the Exec= line with the correct value and output the generated file)
	#  file(READ icons/wesnoth.desktop wesnoth-desktop-orig)
	#string(REGEX REPLACE "(\nName.*=.*)\n" "\\1 (${BINARY_SUFFIX})\n" wesnoth-desktop-modified ${wesnoth-desktop-orig} )
	#  string(REPLACE "Exec=wesnoth" "Exec=${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}" wesnoth-desktop-modified ${wesnoth-desktop-orig} )
	#  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop ${wesnoth-desktop-modified} )

	#execute_process(COMMAND sed "-i" "'s/^\(Name.*=.*\)$/\1TEST/g'" ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop )
	#exec_program(sed ARGS "-i" "'s/^\(Name.*=.*\)$/\1TEST/g'" ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop )
	# install the generated .desktop file
	#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}.desktop DESTINATION ${DESKTOPDIR} )
	install(FILES icons/wesnoth.desktop DESTINATION ${DESKTOPDIR} )
	install(FILES icons/wesnoth-icon.png DESTINATION ${ICONDIR} )

	# do some crude string replacing to have the real binary name in the .desktop file (read in original .desktop file, replace the Exec= line with the correct value and output the generated file)
	#    file(READ icons/wesnoth_editor.desktop wesnoth-editor-desktop-orig)
	#    string(REPLACE "Exec=wesnoth -e" "Exec=${BINARY_PREFIX}wesnoth${BINARY_SUFFIX} -e" wesnoth-editor-desktop-modified ${wesnoth-editor-desktop-orig} )
	#    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX}.desktop ${wesnoth-editor-desktop-modified} )
	# install the generated .desktop file
	#    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX}.desktop DESTINATION ${DESKTOPDIR} )
	install(FILES icons/wesnoth_editor.desktop DESTINATION ${DESKTOPDIR} )
	install(FILES icons/wesnoth_editor-icon.png DESTINATION ${ICONDIR} )
endif(ENABLE_DESKTOP_ENTRY AND ENABLE_GAME)


if(ENABLE_SERVER AND FIFO_DIR)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{DESTDIR}/${FIFO_DIR})")
    # install systemd stuff if it is installed
    if(SYSTEMD_FOUND)
#        configure_file(packaging/systemd/wesnothd.tmpfiles.conf.in ${CMAKE_BINARY_DIR}/wesnothd.conf)
#        configure_file(packaging/systemd/wesnothd.service.in ${CMAKE_BINARY_DIR}/wesnothd.service)
#        install(FILES ${CMAKE_BINARY_DIR}/wesnothd.conf DESTINATION lib/tmpfiles.d)
#       install(FILES ${CMAKE_BINARY_DIR}/wesnothd.service DESTINATION lib/systemd/system)
    endif()
	if(SERVER_UID AND SERVER_GID)
		install(CODE "execute_process(COMMAND chown ${SERVER_UID}:${SERVER_GID} \$ENV{DESTDIR}/${FIFO_DIR})")
	endif()
endif()

# Index for checking states of localized images at runtime.
install(FILES l10n-track DESTINATION ${DATADIR})

#
# uninstall
#

configure_file(
	"${CMAKE_MODULE_PATH}/uninstall.cmake.in"
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
