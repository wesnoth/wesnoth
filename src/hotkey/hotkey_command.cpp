/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "hotkey/hotkey_command.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"

#include <array>
#include <set>
#include <string_view>

static lg::log_domain log_config("config");
#define ERR_G LOG_STREAM(err, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define DBG_G LOG_STREAM(debug, lg::general())
#define ERR_CF LOG_STREAM(err, log_config)

namespace hotkey
{
struct hotkey_command_temp
{
	HOTKEY_COMMAND command;
	std::string_view id;
	std::string_view description;
	bool hidden;
	hk_scopes scope;
	HOTKEY_CATEGORY category;
	std::string_view tooltip;
};

namespace
{
const std::map<HOTKEY_CATEGORY, std::string> category_names {
	{ HKCAT_GENERAL,    N_("General") },
	{ HKCAT_SAVING,     N_("Saved Games") },
	{ HKCAT_MAP,        N_("Map Commands") },
	{ HKCAT_UNITS,      N_("Unit Commands") },
	{ HKCAT_CHAT,       N_("Player Chat") },
	{ HKCAT_REPLAY,     N_("Replay Control") },
	{ HKCAT_WHITEBOARD, N_("Planning Mode") },
	{ HKCAT_SCENARIO,   N_("Scenario Editor") },
	{ HKCAT_PALETTE,    N_("Editor Palettes") },
	{ HKCAT_TOOLS,      N_("Editor Tools") },
	{ HKCAT_CLIPBOARD,  N_("Editor Clipboard") },
	{ HKCAT_DEBUG,      N_("Debug Commands") },
	{ HKCAT_CUSTOM,     N_("Custom WML Commands") },
};

//
// All static hotkeys.
//
// This array should always have the same number of entries as the HOTKEY_COMMAND enum.
// Since HOTKEY_NULL is the last entry in said enum, we can use its index to dynamically
// size the array.
//
constexpr std::array<hotkey_command_temp, HOTKEY_NULL - 1> master_hotkey_list {{
	{ HOTKEY_SCROLL_UP, "scroll-up", N_("Scroll Up"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_SCROLL_DOWN, "scroll-down", N_("Scroll Down"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_SCROLL_LEFT, "scroll-left", N_("Scroll Left"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_SCROLL_RIGHT, "scroll-right", N_("Scroll Right"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },

	{ HOTKEY_CANCEL, N_("cancel"), N_("Cancel"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_SELECT_HEX, "selecthex", N_("Select Hex"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_DESELECT_HEX, "deselecthex", N_("Deselect Hex"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_MOVE_ACTION, "moveaction", N_("Move/Attack"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_SELECT_AND_ACTION, "selectmoveaction", N_("Select/Move/Attack"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_TOUCH_HEX, "touchhex", N_("Touch"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_ANIMATE_MAP, "animatemap", N_("Animate Map"), false, scope_game | scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_CYCLE_UNITS, "cycle", N_("Next Unit"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_CYCLE_BACK_UNITS, "cycleback", N_("Previous Unit"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_UNIT_HOLD_POSITION, "holdposition", N_("Hold Position"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_END_UNIT_TURN, "endunitturn", N_("End Unit Turn"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_LEADER, "leader", N_("Scroll to Leader"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_UNDO, "undo", N_("Undo"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_REDO, "redo", N_("Redo"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_ZOOM_IN, "zoomin", N_("Zoom In"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_ZOOM_OUT, "zoomout", N_("Zoom Out"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_ZOOM_DEFAULT, "zoomdefault", N_("Default Zoom"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_FULLSCREEN, "fullscreen", N_("Toggle Full Screen"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_ACHIEVEMENTS, "achievements", N_("Achievements"), false, scope_game | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_SCREENSHOT, "screenshot", N_("Screenshot"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_MAP_SCREENSHOT, "mapscreenshot", N_("Map Screenshot"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_ACCELERATED, "accelerated", N_("Toggle Accelerated Speed"), false, scope_game, HKCAT_GENERAL, "" },
	{ HOTKEY_TERRAIN_DESCRIPTION, "describeterrain", N_("Terrain Description"), false, scope_game | scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_UNIT_DESCRIPTION, "describeunit", N_("Unit Type Description"), false, scope_game | scope_editor, HKCAT_UNITS, "" },
	{ HOTKEY_RENAME_UNIT, "renameunit", N_("Rename Unit"), false, scope_game | scope_editor, HKCAT_UNITS, "" },
	{ HOTKEY_DELETE_UNIT, "editor-deleteunit", N_("Delete Unit"), false, scope_game | scope_editor, HKCAT_TOOLS, "" },

	{ HOTKEY_SAVE_GAME, "save", N_("Save Game"), false, scope_game, HKCAT_SAVING, "" },
	{ HOTKEY_SAVE_REPLAY, "savereplay", N_("Save Replay"), false, scope_game, HKCAT_SAVING, "" },
	{ HOTKEY_SAVE_MAP, "savemap", N_("Save Map"), false, scope_game, HKCAT_SAVING, "" },
	{ HOTKEY_LOAD_GAME, "load", N_("Load Game"), false, scope_game | scope_main, HKCAT_SAVING, "" },
	{ HOTKEY_LOAD_AUTOSAVES, "menu-autosaves", N_("Load Turn..."), true, scope_game, HKCAT_PLACEHOLDER, "" }, // Menu placeholder
	{ HOTKEY_RECRUIT, "recruit", N_("Recruit"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_REPEAT_RECRUIT, "repeatrecruit", N_("Repeat Recruit"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_RECALL, "recall", N_("Recall"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_LABEL_SETTINGS, "label_settings", N_("Show/Hide Labels"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_ENDTURN, "endturn", N_("End Turn"), false, scope_game, HKCAT_GENERAL, "" },
	//TODO: why has HOTKEY_TOGGLE_ELLIPSES more than scope_game ?
	{ HOTKEY_TOGGLE_ELLIPSES, "toggleellipses", N_("Toggle Ellipses"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_TOGGLE_GRID, "togglegrid", N_("Toggle Grid"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_MOUSE_SCROLL, "mousescroll", N_("Mouse Scrolling"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_STATUS_TABLE, "statustable", N_("Status Table"), false, scope_game, HKCAT_GENERAL, "" },
	{ HOTKEY_MUTE, "mute", N_("Mute"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_SPEAK, "speak", N_("Speak"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_CREATE_UNIT, "createunit", N_("Create Unit (Debug!)"), false, scope_game, HKCAT_DEBUG, "" },
	{ HOTKEY_CHANGE_SIDE, "changeside", N_("Change Side (Debug!)"), false, scope_game, HKCAT_DEBUG, "" },
	{ HOTKEY_KILL_UNIT, "killunit", N_("Kill Unit (Debug!)"), false, scope_game, HKCAT_DEBUG, "" },
	{ HOTKEY_TELEPORT_UNIT, "teleportunit", N_("Teleport Unit (Debug!)"), false, scope_game, HKCAT_DEBUG, "" },
	{ HOTKEY_PREFERENCES, "preferences", N_("Preferences"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_OBJECTIVES, "objectives", N_("Objectives"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_UNIT_LIST, "unitlist", N_("Unit List"), false, scope_game | scope_editor, HKCAT_UNITS, "" },
	{ HOTKEY_STATISTICS, "statistics", N_("Statistics"), false, scope_game, HKCAT_GENERAL, "" },
	{ HOTKEY_STOP_NETWORK, "stopnetwork", N_("Pause Network Game"), false, scope_game, HKCAT_GENERAL, "" },
	{ HOTKEY_START_NETWORK, "startnetwork", N_("Continue Network Game"), false, scope_game, HKCAT_GENERAL, "" },
	{ HOTKEY_SURRENDER, "surrender", N_("Surrender Game"), false, scope_game, HKCAT_SCENARIO, "" },
	{ HOTKEY_QUIT_GAME, "quit", N_("Quit to Menu"), false, scope_game | scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_LABEL_TEAM_TERRAIN, "labelteamterrain", N_("Set Team Label"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_LABEL_TERRAIN, "labelterrain", N_("Set Label"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_CLEAR_LABELS, "clearlabels", N_("Clear Labels"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", N_("Show Enemy Moves"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", N_("Best Possible Enemy Moves"), false, scope_game, HKCAT_UNITS, "" },

	{ HOTKEY_REPLAY_PLAY, "playreplay", N_("Play Replay"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_RESET, "resetreplay", N_("Reset Replay"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_STOP, "stopreplay", N_("Stop Replay"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_NEXT_TURN, "replaynextturn", N_("Next Turn"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_NEXT_SIDE, "replaynextside", N_("Next Side"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_NEXT_MOVE, "replaynextmove", N_("Next Move"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_SHOW_EVERYTHING, "replayshoweverything", N_("View: Full Map"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_SHOW_EACH, "replayshoweach", N_("View: Each Team"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_SHOW_TEAM1, "replayshowteam1", N_("View: Human Team"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_SKIP_ANIMATION, "replayskipanimation", N_("Skip Animation"), false, scope_game, HKCAT_REPLAY, "" },
	{ HOTKEY_REPLAY_EXIT, "replayexit", N_("End Replay"), false, scope_game, HKCAT_REPLAY, "" },
	// Whiteboard commands
	// TRANSLATORS: whiteboard menu entry: toggle planning mode
	{ HOTKEY_WB_TOGGLE, "wbtoggle", N_("whiteboard^Planning Mode"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: execute planned action
	{ HOTKEY_WB_EXECUTE_ACTION, "wbexecuteaction", N_("whiteboard^Execute Action"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: execute all planned actions
	{ HOTKEY_WB_EXECUTE_ALL_ACTIONS, "wbexecuteallactions", N_("whiteboard^Execute All Actions"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: delete planned action
	{ HOTKEY_WB_DELETE_ACTION, "wbdeleteaction", N_("whiteboard^Delete Action"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: move planned action up queue
	{ HOTKEY_WB_BUMP_UP_ACTION, "wbbumpupaction", N_("whiteboard^Move Action Up"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: move planned action down queue
	{ HOTKEY_WB_BUMP_DOWN_ACTION, "wbbumpdownaction", N_("whiteboard^Move Action Down"), false, scope_game, HKCAT_WHITEBOARD, "" },
	// TRANSLATORS: whiteboard menu entry: plan as though the chosen unit were dead
	{ HOTKEY_WB_SUPPOSE_DEAD, "wbsupposedead", N_("whiteboard^Suppose Dead"), true, scope_game, HKCAT_WHITEBOARD, "" },

	{ HOTKEY_QUIT_TO_DESKTOP, "quit-to-desktop", N_("Quit to Desktop"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_EDITOR_MAP_CLOSE, "editor-close-map", N_("Close Map"), false, scope_editor, HKCAT_GENERAL, "" },

	// These are not really hotkey items but menu entries to get expanded.
	// They need to have their own hotkey to control their active state.
	{ HOTKEY_EDITOR_PLAYLIST, "editor-playlist", N_("Switch Time of Day"), true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_SCHEDULE, "menu-editor-schedule", "", true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_MAP_SWITCH, "editor-switch-map", N_("Switch Map"), true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_LOCAL_TIME, "menu-editor-local-time", N_("Assign Local Time"), true, scope_editor, HKCAT_PLACEHOLDER, "" },

	{ HOTKEY_EDITOR_EDIT_UNIT, "editor-edit-unit", N_("New Unit Type"), true, scope_editor, HKCAT_PLACEHOLDER, "" },

	{ HOTKEY_EDITOR_CUSTOM_TODS, "editor-custom-tods", N_("Time Schedule Editor"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_PARTIAL_UNDO, "editor-partial-undo", N_("Partial Undo"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_MAP_NEW, "editor-map-new", N_("New Map"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_SCENARIO_NEW, "editor-scenario-new", N_("New Scenario"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_MAP_LOAD, "editor-map-load", N_("Load Map/Scenario"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_SAVE, "editor-map-save", N_("Save"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_SAVE_AS, "editor-map-save-as", N_("Save Map As"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_SCENARIO_SAVE_AS, "editor-scenario-save-as", N_("Save Scenario As"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_MAP_SAVE_ALL, "editor-map-save-all", N_("Save All Maps"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_REVERT, "editor-map-revert", N_("Revert All Changes"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_INFO, "editor-map-info", N_("Map Information"), false, scope_editor, HKCAT_MAP, "" },

	{ HOTKEY_EDITOR_PALETTE_ITEMS_CLEAR, "editor-palette-items-clear", N_("Clear Selected Item Set"), false, scope_editor, HKCAT_PALETTE, "" },
	{ HOTKEY_EDITOR_PALETTE_ITEM_SWAP, "editor-terrain-palette-swap", N_("Swap Foreground/Background Palette Item"), false, scope_editor, HKCAT_PALETTE, "" },
	{ HOTKEY_EDITOR_PALETTE_GROUPS, "editor-palette-groups", N_("Change Palette Group"), false, scope_editor, HKCAT_PALETTE, "" },
	{ HOTKEY_EDITOR_PALETTE_UPSCROLL, "editor-palette-upscroll", N_("Scroll Palette Left"), false, scope_editor, HKCAT_PALETTE, "" },
	{ HOTKEY_EDITOR_PALETTE_DOWNSCROLL, "editor-palette-downscroll", N_("Scroll Palette Right"), false, scope_editor, HKCAT_PALETTE, "" },
	{ HOTKEY_EDITOR_REMOVE_LOCATION, "editor-remove-location", N_("Remove Location"), false, scope_editor, HKCAT_PALETTE, "" },

	{ HOTKEY_EDITOR_SIDE_NEW, "editor-side-new", N_("Add New Side"), false, scope_editor, HKCAT_SCENARIO, "" },

	{ HOTKEY_EDITOR_TOOL_NEXT, "editor-tool-next", N_("Next Tool"), false, scope_editor, HKCAT_TOOLS, "" },

	{ HOTKEY_EDITOR_TOOL_PAINT, "editor-tool-paint", N_("Paint Tool"), false, scope_editor, HKCAT_TOOLS, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ HOTKEY_EDITOR_TOOL_FILL, "editor-tool-fill", N_("Fill Tool"), false, scope_editor, HKCAT_TOOLS, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ HOTKEY_EDITOR_TOOL_SELECT, "editor-tool-select", N_("Selection Tool"), false, scope_editor, HKCAT_TOOLS, N_("Left mouse button selects or deselects with Ctrl, right brings up a context menu. Hold Shift for magic-wand selection of tiles with same terrain.") },
	{ HOTKEY_EDITOR_TOOL_STARTING_POSITION, "editor-tool-starting-position", N_("Starting Positions Tool"), false, scope_editor, HKCAT_TOOLS,  N_("Left mouse button displays player selection, right clears. Number keys scroll to the starting position, alt+number sets respective starting position under cursor, delete clears.") },
	{ HOTKEY_EDITOR_TOOL_LABEL, "editor-tool-label", N_("Label Tool"), false, scope_editor, HKCAT_TOOLS, N_("Left mouse button sets or drags a label, right clears.") },
	{ HOTKEY_EDITOR_TOOL_UNIT, "editor-tool-unit", N_("Unit Tool"), false, scope_editor, HKCAT_TOOLS, N_("Left mouse button sets a new unit or moves a unit via drag and drop, right brings up a context menu. Needs a defined side.") },
	{ HOTKEY_EDITOR_TOOL_ITEM, "editor-tool-item", N_("Item Tool"), false, scope_editor, HKCAT_TOOLS, N_("Left mouse button sets a new item. Right click removes item.") },
	{ HOTKEY_EDITOR_TOOL_VILLAGE, "editor-tool-village", N_("Village Tool"), false, scope_editor, HKCAT_TOOLS, N_("Left mouse button sets the village ownership to the current side, right clears. Needs a defined side.") },

	{ HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT, "editor-toggle-canrecruit", N_("Can Recruit"), false, scope_editor, HKCAT_TOOLS, N_("Toggle the recruit attribute of a unit.") },
	{ HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE, "editor-toggle-renameable", N_("Can be Renamed"), false, scope_editor, HKCAT_TOOLS, N_("Toggle the unit being renameable.") },

	{ HOTKEY_EDITOR_UNIT_CHANGE_ID, "editor-change-unitid", N_("Change Unit ID"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL, "editor-unit-toggle-loyal", N_("Loyal"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_UNIT_FACING, "menu-unit-facing", "", true, scope_editor, HKCAT_PLACEHOLDER, "" },

	{ HOTKEY_EDITOR_HELP_TEXT_SHOWN, "editor-help-text-shown", N_("Show Tool Information"), false, scope_editor, HKCAT_TOOLS, "" },

	{ HOTKEY_MINIMAP_CODING_UNIT, "minimap-unit-coding", N_("Toggle Minimap Unit Coding"), false, scope_game | scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_MINIMAP_CODING_TERRAIN, "minimap-terrain-coding", N_("Toggle Minimap Terrain Coding"), false, scope_game | scope_editor, HKCAT_MAP, "" },

	{ HOTKEY_MINIMAP_DRAW_UNITS, "minimap-draw-units", N_("Toggle Minimap Unit Drawing"), false, scope_game | scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_MINIMAP_DRAW_VILLAGES, "minimap-draw-villages", N_("Toggle Minimap Village Drawing"), false, scope_game | scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_MINIMAP_DRAW_TERRAIN, "minimap-draw-terrain", N_("Toggle Minimap Terrain Drawing"), false, scope_game | scope_editor, HKCAT_MAP, "" },

	{ HOTKEY_EDITOR_BRUSH_NEXT, "editor-brush-next", N_("Next Brush"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_DEFAULT, "editor-brush-default", N_("Default Brush"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_1, "editor-brush-1", N_("Single Tile"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_2, "editor-brush-2", N_("Radius One"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_3, "editor-brush-3", N_("Radius Two"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_NW_SE, "editor-brush-nw-se", N_("Brush NW-SE"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_BRUSH_SW_NE, "editor-brush-sw-ne", N_("Brush SW-NE"), false, scope_editor, HKCAT_TOOLS, "" },

	{ HOTKEY_EDITOR_SELECTION_CUT, "editor-cut", N_("Cut"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_SELECTION_COPY, "editor-copy", N_("Copy"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_CLIPBOARD_PASTE, "editor-paste", N_("Paste"), false, scope_editor, HKCAT_CLIPBOARD, N_("Left mouse button pastes from the clipboard, right brings up a context menu.") },
	{ HOTKEY_EDITOR_SELECTION_EXPORT, "editor-export-selection-coords", N_("Export Selected Coordinates to System Clipboard"), true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_SELECT_ALL, "editor-select-all", N_("Select All"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_SELECT_INVERSE, "editor-select-inverse", N_("Select Inverse"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_SELECT_NONE, "editor-select-none", N_("Select None"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, "editor-clipboard-rotate-cw", N_("Rotate Clipboard Clockwise"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW, "editor-clipboard-rotate-ccw", N_("Rotate Clipboard Counter-Clockwise"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, "editor-clipboard-flip-horizontal", N_("Flip Clipboard Horizontally"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL, "editor-clipboard-flip-vertical", N_("Flip Clipboard Vertically"), false, scope_editor, HKCAT_CLIPBOARD, "" },
	{ HOTKEY_EDITOR_SELECTION_ROTATE, "editor-selection-rotate", N_("Rotate Selection"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_SELECTION_FLIP, "editor-selection-flip", N_("Flip Selection"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_SELECTION_FILL, "editor-selection-fill", N_("Fill Selection"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_SELECTION_RANDOMIZE, "editor-selection-randomize", N_("Randomize Tiles in Selection"), false, scope_editor, HKCAT_TOOLS, "" },
	{ HOTKEY_EDITOR_MAP_RESIZE, "editor-map-resize", N_("Resize Map"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_GENERATE, "editor-map-generate", N_("Generate Map"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_APPLY_MASK, "editor-map-apply-mask", N_("Apply a Mask"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_MAP_CREATE_MASK_TO, "editor-map-create-mask-to", N_("Create Mask"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_REFRESH, "editor-refresh", N_("Refresh Display"), false, scope_editor, HKCAT_GENERAL, "" },

	{ HOTKEY_EDITOR_UPDATE_TRANSITIONS, "editor-update-transitions", N_("Update Terrain Transitions"), false, scope_editor, HKCAT_MAP, "" },

	// This item is for binding in the preferences
	{ HOTKEY_EDITOR_TOGGLE_TRANSITIONS, "editor-toggle-transitions", N_("Toggle Terrain Transition Update"), false, scope_editor, HKCAT_MAP, "" },
	// The next three are for displaying the different states in the menu
	{ HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, "editor-auto-update-transitions", N_("Auto-update Terrain Transitions"), true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS, "editor-no-update-transitions", N_("Auto-update Terrain Transitions: No"), true, scope_editor, HKCAT_PLACEHOLDER, "" },
	{ HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS, "editor-partial-update-transitions", N_("Auto-update Terrain Transitions: Partial"), true, scope_editor, HKCAT_PLACEHOLDER, "" },

	{ HOTKEY_EDITOR_REFRESH_IMAGE_CACHE, "editor-refresh-image-cache", N_("Refresh Image Cache"), false, scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_EDITOR_DRAW_COORDINATES, "editor-draw-coordinates", N_("Draw Hex Coordinates"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_DRAW_TERRAIN_CODES, "editor-draw-terrain-codes", N_("Draw Terrain Codes"), false, scope_editor, HKCAT_MAP, "" },
	{ HOTKEY_EDITOR_DRAW_NUM_OF_BITMAPS, "editor-draw-num-of-bitmaps", N_("Draw Number of Bitmaps"), false, scope_editor, HKCAT_MAP, "" },

	{ HOTKEY_EDITOR_AREA_SAVE,   "editor-save-area",   N_("Save Selection to Area"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_AREA_RENAME, "editor-rename-area", N_("Rename Selected Area"),   false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_AREA_REMOVE, "editor-remove-area", N_("Remove Selected Area"),   false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_AREA_ADD,    "editor-add-area",    N_("Add New Area"),           false, scope_editor, HKCAT_SCENARIO, "" },

	{ HOTKEY_EDITOR_PBL,             "editor-pbl",      N_("Add-on Publishing Editor"),   false, scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_EDITOR_CHANGE_ADDON_ID, "editor-addon-id", N_("Change Add-on ID"),           false, scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_EDITOR_SELECT_ADDON, "editor-addon-select", N_("Select active Add-on"),           false, scope_editor, HKCAT_GENERAL, "" },
	{ HOTKEY_EDITOR_OPEN_ADDON, "editor-addon-open", N_("Open Add-on folder"),           false, scope_editor, HKCAT_GENERAL, "" },

	{ HOTKEY_EDITOR_SCENARIO_EDIT, "editor-scenario-edit", N_("Edit Scenario Settings"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_SIDE_EDIT, "editor-side-edit", N_("Edit Side"), false, scope_editor, HKCAT_SCENARIO, "" },
	{ HOTKEY_EDITOR_SIDE_REMOVE, "editor-side-remove", N_("Remove Side"), false, scope_editor, HKCAT_SCENARIO, "" },

	{ HOTKEY_DELAY_SHROUD, "delayshroud", N_("Delay Shroud Updates"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_UPDATE_SHROUD, "updateshroud", N_("Update Shroud Now"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_CONTINUE_MOVE, "continue", N_("Continue Interrupted Move"), false, scope_game, HKCAT_UNITS, "" },
	{ HOTKEY_SEARCH, "search", N_("Find Label or Unit"), false, scope_game, HKCAT_MAP, "" },
	{ HOTKEY_SPEAK_ALLY, "speaktoally", N_("Speak to Ally"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_SPEAK_ALL, "speaktoall", N_("Speak to All"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_HELP, "help", N_("Help"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_HELP_ABOUT_SAVELOAD, "help-about-saveload", N_("Help about save-loading"), false, scope_game, HKCAT_GENERAL, N_("Hint: save-loading is unnecessary")},
	{ HOTKEY_CHAT_LOG, "chatlog", N_("Chat Log"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_USER_CMD, "command", N_("Enter Command"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_CUSTOM_CMD, "customcommand", N_("Custom Command"), false, scope_game, HKCAT_CHAT, "" },
	{ HOTKEY_AI_FORMULA, "aiformula", N_("Run Formula"), false, scope_game, HKCAT_DEBUG, "" },
	{ HOTKEY_CLEAR_MSG, "clearmessages", N_("Clear Chat"), false, scope_game, HKCAT_CHAT, "" },

	{ HOTKEY_LANGUAGE, "changelanguage", N_("Change Language"), false, scope_main, HKCAT_GENERAL, "" },
	{ HOTKEY_MP_START_GAME, "mp_startgame", N_("Start Game (MP)"), false, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__RELOAD_WML, "title_screen__reload_wml", N_("Refresh WML"), true , scope_editor | scope_main, HKCAT_PLACEHOLDER, "" },
	{ TITLE_SCREEN__NEXT_TIP, "title_screen__next_tip", N_("Next Tip of the Day"), false, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__PREVIOUS_TIP, "title_screen__previous_tip", N_("Previous Tip of the Day"), false, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__CAMPAIGN, "title_screen__campaign", N_("Start Campaign"), false	, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__MULTIPLAYER, "title_screen__multiplayer", N_("Start Multiplayer Game"), false, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__ADDONS, "title_screen__addons", N_("Manage Add-ons"), false	, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__CORES, "title_screen__cores", N_("Manage Cores"), false	, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__EDITOR, "title_screen__editor", N_("Start Editor"), false, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__CREDITS, "title_screen__credits", N_("Show Credits"), false	, scope_main, HKCAT_GENERAL, "" },
	{ TITLE_SCREEN__TEST, "title_screen__test", N_("Choose Test Scenario"), false	, scope_main, HKCAT_GENERAL, "" },

	{ GLOBAL__HELPTIP, "global__helptip", N_("Show Helptip"), false, scope_game | scope_editor | scope_main, HKCAT_GENERAL, "" },

	{ LUA_CONSOLE, "global__lua__console", N_("Show Lua Console"), false, scope_game | scope_editor | scope_main, HKCAT_DEBUG, ""},

	//This list item must stay at the end since it is used as terminator for iterating.
	{ HOTKEY_NULL, "null", N_("Unrecognized Command"), true, 0, HKCAT_PLACEHOLDER, "" }
}};

const std::set<HOTKEY_COMMAND> toggle_commands {
	HOTKEY_SCROLL_UP,
	HOTKEY_SCROLL_DOWN,
	HOTKEY_SCROLL_LEFT,
	HOTKEY_SCROLL_RIGHT
};

// Contains copies of master_hotkey_list and all current active wml menu hotkeys
std::map<std::string_view, hotkey::hotkey_command> registered_hotkeys;

hk_scopes scope_active(0);
} // end anon namespace

scope_changer::scope_changer()
	: prev_scope_active_(scope_active)
	, restore_(true)
{
}

scope_changer::scope_changer(hk_scopes new_scopes, bool restore)
	: prev_scope_active_(scope_active)
	, restore_(restore)
{
	scope_active = new_scopes;
}

scope_changer::~scope_changer()
{
	// TODO: evaluate whether we need non-restore behavior in the game_config_manager
	if(restore_) {
		scope_active = prev_scope_active_;
	}
}

bool is_scope_active(scope s)
{
	assert(s < SCOPE_COUNT);
	return scope_active[s];
}

bool is_scope_active(hk_scopes s)
{
	// s is a copy because we need one
	s &= scope_active;
	return s.any();
}

const hotkey_command& get_hotkey_command(const std::string& command)
{
	try {
		return registered_hotkeys.at(command);
	} catch(const std::out_of_range&) {
		return hotkey_command::null_command();
	}
}

const std::map<std::string_view, hotkey::hotkey_command>& get_hotkey_commands()
{
	return registered_hotkeys;
}

bool has_hotkey_command(const std::string& id)
{
	return get_hotkey_command(id).command != hotkey::HOTKEY_NULL;
}

wml_hotkey_record::wml_hotkey_record(const std::string& id, const t_string& description, const config& default_hotkey)
	: cleanup_()
{
	if(id == "null") {
		LOG_G << "Couldn't add wml hotkey with null id and description = '" << description << "'.";
		return;
	}

	const auto& [iter, inserted] = registered_hotkeys.try_emplace(
		id, hotkey::HOTKEY_WML, id, description, false, false, scope_game, HKCAT_CUSTOM, t_string(""));

	if(inserted) {
		DBG_G << "Added wml hotkey with id = '" << id << "' and description = '" << description << "'.";
	} else {
		LOG_G << "Hotkey with id '" << id << "' already exists.";
		return;
	}

	if(!default_hotkey.empty() && !has_hotkey_item(id)) {
		hotkey::hotkey_ptr new_item = hotkey::load_from_config(default_hotkey);
		new_item->set_command(id);

		if(new_item->valid()) {
			DBG_G << "added default description for the wml hotkey with id=" + id;
			add_hotkey(new_item);
		} else {
			ERR_CF << "failed to add default hotkey with id=" + id;
		}
	}

	// Record the cleanup handler
	cleanup_ = [id] { registered_hotkeys.erase(id); };
}

wml_hotkey_record::~wml_hotkey_record()
{
	if(cleanup_) {
		cleanup_();
	}
}

hotkey_command::hotkey_command(const hotkey_command_temp& temp_command)
	: command(temp_command.command)
	, id(temp_command.id)
	, description(std::string(temp_command.description), "wesnoth-lib")
	, hidden(temp_command.hidden)
	, toggle(toggle_commands.count(temp_command.command) > 0)
	, scope(temp_command.scope)
	, category(temp_command.category)
	, tooltip(std::string(temp_command.tooltip), "wesnoth-lib")
{
}

hotkey_command::hotkey_command(hotkey::HOTKEY_COMMAND cmd,
		const std::string& id_,
		const t_string& desc,
		bool hid,
		bool tog,
		hotkey::hk_scopes scop,
		hotkey::HOTKEY_CATEGORY cat,
		const t_string& toolt)
	: command(cmd)
	, id(id_)
	, description(desc)
	, hidden(hid)
	, toggle(tog)
	, scope(scop)
	, category(cat)
	, tooltip(toolt)
{
}

const hotkey_command& hotkey_command::null_command()
{
	return registered_hotkeys.at("null");
}

bool hotkey_command::null() const
{
	if(command == HOTKEY_NULL || id == "null") {
		const hotkey_command& null_cmd = null_command();

		if(command == null_cmd.command && id == null_cmd.id && scope == null_cmd.scope
			&& description == null_cmd.description) {
			return true;
		} else {
			ERR_G << "the hotkey command seems to be the null command but it is not 100% sure. This shouldn't happen";
			return true;
		}
	}

	return false;
}

const hotkey_command& hotkey_command::get_command_by_command(hotkey::HOTKEY_COMMAND command)
{
	for(auto& [id, cmd] : registered_hotkeys) {
		if(cmd.command == command) {
			return cmd;
		}
	}

	ERR_G << "No hotkey with requested command '" << command << "' found. Returning null hotkey.";
	return hotkey_command::null_command();
}

void init_hotkey_commands()
{
	registered_hotkeys.clear();

	for(const hotkey_command_temp& cmd : master_hotkey_list) {
		// Initialize the full hotkey from the temp data.
		registered_hotkeys.try_emplace(cmd.id, cmd);
	}
}

t_string get_translatable_category_name(HOTKEY_CATEGORY category)
{
	return {category_names.at(category), "wesnoth-lib"};
}

} // namespace hotkey
