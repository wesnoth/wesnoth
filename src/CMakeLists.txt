## some includes ##
if(MSVC)
	# This seems to break on some Linux systems in combination with strict
	# compilation, so disabled for now. The problem occurred on Gentoo with
	# boost 1.37. Debian with boost 1.34, 1.35 and 1.38 works properly.
	include_directories( ${Boost_INCLUDE_DIR} )

	#additional MSVC includes
	include_directories( ${PANGO_INCLUDE_DIR} )
	include_directories( ${CAIRO_INCLUDE_DIR} )
	include_directories( ${GLIB_INCLUDE_DIR} )
	include_directories( ${GLIBCONFIG_INCLUDE_PATH} )
else(MSVC)
	include_directories( ${FONTCONFIG_INCLUDE_DIRS} )
endif(MSVC)


include_directories( ${LUA_INCLUDE_DIR} )
include_directories( ${SDL_INCLUDE_DIR} )
include_directories( ${PANGOCAIRO_INCLUDE_DIRS} )
include_directories( ${GETTEXT_INCLUDE_DIR} )
include_directories( ${LIBDBUS_INCLUDE_DIRS} )

#optional dependencies
if(SDLIMAGE_INCLUDE_DIR)
  include_directories( ${SDLIMAGE_INCLUDE_DIR} )
endif()
if(SDLMIXER_INCLUDE_DIR)
  include_directories( ${SDLMIXER_INCLUDE_DIR} )
endif()
if(SDLNET_INCLUDE_DIR)
  include_directories( ${SDLNET_INCLUDE_DIR} )
endif()
if(SDLTTF_INCLUDE_DIR)
  include_directories( ${SDLTTF_INCLUDE_DIR} )
endif()
if(ZLIB_INCLUDE_DIR)
  include_directories( ${ZLIB_INCLUDE_DIR} )
endif()

# needed to get include paths in the subfolders correct
include_directories( ${CMAKE_SOURCE_DIR}/src/ )
# needed to have the generated config.h used, too
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

link_directories(${Boost_LIBRARY_DIRS})

if(MSVC)
	# Since MSVC does autolinking of boost adding those files will duplicate
	# the includes and cause build errors.
	set( common-external-libs
		${SDL_LIBRARY}
		${SDLMAIN_LIBRARY}
		${LIBINTL_LIBRARY}
		ws2_32.lib
	)
else(MSVC)
	set( common-external-libs
		${SDL_LIBRARY}
		${Boost_IOSTREAMS_LIBRARY}
		${Boost_REGEX_LIBRARY}
	)
endif(MSVC)

