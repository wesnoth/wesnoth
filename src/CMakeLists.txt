
set(CXX_FLAGS_PROJECT "${CXX_FLAGS_PROJECT} -include boost-patched/bind/arg.hpp")

# store the specified sources list in the specified variable
function(GetSources source_list store_in_var)
  file(STRINGS "../source_lists/${source_list}" sources)
  set(${store_in_var} ${sources} PARENT_SCOPE)
endfunction()

## some includes ##
if(MSVC)
	# This seems to break on some Linux systems in combination with strict
	# compilation, so disabled for now. The problem occurred on Gentoo with
	# boost 1.37. Debian with boost 1.34, 1.35 and 1.38 works properly.
	include_directories(SYSTEM ${Boost_INCLUDE_DIR} )

	#additional MSVC includes
	include_directories(SYSTEM ${PANGO_INCLUDE_DIR} )
	include_directories(SYSTEM ${CAIRO_INCLUDE_DIR} )
	include_directories(SYSTEM ${GLIB_INCLUDE_DIR} )
	include_directories(SYSTEM ${GLIBCONFIG_INCLUDE_PATH} )
else(MSVC)
	include_directories(SYSTEM ${FONTCONFIG_INCLUDE_DIRS} )
endif(MSVC)


include_directories(SYSTEM ${PANGOCAIRO_INCLUDE_DIRS} )
include_directories(SYSTEM ${GETTEXT_INCLUDE_DIR} )
include_directories(SYSTEM ${LIBDBUS_INCLUDE_DIRS} )
include_directories(SYSTEM ${VORBISFILE_INCLUDE_DIR} )


include_directories(SYSTEM ${SDL2_INCLUDE_DIR} )
set(sdl-lib ${SDL2_LIBRARY})
set(sdlmain-lib ${SDL2MAIN_LIBRARY})
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

if(MSVC)
	# Since MSVC does autolinking of boost adding those files will duplicate
	# the includes and cause build errors.
	set(common-external-libs
		${sdl-lib}
		${sdlmain-lib}
		ws2_32.lib
		shlwapi.lib
	)
else(MSVC)
	set(common-external-libs
		${sdl-lib}
		${Boost_IOSTREAMS_LIBRARY}
		${Boost_REGEX_LIBRARY}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
	)
endif(MSVC)

set(common-external-libs
	${common-external-libs}
	${Boost_FILESYSTEM_LIBRARY}
)

