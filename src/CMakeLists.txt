# store the specified sources list in the specified variable
function(GetSources source_list store_in_var)
	file(STRINGS "${CMAKE_SOURCE_DIR}/source_lists/${source_list}" sources)
	foreach(source IN LISTS sources)
	    list(APPEND loaded_sources "${CMAKE_SOURCE_DIR}/src/${source}")
		set(${store_in_var} ${loaded_sources} PARENT_SCOPE)
	endforeach()
endfunction()

# external git submodules
add_subdirectory("modules")

## some includes ##
include_directories(SYSTEM ${PANGOCAIRO_INCLUDE_DIRS})
include_directories(SYSTEM ${GETTEXT_INCLUDE_DIR})
include_directories(SYSTEM ${LIBDBUS_INCLUDE_DIRS})
if(NOT MSVC)
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
	${LIBDBUS_LIBRARY_DIRS}
)

set(common-external-libs ${ICU_DATA_LIBRARY} ${ICU_I18N_LIBRARY} ${ICU_UC_LIBRARY})
if(MSVC)
	set(common-external-libs ${common-external-libs} shlwapi.lib winmm.lib crypt32.lib)
elseif(MINGW)
	set(common-external-libs ${common-external-libs} wsock32 ws2_32 shlwapi winmm crypt32)
