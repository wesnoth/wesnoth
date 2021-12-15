# store the specified sources list in the specified variable
function(GetSources source_list store_in_var)
	file(STRINGS "../source_lists/${source_list}" sources)
	set(${store_in_var} ${sources} PARENT_SCOPE)
endfunction()

if(ENABLE_MYSQL)
	execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "-I" "" MYSQL_CFLAGS "${MYSQL_CFLAGS}")
	string(REGEX REPLACE "-DNDEBUG" "" MYSQL_CFLAGS "${MYSQL_CFLAGS}")
	execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
	add_subdirectory("modules/mariadbpp/")
endif()

## some includes ##
include_directories(SYSTEM ${FONTCONFIG_INCLUDE_DIRS})
include_directories(SYSTEM ${PANGOCAIRO_INCLUDE_DIRS})
include_directories(SYSTEM ${GETTEXT_INCLUDE_DIR})
include_directories(SYSTEM ${LIBDBUS_INCLUDE_DIRS})
include_directories(SYSTEM ${VORBISFILE_INCLUDE_DIR})
if(NOT MSVC)
	include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})
	include_directories(SYSTEM ${SDL2IMAGE_INCLUDE_DIRS})
	include_directories(SYSTEM ${SDL2MIXER_INCLUDE_DIRS})
endif()

if(ZLIB_INCLUDE_DIR)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIR} )
endif()

# needed to get include paths in the subfolders correct
include_directories( ${CMAKE_SOURCE_DIR}/src/ )
# needed to have the generated config.h used, too
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

link_directories(
	${PANGOCAIRO_LIBRARY_DIRS}
	${FONTCONFIG_LIBRARY_DIRS}
	${LIBDBUS_LIBRARY_DIRS}
)

set(common-external-libs ${ICU_DATA_LIBRARY} ${ICU_I18N_LIBRARY} ${ICU_UC_LIBRARY})
if(WIN32)
	set(common-external-libs ${common-external-libs} shlwapi.lib winmm.lib crypt32.lib)
elseif(MINGW)
	set(common-external-libs ${common-external-libs} wsock32 ws2_32 shlwapi winmm)
elseif(APPLE)
	set(common-external-libs ${common-external-libs} ${IOKIT_LIBRARY} ${SECURITY_LIBRARY})
endif()

