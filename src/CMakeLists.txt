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


include_directories(SYSTEM ${SDL_INCLUDE_DIR} )
include_directories(SYSTEM ${PANGOCAIRO_INCLUDE_DIRS} )
include_directories(SYSTEM ${GETTEXT_INCLUDE_DIR} )
include_directories(SYSTEM ${LIBDBUS_INCLUDE_DIRS} )
include_directories(SYSTEM ${LIBINTL_INCLUDE_DIR} )
if(NOT ENABLE_PANDORA)
	include_directories(SYSTEM ${VORBISFILE_INCLUDE_DIR} )
endif(NOT ENABLE_PANDORA)

#optional dependencies
if(SDLIMAGE_INCLUDE_DIR)
	include_directories(SYSTEM ${SDLIMAGE_INCLUDE_DIR} )
endif()
if(SDLMIXER_INCLUDE_DIR)
	include_directories(SYSTEM ${SDLMIXER_INCLUDE_DIR} )
endif()
if(SDLNET_INCLUDE_DIR)
	include_directories(SYSTEM ${SDLNET_INCLUDE_DIR} )
endif()
if(SDLTTF_INCLUDE_DIR)
	include_directories(SYSTEM ${SDLTTF_INCLUDE_DIR} )
endif()
if(ZLIB_INCLUDE_DIR)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIR} )
endif()

set(network_implementation_files network.cpp network_worker.cpp)

if(USE_ANA_NETWORK)
	set(network_implementation_files network_ana.cpp network_manager_ana.cpp)
	message(STATUS "Compiling network module with ana: ${network_implementation_files}")

	set(ANA_LIBRARIES ana-static)

	add_subdirectory(ana/src)
endif(USE_ANA_NETWORK)

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
		${SDL_LIBRARY}
		${SDLMAIN_LIBRARY}
		wesnoth-sdl
		${LIBINTL_LIBRARY}
		ws2_32.lib
	)
else(MSVC)
	set(common-external-libs
		${SDL_LIBRARY}
		wesnoth-sdl
		${Boost_IOSTREAMS_LIBRARY}
		${Boost_REGEX_LIBRARY}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LIBINTL_LIBRARIES}
	)
endif(MSVC)