set(common-external-libs
	${common-external-libs}
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

if(MSVC)
	set(game-external-libs
		${game-external-libs}
		${PANGO_LIBRARY}
		${PANGOCAIRO_LIBRARY}
		${CAIRO_LIBRARY}
		${GOBJECT_LIBRARY}
	)
endif(MSVC)

set(server-external-libs
	${common-external-libs}
	${Boost_SYSTEM_LIBRARIES}
)

set(game-external-libs
	${game-external-libs}
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
	# latest FriBiDi deprecated some functions we rely on...
	set_source_files_properties(
		font/sdl_ttf.cpp
		font/text_surface.cpp
		PROPERTIES COMPILE_FLAGS
			"-Wno-deprecated-declarations -Wno-unused-result"
	)
endif(ENABLE_FRIBIDI AND FRIBIDI_FOUND)

if(X11_FOUND)
	include_directories(SYSTEM ${X11_INCLUDE_DIR} )
	set(game-external-libs ${game-external-libs} ${X11_LIBRARIES})
endif(X11_FOUND)

if(MSVC)
	SOURCE_GROUP("src" REGULAR_EXPRESSION ".*")
	SOURCE_GROUP("src ai" REGULAR_EXPRESSION "ai/.*")
	SOURCE_GROUP("src ai composite" REGULAR_EXPRESSION "ai/composite/.*")
	SOURCE_GROUP("src ai default" REGULAR_EXPRESSION "ai/default/.*")
	SOURCE_GROUP("src ai dfool" REGULAR_EXPRESSION "ai/dfool/.*")
	SOURCE_GROUP("src ai formula" REGULAR_EXPRESSION "ai/formula/.*")
	SOURCE_GROUP("src ai testing" REGULAR_EXPRESSION "ai/testing/.*")
	SOURCE_GROUP("src campaign_server" REGULAR_EXPRESSION "campaign_server/.*")
	SOURCE_GROUP("src editor" REGULAR_EXPRESSION "editor/.*")
	SOURCE_GROUP("src editor action" REGULAR_EXPRESSION "editor/action/.*")
	SOURCE_GROUP("src editor action mouse" REGULAR_EXPRESSION "editor/action/mouse/.*")
	SOURCE_GROUP("src editor palette" REGULAR_EXPRESSION "editor/palette/.*")
	SOURCE_GROUP("src editor toolkit" REGULAR_EXPRESSION "editor/toolkit/.*")
	SOURCE_GROUP("src editor map" REGULAR_EXPRESSION "editor/map/.*")
	SOURCE_GROUP("src gui" REGULAR_EXPRESSION "gui/.*")
	SOURCE_GROUP("src gui auxiliary" REGULAR_EXPRESSION "gui/auxiliary/.*")
	SOURCE_GROUP("src gui auxiliary event" REGULAR_EXPRESSION "gui/core/event/.*")
	SOURCE_GROUP("src gui auxiliary widget_definition" REGULAR_EXPRESSION "gui/core/widget_definition/.*")
	SOURCE_GROUP("src gui auxiliary window_builder" REGULAR_EXPRESSION "gui/core/window_builder/.*")
	SOURCE_GROUP("src gui dialogs" REGULAR_EXPRESSION "gui/dialogs/.*")
	SOURCE_GROUP("src gui dialogs lobby" REGULAR_EXPRESSION "gui/dialogs/lobby/.*")
	SOURCE_GROUP("src lua" REGULAR_EXPRESSION "lua/.*")
	SOURCE_GROUP("src widgets" REGULAR_EXPRESSION "widgets/.*")
	SOURCE_GROUP("src gui widgets" REGULAR_EXPRESSION "gui/widgets/.*")
	SOURCE_GROUP("src pathfind" REGULAR_EXPRESSION "pathfind/.*")
	SOURCE_GROUP("src scripting" REGULAR_EXPRESSION "scripting/.*")
	SOURCE_GROUP("src serialization" REGULAR_EXPRESSION "serialization/.*")
	SOURCE_GROUP("src server" REGULAR_EXPRESSION "server/.*")
	SOURCE_GROUP("src storyscreen" REGULAR_EXPRESSION "storyscreen/.*")
	SOURCE_GROUP("src tests" REGULAR_EXPRESSION "tests/.*")
	SOURCE_GROUP("src tests gui" REGULAR_EXPRESSION "tests/gui/.*")
	SOURCE_GROUP("src tests utils" REGULAR_EXPRESSION "tests/utils/.*")
	SOURCE_GROUP("src tools" REGULAR_EXPRESSION "tools/.*")
	SOURCE_GROUP("src tools schema" REGULAR_EXPRESSION "tools/schema/.*")
	SOURCE_GROUP("src utils" REGULAR_EXPRESSION "utils/.*")
	SOURCE_GROUP("src whiteboard" REGULAR_EXPRESSION "whiteboard/.*")

endif(MSVC)



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


########### libwesnoth-lua ###############

GetSources("lua" libwesnoth-lua_STAT_SRC)

# We explicitly want lua compiled as C++ version, so this line is required:
set_source_files_properties(${libwesnoth-lua_STAT_SRC} PROPERTIES LANGUAGE CXX)

# Inject a header into the Lua sources for Wesnoth-specific changes
# makedepend won't see it so we have to specifically add it as a dependancy.
file(GLOB wesnoth_lua_config wesnoth_lua_config.h)
set_source_files_properties(${libwesnoth-lua_STAT_SRC} PROPERTIES COMPILE_FLAGS "-include ${wesnoth_lua_config}")
set_source_files_properties(${libwesnoth-lua_STAT_SRC} PROPERTIES OBJECT_DEPENDS ${wesnoth_lua_config})

if(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)
	# Assume the compiler is the clang compiler.
	# It needs the files to be forced as c++ manually, it might be a newer
	# version of cmake will be able to this for us.
	#
	# Also silence some Clang specific warnings due to extra parenthesis in if statements when comparing instead
	set_property(SOURCE
		SOURCE ${libwesnoth-lua_STAT_SRC}
		APPEND_STRING PROPERTY COMPILE_FLAGS
			" -x c++ -Wno-parentheses-equality -Wno-conditional-uninitialized"
	)
endif(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-lua ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${libwesnoth-lua_STAT_SRC})


########### Old style cast flags ###############

# Disable the setting of -Wold-style-cast on some targets.
# old style casts are not wanted by our coding style but some C based code
# uses it. Force the flag off for these files.
if(HAS_COMPILER_FLAG_WOLD_STYLE_CAST)
	set(CXX_FLAG_NO_OLD_STYLE_CAST "-Wno-old-style-cast")
endif()

if(HAS_COMPILER_FLAG_WUSELESS_CAST)
	set(CXX_FLAG_NO_USELESS_CAST "-Wno-useless-cast")
endif()

set_target_properties(wesnoth-lua
	PROPERTIES
		COMPILE_FLAGS
			"${CXX_FLAG_NO_OLD_STYLE_CAST} ${CXX_FLAG_NO_USELESS_CAST}"
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

########### Wesnoth main source files ###############

# Depending on whether the game or the game and the tests are build the main
# sources are compiled in the game library or not. If only one target is needed
# the putting in the game library solution is slower. If both targets are build
# the library is faster (especially if the user doesn't use ccache (or
# something similar)).

GetSources("wesnoth" wesnoth-main_SRC)

########### libwesnoth-game ###############
# rename libwesnoth.a to libwesnoth-game.a to have clearer targets

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

if(ENABLE_GAME AND ENABLE_TESTS)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		${wesnoth-main_SRC}
	)
endif(ENABLE_GAME AND ENABLE_TESTS)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-game ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${libwesnoth-game_STAT_SRC})

if(ENABLE_GAME AND ENABLE_TESTS)
	target_link_libraries(wesnoth-game
		wesnoth-core
		wesnoth-lua
	)
endif(ENABLE_GAME AND ENABLE_TESTS)

########### Wesnoth ###############

if(ENABLE_GAME)
	if(ENABLE_TESTS)
		set(wesnoth_SRC
			wesnoth.cpp
		)
		set(wesnoth_LIB
			wesnoth-core
			wesnoth-lua
			wesnoth-game
			wesnoth-sdl
		)
	else(ENABLE_TESTS)
		set(wesnoth_SRC
			wesnoth.cpp
			${wesnoth-main_SRC}
		)
		set(wesnoth_LIB
			wesnoth-core
			wesnoth-lua
			wesnoth-game
			wesnoth-sdl
		)
	endif(ENABLE_TESTS)

	if (WINDOWS)
		add_executable(wesnoth WIN32 ${wesnoth_SRC})
	elseif(APPLE)
		add_executable(wesnoth #MACOSX_BUNDLE
				${wesnoth_SRC})
	else()
		add_executable(wesnoth ${wesnoth_SRC})
	endif()

	target_link_libraries(wesnoth
		${wesnoth_LIB}
		${game-external-libs}
	)
	set_target_properties(wesnoth
		PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnoth${BINARY_SUFFIX}
	)

	install(TARGETS wesnoth DESTINATION ${BINDIR})
endif(ENABLE_GAME)

########### Wesnothd Server ###############

if(ENABLE_SERVER)

  GetSources("wesnothd" wesnothd_SRC)

  add_executable(wesnothd WIN32
	  ${wesnothd_SRC}
  )

  target_link_libraries(wesnothd wesnoth-core ${server-external-libs} ${Boost_RANDOM_LIBRARY})
  set_target_properties(wesnothd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX})

  install(TARGETS wesnothd DESTINATION ${BINDIR})