set(game-external-libs
	${common-external-libs}
	${PANGOCAIRO_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
	${LIBDBUS_LIBRARIES}
	${VORBISFILE_LIBRARIES}
)

if(NOT MSVC)
	set(game-external-libs
		${game-external-libs}
		${SDL2_LIBRARIES}
		${SDL2IMAGE_LIBRARIES}
		${SDL2MIXER_LIBRARIES}
	)
endif()

if(HISTORY_FOUND)
	set(game-external-libs ${game-external-libs} ${HISTORY_LIBRARY})
endif()

# get source lists
GetSources("lua" lua_sources)
GetSources("libwesnoth_sdl" wesnoth_sdl_sources)
GetSources("libwesnoth_widgets" wesnoth_widget-sources)
GetSources("libwesnoth" wesnoth_game_sources)
GetSources("libwesnoth_core" wesnoth_core_sources)
GetSources("wesnoth" wesnoth_sources)

# We explicitly want lua compiled as C++ version, so this line is required:
set_source_files_properties(${lua_sources} PROPERTIES LANGUAGE CXX)

# Inject a header into the Lua sources for Wesnoth-specific changes
# makedepend won't see it so we have to specifically add it as a dependency.
file(GLOB wesnoth_lua_config wesnoth_lua_config.h)
set_source_files_properties(${lua_sources} PROPERTIES OBJECT_DEPENDS ${wesnoth_lua_config})

if(WIN32)
	set(wesnoth_core_sources ${wesnoth_core_sources} log_windows.cpp)
	set(wesnoth_game_sources ${wesnoth_game_sources} desktop/windows_tray_notification.cpp desktop/windows_battery_info.cpp)
	
	set_source_files_properties(${lua_sources} PROPERTIES COMPILE_FLAGS "/FI\"${wesnoth_lua_config}\"")
	# silence an implicit bitshift conversion warning
	set_property(SOURCE SOURCE ${lua_sources} APPEND_STRING PROPERTY COMPILE_FLAGS " /wd4334")
else()
	set_source_files_properties(${lua_sources} PROPERTIES COMPILE_FLAGS "-include \"${wesnoth_lua_config}\" -Wno-old-style-cast -Wno-useless-cast -Wno-stringop-overflow")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# silence a Clang specific warning due to extra parenthesis in if statements
	set_property(SOURCE SOURCE ${lua_sources} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-parentheses-equality")
	# silence a Clang specific warning when compiling the lua code
	set_property(SOURCE SOURCE ${lua_sources}	APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-string-plus-int")
endif()

# On apple only
if(APPLE)
	set(wesnoth_game_sources
		${wesnoth_game_sources}
		desktop/apple_battery_info.mm
		desktop/apple_notification.mm
		desktop/apple_version.mm
		desktop/apple_video.mm
	)
endif()

# For libdbus (essentially just for linux), this file needs to be linked, as its header is included #ifdef HAVE_LIBDBUS
if(LIBDBUS_FOUND)
	set(wesnoth_game_sources ${wesnoth_game_sources} desktop/dbus_features.cpp)
endif()

# Depending on the flag create a real file or an empty dummy.
#
# Since the code is executed every build run of Wesnoth we need to make sure
# that no modifications don't turn into a rebuild. Therefore a the dummy is
# created and the second target only copies the file if modified.
if(ENABLE_DISPLAY_REVISION)
	add_custom_target(wesnoth-revision_dummy
		COMMAND ${CMAKE_SOURCE_DIR}/utils/autorevision.sh -t h > ${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

	add_custom_target(wesnoth-revision
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/revision.dummy
			${CMAKE_CURRENT_BINARY_DIR}/revision.h
	)

	add_dependencies(wesnoth-revision wesnoth-revision_dummy)
	set_source_files_properties(game_config.cpp PROPERTIES COMPILE_DEFINITIONS "LOAD_REVISION")
endif()

########### Wesnoth ###############

add_library(wesnoth-common STATIC ${wesnoth_core_sources})
if(ENABLE_GAME OR ENABLE_TESTS)
	add_library(wesnoth-client STATIC ${wesnoth_sources} ${lua_sources} ${wesnoth_game_sources} ${wesnoth_sdl_sources})

	# widgets need special handling since otherwise the way they're registered causes the linker to remove them since it incorrectly thinks they're unused
	add_library(wesnoth-widgets STATIC ${wesnoth_widget-sources})
	if(APPLE)
		set(WIDGETS_LIB -Wl,-force_load wesnoth-widgets)
	elseif(NOT WIN32)
		set(WIDGETS_LIB -Wl,--whole-archive wesnoth-widgets -Wl,--no-whole-archive)
	else()
		# handled by /WHOLEARCHIVE below
		set(WIDGETS_LIB wesnoth-widgets)
	endif()
endif()

if(ENABLE_GAME)
	if(WIN32)
		add_executable(wesnoth WIN32 wesnoth.cpp ../packaging/windows/wesnoth.rc)
		target_link_options(wesnoth PRIVATE /WX /WHOLEARCHIVE:wesnoth-widgets)
	elseif(APPLE)
		add_executable(wesnoth wesnoth.cpp macosx/SDLMain.mm)
	else()
		add_executable(wesnoth wesnoth.cpp)
	endif()

	target_link_libraries(wesnoth
		wesnoth-common
		${WIDGETS_LIB}
		wesnoth-client
		wesnoth-common
		${game-external-libs}
		OpenSSL::Crypto
		OpenSSL::SSL
		Boost::iostreams
		Boost::program_options
		Boost::regex
		Boost::system
		Boost::thread
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
	)
	if(MSVC)
		target_link_libraries(wesnoth SDL2::SDL2)
		target_link_libraries(wesnoth SDL2::SDL2main)
		target_link_libraries(wesnoth SDL2::SDL2_image)
		target_link_libraries(wesnoth SDL2::SDL2_mixer)
	endif()

	if(ENABLE_DISPLAY_REVISION)
		add_dependencies(wesnoth wesnoth-revision)
	endif()

	set_target_properties(wesnoth PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnoth${BINARY_SUFFIX})

	install(TARGETS wesnoth DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

########### Boost Unit tests ###############

if(ENABLE_TESTS)
	add_definitions(-DBOOST_TEST_DYN_LINK)

	GetSources("boost_unit_tests" boost_tests_sources)
	add_executable(boost_unit_tests ${boost_tests_sources})

	if(WIN32)
		target_link_options(boost_unit_tests PRIVATE /WX /WHOLEARCHIVE:wesnoth-widgets)
	endif()

	target_link_libraries(boost_unit_tests
		wesnoth-common
		${WIDGETS_LIB}
		wesnoth-client
		wesnoth-common
		${game-external-libs}
		OpenSSL::Crypto
		OpenSSL::SSL
		Boost::iostreams
		Boost::program_options
		Boost::regex
		Boost::system
		Boost::thread
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
		Boost::unit_test_framework
	)
	if(MSVC)
		target_link_libraries(boost_unit_tests SDL2::SDL2)
		target_link_libraries(boost_unit_tests SDL2::SDL2main)
		target_link_libraries(boost_unit_tests SDL2::SDL2_image)
		target_link_libraries(boost_unit_tests SDL2::SDL2_mixer)
	endif()

	if(ENABLE_DISPLAY_REVISION)
		add_dependencies(boost_unit_tests wesnoth-revision)
	endif()

	set_target_properties(boost_unit_tests PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}boost_unit_tests${BINARY_SUFFIX})
endif()

########### Wesnothd Server ###############

if(ENABLE_SERVER)
	GetSources("wesnothd" wesnothd_sources)

	if(WIN32)
		add_executable(wesnothd ${wesnothd_sources} ../packaging/windows/wesnothd.rc)
	else()
		add_executable(wesnothd ${wesnothd_sources})
	endif()

	if(ENABLE_MYSQL)
		target_include_directories(wesnothd SYSTEM PRIVATE ${MYSQL_CFLAGS})
		target_compile_definitions(wesnothd PRIVATE HAVE_MYSQLPP)
		target_link_libraries(wesnothd mariadbclientpp)
	endif()

	target_link_libraries(wesnothd
		wesnoth-common
		${common-external-libs}
		${MYSQL_LIBS}
		OpenSSL::Crypto
		OpenSSL::SSL
		Boost::iostreams
		Boost::program_options
		Boost::regex
		Boost::system
		Boost::thread
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
	)
	if(WIN32)
		target_link_options(wesnothd PRIVATE /WX)
	endif()

	set_target_properties(wesnothd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX})
	if(ENABLE_DISPLAY_REVISION)
		add_dependencies(wesnothd wesnoth-revision)
	endif()

	install(TARGETS wesnothd DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

########### Campaignd Server ###############

if(ENABLE_CAMPAIGN_SERVER)
	GetSources("campaignd" campaignd_sources)

	add_executable(campaignd ${campaignd_sources})

	if(ENABLE_MYSQL)
		target_include_directories(campaignd SYSTEM PRIVATE ${MYSQL_CFLAGS})
		target_compile_definitions(campaignd PRIVATE HAVE_MYSQLPP)
		target_link_libraries(campaignd mariadbclientpp)
	endif()

	target_link_libraries(
		campaignd
		wesnoth-common
		${common-external-libs}
		${MYSQL_LIBS}
		OpenSSL::Crypto
		OpenSSL::SSL
		Boost::iostreams
		Boost::program_options
		Boost::regex
		Boost::system
		Boost::thread
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
	)
	if(WIN32)
		target_link_options(campaignd PRIVATE /WX)
	endif()

	set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})
	if(ENABLE_DISPLAY_REVISION)
		add_dependencies(campaignd wesnoth-revision)
	endif()

	install(TARGETS campaignd DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