set(game-external-libs
	${common-external-libs}
	${Boost_SYSTEM_LIBRARIES}
	${SDLIMAGE_LIBRARY}
	${SDLMIXER_LIBRARY}
	${ANA_LIBRARIES}
	${SDLTTF_LIBRARY}
	${PANGOCAIRO_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
	${LIBDBUS_LIBRARIES}
)

if(NOT ENABLE_PANDORA)
	set(game-external-libs
		${game-external-libs}
		${VORBISFILE_LIBRARIES}
	)
endif(NOT ENABLE_PANDORA)

if(MSVC)
	set(game-external-libs
		${game-external-libs}
		${PANGO_LIBRARY}
		${PANGOCAIRO_LIBRARY}
		${CAIRO_LIBRARY}
		${GOBJECT_LIBRARY}
	)
endif(MSVC)

if(USE_ANA_NETWORK)
	set(server-external-libs
		${common-external-libs}
		${ANA_LIBRARIES}
)
else(USE_ANA_NETWORK)
	set(server-external-libs
		${common-external-libs}
		${SDLNET_LIBRARY}
	)
	set(game-external-libs
		${game-external-libs}
		${SDLNET_LIBRARY}
	)
endif(USE_ANA_NETWORK)

set(tools-external-libs
	${common-external-libs}
	${SDLIMAGE_LIBRARY}
)

if(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)
	include_directories(SYSTEM ${FRIBIDI_INCLUDE_DIR} )
	set(game-external-libs ${game-external-libs} ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI AND FRIBIDI_LIBRARIES)

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
	SOURCE_GROUP("src gui auxiliary event" REGULAR_EXPRESSION "gui/auxiliary/event/.*")
	SOURCE_GROUP("src gui auxiliary widget_definition" REGULAR_EXPRESSION "gui/auxiliary/widget_definition/.*")
	SOURCE_GROUP("src gui auxiliary window_builder" REGULAR_EXPRESSION "gui/auxiliary/window_builder/.*")
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

set(libwesnoth-core_STAT_SRC
	color_range.cpp
	config.cpp
	filesystem.cpp
	game_config.cpp
	gettext.cpp
	hash.cpp
	log.cpp
	md5.cpp
	thread.cpp
	tstring.cpp
	util.cpp
	version.cpp
	serialization/binary_or_text.cpp
	serialization/parser.cpp
	serialization/preprocessor.cpp
	serialization/string_utils.cpp
	serialization/tokenizer.cpp
	serialization/unicode.cpp
	serialization/validator.cpp
)

# a 'lib' is automatically set in front when creating the library (as in the filename)
# internal reference is the name given here
add_library(wesnoth-core ${LIBRARY_TYPE} EXCLUDE_FROM_ALL ${libwesnoth-core_STAT_SRC})

# Depending on the flag create a real file or an empty dummy.
#
# Since the code is executed every build run of Wesnoth we need to make sure
# that no modifications don't turn into a rebuild. Therefore a the dummy is
# created and the second target only copies the file if modified.
if(ENABLE_DISPLAY_REVISION)
	add_custom_target(wesnoth-revision_dummy
		COMMAND
			${CMAKE_SOURCE_DIR}/utils/autorevision
				-th
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

set(libwesnoth-lua_STAT_SRC
	lua_jailbreak_exception.cpp
	lua/lapi.cpp
	lua/lbitlib.cpp
	lua/lcode.cpp
	lua/lcorolib.cpp
	lua/lctype.cpp
	lua/ldebug.cpp
	lua/ldo.cpp
	lua/ldump.cpp
	lua/lfunc.cpp
	lua/lgc.cpp
	lua/llex.cpp
	lua/lmem.cpp
	lua/lobject.cpp
	lua/lopcodes.cpp
	lua/lparser.cpp
	lua/lstate.cpp
	lua/lstring.cpp
	lua/ltable.cpp
	lua/ltm.cpp
	lua/lundump.cpp
	lua/lvm.cpp
	lua/lzio.cpp
	lua/lauxlib.cpp
	lua/lbaselib.cpp
	lua/ldblib.cpp
	lua/liolib.cpp
	lua/lmathlib.cpp
	lua/loslib.cpp
	lua/ltablib.cpp
	lua/lstrlib.cpp
	lua/loadlib.cpp
	lua/linit.cpp
)
# We explicitly want lua compiled as C++ version, so this line is required:
set_source_files_properties(${libwesnoth-lua_STAT_SRC} PROPERTIES LANGUAGE CXX)

if(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)
	# Assume the compiler is the clang compiler.
	# It needs the files to be forced as c++ manually, it might be a newer
	# version of cmake will be able to this for us.
	#
	# Also silence some Clang specific warnings due to extra parenthesis in if statements when comparing instead
	set_source_files_properties(
		${libwesnoth-lua_STAT_SRC}
		PROPERTIES COMPILE_FLAGS
			"-x c++ -Wno-parentheses-equality"
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
set_source_files_properties(
	clipboard.cpp
	network.cpp
	network_worker.cpp
	PROPERTIES
		COMPILE_FLAGS "${CXX_FLAG_NO_OLD_STYLE_CAST}"
)
set_source_files_properties(
	desktop_util.cpp
	text.cpp
	PROPERTIES
		COMPILE_FLAGS
			"${CXX_FLAG_NO_OLD_STYLE_CAST} ${CXX_FLAG_NO_USELESS_CAST}"
)
if(LIBDBUS_FOUND)
	set_source_files_properties(
		game_display.cpp
		PROPERTIES
			COMPILE_FLAGS "${CXX_FLAG_NO_OLD_STYLE_CAST}"
	)
endif()

########### Helper libraries ###############

set(wesnoth-sdl_SRC
	sdl/alpha.cpp
	sdl/exception.cpp
	sdl/texture.cpp
	sdl/window.cpp
)

add_library(wesnoth-sdl
	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-sdl_SRC}
)

target_link_libraries(wesnoth-sdl
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
)

# The main library is rather huge especially in debug mode, causing quite a bit
# of overhead packing objects that haven't been changed for quite a while.
# Attempting to solve the issue with several smaller libraries when adding new
# files.

##### Gui types

# This library contains the basic types for the gui library.
# - Public Enumerates
# - Small structs
set(wesnoth-gui_types_SRC
	gui/lib/types/point.cpp
)

add_library(wesnoth-gui_types
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_types_SRC}
)

##### Gui event

# This library contains the basic event handling for the gui library. It
# converts the SDL events to internal events and allows other code to
# connect to the events based on slots.
set(wesnoth-gui_event_SRC
	gui/auxiliary/event/dispatcher.cpp
	gui/auxiliary/event/distributor.cpp
	gui/auxiliary/event/handler.cpp
)

add_library(wesnoth-gui_event
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_event_SRC}
)


