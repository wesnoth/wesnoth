## some includes ##
include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${SDLIMAGE_INCLUDE_DIR} )
include_directories( ${SDLMIXER_INCLUDE_DIR} )
include_directories( ${SDLNET_INCLUDE_DIR} )
include_directories( ${SDLTTF_INCLUDE_DIR} )
include_directories( ${SDL_INCLUDE_DIR} )
include_directories( ${ZLIB_INCLUDE_DIR} )
# needed to get include paths in the subfolders correct
include_directories( ${CMAKE_SOURCE_DIR}/src/ )

link_directories(${Boost_LIBRARY_DIRS})

set( the-external-libs
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${SDL_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_REGEX_LIBRARY}
)

if(ENABLE_PYTHON AND PYTHON_LIBRARY)
  include_directories( ${PYTHON_INCLUDE_PATH} )
  set(the-external-libs ${the-external-libs} ${PYTHON_LIBRARIES})
endif(ENABLE_PYTHON AND PYTHON_LIBRARY)

if(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)
  include_directories( ${FRIBIDI_INCLUDE_PATH} )
  set(the-external-libs ${the-external-libs} ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)

#
# Create targets for revision.hpp
#

find_program(SVNVERSION_EXECUTABLE "svnversion")

set(REVISION_FILE)
if(SVNVERSION_EXECUTABLE)
  execute_process(COMMAND ${SVNVERSION_EXECUTABLE} -n ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE SVN_VERSION)
  if(SVN_VERSION MATCHES [0-9]+.*)
    add_custom_target(revision-stamp ALL
                      COMMAND ${CMAKE_COMMAND}
                      -DSVNVERSION_EXECUTABLE="${SVNVERSION_EXECUTABLE}" -DSRC_DIR="${CMAKE_SOURCE_DIR}"
                      -P "${CMAKE_SOURCE_DIR}/cmake/revision.cmake"
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Updating revision")

    add_custom_command(OUTPUT revision.hpp
                       COMMAND ${CMAKE_COMMAND}
                       -E copy ${CMAKE_CURRENT_BINARY_DIR}/revision-stamp ${CMAKE_CURRENT_SOURCE_DIR}/revision.hpp
                       DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/revision-stamp)

    set(REVISION_FILE revision.hpp)
    add_definitions(-DHAVE_REVISION)
  endif(SVN_VERSION MATCHES [0-9]+.*)
  
endif(SVNVERSION_EXECUTABLE)


########### libwesnoth-core ###############

SET(libwesnoth-core_STAT_SRC
    color_range.cpp
    config.cpp
    filesystem.cpp
    game_config.cpp
    gettext.cpp
    log.cpp
    map.cpp
    thread.cpp
    tstring.cpp
    util.cpp
    serialization/binary_or_text.cpp
    serialization/binary_wml.cpp
    serialization/parser.cpp
    serialization/preprocessor.cpp
    serialization/string_utils.cpp
    serialization/tokenizer.cpp
    ${REVISION_FILE}
)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-core STATIC EXCLUDE_FROM_ALL ${libwesnoth-core_STAT_SRC})

########### libwesnoth-game ###############
# rename libwesnoth.a to libwesnoth-game.a to have clearer targets

SET(libwesnoth-game_STAT_SRC
    astarnode.cpp
    astarsearch.cpp
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
    key.cpp
    language.cpp
    loadscreen.cpp
    map_create.cpp
    map_label.cpp
    mapgen.cpp
    mapgen_dialog.cpp
    marked-up_text.cpp
    minimap.cpp
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
    terrain.cpp
    terrain_translation.cpp
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

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-game STATIC EXCLUDE_FROM_ALL ${libwesnoth-game_STAT_SRC})

########### Wesnoth ###############

IF(ENABLE_GAME)

SET(wesnoth-main_SRC
    about.cpp
    actions.cpp
    addons_manager.cpp
    ai.cpp
    ai_dfool.cpp
    ai_attack.cpp
    ai_move.cpp
    ai_python.cpp
    ai_village.cpp
    animated_game.cpp
    attack_prediction.cpp
    callable_objects.cpp
    config_adapter.cpp
    dialogs.cpp
    floating_textbox.cpp
    formula.cpp
    formula_ai.cpp
    formula_function.cpp
    formula_tokenizer.cpp
    game_display.cpp
    game_events.cpp
    game_preferences.cpp
    game_preferences_display.cpp
    gamestatus.cpp
    generate_report.cpp
    generic_event.cpp
    gui/dialogs/addon_connect.cpp
    gui/dialogs/language_selection.cpp
    gui/widgets/button.cpp
    gui/widgets/canvas.cpp
    gui/widgets/control.cpp
    gui/widgets/container.cpp
    gui/widgets/event_handler.cpp
    gui/widgets/grid.cpp
    gui/widgets/helper.cpp
    gui/widgets/label.cpp
    gui/widgets/listbox.cpp
    gui/widgets/panel.cpp
    gui/widgets/settings.cpp
    gui/widgets/spacer.cpp
    gui/widgets/scrollbar.cpp
    gui/widgets/text.cpp
    gui/widgets/text_box.cpp
    gui/widgets/toggle_button.cpp 
    gui/widgets/tooltip.cpp
    gui/widgets/vertical_scrollbar.cpp
    gui/widgets/widget.cpp
    gui/widgets/window.cpp
    gui/widgets/window_builder.cpp
    halo.cpp
    help.cpp
    intro.cpp
    leader_list.cpp
    menu_events.cpp
    mouse_events.cpp
    multiplayer.cpp
    multiplayer_ui.cpp
    multiplayer_wait.cpp
    multiplayer_connect.cpp
    multiplayer_create.cpp
    multiplayer_lobby.cpp
    network.cpp
    network_worker.cpp
    pathfind.cpp
    playcampaign.cpp
    play_controller.cpp
    playmp_controller.cpp
    playsingle_controller.cpp
    playturn.cpp
    publish_campaign.cpp
    replay.cpp
    replay_controller.cpp
    sha1.cpp
    settings.cpp
    statistics.cpp
    team.cpp
    terrain_filter.cpp
    titlescreen.cpp
    tooltips.cpp
    unit.cpp
    unit_abilities.cpp
    unit_animation.cpp
    unit_display.cpp
    unit_frame.cpp
    unit_map.cpp
    unit_types.cpp
    upload_log.cpp
    variable.cpp
    variant.cpp
    widgets/combo.cpp
    widgets/scrollpane.cpp
)