set( game-external-libs
    ${common-external-libs}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${PANGOCAIRO_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${LUA_LIBRARIES}
    ${LIBDBUS_LIBRARIES}
)

if(MSVC)
	set( game-external-libs
		${game-external-libs}
		${PANGO_LIBRARY}
		${PANGOCAIRO_LIBRARY}
		${CAIRO_LIBRARY}
		${GOBJECT_LIBRARY}
	)
endif(MSVC)


set( server-external-libs
    ${common-external-libs}
    ${SDLNET_LIBRARY}
)

set( tools-external-libs
    ${common-external-libs}
    ${SDLIMAGE_LIBRARY}
)

if(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)
  include_directories( ${FRIBIDI_INCLUDE_DIR} )
  set(game-external-libs ${game-external-libs} ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)

if(X11_FOUND)
  include_directories( ${X11_INCLUDE_DIR} )
  set(game-external-libs ${game-external-libs} ${X11_LIBRARIES})
endif(X11_FOUND)
#
# Create targets for revision.hpp
#

find_program(SVNVERSION_EXECUTABLE "svnversion")

set(REVISION_FILE)
if(SVNVERSION_EXECUTABLE)
	execute_process(COMMAND ${SVNVERSION_EXECUTABLE} -n ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE SVN_VERSION)
	if(SVN_VERSION MATCHES [0-9]+.*)
		add_custom_target(update_revision ALL
			COMMAND ${CMAKE_COMMAND}
				-DSVNVERSION_EXECUTABLE="${SVNVERSION_EXECUTABLE}"
				-DSRC_DIR="${CMAKE_SOURCE_DIR}"
				-P "${CMAKE_MODULE_PATH}/revision.cmake"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "Updating revision")

		set(REVISION_FILE revision.hpp)
		set_source_files_properties(
			revision.hpp
			PROPERTIES GENERATED true)
		add_definitions(-DHAVE_REVISION)
	endif(SVN_VERSION MATCHES [0-9]+.*)
endif(SVNVERSION_EXECUTABLE)

if(MSVC)
	SOURCE_GROUP("src" REGULAR_EXPRESSION ".*")
	SOURCE_GROUP("src ai" REGULAR_EXPRESSION "ai/.*")
	SOURCE_GROUP("src ai ai2" REGULAR_EXPRESSION "ai/ai2/.*")
	SOURCE_GROUP("src ai composite" REGULAR_EXPRESSION "ai/composite/.*")
	SOURCE_GROUP("src ai default" REGULAR_EXPRESSION "ai/default/.*")
	SOURCE_GROUP("src ai dfool" REGULAR_EXPRESSION "ai/dfool/.*")
	SOURCE_GROUP("src ai formula" REGULAR_EXPRESSION "ai/formula/.*")
	SOURCE_GROUP("src ai testing" REGULAR_EXPRESSION "ai/testing/.*")
	SOURCE_GROUP("src campaign_server" REGULAR_EXPRESSION "campaign_server/.*")
	SOURCE_GROUP("src editor" REGULAR_EXPRESSION "editor/.*")
	SOURCE_GROUP("src gui" REGULAR_EXPRESSION "gui/.*")
	SOURCE_GROUP("src gui auxiliary" REGULAR_EXPRESSION "gui/auxiliary/.*")
	SOURCE_GROUP("src gui auxiliary event" REGULAR_EXPRESSION "gui/auxiliary/event/.*")
	SOURCE_GROUP("src gui auxiliary widget_definition" REGULAR_EXPRESSION "gui/auxiliary/widget_definition/.*")
	SOURCE_GROUP("src gui auxiliary window_builder" REGULAR_EXPRESSION "gui/auxiliary/window_builder/.*")
	SOURCE_GROUP("src gui dialogs" REGULAR_EXPRESSION "gui/dialogs/.*")
	SOURCE_GROUP("src gui dialogs lobby" REGULAR_EXPRESSION "gui/dialogs/lobby/.*")
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


endif(MSVC)

########### libwesnoth-core ###############

SET(libwesnoth-core_STAT_SRC
    color_range.cpp
    config.cpp
    filesystem.cpp
    game_config.cpp
    gettext.cpp
    hash.cpp
    log.cpp
    map.cpp
    map_location.cpp
    md5.cpp
    thread.cpp
    tstring.cpp
    util.cpp
    version.cpp
    serialization/binary_or_text.cpp
    serialization/binary_wml.cpp
    serialization/parser.cpp
    serialization/preprocessor.cpp
    serialization/string_utils.cpp
    serialization/tokenizer.cpp
    ${REVISION_FILE}
)

if(ENABLE_POOL_ALLOC AND NOT WIN32)
  SET(libwesnoth-core_STAT_SRC
      ${libwesnoth-core_STAT_SRC}
      malloc.c
      poolalloc.c
  )
endif(ENABLE_POOL_ALLOC AND NOT WIN32)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-core STATIC EXCLUDE_FROM_ALL ${libwesnoth-core_STAT_SRC})

if(SVNVERSION_EXECUTABLE)
	add_dependencies(wesnoth-core update_revision)
endif(SVNVERSION_EXECUTABLE)
########### Wesnoth main source files ###############

# Depending on whether the game or the game and the tests are build the main
# sources are compiled in the game library or not. If only one target is needed
# the putting in the game library solution is slower. If both targets are build
# the library is faster (especially if the user doesn't use ccache (or
# something similar)).

