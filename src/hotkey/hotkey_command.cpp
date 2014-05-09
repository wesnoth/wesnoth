/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gettext.hpp"
#include "hotkey_command.hpp"
#include "hotkey_item.hpp"
#include "config.hpp"
#include "preferences.hpp"

#include "boost/foreach.hpp"
#include "log.hpp"

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general)
#define LOG_G  LOG_STREAM(info,  lg::general)
#define DBG_G  LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err,   log_config)


namespace {

// this contains all static hotkeys
hotkey::hotkey_command_temp hotkey_list_[] = {

	{ hotkey::HOTKEY_CANCEL, N_("cancel"), N_("Cancel"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_SELECT_HEX, "selecthex", N_("Select Hex"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_DESELECT_HEX, "deselecthex", N_("Deselect Hex"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_MOVE_ACTION, "moveaction", N_("Move/Attack"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SELECT_AND_ACTION, "selectmoveaction", N_("Select/Move/Attack"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_ANIMATE_MAP, "animatemap", N_("Animate Map"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_CYCLE_UNITS, "cycle", N_("Next Unit"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CYCLE_BACK_UNITS, "cycleback", N_("Previous Unit"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_UNIT_HOLD_POSITION, "holdposition", N_("Hold Position"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_END_UNIT_TURN, "endunitturn", N_("End Unit Turn"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_LEADER, "leader", N_("Leader"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_UNDO, "undo", N_("Undo"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_REDO, "redo", N_("Redo"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_ZOOM_IN, "zoomin", N_("Zoom In"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_ZOOM_OUT, "zoomout", N_("Zoom Out"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_ZOOM_DEFAULT, "zoomdefault", N_("Default Zoom"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_FULLSCREEN, "fullscreen", N_("Toggle Full Screen"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_SCREENSHOT, "screenshot", N_("Screenshot"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_MAP_SCREENSHOT, "mapscreenshot", N_("Map Screenshot"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_ACCELERATED, "accelerated", N_("Accelerated"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_TERRAIN_DESCRIPTION, "describeterrain", N_("Terrain Description"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_UNIT_DESCRIPTION, "describeunit", N_("Unit Description"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_RENAME_UNIT, "renameunit", N_("Rename Unit"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_DELETE_UNIT, "editor-deleteunit", N_("Delete Unit"), false, hotkey::SCOPE_GENERAL, "" },

	{ hotkey::HOTKEY_SAVE_GAME, "save", N_("Save Game"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SAVE_REPLAY, "savereplay", N_("Save Replay"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SAVE_MAP, "savemap", N_("Save Map"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_LOAD_GAME, "load", N_("Load Game"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_RECRUIT, "recruit", N_("Recruit"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPEAT_RECRUIT, "repeatrecruit", N_("Repeat Recruit"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_RECALL, "recall", N_("Recall"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_ENDTURN, "endturn", N_("End Turn"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_TOGGLE_ELLIPSES, "toggleellipses", N_("Toggle Ellipses"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_TOGGLE_GRID, "togglegrid", N_("Toggle Grid"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_MOUSE_SCROLL, "mousescroll", N_("Mouse Scrolling"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_STATUS_TABLE, "statustable", N_("Status Table"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_MUTE, "mute", N_("Mute"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_SPEAK, "speak", N_("Speak"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CREATE_UNIT, "createunit", N_("Create Unit (Debug!)"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CHANGE_SIDE, "changeside", N_("Change Side (Debug!)"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_KILL_UNIT, "killunit", N_("Kill Unit (Debug!)"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_PREFERENCES, "preferences", N_("Preferences"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_OBJECTIVES, "objectives", N_("Scenario Objectives"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_UNIT_LIST, "unitlist", N_("Unit List"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_STATISTICS, "statistics", N_("Statistics"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_STOP_NETWORK, "stopnetwork", N_("Pause Network Game"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_START_NETWORK, "startnetwork", N_("Continue Network Game"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_QUIT_GAME, "quit", N_("Quit Game"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_QUIT_GAME, "quit-editor", N_("Quit Editor"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_LABEL_TEAM_TERRAIN, "labelteamterrain", N_("Set Team Label"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_LABEL_TERRAIN, "labelterrain", N_("Set Label"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CLEAR_LABELS, "clearlabels", N_("Clear Labels"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", N_("Show Enemy Moves"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", N_("Best Possible Enemy Moves"), false, hotkey::SCOPE_GAME, "" },

	{ hotkey::HOTKEY_REPLAY_PLAY, "playreplay", N_("Play Replay"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_RESET, "resetreplay", N_("Reset Replay"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_STOP, "stopreplay", N_("Stop Replay"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_NEXT_TURN, "replaynextturn", N_("Next Turn"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_NEXT_SIDE, "replaynextside", N_("Next Side"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING, "replayshoweverything", N_("Full Map"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_SHOW_EACH, "replayshoweach", N_("Each Team"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_SHOW_TEAM1, "replayshowteam1", N_("Team 1"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_REPLAY_SKIP_ANIMATION, "replayskipanimation", N_("Skip Animation"), false, hotkey::SCOPE_GAME, "" },
	// Whiteboard commands
	// TRANSLATORS: whiteboard menu entry: toggle planning mode
	{ hotkey::HOTKEY_WB_TOGGLE, "wbtoggle", N_("whiteboard^Planning Mode"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: execute planned action
	{ hotkey::HOTKEY_WB_EXECUTE_ACTION, "wbexecuteaction", N_("whiteboard^Execute Action"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: execute all planned actions
	{ hotkey::HOTKEY_WB_EXECUTE_ALL_ACTIONS, "wbexecuteallactions", N_("whiteboard^Execute All Actions"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: delete planned action
	{ hotkey::HOTKEY_WB_DELETE_ACTION, "wbdeleteaction", N_("whiteboard^Delete Action"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: move planned action up queue
	{ hotkey::HOTKEY_WB_BUMP_UP_ACTION, "wbbumpupaction", N_("whiteboard^Move Action Up"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: move planned action down queue
	{ hotkey::HOTKEY_WB_BUMP_DOWN_ACTION, "wbbumpdownaction", N_("whiteboard^Move Action Down"), false, hotkey::SCOPE_GAME, "" },
	// TRANSLATORS: whiteboard menu entry: plan as though the chosen unit were dead
	{ hotkey::HOTKEY_WB_SUPPOSE_DEAD, "wbsupposedead", N_("whiteboard^Suppose Dead"), false, hotkey::SCOPE_GAME, "" },

	{ hotkey::HOTKEY_EDITOR_QUIT_TO_DESKTOP, "editor-quit-to-desktop", N_("Quit to Desktop"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_CLOSE, "editor-close-map", N_("Close Map"), false, hotkey::SCOPE_EDITOR, "" },

	// These are not really hotkey items but menu entries to get expanded.
	// They need to have their own hotkey to control their active state.
	{ hotkey::HOTKEY_EDITOR_PLAYLIST, "editor-playlist", N_("Switch Time of Day"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_SWITCH, "editor-switch-map", N_("Switch Map"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_LOCAL_TIME, "menu-editor-local-time", N_("Assign Local Time"), true, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_CUSTOM_TODS, "editor-custom-tods", N_("Custom Time of Day Creator"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PARTIAL_UNDO, "editor-partial-undo", N_("Partial Undo"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_NEW, "editor-map-new", N_("New Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SCENARIO_NEW, "editor-scenario-new", N_("New Scenario"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_LOAD, "editor-map-load", N_("Load Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE, "editor-map-save", N_("Save Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE_AS, "editor-map-save-as", N_("Save Map As"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SCENARIO_SAVE_AS, "editor-scenario-save-as", N_("Save Scenario As"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE_ALL, "editor-map-save-all", N_("Save All Maps"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_REVERT, "editor-map-revert", N_("Revert All Changes"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_INFO, "editor-map-info", N_("Map Information"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_PALETTE_ITEMS_CLEAR, "editor-palette-items-clear", N_("Clear Selected Item Set"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PALETTE_ITEM_SWAP, "editor-terrain-palette-swap", N_("Swap Foreground/Background Palette Item"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PALETTE_GROUPS, "editor-palette-groups", N_("Change Palette Group"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PALETTE_UPSCROLL, "editor-palette-upscroll", N_("Scroll Palette Left"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PALETTE_DOWNSCROLL, "editor-palette-downscroll", N_("Scroll Palette Right"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_SIDE_NEW, "editor-side-new", N_("Add New Side"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_TOOL_NEXT, "editor-tool-next", N_("Next Tool"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_TOOL_PAINT, "editor-tool-paint", N_("Paint Tool"), false, hotkey::SCOPE_EDITOR, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_FILL, "editor-tool-fill", N_("Fill Tool"), false, hotkey::SCOPE_EDITOR, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_SELECT, "editor-tool-select", N_("Selection Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button selects or deselects with Ctrl, right brings up a context menu. Hold Shift for magic-wand selection of tiles with same terrain.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION, "editor-tool-starting-position", N_("Starting Positions Tool"), false, hotkey::SCOPE_EDITOR,  N_("Left mouse button displays player selection, right clears. Number keys scroll to the starting position, alt+number sets respective starting position under cursor, delete clears.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_LABEL, "editor-tool-label", N_("Label Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets or drags a label, right clears.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_UNIT, "editor-tool-unit", N_("Unit Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets a new unit or moves a unit via drag and drop, right brings up a context menu. Needs a defined side.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_ITEM, "editor-tool-item", N_("Item Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets a new item or moves it via drag and drop, right clears. Not implemented yet.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_SOUNDSOURCE, "editor-tool-soundsource", N_("Sound Source Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets or drags a sound source, right clears. Not implemented yet.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_VILLAGE, "editor-tool-village", N_("Village Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets the village ownership to the current side, right clears. Needs a defined side.") },

	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT, "editor-toggle-canrecruit", N_("Can Recruit"), false, hotkey::SCOPE_EDITOR, N_("Toggle the recruit attribute of a unit.") },
	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE, "editor-toggle-renameable", N_("Can be Renamed"), false, hotkey::SCOPE_EDITOR, N_("Toggle the unit being renameable.") },
	{ hotkey::HOTKEY_EDITOR_UNIT_RECRUIT_ASSIGN, "editor-unit-recruit", N_("Assign Recruit List"), false, hotkey::SCOPE_EDITOR, N_("Assign the selected unit set as recruit list to the unit.") },

	{ hotkey::HOTKEY_EDITOR_UNIT_CHANGE_ID, "editor-change-unitid", N_("Change Unit ID"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL, "editor-unit-toggle-loyal", N_("Loyal"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_MINIMAP_CODING_UNIT, "minimap-unit-coding", N_("Toggle Minimap Unit Coding"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_MINIMAP_CODING_TERRAIN, "minimap-terrain-coding", N_("Toggle Minimap Terrain Coding"), false, hotkey::SCOPE_GENERAL, "" },

	{ hotkey::HOTKEY_MINIMAP_DRAW_UNITS, "minimap-draw-units", N_("Toggle Minimap Unit Drawing"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_MINIMAP_DRAW_VILLAGES, "minimap-draw-villages", N_("Toggle Minimap Village Drawing"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_MINIMAP_DRAW_TERRAIN, "minimap-draw-terrain", N_("Toggle Minimap Terrain Drawing"), false, hotkey::SCOPE_GENERAL, "" },

	{ hotkey::HOTKEY_EDITOR_BRUSH_NEXT, "editor-brush-next", N_("Next Brush"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_DEFAULT, "editor-brush-default", N_("Default Brush"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_1, "editor-brush-1", N_("Single Tile"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_2, "editor-brush-2", N_("Radius One"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_3, "editor-brush-3", N_("Radius Two"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_NW_SE, "editor-brush-nw-se", N_("Brush NW-SE"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_BRUSH_SW_NE, "editor-brush-sw-ne", N_("Brush SW-NE"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_SELECTION_CUT, "editor-cut", N_("Cut"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_COPY, "editor-copy", N_("Copy"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_PASTE, "editor-paste", N_("Paste"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button pastes from the clipboard, right brings up a context menu.") },
	{ hotkey::HOTKEY_EDITOR_SELECTION_EXPORT, "editor-export-selection-coords", N_("Export Selected Coordinates to System Clipboard"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECT_ALL, "editor-select-all", N_("Select All"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECT_INVERSE, "editor-select-inverse", N_("Select Inverse"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECT_NONE, "editor-select-none", N_("Select None"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, "editor-clipboard-rotate-cw", N_("Rotate Clipboard Clockwise"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW, "editor-clipboard-rotate-ccw", N_("Rotate Clipboard Counter-Clockwise"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, "editor-clipboard-flip-horizontal", N_("Flip Clipboard Horizontally"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL, "editor-clipboard-flip-vertical", N_("Flip Clipboard Vertically"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_ROTATE, "editor-selection-rotate", N_("Rotate Selection"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FLIP, "editor-selection-flip", N_("Flip Selection"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FILL, "editor-selection-fill", N_("Fill Selection"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_GENERATE, "editor-selection-generate", N_("Generate Tiles in Selection"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SELECTION_RANDOMIZE, "editor-selection-randomize", N_("Randomize Tiles in Selection"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_RESIZE, "editor-map-resize", N_("Resize Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_ROTATE, "editor-map-rotate", N_("Rotate Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_GENERATE, "editor-map-generate", N_("Generate Map"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_APPLY_MASK, "editor-map-apply-mask", N_("Apply a Mask"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_MAP_CREATE_MASK_TO, "editor-map-create-mask-to", N_("Create Mask"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_REFRESH, "editor-refresh", N_("Refresh Display"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_UPDATE_TRANSITIONS, "editor-update-transitions", N_("Update Terrain Transitions"), false, hotkey::SCOPE_EDITOR, "" },

	// This item is for binding in the preferences
	{ hotkey::HOTKEY_EDITOR_TOGGLE_TRANSITIONS, "editor-toggle-transitions", N_("Toggle Terrain Transition Update"), false, hotkey::SCOPE_EDITOR, "" },
	// The next three are for displaying the different states in the menu
	{ hotkey::HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, "editor-auto-update-transitions", N_("Auto-update Terrain Transitions"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS, "editor-no-update-transitions", N_("Auto-update Terrain Transitions: No"), true, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS, "editor-partial-update-transitions", N_("Auto-update Terrain Transitions: Partial"), true, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_REFRESH_IMAGE_CACHE, "editor-refresh-image-cache", N_("Refresh Image Cache"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_DRAW_COORDINATES, "editor-draw-coordinates", N_("Draw Hex Coordinates"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_DRAW_TERRAIN_CODES, "editor-draw-terrain-codes", N_("Draw Terrain Codes"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_AREA_SAVE,   "editor-save-area",   N_("Save Selection to Area"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_AREA_RENAME, "editor-rename-area", N_("Rename Selected Area"),   false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_AREA_REMOVE, "editor-remove-area", N_("Remove Selected Area"),   false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_AREA_ADD,    "editor-add-area",    N_("Add New Area"),           false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_EDITOR_SCENARIO_EDIT, "editor-scenario-edit", N_("Edit Scenario"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SIDE_EDIT, "editor-side-edit", N_("Edit Side"), false, hotkey::SCOPE_EDITOR, "" },
	{ hotkey::HOTKEY_EDITOR_SIDE_REMOVE, "editor-side-remove", N_("Remove Side"), false, hotkey::SCOPE_EDITOR, "" },

	{ hotkey::HOTKEY_DELAY_SHROUD, "delayshroud", N_("Delay Shroud Updates"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_UPDATE_SHROUD, "updateshroud", N_("Update Shroud Now"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CONTINUE_MOVE, "continue", N_("Continue Move"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SEARCH, "search", N_("Find Label or Unit"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SPEAK_ALLY, "speaktoally", N_("Speak to Ally"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_SPEAK_ALL, "speaktoall", N_("Speak to All"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_HELP, "help", N_("Help"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_CHAT_LOG, "chatlog", N_("View Chat Log"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_USER_CMD, "command", N_("Enter User Command"), false, hotkey::SCOPE_GENERAL, "" },
	{ hotkey::HOTKEY_CUSTOM_CMD, "customcommand", N_("Custom Command"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_AI_FORMULA, "aiformula", N_("Run Formula"), false, hotkey::SCOPE_GAME, "" },
	{ hotkey::HOTKEY_CLEAR_MSG, "clearmessages", N_("Clear Messages"), false, hotkey::SCOPE_GAME, "" },

	{ hotkey::HOTKEY_LANGUAGE, "changelanguage", N_("Change Language"), false, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__RELOAD_WML, "title_screen__reload_wml", N_("Refresh WML"), true ,hotkey::SCOPE_GENERAL, "" },
	{ hotkey::TITLE_SCREEN__NEXT_TIP, "title_screen__next_tip", N_("Next Tip of the Day"), false, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__PREVIOUS_TIP, "title_screen__previous_tip", N_("Previous Tip of the Day"), false, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__TUTORIAL, "title_screen__tutorial", N_("Start Tutorial"), false	, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__CAMPAIGN, "title_screen__campaign", N_("Start Campaign"), false	, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__MULTIPLAYER, "title_screen__multiplayer", N_("Start Multiplayer Game"), false, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__ADDONS, "title_screen__addons", N_("Manage Add-ons"), false	, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__CORES, "title_screen__cores", N_("Manage Cores"), false	, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__EDITOR, "title_screen__editor", N_("Start Editor"), false, hotkey::SCOPE_MAIN_MENU, "" },
	{ hotkey::TITLE_SCREEN__CREDITS, "title_screen__credits", N_("Show Credits"), false	, hotkey::SCOPE_MAIN_MENU, "" },

	{ hotkey::GLOBAL__HELPTIP, "global__helptip", N_("Show Helptip"), false, hotkey::SCOPE_GENERAL, "" },

	//This list item must stay at the end since it is used as terminator for iterating.
	{ hotkey::HOTKEY_NULL, "null", N_("Unrecognized Command"), true, hotkey::SCOPE_COUNT, "" }
};

// contains copies of hotkey_list_ and all current active wml menu hotkeys
// maybe known_hotkeys is not a fitting name anymore.
boost::ptr_vector<hotkey::hotkey_command> known_hotkeys;

// the size_t are indexes for known_hotkeys, because known_hotkeys begins with input_list_, they are also indexes for input_list_.
std::map<std::string, size_t> command_map_;

//
std::vector<bool> scope_active_(hotkey::SCOPE_COUNT, false);

}

namespace hotkey {
scope_changer::scope_changer()
: prev_scope_active_(scope_active_)
{}

scope_changer::~scope_changer()
{
	scope_active_.swap(prev_scope_active_);
}


void deactivate_all_scopes()
{
	for (int i = 0; i < hotkey::SCOPE_COUNT; ++i) {
		scope_active_[i] = false;
	}
}

void set_scope_active(scope s, bool set)
{
	scope_active_[s] = set;
}

bool is_scope_active(scope s)
{
	return scope_active_[s];
}


const hotkey_command& get_hotkey_command(const std::string& command)
{
	if (command_map_.find(command) == command_map_.end())
	{
		return get_hotkey_null();
	}

	return known_hotkeys[command_map_[command]];
}

const boost::ptr_vector<hotkey_command>& get_hotkey_commands()
{
	return known_hotkeys;
}


// Returns whether a hotkey was deleted.
bool remove_wml_hotkey(const std::string& id)
{
	const hotkey::hotkey_command& command = get_hotkey_command(id);
	if(command.id == hotkey::HOTKEY_NULL)
	{
		LOG_G << "remove_wml_hotkey: command with id=" + id + " doesn't exist\n";
		return false;
	}
	else if (command.id != hotkey::HOTKEY_WML)
	{
		LOG_G << "remove_wml_hotkey: command with id=" + id + " cannot be removed because it is no wml menu hotkey\n";
		return false;
	}
	else
	{
		LOG_G << "removing wml hotkey with id=" + id + "\n";
		for(boost::ptr_vector<hotkey_command>::iterator itor = known_hotkeys.begin(); itor != known_hotkeys.end(); ++itor)
		{
			if(itor->command == id)
			{
				known_hotkeys.erase(itor);
				break;
			}
		}
		//command_map_ might be all wrong now, so we need to rebuild.
		command_map_.clear();
		for(size_t index = 0; index < known_hotkeys.size(); index++)
		{
			command_map_[known_hotkeys[index].command] = index;
		}
		return true;
	}
}

bool has_hotkey_command(const std::string& id)
{
	return get_hotkey_command(id).id != hotkey::HOTKEY_NULL;
}

void add_wml_hotkey(const std::string& id, const t_string& description, const config& default_hotkey)
{
	if(id == "null")
	{
		LOG_G << "Couldn't add wml hotkey with null id and description = '" << description << "'.\n";
		return;
	}
	else
	{
		if(has_hotkey_command(id))
		{
			LOG_G << "Hotkey with id '" << id << "' already exists. Deleting the old hotkey_command.\n";
			remove_wml_hotkey(id);
		}
		DBG_G << "Added wml hotkey with id = '" << id << "' and description = '" << description << "'.\n";
		known_hotkeys.push_back(new hotkey_command(hotkey::HOTKEY_WML, id, description, false, hotkey::SCOPE_GAME, t_string("")));

		command_map_[id] = known_hotkeys.size() - 1;

		if(!default_hotkey.empty() && !has_hotkey_item(id))
		{
			hotkey_item new_item(default_hotkey, true);
			new_item.set_command(id);
			if(new_item.valid())
			{
				DBG_G << "added default description for the wml hotkey with id=" + id;
				add_hotkey(new_item);
			}
			else
			{
				ERR_CF << "failed to add default hotkey with id=" + id;
			}
		}
	}
}




hotkey_command::hotkey_command()
	: id(HOTKEY_NULL), command(""), description(""), hidden(true), scope(SCOPE_GENERAL), tooltip("")
{
	ERR_G << "hotkey_command's default constructor called. This shouldn't happen, because all its members are const.";
}

hotkey_command::hotkey_command(hotkey::HOTKEY_COMMAND cmd, const std::string& id_, const t_string& desc, bool hid, hotkey::scope scop, const t_string& toolt)
	: id(cmd), command(id_), description(desc), hidden(hid), scope(scop), tooltip(toolt)
{
}

const hotkey_command& hotkey_command::null_command()
{
	return get_hotkey_null();
}

bool hotkey_command::null() const
{
	if(id == HOTKEY_NULL || command == "null")
	{
		const hotkey_command& null_cmd = null_command();
		if(id == null_cmd.id && command == null_cmd.command && scope == null_cmd.scope && description == null_cmd.description)
			return true;
		else
		{
			ERR_G << "the hotkey command seems to be the null command but it is not 100% sure. This shouldn't happen";
			return true;
		}
	}
	return false;
}


const hotkey_command& hotkey_command::get_command_by_command(hotkey::HOTKEY_COMMAND command)
{
	BOOST_FOREACH(hotkey_command& cmd, known_hotkeys)
	{
		if(cmd.id == command)
			return cmd;
	}
	ERR_G << " \"get_command_by_command\" returned get_hotkey_null() because no hotkey_command had the requested number:" << command;
	return get_hotkey_null();
}


const hotkey_command& get_hotkey_null()
{
	//it is the last entry in that array, and the indexes in hotkey_list_ and known_hotkeys are the same.
	return known_hotkeys[sizeof(hotkey_list_) / sizeof(hotkey_list_[0])  - 1];
}

void delete_all_wml_hotkeys()
{
	while(known_hotkeys.back().id == hotkey::HOTKEY_WML)
	{
		command_map_.erase(known_hotkeys.back().command);
		//according to some page in the Internet  .back() returns a reference not an iterator, so i use this.
		boost::ptr_vector<hotkey_command>::iterator last_element = known_hotkeys.end();
		--last_element;
		//boost::ptr_vector<hotkey_command> will manage the deleting of the object for me.
		known_hotkeys.erase(last_element);
	}
}

const std::string get_description(const std::string& command)
{
	return get_hotkey_command(command).description;
}

const std::string get_tooltip(const std::string& command)
{
	// the null hotkey_command has the "" tooltip
	return get_hotkey_command(command).tooltip;
}

void init_hotkey_commands()  {
	//the size value is just random set.
	boost::ptr_vector<hotkey_command> known_hotkeys_temp(200);
	known_hotkeys = known_hotkeys_temp;

	size_t i = 0;
	BOOST_FOREACH(hotkey_command_temp& cmd, hotkey_list_)
	{
		known_hotkeys.push_back( new hotkey_command(cmd.id, cmd.command, t_string(cmd.description, "wesnoth-lib"), cmd.hidden, cmd.scope, t_string(cmd.tooltip, "wesnoth-lib")));
		command_map_[cmd.command] = i;
		i++;
	}
}

void clear_hotkey_commands() {
	command_map_.clear();
}

HOTKEY_COMMAND get_id(const std::string& command)
{
	return get_hotkey_command(command).id;
}

}