##### Gui iterator

# This library contains the basic iterator for the gui library.
# It is used to create visitors for the GUI widget tree in a window.
set(wesnoth-gui_iterator_SRC
	gui/auxiliary/iterator/iterator.cpp
	gui/auxiliary/iterator/walker_grid.cpp
	gui/auxiliary/iterator/walker_widget.cpp
)

add_library(wesnoth-gui_iterator
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_iterator_SRC}
)


##### Gui placer

# This library contains the basic placer for the gui library.
# The placer is used is some widgets to determine how to place its content.
set(wesnoth-gui_placer_SRC
	gui/auxiliary/placer.cpp
	gui/auxiliary/placer/horizontal_list.cpp
	gui/auxiliary/placer/vertical_list.cpp
)

add_library(wesnoth-gui_placer
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_placer_SRC}
)


##### GUI tooltip

# This library contains the tooltip handling code.
set(wesnoth-gui_tooltip_SRC
	gui/auxiliary/tips.cpp
	gui/dialogs/tip.cpp
)

add_library(wesnoth-gui_tooltip
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_tooltip_SRC}
)


##### GUI widget

# This library contains the widget code.
# It contains the core code of the wigets which determines the
# behavior of the widgets. It also contains the builder code
# for the widgets since the code has inter dependencies.
set(wesnoth-gui_widget_SRC
	gui/widgets/button.cpp
	gui/widgets/container.cpp
	gui/widgets/control.cpp
	gui/widgets/debug.cpp
	gui/widgets/drawing.cpp
	gui/widgets/generator.cpp
	gui/widgets/grid.cpp
	gui/widgets/helper.cpp
	gui/widgets/horizontal_scrollbar.cpp
	gui/widgets/image.cpp
	gui/widgets/label.cpp
	gui/widgets/list.cpp
	gui/widgets/listbox.cpp
	gui/widgets/matrix.cpp
	gui/widgets/minimap.cpp
	gui/widgets/multi_page.cpp
	gui/widgets/pane.cpp
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
	gui/widgets/viewport.cpp
	gui/widgets/widget.cpp
	gui/widgets/window.cpp
	gui/auxiliary/window_builder.cpp
	gui/auxiliary/window_builder/button.cpp
	gui/auxiliary/window_builder/control.cpp
	gui/auxiliary/window_builder/drawing.cpp
	gui/auxiliary/window_builder/helper.cpp
	gui/auxiliary/window_builder/horizontal_listbox.cpp
	gui/auxiliary/window_builder/horizontal_scrollbar.cpp
	gui/auxiliary/window_builder/instance.cpp
	gui/auxiliary/window_builder/image.cpp
	gui/auxiliary/window_builder/label.cpp
	gui/auxiliary/window_builder/listbox.cpp
	gui/auxiliary/window_builder/matrix.cpp
	gui/auxiliary/window_builder/minimap.cpp
	gui/auxiliary/window_builder/multi_page.cpp
	gui/auxiliary/window_builder/pane.cpp
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
	gui/auxiliary/window_builder/viewport.cpp
)

add_library(wesnoth-gui_widget
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_widget_SRC}
)