set(wesnoth-main_SRC
    about.cpp
    actions.cpp
    addon_checks.cpp
    addon_management.cpp
    ai/actions.cpp
    ai/composite/ai.cpp
    ai/composite/aspect.cpp
    ai/composite/component.cpp
    ai/composite/contexts.cpp
    ai/composite/engine.cpp
    ai/composite/engine_default.cpp
    ai/composite/engine_fai.cpp
    ai/composite/engine_lua.cpp
    ai/composite/goal.cpp
    ai/composite/rca.cpp
    ai/composite/stage.cpp
    ai/configuration.cpp
    ai/contexts.cpp
    ai/default/ai.cpp
    ai/default/attack.cpp
    ai/default/contexts.cpp
    ai/default/move.cpp
    ai/default/village.cpp
    ai/formula/ai.cpp
    ai/formula/callable_objects.cpp
    ai/formula/candidates.cpp
    ai/formula/function_table.cpp
    ai/formula/stage_rca_formulas.cpp
    ai/formula/stage_side_formulas.cpp
    ai/formula/stage_unit_formulas.cpp
    ai/game_info.cpp
    ai/gamestate_observer.cpp
    ai/interface.cpp
    ai/manager.cpp
    ai/registry.cpp
    ai/testing/aspect_attacks.cpp
    ai/testing/ca.cpp
    ai/testing/ca_default_move_to_targets.cpp
    ai/testing/ca_testing_move_to_targets.cpp
    ai/testing/stage_rca.cpp
    ai/testing/stage_fallback.cpp
    ai/testing.cpp
    animated_game.cpp
    attack_prediction.cpp
    attack_prediction_display.cpp
    callable_objects.cpp
    config_cache.cpp
    controller_base.cpp
    dialogs.cpp
    floating_textbox.cpp
    formula.cpp
    formula_debugger.cpp
    formula_debugger_fwd.cpp
    formula_function.cpp
    formula_tokenizer.cpp
    formula_string_utils.cpp
    game_display.cpp
    game_events.cpp
    game_preferences.cpp
    game_preferences_display.cpp
    gamestatus.cpp
    generate_report.cpp
    generic_event.cpp
    gui/auxiliary/canvas.cpp
    gui/auxiliary/event/dispatcher.cpp
    gui/auxiliary/event/distributor.cpp
    gui/auxiliary/event/handler.cpp
    gui/auxiliary/log.cpp
    gui/auxiliary/timer.cpp
    gui/auxiliary/widget_definition.cpp
    gui/auxiliary/widget_definition/button.cpp
    gui/auxiliary/widget_definition/horizontal_scrollbar.cpp
    gui/auxiliary/widget_definition/image.cpp
    gui/auxiliary/widget_definition/label.cpp
    gui/auxiliary/widget_definition/listbox.cpp
    gui/auxiliary/widget_definition/menubar.cpp
    gui/auxiliary/widget_definition/minimap.cpp
    gui/auxiliary/widget_definition/multi_page.cpp
    gui/auxiliary/widget_definition/panel.cpp
    gui/auxiliary/widget_definition/repeating_button.cpp
    gui/auxiliary/widget_definition/scroll_label.cpp
    gui/auxiliary/widget_definition/scrollbar_panel.cpp
    gui/auxiliary/widget_definition/slider.cpp
    gui/auxiliary/widget_definition/spacer.cpp
    gui/auxiliary/widget_definition/stacked_widget.cpp
    gui/auxiliary/widget_definition/text_box.cpp
    gui/auxiliary/widget_definition/toggle_button.cpp
    gui/auxiliary/widget_definition/toggle_panel.cpp
    gui/auxiliary/widget_definition/tooltip.cpp
    gui/auxiliary/widget_definition/tree_view.cpp
    gui/auxiliary/widget_definition/vertical_scrollbar.cpp
    gui/auxiliary/widget_definition/window.cpp
    gui/auxiliary/window_builder/button.cpp
    gui/auxiliary/window_builder/control.cpp
    gui/auxiliary/window_builder/helper.cpp
    gui/auxiliary/window_builder/horizontal_listbox.cpp
    gui/auxiliary/window_builder/horizontal_scrollbar.cpp
    gui/auxiliary/window_builder/image.cpp
    gui/auxiliary/window_builder/label.cpp
    gui/auxiliary/window_builder/listbox.cpp
    gui/auxiliary/window_builder/minimap.cpp
    gui/auxiliary/window_builder/menubar.cpp
    gui/auxiliary/window_builder/multi_page.cpp
    gui/auxiliary/window_builder/repeating_button.cpp
    gui/auxiliary/window_builder/scroll_label.cpp
    gui/auxiliary/window_builder/scrollbar_panel.cpp
    gui/auxiliary/window_builder/slider.cpp
    gui/auxiliary/window_builder/spacer.cpp
    gui/auxiliary/window_builder/stacked_widget.cpp
    gui/auxiliary/window_builder/text_box.cpp
    gui/auxiliary/window_builder/toggle_button.cpp
    gui/auxiliary/window_builder/vertical_scrollbar.cpp
    gui/auxiliary/window_builder/panel.cpp
    gui/auxiliary/window_builder/password_box.cpp
    gui/auxiliary/window_builder/toggle_panel.cpp
    gui/auxiliary/window_builder/tree_view.cpp
    gui/auxiliary/window_builder.cpp
    gui/dialogs/addon_connect.cpp
    gui/dialogs/addon_list.cpp
    gui/dialogs/campaign_selection.cpp
    gui/dialogs/chat_log.cpp
    gui/dialogs/dialog.cpp
    gui/dialogs/formula_debugger.cpp
    gui/dialogs/game_load.cpp
    gui/dialogs/game_delete.cpp
    gui/dialogs/game_save.cpp
    gui/dialogs/gamestate_inspector.cpp
    gui/dialogs/language_selection.cpp
    gui/dialogs/lobby/lobby_data.cpp
    gui/dialogs/lobby/lobby_info.cpp
    gui/dialogs/lobby_main.cpp
    gui/dialogs/lobby_player_info.cpp
    gui/dialogs/message.cpp
    gui/dialogs/mp_connect.cpp
    gui/dialogs/mp_create_game.cpp
    gui/dialogs/mp_method_selection.cpp
    gui/dialogs/mp_cmd_wrapper.cpp
    gui/dialogs/title_screen.cpp
    gui/dialogs/transient_message.cpp
    gui/dialogs/unit_create.cpp
    gui/dialogs/wml_message.cpp
    gui/widgets/button.cpp
    gui/widgets/control.cpp
    gui/widgets/container.cpp
    gui/widgets/debug.cpp
    gui/widgets/generator.cpp
    gui/widgets/grid.cpp
    gui/widgets/helper.cpp
    gui/widgets/horizontal_scrollbar.cpp
    gui/widgets/image.cpp
    gui/widgets/label.cpp
    gui/widgets/listbox.cpp
    gui/widgets/menubar.cpp
    gui/widgets/minimap.cpp
    gui/widgets/multi_page.cpp
    gui/widgets/panel.cpp
    gui/widgets/password_box.cpp
    gui/widgets/repeating_button.cpp
    gui/widgets/settings.cpp
    gui/widgets/scroll_label.cpp
    gui/widgets/scrollbar_container.cpp
    gui/widgets/scrollbar_panel.cpp
    gui/widgets/scrollbar.cpp
    gui/widgets/slider.cpp 
    gui/widgets/spacer.cpp
    gui/widgets/stacked_widget.cpp
    gui/widgets/text.cpp
    gui/widgets/text_box.cpp
    gui/widgets/toggle_button.cpp 
    gui/widgets/toggle_panel.cpp 
    gui/widgets/tooltip.cpp
    gui/widgets/tree_view.cpp
    gui/widgets/tree_view_node.cpp
    gui/widgets/vertical_scrollbar.cpp
    gui/widgets/widget.cpp
    gui/widgets/window.cpp
    halo.cpp
    help.cpp
    intro.cpp
    leader_list.cpp
    menu_events.cpp
    mouse_events.cpp
    mouse_handler_base.cpp
	mp_game_settings.cpp
    multiplayer.cpp
    multiplayer_ui.cpp
    multiplayer_wait.cpp
    multiplayer_connect.cpp
    multiplayer_create.cpp
    multiplayer_lobby.cpp
    network.cpp
    network_worker.cpp
    playcampaign.cpp
    play_controller.cpp
    playmp_controller.cpp
    playsingle_controller.cpp
    playturn.cpp
    portrait.cpp
    replay.cpp
    replay_controller.cpp
    resources.cpp
    save_blocker.cpp
    savegame.cpp
    savegame_config.cpp
    scripting/lua.cpp
    sha1.cpp
    settings.cpp
    statistics.cpp
    statistics_dialog.cpp
    storyscreen/interface.cpp
    storyscreen/controller.cpp
    storyscreen/part.cpp
    storyscreen/render.cpp
    team.cpp
    terrain_filter.cpp
    titlescreen.cpp
    tod_manager.cpp
    tooltips.cpp
    unit.cpp
    unit_abilities.cpp
    unit_animation.cpp
    unit_display.cpp
    unit_frame.cpp
	unit_id.cpp
    unit_map.cpp
    unit_types.cpp
    variable.cpp
    variant.cpp
    widgets/combo.cpp
    widgets/combo_drag.cpp
    widgets/drop_target.cpp
    widgets/scrollpane.cpp
)