elseif(APPLE)
	set(common-external-libs ${common-external-libs} ${APPKIT_LIBRARY} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${SECURITY_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(common-external-libs ${common-external-libs} socket)
endif()

if(ENABLE_SYSTEM_LUA)
	add_compile_definitions(HAVE_SYSTEM_LUA=1)
	include_directories( SYSTEM ${LUA_INCLUDE_DIR} )
	set(common-external-libs ${common-external-libs} ${LUA_LIBRARIES})
endif()

set(game-external-libs
	${common-external-libs}
	${PANGOCAIRO_LIBRARIES}
	${LIBDBUS_LIBRARIES}
	VorbisFile::VorbisFile
)

if(NOT MSVC)
	set(game-external-libs
		${game-external-libs}
		${SDL2IMAGE_LIBRARIES}
		${SDL2MIXER_LIBRARIES}
	)
endif()

if(HISTORY_FOUND)
	set(game-external-libs ${game-external-libs} History::History)
endif()

# get source lists
GetSources("libwesnoth_sdl" wesnoth_sdl_sources)
GetSources("libwesnoth_widgets" wesnoth_widget-sources)
GetSources("libwesnoth" wesnoth_game_sources)
GetSources("libwesnoth_core" wesnoth_core_sources)
GetSources("wesnoth" wesnoth_sources)

if(WIN32)
	set(wesnoth_core_sources ${wesnoth_core_sources} log_windows.cpp)
	set(wesnoth_game_sources ${wesnoth_game_sources} desktop/windows_tray_notification.cpp desktop/windows_battery_info.cpp)
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
	set_source_files_properties(game_version.cpp PROPERTIES COMPILE_DEFINITIONS "LOAD_REVISION")
endif()

########### Conf Tests ###########

if((ENABLE_GAME OR ENABLE_TESTS) AND NOT MSVC)
	# test for SDL2
	add_executable(sdl2 conftests/sdl2.cpp)
	set_target_properties(sdl2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2 ${game-external-libs})
	# cmake checks the version elsewhere already, but scons uses this, which is why the three arguments for major.minor.patchlevel are 0 here
	add_test(NAME SDL2_SUPPORT COMMAND sdl2 0 0 0)

	# test for SDL2_image
	add_executable(sdl2_image conftests/sdl2_image.cpp)
	set_target_properties(sdl2_image PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_image ${game-external-libs})
	add_test(NAME SDL2_IMAGE_SUPPORT COMMAND sdl2_image)

	# test for SDL2_mixer
	add_executable(sdl2_mixer conftests/sdl2_mixer.cpp)
	set_target_properties(sdl2_mixer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_mixer ${game-external-libs})
	add_test(NAME SDL2_MIXER_SUPPORT COMMAND sdl2_mixer)

	# test for JPG support in SDL2
	add_executable(sdl2_jpg conftests/sdl2_jpg.cpp)
	set_target_properties(sdl2_jpg PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_jpg ${game-external-libs})
	add_test(NAME SDL2_JPG_SUPPORT COMMAND sdl2_jpg "${CMAKE_SOURCE_DIR}/data/core/images/scons_conftest_images/end-n.jpg")

	# test for PNG support in SDL2
	add_executable(sdl2_png conftests/sdl2_png.cpp)
	set_target_properties(sdl2_png PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_png ${game-external-libs})
	add_test(NAME SDL2_PNG_SUPPORT COMMAND sdl2_png "${CMAKE_SOURCE_DIR}/data/core/images/scons_conftest_images/end-n.png")

	# test for WEBP support in SDL2
	add_executable(sdl2_webp conftests/sdl2_webp.cpp)
	set_target_properties(sdl2_webp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_webp ${game-external-libs})
	add_test(NAME SDL2_WEBP_SUPPORT COMMAND sdl2_webp "${CMAKE_SOURCE_DIR}/data/core/images/scons_conftest_images/end-n.webp")

	# test for audio support in SDL2
	add_executable(sdl2_audio conftests/sdl2_audio.cpp)
	set_target_properties(sdl2_audio PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing)
	target_link_libraries(sdl2_audio ${game-external-libs})
	add_test(NAME SDL2_AUDIO_SUPPORT COMMAND sdl2_audio "${CMAKE_SOURCE_DIR}/data/core/music/main_menu.ogg")

	add_custom_target(conftests
		COMMAND ${CMAKE_COMMAND} -E env "SDL_AUDIODRIVER=dummy" ${CMAKE_CTEST_COMMAND}
		DEPENDS
			sdl2
			sdl2_image
			sdl2_mixer
			sdl2_jpg
			sdl2_png
			sdl2_webp
			sdl2_audio
	)
endif()

########### Wesnoth ###############

add_library(wesnoth-common STATIC ${wesnoth_core_sources})
if(ENABLE_GAME OR ENABLE_TESTS)
	add_library(wesnoth-client STATIC ${wesnoth_sources} ${wesnoth_game_sources} ${wesnoth_sdl_sources})
	if(NOT ENABLE_SYSTEM_LUA)
    	target_link_libraries(wesnoth-client lua)
	endif()
	# widgets need special handling since otherwise the way they're registered causes the linker to remove them since it incorrectly thinks they're unused
	add_library(wesnoth-widgets STATIC ${wesnoth_widget-sources})
	if(APPLE)
		set(WIDGETS_LIB -Wl,-force_load wesnoth-widgets)
	elseif(NOT MSVC)
		set(WIDGETS_LIB -Wl,--whole-archive wesnoth-widgets -Wl,--no-whole-archive)
	else()
		# handled by /WHOLEARCHIVE below
		set(WIDGETS_LIB wesnoth-widgets)
	endif()
endif()

if(ENABLE_GAME)
	if(WIN32)
		add_executable(wesnoth WIN32 wesnoth.cpp ../packaging/windows/wesnoth.rc)
		if(MSVC)
			target_link_options(wesnoth PRIVATE /WX /WHOLEARCHIVE:wesnoth-widgets)
		endif()
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
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
		Fontconfig::Fontconfig
		SDL2::SDL2
		SDL2::SDL2main
		CURL::libcurl
	)
	if(MSVC)
		target_link_libraries(wesnoth SDL2_image::SDL2_image)
		target_link_libraries(wesnoth SDL2_mixer::SDL2_mixer)
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

	if(MSVC)
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
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
		Boost::unit_test_framework
		Fontconfig::Fontconfig
		SDL2::SDL2
		SDL2::SDL2main
		CURL::libcurl
	)
	if(MSVC)
		target_link_libraries(boost_unit_tests SDL2_image::SDL2_image)
		target_link_libraries(boost_unit_tests SDL2_mixer::SDL2_mixer)
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
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
	)
	if(MSVC)
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
		Boost::random
		Boost::coroutine
		Boost::locale
		Boost::filesystem
	)
	if(MSVC)
		target_link_options(campaignd PRIVATE /WX)
	endif()

	set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})
	if(ENABLE_DISPLAY_REVISION)
		add_dependencies(campaignd wesnoth-revision)
	endif()

	install(TARGETS campaignd DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
