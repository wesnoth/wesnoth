# vi: syntax=python:et:ts=4

import commands, os
from subprocess import Popen, PIPE
from glob import glob

Import("*")

for env in [test_env, client_env, env]:
    env.Append(CPPDEFINES = "$EXTRA_DEFINE")

#color_range.cpp should be removed, but game_config depends on it.
#game_config has very few things that are needed elsewhere, it should be
#removed.  Requires moving path and version at least to other files.

libwesnoth_core_sources = Split("""
    color_range.cpp
    config.cpp
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
    serialization/parser.cpp
    serialization/preprocessor.cpp
    serialization/schema_validator.cpp
    serialization/string_utils.cpp
    serialization/tokenizer.cpp
    serialization/validator.cpp
    tools/schema/tag.cpp
    """)
if env["pool_alloc"]:
    libwesnoth_core_sources.extend(Split("""
        malloc.c
        poolalloc.c
        """))

libwesnoth_core_sources.extend(env.Object("network_worker.cpp", EXTRA_DEFINE = env['raw_sockets'] and "NETWORK_USE_RAW_SOCKETS" or None))

if env["use_network_ana"]:
    client_env.Append(CPPPATH = ["#/src/ana/api"])
    libwesnoth_core_sources.append("network_ana.cpp")
    libwesnoth_core_sources.append("network_manager_ana.cpp")
    ana = SConscript("ana/src/SConscript", exports = {"env" : client_env})
else:
    libwesnoth_core_sources.append("network.cpp")
    ana = []

game_config_env = env.Clone()
filesystem_env = env.Clone()
if env["PLATFORM"] != "win32":
    game_config_env.Append(CPPDEFINES = "WESNOTH_PATH='\"$datadir\"'")
    if env['localedirname']:
        filesystem_env.Append(CPPDEFINES = "LOCALEDIR='\"$localedirname\"'")
        if not os.path.isabs(env['localedirname']):
            filesystem_env.Append(CPPDEFINES = "HAS_RELATIVE_LOCALEDIR")
    if env['version_suffix'] and not env['prefsdir']:
        filesystem_env['prefsdir'] = ".wesnoth$version_suffix"
    if filesystem_env['prefsdir']:
        filesystem_env.Append(CPPDEFINES = "PREFERENCES_DIR='\"$prefsdir\"'")

libwesnoth_core_sources.extend([
    game_config_env.Object("game_config.cpp"),
    filesystem_env.Object("filesystem.cpp")
    ])

libwesnoth_core = [env.Library("wesnoth_core", libwesnoth_core_sources), ana]

libwesnoth_sources = Split("""
    arrow.cpp
    pathfind/astarsearch.cpp
    builder.cpp
    cavegen.cpp
    clipboard.cpp
    construct_dialog.cpp
    cursor.cpp
    display.cpp
    events.cpp
    generic_event.cpp
    hotkeys.cpp
    image.cpp
    image_modifications.cpp
    joystick.cpp
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
    show_dialog.cpp
    sound.cpp
    soundsource.cpp
    sound_music_track.cpp
    terrain.cpp
    terrain_translation.cpp
    text.cpp
    time_of_day.cpp
    tooltips.cpp
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
    """)
libwesnoth_sources.extend([
    client_env.Object("font.cpp", EXTRA_DEFINE = client_env['fribidi'] and "HAVE_FRIBIDI" or None),
    ])

libwesnoth = client_env.Library("wesnoth", libwesnoth_sources)

libwesnothd_sources = Split("""
    loadscreen_empty.cpp
    tools/dummy_video.cpp
    """)
libwesnothd = env.Library("wesnothd", libwesnothd_sources)

libcampaignd_sources = Split("""
    addon/validation.cpp
    """)
libcampaignd = env.Library("campaignd", libcampaignd_sources, OBJPREFIX = "campaignd_")

libwesnoth_sdl_sources = Split("""
    sdl_utils.cpp
    """)
libwesnoth_sdl = client_env.Library("wesnoth_sdl", libwesnoth_sdl_sources)

libcutter_sources = Split("""
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    """)
libcutter = client_env.Library("cutter", libcutter_sources)