endif(ENABLE_SERVER)

########### Campaign Server ###############

if(ENABLE_CAMPAIGN_SERVER)
  
  GetSources("campaignd" campaignd_SRC)

  check_library_exists(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_RT)
  if(HAVE_CLOCK_GETTIME_RT)
	  set(LIBRT rt)
  endif()

  add_executable(campaignd ${campaignd_SRC})
  target_link_libraries(campaignd
	  wesnoth-core
	  ${server-external-libs}
	  ${CRYPTO_LIBRARY}
	  ${Boost_RANDOM_LIBRARY}
	  ${LIBRT}
	  )
  set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})

  install(TARGETS campaignd DESTINATION ${BINDIR})

endif(ENABLE_CAMPAIGN_SERVER)

########### Unit tests ###############

if(ENABLE_TESTS)

	add_definitions(-DBOOST_TEST_DYN_LINK)
  
  GetSources("test" test_SRC)
  
	if(ENABLE_GAME)
		set(test_LIB
			wesnoth-core
			wesnoth-game
			wesnoth-sdl
			wesnoth-lua
		)
	else(ENABLE_GAME)
		set(test_SRC
			${test_SRC}
			${wesnoth-main_SRC}
		)
		set(test_LIB
			wesnoth-core
			wesnoth-game
			wesnoth-lua
			wesnoth-sdl
		)
	endif(ENABLE_GAME)

	set_source_files_properties(
		tests/test_util.cpp
		PROPERTIES
			COMPILE_FLAGS
				"${CXX_FLAG_NO_USELESS_CAST}"
	)

	add_executable(test
		${test_SRC}
	)
	target_link_libraries(test
		${test_LIB}
		${game-external-libs}
		boost_unit_test_framework
	)
	set_target_properties(test
		PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}test${BINARY_SUFFIX}
	)

endif(ENABLE_TESTS)