target_link_libraries(wesnoth-gui_widget
	wesnoth-gui_event
	wesnoth-gui_iterator
	wesnoth-gui_placer
	wesnoth-gui_tooltip
	wesnoth-gui_types
)


##### Gui widget definition

# This library contains the widget definitions for the gui library.
# The widget definitions serialise the widget definition WML code into
# its own structures. The definitions define the looks of the widgets.
set(wesnoth-gui_widget_definition_SRC
	gui/auxiliary/widget_definition.cpp
	gui/auxiliary/widget_definition/button.cpp
	gui/auxiliary/widget_definition/drawing.cpp
	gui/auxiliary/widget_definition/horizontal_scrollbar.cpp
	gui/auxiliary/widget_definition/image.cpp
	gui/auxiliary/widget_definition/label.cpp
	gui/auxiliary/widget_definition/listbox.cpp
	gui/auxiliary/widget_definition/matrix.cpp
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
)

add_library(wesnoth-gui_widget_definition
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui_widget_definition_SRC}
)


##### GUI1 widgets

# This library contains the widgets from gui1.
set(wesnoth-gui1_widgets_SRC
	widgets/combo.cpp
	widgets/combo_drag.cpp
	widgets/drop_target.cpp
	widgets/scrollpane.cpp
)

add_library(wesnoth-gui1_widgets
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-gui1_widgets_SRC}
)


##### GUI1 widgets

# This library contains the schema validator.
set(wesnoth-schema_validator_SRC
	serialization/schema_validator.cpp
	tools/schema/tag.cpp
)

add_library(wesnoth-schema_validator
   	${LIBRARY_TYPE}
	EXCLUDE_FROM_ALL
	${wesnoth-schema_validator_SRC}
)


########### Wesnoth main source files ###############

# Depending on whether the game or the game and the tests are build the main
# sources are compiled in the game library or not. If only one target is needed
# the putting in the game library solution is slower. If both targets are build
# the library is faster (especially if the user doesn't use ccache (or
# something similar)).