# Used by both 'wesnoth' and 'test' targets
wesnoth_sources = Split("""
    about.cpp
    actions.cpp
    addon/manager.cpp
    addon/validation.cpp
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
    ai/formula/ai.cpp
    ai/formula/callable_objects.cpp
    ai/formula/candidates.cpp
    ai/formula/function_table.cpp
    ai/formula/stage_side_formulas.cpp
    ai/formula/stage_unit_formulas.cpp
    ai/game_info.cpp
    ai/gamestate_observer.cpp
    ai/interface.cpp
    ai/lua/core.cpp
    ai/lua/lua_object.cpp
    ai/manager.cpp
    ai/registry.cpp
    ai/testing.cpp
    ai/testing/aspect_attacks.cpp
    ai/testing/ca.cpp
    ai/testing/ca_global_fallback.cpp
    ai/testing/ca_testing_move_to_targets.cpp
    ai/testing/ca_testing_recruitment.cpp
    ai/testing/stage_fallback.cpp
    ai/testing/stage_rca.cpp
    animated_game.cpp
    attack_prediction.cpp
    attack_prediction_display.cpp
    callable_objects.cpp
    commandline_options.cpp
    config_cache.cpp
    controller_base.cpp
    dialogs.cpp
    editor/action.cpp
    editor/brush.cpp
    editor/editor_controller.cpp
    editor/editor_display.cpp
    editor/editor_layout.cpp
    editor/editor_main.cpp
    editor/editor_map.cpp
    editor/editor_palettes.cpp
    editor/editor_preferences.cpp
    editor/map_context.cpp
    editor/map_fragment.cpp
    editor/mouse_action.cpp
    filechooser.cpp
    floating_textbox.cpp
    formula.cpp
    formula_debugger.cpp
    formula_debugger_fwd.cpp
    formula_function.cpp
    formula_string_utils.cpp
    formula_tokenizer.cpp
    game_controller.cpp
    game_controller_abstract.cpp
    game_controller_new.cpp
    game_display.cpp
    game_errors.cpp
    game_events.cpp
    game_instance.cpp
    game_preferences.cpp
    gamestatus.cpp
    gui/auxiliary/canvas.cpp
    gui/auxiliary/event/dispatcher.cpp
    gui/auxiliary/event/distributor.cpp
    gui/auxiliary/event/handler.cpp
    gui/auxiliary/iterator/iterator.cpp
    gui/auxiliary/iterator/walker_grid.cpp
    gui/auxiliary/iterator/walker_widget.cpp
    gui/auxiliary/log.cpp
    gui/auxiliary/old_markup.cpp
    gui/auxiliary/timer.cpp
    gui/auxiliary/tips.cpp
    gui/auxiliary/widget_definition.cpp
    gui/auxiliary/widget_definition/button.cpp
    gui/auxiliary/widget_definition/drawing.cpp
    gui/auxiliary/widget_definition/horizontal_scrollbar.cpp
    gui/auxiliary/widget_definition/image.cpp
    gui/auxiliary/widget_definition/label.cpp
    gui/auxiliary/widget_definition/listbox.cpp
    gui/auxiliary/widget_definition/minimap.cpp
    gui/auxiliary/widget_definition/multi_page.cpp
    gui/auxiliary/widget_definition/panel.cpp
    gui/auxiliary/widget_definition/progress_bar.cpp
    gui/auxiliary/widget_definition/repeating_button.cpp
    gui/auxiliary/widget_definition/scroll_label.cpp
    gui/auxiliary/widget_definition/scrollbar_panel.cpp
    gui/auxiliary/widget_definition/slider.cpp
    gui/auxiliary/widget_definition/spacer.cpp
    gui/auxiliary/widget_definition/stacked_widget.cpp
    gui/auxiliary/widget_definition/text_box.cpp
    gui/auxiliary/widget_definition/toggle_button.cpp
    gui/auxiliary/widget_definition/toggle_panel.cpp
    gui/auxiliary/widget_definition/tree_view.cpp
    gui/auxiliary/widget_definition/vertical_scrollbar.cpp
    gui/auxiliary/widget_definition/window.cpp
    gui/auxiliary/window_builder.cpp
    gui/auxiliary/window_builder/button.cpp
    gui/auxiliary/window_builder/control.cpp
    gui/auxiliary/window_builder/drawing.cpp
    gui/auxiliary/window_builder/helper.cpp
    gui/auxiliary/window_builder/horizontal_listbox.cpp
    gui/auxiliary/window_builder/horizontal_scrollbar.cpp
    gui/auxiliary/window_builder/image.cpp
    gui/auxiliary/window_builder/label.cpp
    gui/auxiliary/window_builder/listbox.cpp
    gui/auxiliary/window_builder/minimap.cpp
    gui/auxiliary/window_builder/multi_page.cpp
    gui/auxiliary/window_builder/panel.cpp
    gui/auxiliary/window_builder/password_box.cpp
    gui/auxiliary/window_builder/progress_bar.cpp
    gui/auxiliary/window_builder/repeating_button.cpp
    gui/auxiliary/window_builder/scroll_label.cpp
    gui/auxiliary/window_builder/scrollbar_panel.cpp
    gui/auxiliary/window_builder/slider.cpp
    gui/auxiliary/window_builder/spacer.cpp
    gui/auxiliary/window_builder/stacked_widget.cpp
    gui/auxiliary/window_builder/text_box.cpp
    gui/auxiliary/window_builder/toggle_button.cpp
    gui/auxiliary/window_builder/toggle_panel.cpp
    gui/auxiliary/window_builder/tree_view.cpp
    gui/auxiliary/window_builder/vertical_scrollbar.cpp
    gui/dialogs/addon/description.cpp
    gui/dialogs/addon/uninstall_list.cpp
    gui/dialogs/addon_connect.cpp
    gui/dialogs/addon_list.cpp
    gui/dialogs/campaign_difficulty.cpp
    gui/dialogs/campaign_selection.cpp
    gui/dialogs/chat_log.cpp
    gui/dialogs/data_manage.cpp
    gui/dialogs/debug_clock.cpp
    gui/dialogs/dialog.cpp
    gui/dialogs/edit_label.cpp
    gui/dialogs/editor_generate_map.cpp
    gui/dialogs/editor_new_map.cpp
    gui/dialogs/editor_resize_map.cpp
    gui/dialogs/editor_set_starting_position.cpp
    gui/dialogs/editor_settings.cpp
    gui/dialogs/folder_create.cpp
    gui/dialogs/formula_debugger.cpp
    gui/dialogs/game_delete.cpp
    gui/dialogs/game_load.cpp
    gui/dialogs/game_save.cpp
    gui/dialogs/gamestate_inspector.cpp
    gui/dialogs/language_selection.cpp
    gui/dialogs/lobby/lobby_data.cpp
    gui/dialogs/lobby/lobby_info.cpp
    gui/dialogs/lobby_main.cpp
    gui/dialogs/lobby_player_info.cpp
    gui/dialogs/message.cpp
    gui/dialogs/mp_cmd_wrapper.cpp
    gui/dialogs/mp_change_control.cpp
    gui/dialogs/mp_connect.cpp
    gui/dialogs/mp_create_game.cpp
    gui/dialogs/mp_create_game_set_password.cpp
    gui/dialogs/mp_login.cpp
    gui/dialogs/mp_method_selection.cpp
    gui/dialogs/network_transmission.cpp
    gui/dialogs/popup.cpp
    gui/dialogs/simple_item_selector.cpp
    gui/dialogs/tip.cpp
    gui/dialogs/title_screen.cpp
    gui/dialogs/transient_message.cpp
    gui/dialogs/unit_attack.cpp
    gui/dialogs/unit_create.cpp
    gui/dialogs/wml_message.cpp
    gui/widgets/button.cpp
    gui/widgets/container.cpp
    gui/widgets/control.cpp
    gui/widgets/drawing.cpp
    gui/widgets/generator.cpp
    gui/widgets/grid.cpp
    gui/widgets/helper.cpp
    gui/widgets/horizontal_scrollbar.cpp
    gui/widgets/image.cpp
    gui/widgets/label.cpp
    gui/widgets/list.cpp
    gui/widgets/listbox.cpp
    gui/widgets/minimap.cpp
    gui/widgets/multi_page.cpp
    gui/widgets/panel.cpp
    gui/widgets/password_box.cpp
    gui/widgets/progress_bar.cpp
    gui/widgets/repeating_button.cpp
    gui/widgets/scroll_label.cpp
    gui/widgets/scrollbar.cpp
    gui/widgets/scrollbar_container.cpp
    gui/widgets/scrollbar_panel.cpp
    gui/widgets/settings.cpp
    gui/widgets/slider.cpp 
    gui/widgets/spacer.cpp
    gui/widgets/stacked_widget.cpp
    gui/widgets/text.cpp
    gui/widgets/text_box.cpp
    gui/widgets/toggle_button.cpp 
    gui/widgets/toggle_panel.cpp 
    gui/widgets/tree_view.cpp
    gui/widgets/tree_view_node.cpp
    gui/widgets/vertical_scrollbar.cpp
    gui/widgets/widget.cpp
    gui/widgets/window.cpp
    halo.cpp
    help.cpp
    intro.cpp
    leader_list.cpp
    lobby_preferences.cpp
    menu_events.cpp
    mouse_events.cpp
    mouse_handler_base.cpp
    mp_game_settings.cpp
    multiplayer.cpp
    multiplayer_connect.cpp
    multiplayer_create.cpp
    multiplayer_lobby.cpp
    multiplayer_ui.cpp
    multiplayer_wait.cpp
    network_asio.cpp
    pathfind/pathfind.cpp
    pathfind/teleport.cpp
    persist_context.cpp
    persist_manager.cpp
    persist_var.cpp
    play_controller.cpp
    playcampaign.cpp
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
    scripting/debug_lua.cpp
    scripting/lua.cpp
    settings.cpp
    sha1.cpp
    side_filter.cpp
    statistics.cpp
    statistics_dialog.cpp
    storyscreen/controller.cpp
    storyscreen/interface.cpp
    storyscreen/part.cpp
    storyscreen/render.cpp
    team.cpp
    terrain_filter.cpp
    tod_manager.cpp
    unit.cpp
    unit_abilities.cpp
    unit_animation.cpp
    unit_display.cpp
    unit_frame.cpp
    unit_helper.cpp
    unit_id.cpp
    unit_map.cpp
    unit_types.cpp
    variable.cpp
    variant.cpp
    whiteboard/action.cpp
    whiteboard/attack.cpp
    whiteboard/highlight_visitor.cpp
    whiteboard/manager.cpp
    whiteboard/mapbuilder.cpp
    whiteboard/move.cpp
    whiteboard/recall.cpp
    whiteboard/recruit.cpp
    whiteboard/side_actions.cpp
    whiteboard/suppose_dead.cpp
    whiteboard/utility.cpp
    whiteboard/validate_visitor.cpp
    widgets/combo.cpp
    widgets/combo_drag.cpp
    widgets/drop_target.cpp
    widgets/scrollpane.cpp
    """)