if(ENABLE_EDITOR)
	set(wesnoth-main_SRC
		gui/dialogs/editor_generate_map.cpp
		gui/dialogs/editor_new_map.cpp
		gui/dialogs/editor_resize_map.cpp
		gui/dialogs/editor_settings.cpp
		editor/action.cpp
		editor/brush.cpp
		editor/editor_main.cpp
		editor/editor_controller.cpp
		editor/editor_display.cpp
		editor/editor_layout.cpp
		editor/editor_map.cpp
		editor/editor_palettes.cpp
		editor/editor_preferences.cpp
		editor/map_context.cpp
		editor/map_fragment.cpp
		editor/mouse_action.cpp
		${wesnoth-main_SRC}
	)
endif(ENABLE_EDITOR)

########### libwesnoth-game ###############
# rename libwesnoth.a to libwesnoth-game.a to have clearer targets

SET(libwesnoth-game_STAT_SRC
    builder.cpp
    cavegen.cpp
    clipboard.cpp
    construct_dialog.cpp
    cursor.cpp
    display.cpp
    events.cpp
    filechooser.cpp
    font.cpp
    hotkeys.cpp
    image.cpp
    image_function.cpp
    key.cpp
    language.cpp
    loadscreen.cpp
    lobby_preferences.cpp
    map_create.cpp
    map_label.cpp
    mapgen.cpp
    mapgen_dialog.cpp
    marked-up_text.cpp
    minimap.cpp
    pathfind/astarsearch.cpp
    pathutils.cpp
    preferences.cpp
    preferences_display.cpp
    race.cpp
    random.cpp
    reports.cpp
    sdl_utils.cpp
    show_dialog.cpp
    sound.cpp
    soundsource.cpp
    sound_music_track.cpp
    terrain.cpp
    terrain_translation.cpp
    text.cpp
    time_of_day.cpp
    video.cpp
    theme.cpp
    widgets/button.cpp
    widgets/file_menu.cpp
    widgets/label.cpp
    widgets/menu.cpp
    widgets/menu_style.cpp
    widgets/progressbar.cpp
    widgets/scrollarea.cpp
    widgets/scrollbar.cpp
    widgets/slider.cpp
    widgets/textbox.cpp
    widgets/widget.cpp
    wml_exception.cpp
)

    SET(libwesnoth-game_STAT_SRC
        ${libwesnoth-game_STAT_SRC}
        pathfind/pathfind.cpp
    )