SET(wesnoth_SRC
    game.cpp
    ${wesnoth-main_SRC}
)

ADD_EXECUTABLE(${BINARY_PREFIX}wesnoth${BINARY_SUFFIX} ${wesnoth_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}wesnoth${BINARY_SUFFIX} wesnoth-core wesnoth-game ${the-external-libs})

INSTALL(TARGETS ${BINARY_PREFIX}wesnoth${BINARY_SUFFIX} DESTINATION ${BINDIR})

ENDIF(ENABLE_GAME)

########### Editor ###############

IF(ENABLE_EDITOR)

SET(wesnoth_editor_SRC
    editor/editor.cpp
    editor/editor_layout.cpp
    editor/map_manip.cpp
    editor/editor_display.cpp
    editor/editor_palettes.cpp
    editor/editor_main.cpp
    editor/editor_dialogs.cpp
    editor/editor_undo.cpp
    animated_editor.cpp
    generic_event.cpp
    tooltips.cpp 
)

ADD_EXECUTABLE(${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX} ${wesnoth_editor_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX} wesnoth-core wesnoth-game ${the-external-libs})

INSTALL(TARGETS ${BINARY_PREFIX}wesnoth_editor${BINARY_SUFFIX} DESTINATION ${BINDIR})

ENDIF(ENABLE_EDITOR)

########### Wesnothd Server ###############

IF(ENABLE_SERVER)

SET(wesnothd_SRC
    server/game.cpp
    server/input_stream.cpp
    server/metrics.cpp
    server/player.cpp
    server/proxy.cpp
    server/server.cpp
    server/simple_wml.cpp
    network.cpp
    network_worker.cpp
    loadscreen_empty.cpp 
)

ADD_EXECUTABLE(${BINARY_PREFIX}wesnothd${BINARY_SUFFIX} ${wesnothd_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}wesnothd${BINARY_SUFFIX} wesnoth-core ${the-external-libs})

INSTALL(TARGETS ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX} DESTINATION ${BINDIR})

ENDIF(ENABLE_SERVER)

########### Campaign Server ###############

IF(ENABLE_CAMPAIGN_SERVER)

SET(campaignd_SRC
    campaign_server/campaign_server.cpp
    network.cpp
    network_worker.cpp
    publish_campaign.cpp
    loadscreen_empty.cpp
)

ADD_EXECUTABLE(${BINARY_PREFIX}campaignd${BINARY_SUFFIX} ${campaignd_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}campaignd${BINARY_SUFFIX} wesnoth-core ${the-external-libs})

INSTALL(TARGETS ${BINARY_PREFIX}campaignd${BINARY_SUFFIX} DESTINATION ${BINDIR})

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

ADD_EXECUTABLE(${BINARY_PREFIX}exploder${BINARY_SUFFIX} ${exploder_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}exploder${BINARY_SUFFIX} wesnoth-core ${the-external-libs} png12)

INSTALL(TARGETS ${BINARY_PREFIX}exploder${BINARY_SUFFIX} DESTINATION ${BINDIR})

SET(cutter_SRC
    tools/cutter.cpp
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    tools/dummy_video.cpp
    sdl_utils.cpp
    loadscreen_empty.cpp
)

ADD_EXECUTABLE(${BINARY_PREFIX}cutter${BINARY_SUFFIX} ${cutter_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}cutter${BINARY_SUFFIX} wesnoth-core ${the-external-libs} png12)

INSTALL(TARGETS ${BINARY_PREFIX}cutter${BINARY_SUFFIX} DESTINATION ${BINDIR})

ENDIF(ENABLE_TOOLS)

########### Unit tests ###############

IF(ENABLE_TESTS)

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

SET(test_SRC
    tests/main.cpp
    tests/test_util.cpp
    ${wesnoth-main_SRC}
)

ADD_EXECUTABLE(${BINARY_PREFIX}test${BINARY_SUFFIX} ${test_SRC})
TARGET_LINK_LIBRARIES(${BINARY_PREFIX}test${BINARY_SUFFIX} wesnoth-core wesnoth-game ${the-external-libs} boost_unit_test_framework)

ENDIF(ENABLE_TESTS)

########### install files ###############

#install(FILES  DESTINATION /share/@DATADIR@)