wesnoth_sources.extend(client_env.Object("game_preferences_display.cpp", EXTRA_DEFINE = env["PLATFORM"] != "win32" and "WESNOTH_PREFIX='\"$prefix\"'" or None))

libwesnoth_extras = client_env.Library("wesnoth_extras", wesnoth_sources)

libwesnoth_extras.extend(SConscript("lua/SConscript"))

#
# Target declarations
#

def error_action(target, source, env):
    from SCons.Errors import UserError
    raise UserError, "Target disabled because its prerequisites are not met"

def WesnothProgram(env, target, source, can_build, **kw):
    if can_build:
        if env["build"] == "base":
            bin = env.Program(target, source, **kw)
        else:
            bin = env.Program("#/" + target + build_suffix, source, **kw)
        env.Alias(target, bin)
    else:
        bin = env.Alias(target, [], error_action)
        AlwaysBuild(bin)
    exec target + " = bin"
    Export(target)

for env in [test_env, client_env, env]:
    env.AddMethod(WesnothProgram)

game_cpp = client_env.Object("game.cpp", EXTRA_DEFINE = not env["pool_alloc"] and "DISABLE_POOL_ALLOC" or None);

wesnoth_objects = [game_cpp, libwesnoth_extras, libwesnoth_core, libwesnoth_sdl,
                   libwesnoth, env["wesnoth_res"]]