set(wesnoth-main_SRC
	about.cpp
	actions/attack.cpp
	actions/create.cpp
	actions/heal.cpp
	actions/move.cpp
	actions/undo.cpp
	actions/vision.cpp
	addon/client.cpp
	addon/info.cpp
	addon/manager.cpp
	addon/manager_ui.cpp
	addon/state.cpp
	addon/validation.cpp
	ai/actions.cpp
	ai/akihara/recruitment.cpp
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
	ai/lua/unit_advancements_aspect.cpp
	ai/manager.cpp
	ai/recruitment/recruitment.cpp
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
	config_assign.cpp
	controller_base.cpp
	desktop_util.cpp
	dialogs.cpp
	editor/action/action.cpp
	editor/action/action_item.cpp
	editor/action/action_label.cpp
	editor/action/action_unit.cpp
	editor/action/action_village.cpp
	editor/action/action_select.cpp
	editor/action/mouse/mouse_action.cpp
	editor/action/mouse/mouse_action_item.cpp
	editor/action/mouse/mouse_action_map_label.cpp
	editor/action/mouse/mouse_action_unit.cpp
	editor/action/mouse/mouse_action_village.cpp
	editor/action/mouse/mouse_action_select.cpp
	editor/map/editor_map.cpp
	editor/map/map_context.cpp
	editor/map/map_fragment.cpp
	editor/palette/palette_manager.cpp
	editor/palette/tristate_button.cpp
	editor/toolkit/brush.cpp
	editor/palette/editor_palettes.cpp
	editor/palette/item_palette.cpp
	editor/palette/terrain_palettes.cpp
	editor/palette/unit_palette.cpp
	editor/editor_controller.cpp
	editor/editor_display.cpp
	editor/editor_main.cpp
	editor/editor_preferences.cpp
	editor/toolkit/editor_toolkit.cpp
	editor/map/context_manager.cpp
	filechooser.cpp
	flg_manager.cpp
	floating_textbox.cpp
	formula.cpp
	formula_debugger.cpp
	formula_debugger_fwd.cpp
	formula_function.cpp
	formula_string_utils.cpp
	formula_tokenizer.cpp
	game_config_manager.cpp
	game_controller.cpp
	game_display.cpp
	game_errors.cpp
	game_events/action_wml.cpp
	game_events/conditional_wml.cpp
	game_events/entity_location.cpp
	game_events/handlers.cpp
	game_events/menu_item.cpp
	game_events/pump.cpp
	game_events/wmi_container.cpp
	game_preferences.cpp
	game_preferences_display.cpp
	gamestatus.cpp
	generic_event.cpp
	gui/auxiliary/canvas.cpp
	gui/auxiliary/log.cpp
	gui/auxiliary/old_markup.cpp
	gui/auxiliary/timer.cpp
	gui/dialogs/addon/description.cpp
	gui/dialogs/addon/filter_options.cpp
	gui/dialogs/addon/uninstall_list.cpp
	gui/dialogs/addon_connect.cpp
	gui/dialogs/addon_list.cpp
	gui/dialogs/campaign_difficulty.cpp
	gui/dialogs/campaign_selection.cpp
	gui/dialogs/core_selection.cpp
	gui/dialogs/chat_log.cpp
	gui/dialogs/data_manage.cpp
	gui/dialogs/debug_clock.cpp
	gui/dialogs/dialog.cpp
	gui/dialogs/edit_label.cpp
	gui/dialogs/editor/custom_tod.cpp
	gui/dialogs/editor/editor_edit_label.cpp
	gui/dialogs/editor/editor_edit_side.cpp
	gui/dialogs/editor/editor_edit_scenario.cpp
	gui/dialogs/editor_generate_map.cpp
	gui/dialogs/editor_new_map.cpp
	gui/dialogs/editor_resize_map.cpp
	gui/dialogs/editor_set_starting_position.cpp
	gui/dialogs/folder_create.cpp
	gui/dialogs/formula_debugger.cpp
	gui/dialogs/game_delete.cpp
	gui/dialogs/game_load.cpp
	gui/dialogs/game_paths.cpp
	gui/dialogs/game_save.cpp
	gui/dialogs/gamestate_inspector.cpp
	gui/dialogs/language_selection.cpp
	gui/dialogs/lobby/lobby_data.cpp
	gui/dialogs/lobby/lobby_info.cpp
	gui/dialogs/lobby_main.cpp
	gui/dialogs/lobby_player_info.cpp
	gui/dialogs/message.cpp
	gui/dialogs/mp_change_control.cpp
	gui/dialogs/mp_cmd_wrapper.cpp
	gui/dialogs/mp_connect.cpp
	gui/dialogs/mp_create_game.cpp
	gui/dialogs/mp_create_game_set_password.cpp
	gui/dialogs/mp_depcheck_confirm_change.cpp
	gui/dialogs/mp_depcheck_select_new.cpp
	gui/dialogs/mp_host_game_prompt.cpp
	gui/dialogs/mp_login.cpp
	gui/dialogs/mp_method_selection.cpp
	gui/dialogs/network_transmission.cpp
	gui/dialogs/popup.cpp
	gui/dialogs/edit_text.cpp
	gui/dialogs/screenshot_notification.cpp
	gui/dialogs/simple_item_selector.cpp
	gui/dialogs/theme_list.cpp
	gui/dialogs/title_screen.cpp
	gui/dialogs/transient_message.cpp
	gui/dialogs/unit_attack.cpp
	gui/dialogs/unit_create.cpp
	gui/dialogs/wml_error.cpp
	gui/dialogs/wml_message.cpp
	halo.cpp
	help.cpp
	hotkey/hotkey_preferences_display.cpp
	intro.cpp
	leader_scroll_dialog.cpp
	map.cpp
	map_location.cpp
	menu_events.cpp
	mouse_events.cpp
	mouse_handler_base.cpp
	movetype.cpp
	mp_depcheck.cpp
	mp_game_settings.cpp
	mp_game_utils.cpp
	mp_options.cpp
	multiplayer.cpp
	multiplayer_configure.cpp
	multiplayer_connect.cpp
	multiplayer_connect_engine.cpp
	multiplayer_create.cpp
	multiplayer_create_engine.cpp
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
	playturn_network_adapter.cpp
	portrait.cpp
	random_new.cpp
	random_new_deterministic.cpp
	random_new_synced.cpp
	replay.cpp
	replay_helper.cpp
	replay_controller.cpp
	resources.cpp
	save_blocker.cpp
	savegame.cpp
	scripting/debug_lua.cpp
	scripting/lua.cpp
	scripting/lua_api.cpp
	scripting/lua_types.cpp
	settings.cpp
	sha1.cpp
	side_filter.cpp
	statistics.cpp
	statistics_dialog.cpp
	storyscreen/controller.cpp
	storyscreen/interface.cpp
	storyscreen/part.cpp
	storyscreen/render.cpp
	strftime.cpp
	synced_checkup.cpp
	synced_context.cpp
	synced_commands.cpp
	team.cpp
	terrain_filter.cpp
	tod_manager.cpp
	tooltips.cpp
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
	whiteboard/highlighter.cpp
	whiteboard/manager.cpp
	whiteboard/mapbuilder.cpp
	whiteboard/move.cpp
	whiteboard/recall.cpp
	whiteboard/recruit.cpp
	whiteboard/side_actions.cpp
	whiteboard/suppose_dead.cpp
	whiteboard/utility.cpp
	${network_implementation_files} # network.cpp and network_worker.cpp are included by default (without USE_ANA_NETWORK)
)

