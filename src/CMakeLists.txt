
set(CXX_FLAGS_PROJECT "${CXX_FLAGS_PROJECT} -include boost-patched/bind/arg.hpp")

# store the specified sources list in the specified variable
function(GetSources source_list store_in_var)
	file(STRINGS "../source_lists/${source_list}" sources)
	set(${store_in_var} ${sources} PARENT_SCOPE)
endfunction()

## some includes ##
include_directories(SYSTEM ${FONTCONFIG_INCLUDE_DIRS} )
include_directories(SYSTEM ${PANGOCAIRO_INCLUDE_DIRS} )
include_directories(SYSTEM ${GETTEXT_INCLUDE_DIR} )
include_directories(SYSTEM ${LIBDBUS_INCLUDE_DIRS} )
include_directories(SYSTEM ${VORBISFILE_INCLUDE_DIR} )


include_directories(SYSTEM ${SDL2_INCLUDE_DIR} )
set(sdl-lib ${SDL2_LIBRARY})
#optional dependencies
if(SDL2IMAGE_INCLUDE_DIR)
	include_directories(SYSTEM ${SDL2IMAGE_INCLUDE_DIR} )
	set(sdl_image-lib ${SDL2_IMAGE_LIBRARY})
endif()
if(SDL2MIXER_INCLUDE_DIR)
	include_directories(SYSTEM ${SDL2MIXER_INCLUDE_DIR} )
	set(sdl_mixer-lib ${SDL2_MIXER_LIBRARY})
endif()
if(SDL2TTF_INCLUDE_DIR)
	include_directories(SYSTEM ${SDL2TTF_INCLUDE_DIR} )
	set(sdl_ttf-lib ${SDL2_TTF_LIBRARY})
endif()

if(ZLIB_INCLUDE_DIR)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIR} )
endif()

# needed to get include paths in the subfolders correct
include_directories( ${CMAKE_SOURCE_DIR}/src/ )
# needed to have the generated config.h used, too
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

link_directories(
	${Boost_LIBRARY_DIRS}
	${PANGOCAIRO_LIBRARY_DIRS}
	${FONTCONFIG_LIBRARY_DIRS}
	${LIBDBUS_LIBRARY_DIRS}
)

set(common-external-libs
	${sdl-lib}
	${Boost_IOSTREAMS_LIBRARY}
	${Boost_REGEX_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

set(common-external-libs
	${common-external-libs}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_LOCALE_LIBRARY}
)

set(game-external-libs
	${common-external-libs}
	${CRYPTO_LIBRARY}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_RANDOM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${sdl_image-lib}
	${sdl_mixer-lib}
	${sdl_ttf-lib}
	${PANGOCAIRO_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
	${LIBDBUS_LIBRARIES}
	${VORBISFILE_LIBRARIES}
)

if(ENABLE_LIBPNG AND PNG_FOUND)
	set(game-external-libs
		${game-external-libs}
		${PNG_LIBRARIES}
	)
endif(ENABLE_LIBPNG AND PNG_FOUND)

if(ENABLE_HISTORY AND HISTORY_FOUND)
	set(game-external-libs
		${game-external-libs}
		${HISTORY_LIBRARY}
	)
endif(ENABLE_HISTORY AND HISTORY_FOUND)

set(server-external-libs
	${common-external-libs}
	${Boost_SYSTEM_LIBRARIES}
)

set(tools-external-libs
	${common-external-libs}
	${sdl_image-lib}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_RANDOM_LIBRARY}
)

if(ENABLE_FRIBIDI AND FRIBIDI_FOUND)
	set(game-external-libs ${game-external-libs} ${FRIBIDI_LIBRARIES})
	include_directories(SYSTEM ${FRIBIDI_INCLUDE_DIRS} )
endif(ENABLE_FRIBIDI AND FRIBIDI_FOUND)

if(X11_FOUND)
	include_directories(SYSTEM ${X11_INCLUDE_DIR} )
	set(game-external-libs ${game-external-libs} ${X11_LIBRARIES})
endif(X11_FOUND)

########### Set Library mode ###############

if(ENABLE_SHARED_LIBRARIES)
	set(LIBRARY_TYPE SHARED)
else(ENABLE_SHARED_LIBRARIES)
	set(LIBRARY_TYPE STATIC)
endif(ENABLE_SHARED_LIBRARIES)


########### libwesnoth-core ###############

GetSources("libwesnoth_core" libwesnoth-core_STAT_SRC)

if(WIN32)
	set(libwesnoth-core_STAT_SRC
		${libwesnoth-core_STAT_SRC}
		log_windows.cpp
	)
endif()

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-core ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${libwesnoth-core_STAT_SRC})
target_link_libraries(wesnoth-core ${Boost_FILESYSTEM_LIBRARY})

# Depending on the flag create a real file or an empty dummy.
#
# Since the code is executed every build run of Wesnoth we need to make sure
# that no modifications don't turn into a rebuild. Therefore a the dummy is
# created and the second target only copies the file if modified.
if(ENABLE_DISPLAY_REVISION)
	add_custom_target(wesnoth-revision_dummy
		COMMAND
			${CMAKE_SOURCE_DIR}/utils/autorevision.sh
				-t h
				> ${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
		WORKING_DIRECTORY
			${CMAKE_SOURCE_DIR}
	)

	add_custom_target(wesnoth-revision
		COMMAND
			${CMAKE_COMMAND} -E
				copy_if_different
					${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
					${CMAKE_CURRENT_BINARY_DIR}/revision.h
	)

	add_dependencies(wesnoth-revision wesnoth-revision_dummy)
	add_dependencies(wesnoth-core wesnoth-revision)
	set_source_files_properties(
		game_config.cpp
		PROPERTIES
			COMPILE_DEFINITIONS "LOAD_REVISION"
	)
endif()


########### lua ###############

GetSources("lua" lua_STAT_SRC)

# We explicitly want lua compiled as C++ version, so this line is required:
set_source_files_properties(${lua_STAT_SRC} PROPERTIES LANGUAGE CXX)

# Inject a header into the Lua sources for Wesnoth-specific changes
# makedepend won't see it so we have to specifically add it as a dependency.
file(GLOB wesnoth_lua_config wesnoth_lua_config.h)
set_source_files_properties(${lua_STAT_SRC} PROPERTIES COMPILE_FLAGS "-include ${wesnoth_lua_config}")
set_source_files_properties(${lua_STAT_SRC} PROPERTIES OBJECT_DEPENDS ${wesnoth_lua_config})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
# silence a Clang specific warning due to extra parenthesis in if statements when comparing instead
	set_property(SOURCE
		SOURCE ${lua_STAT_SRC}
		APPEND_STRING PROPERTY COMPILE_FLAGS
			" -Wno-parentheses-equality"
	)
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(lua ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${lua_STAT_SRC})


########### Old style cast flags ###############

# Disable the setting of -Wold-style-cast and Wuseless-cast on some targets.
# old style casts are not wanted by our coding style but some C based code
# uses it. Force the flag off for these files.
set_target_properties(lua
	PROPERTIES
	COMPILE_FLAGS
	"-Wno-old-style-cast -Wno-useless-cast"
)


########### Helper libraries ###############

GetSources("libwesnoth_sdl" wesnoth-sdl_SRC)

# If requested, compile and link this file also to be able to save png files
if(ENABLE_LIBPNG AND PNG_FOUND)
	set(wesnoth-sdl_SRC
		${wesnoth-sdl_SRC}
		SDL_SavePNG/savepng.c
	)
endif(ENABLE_LIBPNG AND PNG_FOUND)


add_library(wesnoth-sdl
	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-sdl_SRC}
)

target_link_libraries(wesnoth-sdl
	${sdl-lib}
	${sdl_image-lib}
)

GetSources("libwesnoth_widgets" wesnoth-widgets_SRC)

add_library(wesnoth-widgets
	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-widgets_SRC}
)

if (APPLE)
	set(WIDGETS_LIB -Wl,-force_load wesnoth-widgets)
else()
	set(WIDGETS_LIB -Wl,--whole-archive wesnoth-widgets -Wl,--no-whole-archive)
endif()

########### Wesnoth main source files ###############

# create libwesnoth-client.a if building the client or the boost unit tests

if(ENABLE_GAME OR ENABLE_TESTS)
	GetSources("wesnoth" wesnoth-main_SRC)
	add_library(wesnoth-client ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${wesnoth-main_SRC})
endif(ENABLE_GAME OR ENABLE_TESTS)

########### libwesnoth-game ###############

GetSources("libwesnoth" libwesnoth-game_STAT_SRC)

# On windows only, this file needs to be linked, as its header is included #ifdef win32.
if(WIN32)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		desktop/windows_tray_notification.cpp
	)
endif(WIN32)

# On apple only
if(APPLE)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		desktop/apple_notification.mm
	)
endif(APPLE)

# For libdbus (essentially just for linux), this file needs to be linked, as its header is included #ifdef HAVE_LIBDBUS
if(LIBDBUS_FOUND)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		desktop/dbus_notification.cpp
	)
endif(LIBDBUS_FOUND)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-game ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${libwesnoth-game_STAT_SRC})

########### Wesnoth ###############

if(ENABLE_GAME)
	add_executable(wesnoth WIN32 wesnoth.cpp)

	target_link_libraries(wesnoth
		wesnoth-client
		lua
		wesnoth-core
		wesnoth-game
		wesnoth-sdl
		${WIDGETS_LIB}
		wesnoth-client
		${game-external-libs}
	)
	set_target_properties(wesnoth
		PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}
	)

	install(TARGETS wesnoth DESTINATION ${CMAKE_INSTALL_BINDIR})
endif(ENABLE_GAME)

########### Wesnothd Server ###############

if(ENABLE_SERVER)

	GetSources("wesnothd" wesnothd_SRC)

	add_executable(wesnothd WIN32 ${wesnothd_SRC})

	target_link_libraries(wesnothd wesnoth-core ${server-external-libs} ${Boost_RANDOM_LIBRARY})
	
	set_target_properties(wesnothd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX})

	install(TARGETS wesnothd DESTINATION ${CMAKE_INSTALL_BINDIR})

endif(ENABLE_SERVER)

########### Campaign Server ###############

if(ENABLE_CAMPAIGN_SERVER)

	GetSources("campaignd" campaignd_SRC)

	add_executable(campaignd ${campaignd_SRC})
	
	target_link_libraries(campaignd
			wesnoth-core
			${server-external-libs}
			${CRYPTO_LIBRARY}
			${Boost_RANDOM_LIBRARY}
		)
	set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})

	install(TARGETS campaignd DESTINATION ${CMAKE_INSTALL_BINDIR})

endif(ENABLE_CAMPAIGN_SERVER)

########### Unit tests ###############

if(ENABLE_TESTS)

	add_definitions(-DBOOST_TEST_DYN_LINK)
	
	GetSources("boost_unit_tests" test_SRC)

	set_source_files_properties(
		tests/test_util.cpp
		PROPERTIES
		COMPILE_FLAGS
		"${CXX_FLAG_NO_USELESS_CAST}"
	)

	add_executable(boost_unit_tests ${test_SRC})
	
	target_link_libraries(boost_unit_tests
		wesnoth-client
		lua
		wesnoth-core
		wesnoth-game
		wesnoth-sdl
		${WIDGETS_LIB}
		wesnoth-client
		${game-external-libs}
		boost_unit_test_framework
	)
	set_target_properties(boost_unit_tests
		PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}boost_unit_tests${BINARY_SUFFIX}
	)

endif(ENABLE_TESTS)