if env["host"] in ["x86_64-nacl", "i686-nacl"]:
    wesnoth_objects += [SConscript("nacl/SConscript")]
client_env.WesnothProgram("wesnoth", wesnoth_objects, have_client_prereqs)

campaignd_sources = Split("""
    server/input_stream.cpp
    """)

if env["PLATFORM"] == "win32": env["fifodir"] = ""
campaignd_sources.extend(env.Object("campaign_server/campaign_server.cpp", EXTRA_DEFINE = env['fifodir'] and "FIFODIR='\"$fifodir\"'" or None))

env.WesnothProgram("campaignd", campaignd_sources + [libwesnoth_core, libwesnothd, libcampaignd], have_server_prereqs)

wesnothd_sources = Split("""
    server/ban.cpp
    server/forum_user_handler.cpp
    server/game.cpp
    server/input_stream.cpp
    server/metrics.cpp
    server/player.cpp
    server/player_network.cpp
    server/proxy.cpp
    server/room.cpp
    server/room_manager.cpp
    server/sample_user_handler.cpp
    server/simple_wml.cpp
    server/user_handler.cpp
    """)
wesnothd_sources.extend(env.Object("server/server.cpp", EXTRA_DEFINE = env['fifodir'] and "FIFODIR='\"$fifodir\"'" or None))