########### libwesnoth-game ###############
# rename libwesnoth.a to libwesnoth-game.a to have clearer targets

set(libwesnoth-game_STAT_SRC
	arrow.cpp
	builder.cpp
	clipboard.cpp
	construct_dialog.cpp
	cursor.cpp
	display.cpp
	events.cpp
	font.cpp
	format_time_summary.cpp
  	generators/cavegen.cpp
	generators/map_create.cpp
	generators/mapgen.cpp
	generators/mapgen_dialog.cpp
	generators/yamg/ya_mapgen.cpp
	generators/yamg/yamg_hex.cpp
	generators/yamg/yamg_hexheap.cpp
	generators/yamg/yamg_params.cpp
	hotkey/command_executor.cpp
	hotkey/hotkey_item.cpp
	hotkey/hotkey_command.cpp
	hotkey/hotkey_manager.cpp
	image.cpp
	image_modifications.cpp
	joystick.cpp
	game_end_exceptions.cpp
	key.cpp
	language.cpp
	loadscreen.cpp
	lobby_preferences.cpp
	map_label.cpp
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
	simple_rng.cpp
	sound.cpp
	soundsource.cpp
	sound_music_track.cpp
	terrain.cpp
	terrain_translation.cpp
	text.cpp
	time_of_day.cpp
	tracer.cpp
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

# On windows only, this file needs to be linked, as its header is included #ifdef win32.
if(MSVC)
	set(libwesnoth-game_STAT_SRC
		${libwesnoth-game_STAT_SRC}
		windows_tray_notification.cpp
	)
endif(MSVC)

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
			game.cpp
		)
		set(wesnoth_LIB
			wesnoth-game
			wesnoth-gui_types
			wesnoth-gui_event
			wesnoth-gui_iterator
			wesnoth-gui_placer
			wesnoth-gui_tooltip
			wesnoth-gui_widget
			wesnoth-gui_widget_definition
			wesnoth-gui1_widgets
			wesnoth-schema_validator
		)
	else(ENABLE_TESTS)
		set(wesnoth_SRC
			game.cpp
			${wesnoth-gui_types_SRC}
			${wesnoth-gui_event_SRC}
			${wesnoth-gui_iterator_SRC}
			${wesnoth-gui_placer_SRC}
			${wesnoth-gui_tooltip_SRC}
			${wesnoth-gui_widget_SRC}
			${wesnoth-gui_widget_definition_SRC}
			${wesnoth-gui1_widgets_SRC}
			${wesnoth-schema_validator_SRC}
			${wesnoth-main_SRC}
		)
		set(wesnoth_LIB
			wesnoth-core
			wesnoth-lua
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

if(ENABLE_SERVER)

set(wesnothd_SRC
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
	${network_implementation_files}
	loadscreen_empty.cpp
)

add_executable(wesnothd WIN32
	${wesnothd_SRC}
)

target_link_libraries(wesnothd wesnoth-core ${server-external-libs})
set_target_properties(wesnothd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesnothd${BINARY_SUFFIX})

install(TARGETS wesnothd DESTINATION ${BINDIR})

endif(ENABLE_SERVER)

########### Campaign Server ###############

if(ENABLE_CAMPAIGN_SERVER)

set(campaignd_SRC
	network_worker.cpp # NEEDED when compiling with ANA support
	addon/validation.cpp
	campaign_server/campaign_server.cpp
	server/input_stream.cpp
	${network_implementation_files}
	loadscreen_empty.cpp
)

add_executable(campaignd ${campaignd_SRC})
target_link_libraries(campaignd
	wesnoth-core
	${server-external-libs}
	${SDLNET_LIBRARY} 				# NEEDED with ANA
	${LIBINTL_LIBRARIES}
	)
set_target_properties(campaignd PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}campaignd${BINARY_SUFFIX})

install(TARGETS campaignd DESTINATION ${BINDIR})

endif(ENABLE_CAMPAIGN_SERVER)

########### Castle building helpers ###############

if(ENABLE_TOOLS)

set(exploder_SRC
	tools/exploder.cpp
	tools/exploder_utils.cpp
	tools/exploder_cutter.cpp
	tools/exploder_composer.cpp
	tools/dummy_video.cpp
	sdl_utils.cpp
	tracer.cpp
	loadscreen_empty.cpp
)

add_executable(exploder ${exploder_SRC})
target_link_libraries(exploder wesnoth-core png ${tools-external-libs})
set_target_properties(exploder PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}exploder${BINARY_SUFFIX})

install(TARGETS exploder DESTINATION ${BINDIR})

set(cutter_SRC
	tools/cutter.cpp
	tools/exploder_utils.cpp
	tools/exploder_cutter.cpp
	tools/dummy_video.cpp
	sdl_utils.cpp
	tracer.cpp
	loadscreen_empty.cpp
)

add_executable(cutter ${cutter_SRC})
target_link_libraries(cutter wesnoth-core png ${tools-external-libs})
set_target_properties(cutter PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}cutter${BINARY_SUFFIX})

install(TARGETS cutter DESTINATION ${BINDIR})

set(schema_generator_SRC
	tools/schema/schema_generator.cpp
	tools/schema/sourceparser.cpp
	tools/schema/error_container.cpp
	tools/schema/tag.cpp
	filesystem.cpp
	loadscreen_empty.cpp
)

add_executable(schema_generator ${schema_generator_SRC})
target_link_libraries(schema_generator wesnoth-core ${tools-external-libs} ${Boost_REGEX_LIBRARY})
set_target_properties(schema_generator PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}schema_generator${BINARY_SUFFIX})

install(TARGETS schema_generator DESTINATION ${BINDIR})

set(schema_validator_SRC
	tools/validator/validator_tool.cpp
	serialization/schema_validator.cpp
	serialization/validator.cpp
	tools/schema/tag.cpp
	filesystem.cpp
	config_cache.cpp
	sha1.cpp
	loadscreen_empty.cpp
)

add_executable(schema_validator ${schema_validator_SRC})
target_link_libraries(schema_validator wesnoth-core ${common-external-libs})
set_target_properties(schema_validator PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}schema_validator${BINARY_SUFFIX}
							COMPILE_DEFINITIONS VALIDATION_ERRORS_LOG)

install(TARGETS schema_validator DESTINATION ${BINDIR})

set(wesmage_SRC
	wesmage/wesmage.cpp
	wesmage/exit.cpp
	wesmage/filter.cpp
	wesmage/options.cpp
	tools/dummy_video.cpp
	tools/exploder_utils.cpp
	sdl_utils.cpp
	tracer.cpp
	loadscreen_empty.cpp
)

add_executable(wesmage ${wesmage_SRC})
target_link_libraries(wesmage wesnoth-core png ${tools-external-libs})
set_target_properties(wesmage PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}wesmage${BINARY_SUFFIX})

install(TARGETS wesmage DESTINATION ${BINDIR})

endif(ENABLE_TOOLS)


########### SDL2 test tool  ###############

if(ENABLE_SDL2_TOOLS)

	add_executable(sdl2
		tools/sdl2/sdl2.cpp
		tools/sdl2/window.cpp
	)

	target_link_libraries(sdl2
		wesnoth-sdl
		wesnoth-core
		wesnoth-game
		wesnoth-gui_types
		wesnoth-gui_event
		wesnoth-gui_iterator
		wesnoth-gui_placer
		wesnoth-gui_tooltip
		wesnoth-gui_widget
		wesnoth-gui_widget_definition
		wesnoth-gui1_widgets
		wesnoth-schema_validator
		wesnoth-lua
		${game-external-libs}
	)

endif()

########### Unit tests ###############

if(ENABLE_TESTS)

	add_definitions(-DBOOST_TEST_DYN_LINK)

	set(test_SRC
		tests/floating_point_emulation.cpp
		tests/main.cpp
		tests/utils/fake_display.cpp
		tests/utils/fake_event_source.cpp
		tests/utils/game_config_manager.cpp
		tests/utils/play_scenario.cpp
		tests/test_addons.cpp
		tests/test_commandline_options.cpp
		tests/test_config_cache.cpp
		tests/test_formula_ai.cpp
		tests/test_formula_function.cpp
		tests/test_image_modifications.cpp
		tests/test_lexical_cast.cpp
		tests/test_network_worker.cpp
		tests/test_mp_connect.cpp
		tests/test_team.cpp
		tests/test_unit_map.cpp
		tests/test_util.cpp
		tests/test_sdl_utils.cpp
		tests/test_serialization.cpp
		tests/test_version.cpp
		tests/test_whiteboard_side_actions.cpp
		tests/gui/iterator.cpp
		tests/gui/fire_event.cpp
		tests/gui/test_drop_target.cpp
		tests/gui/test_gui2.cpp
		tests/gui/visitor.cpp
		tests/gui/test_save_dialog.cpp
	)
	if(ENABLE_GAME)
		set(test_LIB
			wesnoth-core
			wesnoth-game
			wesnoth-gui_types
			wesnoth-gui_event
			wesnoth-gui_iterator
			wesnoth-gui_placer
			wesnoth-gui_tooltip
			wesnoth-gui_widget
			wesnoth-gui_widget_definition
			wesnoth-gui1_widgets
			wesnoth-schema_validator
			wesnoth-lua
		)
	else(ENABLE_GAME)
		set(test_SRC
			${test_SRC}
			${wesnoth-gui_types_SRC}
			${wesnoth-gui_event_SRC}
			${wesnoth-gui_iterator_SRC}
			${wesnoth-gui_placer_SRC}
			${wesnoth-gui_widget_definition_SRC}
			${wesnoth-gui_tooltip_SRC}
			${wesnoth-gui_widget_SRC}
			${wesnoth-gui1_widgets_SRC}
			${wesnoth-schema_validator_SRC}
			${wesnoth-main_SRC}
		)
		set(test_LIB
			wesnoth-core
			wesnoth-game
			wesnoth-lua
		)
	endif(ENABLE_GAME)

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

	if(ENABLE_TOOLS)
		# This tool is used to create the images for the sdl_utils unit test.
		# Due to its unique nature the program is never installed.
		set(create_images_SRC
			tests/create_images.cpp
			tools/dummy_video.cpp
			tools/exploder_utils.cpp
			sdl_utils.cpp
			tracer.cpp
			loadscreen_empty.cpp
		)

		add_executable(create_images ${create_images_SRC})
		target_link_libraries(create_images wesnoth-core png ${tools-external-libs})
		set_target_properties(create_images PROPERTIES OUTPUT_NAME ${BINARY_PREFIX}create_images${BINARY_SUFFIX})
	endif(ENABLE_TOOLS)

endif(ENABLE_TESTS)