if(ENABLE_GAME AND ENABLE_TESTS)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		${wesnoth-main_SRC}
	)
endif(ENABLE_GAME AND ENABLE_TESTS)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-game STATIC EXCLUDE_FROM_ALL ${libwesnoth-game_STAT_SRC})

if(ENABLE_GAME AND ENABLE_TESTS)
	target_link_libraries(wesnoth-game
		wesnoth-core
	)
endif(ENABLE_GAME AND ENABLE_TESTS)

########### Wesnoth ###############

if(ENABLE_GAME)
	if(ENABLE_TESTS)
		set(wesnoth_SRC
			game.cpp
		)
		set(wesnoth_LIB
			wesnoth-game
		)
	else(ENABLE_TESTS)
		set(wesnoth_SRC
			game.cpp
			${wesnoth-main_SRC}
		)
		set(wesnoth_LIB
			wesnoth-core
			wesnoth-game
		)
	endif(ENABLE_TESTS)

	add_executable(wesnoth WIN32
		${wesnoth_SRC}
	)

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

IF(ENABLE_SERVER)

SET(wesnothd_SRC
	server/ban.cpp
    server/game.cpp
    server/input_stream.cpp
    server/metrics.cpp
    server/player.cpp
    server/player_network.cpp
    server/proxy.cpp
    server/room.cpp
    server/room_manager.cpp
    server/server.cpp
    server/simple_wml.cpp
    server/user_handler.cpp
    server/forum_user_handler.cpp
    server/sample_user_handler.cpp
    network.cpp
    network_worker.cpp
    loadscreen_empty.cpp 
)

add_executable(wesnothd WIN32
	${wesnothd_SRC}
)

target_link_libraries(wesnothd wesnoth-core ${server-external-libs})
set_target_properties(wesnothd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX})

install(TARGETS wesnothd DESTINATION ${BINDIR})

ENDIF(ENABLE_SERVER)

########### Campaign Server ###############

IF(ENABLE_CAMPAIGN_SERVER)

SET(campaignd_SRC
    addon_checks.cpp
    campaign_server/campaign_server.cpp
    server/input_stream.cpp
    network.cpp
    network_worker.cpp
    loadscreen_empty.cpp
)

add_executable(campaignd ${campaignd_SRC})
target_link_libraries(campaignd wesnoth-core ${server-external-libs})
set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})

install(TARGETS campaignd DESTINATION ${BINDIR})

ENDIF(ENABLE_CAMPAIGN_SERVER)

########### Castle building helpers ###############

IF(ENABLE_TOOLS)

SET(exploder_SRC
    tools/exploder.cpp
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    tools/exploder_composer.cpp
    tools/dummy_video.cpp
    sdl_utils.cpp
    loadscreen_empty.cpp
)

add_executable(exploder ${exploder_SRC})
target_link_libraries(exploder wesnoth-core png12 ${tools-external-libs})
set_target_properties(exploder PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}exploder${BINARY_SUFFIX})

install(TARGETS exploder DESTINATION ${BINDIR})

SET(cutter_SRC
    tools/cutter.cpp
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    tools/dummy_video.cpp
    sdl_utils.cpp
    loadscreen_empty.cpp
)

add_executable(cutter ${cutter_SRC})
target_link_libraries(cutter wesnoth-core png12 ${tools-external-libs})
set_target_properties(cutter PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}cutter${BINARY_SUFFIX})

install(TARGETS cutter DESTINATION ${BINDIR})

ENDIF(ENABLE_TOOLS)

########### Unit tests ###############

if(ENABLE_TESTS)

	add_definitions(-DBOOST_TEST_DYN_LINK)

	set(test_SRC
		tests/main.cpp
		tests/utils/fake_display.cpp 
		tests/utils/fake_event_source.cpp 
		tests/utils/game_config_manager.cpp
		tests/utils/play_scenario.cpp
		tests/test_config_cache.cpp
		tests/test_formula_ai.cpp
		tests/test_lexical_cast.cpp	
		tests/test_network_worker.cpp
		tests/test_policy.cpp
		tests/test_team.cpp
		tests/test_util.cpp
		tests/test_serialization.cpp
		tests/test_version.cpp
		tests/gui/test_drop_target.cpp
		tests/gui/test_gui2.cpp
		tests/gui/test_save_dialog.cpp
	)
	if(NOT ENABLE_GAME)
		set(test_SRC
			${test_SRC}
			${wesnoth-main_SRC}
		)
	endif(NOT ENABLE_GAME)

	add_executable(test
		${test_SRC}
	)
	target_link_libraries(test
		wesnoth-core
		wesnoth-game
		${game-external-libs}
		boost_unit_test_framework
	)
	set_target_properties(test
		PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}test${BINARY_SUFFIX}
	)
endif(ENABLE_TESTS)