env.WesnothProgram("wesnothd", wesnothd_sources + [libwesnoth_core, libwesnothd], have_server_prereqs)

cutter_sources = Split("""
    tools/cutter.cpp
    """)
client_env.WesnothProgram("cutter", cutter_sources + [libcutter, libwesnoth_core, libwesnoth_sdl, libwesnothd, libwesnoth], have_client_prereqs, LIBS = ["$LIBS", "png"])

exploder_sources = Split("""
    tools/exploder.cpp
    tools/exploder_composer.cpp
    """)
client_env.WesnothProgram("exploder", exploder_sources + [libcutter, libwesnoth_core, libwesnoth_sdl, libwesnothd, libwesnoth], have_client_prereqs, LIBS = ["$LIBS", "png"])

schema_generator_sources = Split("""
    tools/schema/schema_generator.cpp
    tools/schema/sourceparser.cpp
    tools/schema/error_container.cpp
    """)
client_env.WesnothProgram("schema_generator", schema_generator_sources + [libwesnoth_core, libwesnothd], have_client_prereqs)

test_utils_sources = Split("""
    tests/utils/game_config_manager.cpp
    tests/utils/fake_event_source.cpp
    tests/utils/fake_display.cpp
    """)

libtest_utils = test_env.Library("test_utils", test_utils_sources)

test_sources = Split("""
    tests/main.cpp
    tests/test_commandline_options.cpp
    tests/test_formula_ai.cpp
    tests/test_formula_function.cpp
    tests/test_image_modifications.cpp
    tests/test_lexical_cast.cpp
    tests/test_network_worker.cpp
    tests/test_team.cpp
    tests/test_unit_map.cpp
    tests/test_util.cpp
    tests/test_serialization.cpp
    tests/test_version.cpp
    tests/gui/fire_event.cpp
    tests/gui/iterator.cpp
    tests/gui/test_drop_target.cpp
    tests/gui/test_gui2.cpp
    tests/gui/test_save_dialog.cpp
    tests/gui/visitor.cpp
    tests/utils/play_scenario.cpp
    """)
test_sources.extend(test_env.Object("tests/test_config_cache.cpp"))

test = test_env.WesnothProgram("test", test_sources +  [libwesnoth_extras, libwesnoth_core, libwesnoth_sdl, libwesnoth, libwesnoth_extras, libtest_utils], have_test_prereqs)

if env["svnrev"] != "" and env["svnrev"] != "exported":
    revision_define = "#define REVISION \"%s\"\n" % env["svnrev"]
    env.Command("#/src/revision.hpp", Value(env["svnrev"]), Action(
                    lambda target, source, env: open(str(target[0]), "w").write(revision_define),
                    "Generating revision.hpp..."
                    ))
    game_config_env.Append(CPPDEFINES = 'HAVE_REVISION')

sources = []
if "TAGS" in COMMAND_LINE_TARGETS:
    sources = [ Glob(os.path.join(dir, pattern)) for dir in ["", "*", "*/*"] for pattern in ["*.cpp", "*.hpp"] ]

Export("sources")

# Local variables:
# mode: python
# end:
