/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "construct_dialog.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "theme.hpp"
#include "wesconfig.h"
#include "wml_separators.hpp"

#include <map>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general)
#define LOG_G  LOG_STREAM(info,  lg::general)
#define DBG_G  LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err,   log_config)

namespace {

std::vector<hotkey::hotkey_item> hotkeys_;
std::map<std::string, size_t> command_map_;

hotkey::hotkey_item null_hotkey_("null");

std::vector<bool> scope_active_(hotkey::SCOPE_COUNT, false);

config default_hotkey_cfg_;

}


namespace hotkey {

const input_controll input_list_[] = {
		{ hotkey::INPUT_SCROLL_HORIZONTAL, "scroll-horizontal", N_("Scroll Viewport Horizontally"), false, hotkey::SCOPE_GENERAL },
		{ hotkey::INPUT_SCROLL_VERTICAL,   "scroll-vertical",   N_("Scroll Viewport Vertically"), false, hotkey::SCOPE_GENERAL }
};

const hotkey_command hotkey_list_[] = {

	{ hotkey::HOTKEY_CANCEL, "cancel", N_("Cancel"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_LEFT_MOUSE_CLICK, "leftmouseclick", N_("Left Mouse Click"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_RIGHT_MOUSE_CLICK, "rightmouseclick", N_("Right Mouse Click"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_ANIMATE_MAP, "animatemap", N_("Animate Map"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_CYCLE_UNITS, "cycle", N_("Next Unit"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CYCLE_BACK_UNITS, "cycleback", N_("Previous Unit"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_UNIT_HOLD_POSITION, "holdposition", N_("Hold Position"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_END_UNIT_TURN, "endunitturn", N_("End Unit Turn"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_LEADER, "leader", N_("Leader"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_UNDO, "undo", N_("Undo"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_REDO, "redo", N_("Redo"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_ZOOM_IN, "zoomin", N_("Zoom In"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_ZOOM_OUT, "zoomout", N_("Zoom Out"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_ZOOM_DEFAULT, "zoomdefault", N_("Default Zoom"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_FULLSCREEN, "fullscreen", N_("Toggle Full Screen"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_SCREENSHOT, "screenshot", N_("Screenshot"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_MAP_SCREENSHOT, "mapscreenshot", N_("Map Screenshot"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_ACCELERATED, "accelerated", N_("Accelerated"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_UNIT_DESCRIPTION, "describeunit", N_("Unit Description"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_RENAME_UNIT, "renameunit", N_("Rename Unit"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_DELETE_UNIT, "editor-deleteunit", N_("Delete Unit"), false, hotkey::SCOPE_GENERAL, NULL },

	{ hotkey::HOTKEY_SAVE_GAME, "save", N_("Save Game"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SAVE_REPLAY, "savereplay", N_("Save Replay"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SAVE_MAP, "savemap", N_("Save Map"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_LOAD_GAME, "load", N_("Load Game"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_RECRUIT, "recruit", N_("Recruit"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPEAT_RECRUIT, "repeatrecruit", N_("Repeat Recruit"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_RECALL, "recall", N_("Recall"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_ENDTURN, "endturn", N_("End Turn"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_TOGGLE_ELLIPSES, "toggleellipses", N_("Toggle Ellipses"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_TOGGLE_GRID, "togglegrid", N_("Toggle Grid"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_MOUSE_SCROLL, "mousescroll", N_("Mouse Scrolling"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_STATUS_TABLE, "statustable", N_("Status Table"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_MUTE, "mute", N_("Mute"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_SPEAK, "speak", N_("Speak"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CREATE_UNIT, "createunit", N_("Create Unit (Debug!)"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CHANGE_SIDE, "changeside", N_("Change Side (Debug!)"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_PREFERENCES, "preferences", N_("Preferences"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_OBJECTIVES, "objectives", N_("Scenario Objectives"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_UNIT_LIST, "unitlist", N_("Unit List"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_STATISTICS, "statistics", N_("Statistics"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_STOP_NETWORK, "stopnetwork", N_("Pause Network Game"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_START_NETWORK, "startnetwork", N_("Continue Network Game"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_QUIT_GAME, "quit", N_("Quit Game"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_QUIT_GAME, "quit-editor", N_("Quit Editor"), true, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_LABEL_TEAM_TERRAIN, "labelteamterrain", N_("Set Team Label"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_LABEL_TERRAIN, "labelterrain", N_("Set Label"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CLEAR_LABELS, "clearlabels", N_("Clear Labels"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", N_("Show Enemy Moves"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", N_("Best Possible Enemy Moves"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_PLAY_REPLAY, "playreplay", N_("Play Replay"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_RESET_REPLAY, "resetreplay", N_("Reset Replay"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_STOP_REPLAY, "stopreplay", N_("Stop Replay"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_NEXT_TURN, "replaynextturn", N_("Next Turn"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_NEXT_SIDE, "replaynextside", N_("Next Side"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING, "replayshoweverything", N_("Full Map"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_SHOW_EACH, "replayshoweach", N_("Each Team"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_SHOW_TEAM1, "replayshowteam1", N_("Team 1"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_REPLAY_SKIP_ANIMATION, "replayskipanimation", N_("Skip Animation"), false, hotkey::SCOPE_GAME, NULL },
	// Whiteboard commands
	// TRANSLATORS: whiteboard menu entry: toggle planning mode
	{ hotkey::HOTKEY_WB_TOGGLE, "wbtoggle", N_("whiteboard^Planning Mode"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: execute planned action
	{ hotkey::HOTKEY_WB_EXECUTE_ACTION, "wbexecuteaction", N_("whiteboard^Execute Action"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: execute all planned actions
	{ hotkey::HOTKEY_WB_EXECUTE_ALL_ACTIONS, "wbexecuteallactions", N_("whiteboard^Execute All Actions"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: delete planned action
	{ hotkey::HOTKEY_WB_DELETE_ACTION, "wbdeleteaction", N_("whiteboard^Delete Action"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: move planned action up queue
	{ hotkey::HOTKEY_WB_BUMP_UP_ACTION, "wbbumpupaction", N_("whiteboard^Move Action Up"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: move planned action down queue
	{ hotkey::HOTKEY_WB_BUMP_DOWN_ACTION, "wbbumpdownaction", N_("whiteboard^Move Action Down"), false, hotkey::SCOPE_GAME, NULL },
	// TRANSLATORS: whiteboard menu entry: plan as though the chosen unit were dead
	{ hotkey::HOTKEY_WB_SUPPOSE_DEAD, "wbsupposedead", N_("whiteboard^Suppose Dead"), false, hotkey::SCOPE_GAME, NULL },

	{ hotkey::HOTKEY_EDITOR_QUIT_TO_DESKTOP, "editor-quit-to-desktop", N_("Quit to Desktop"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CLOSE_MAP, "editor-close-map", N_("Close Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SWITCH_MAP, "editor-switch-map", N_("Switch Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SWITCH_AREA, "editor-switch-area", N_("Switch Area"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_SETTINGS, "editor-settings", N_("Editor Settings"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PARTIAL_UNDO, "editor-partial-undo", N_("Partial Undo"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_NEW, "editor-map-new", N_("New Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_LOAD, "editor-map-load", N_("Load Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE, "editor-map-save", N_("Save Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE_AS, "editor-map-save-as", N_("Save Map As"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE_ALL, "editor-map-save-all", N_("Save All Maps"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_REVERT, "editor-map-revert", N_("Revert All Changes"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_INFO, "editor-map-info", N_("Map Information"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SIDE_NEW, "editor-side-new", N_("Add New Side"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SIDE_SWITCH, "editor-switch-side", N_("Switch Side"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PALETTE_ITEM_SWAP, "editor-terrain-palette-swap", N_("Swap Foreground/Background Terrains"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PALETTE_GROUPS, "editor-palette-groups", N_("Change Palette Group"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PALETTE_UPSCROLL, "editor-palette-upscroll", N_("Scroll Palette Left"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PALETTE_DOWNSCROLL, "editor-palette-downscroll", N_("Scroll Palette Right"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_SWITCH_TIME, "editor-switch-time", N_("Switch Time of Day"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_TOOL_NEXT, "editor-tool-next", N_("Next Tool"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_TOOL_PAINT, "editor-tool-paint", N_("Paint Tool"), false, hotkey::SCOPE_EDITOR, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_FILL, "editor-tool-fill", N_("Fill Tool"), false, hotkey::SCOPE_EDITOR, N_("Use left/right mouse button to draw fore-/background terrain. Hold Shift to paint base layer only. Ctrl+click to sample terrain under cursor.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_SELECT, "editor-tool-select", N_("Selection Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button selects, right deselects. Hold Shift for magic-wand selection of tiles with same terrain.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION, "editor-tool-starting-position", N_("Starting Positions Tool"), false, hotkey::SCOPE_EDITOR,  N_("Left mouse button displays player selection, right clears. Number keys scroll to the starting position, alt+number sets respective starting position under cursor, delete clears.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_LABEL, "editor-tool-label", N_("Label Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets or drags a label, right clears.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_UNIT, "editor-tool-unit", N_("Unit Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets a new unit or moves a unit via drag and drop, right clears. Not implemented yet.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_ITEM, "editor-tool-item", N_("Item Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets a new item or moves it via drag and drop, right clears. Not implemented yet.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_SOUNDSOURCE, "editor-tool-soundsource", N_("Soundsource Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets or drags a sound source, right clears. Not implemented yet.") },
	{ hotkey::HOTKEY_EDITOR_TOOL_VILLAGE, "editor-tool-village", N_("Village Tool"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button sets the village ownership to the current side, right clears. Not implemented yet.") },

	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT, "editor-toggle-canrecruit", N_("Canrecruit"), false, hotkey::SCOPE_EDITOR, N_("Toggle the recruit attribute of a unit.") },
	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE, "editor-toggle-renameable", N_("Can be renamed"), false, hotkey::SCOPE_EDITOR, N_("Toggle the unit being renameable.") },

	{ hotkey::HOTKEY_EDITOR_UNIT_CHANGE_ID, "editor-change-unitid", N_("Change Unit ID"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL, "editor-unit-toggle-loyal", N_("Loyal"), false, hotkey::SCOPE_EDITOR, NULL },


	{ hotkey::HOTKEY_EDITOR_BRUSH_NEXT, "editor-brush-next", N_("Next Brush"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_DEFAULT, "editor-brush-default", N_("Default Brush"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_1, "editor-brush-1", N_("Single Tile"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_2, "editor-brush-2", N_("Radius One"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_3, "editor-brush-3", N_("Radius Two"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_NW_SE, "editor-brush-nw-se", N_("Brush NW-SE"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_BRUSH_SW_NE, "editor-brush-sw-ne", N_("Brush SW-NE"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CUT, "editor-cut", N_("Cut"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_COPY, "editor-copy", N_("Copy"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PASTE, "editor-paste", N_("Paste"), false, hotkey::SCOPE_EDITOR, N_("Left mouse button pastes from the clipboard, right brings up a context menu.") },
	{ hotkey::HOTKEY_EDITOR_EXPORT_SELECTION_COORDS, "editor-export-selection-coords", N_("Export Selected Coordinates to System Clipboard"), true, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECT_ALL, "editor-select-all", N_("Select All"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECT_INVERSE, "editor-select-inverse", N_("Select Inverse"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECT_NONE, "editor-select-none", N_("Select None"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, "editor-clipboard-rotate-cw", N_("Rotate Clipboard Clockwise"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW, "editor-clipboard-rotate-ccw", N_("Rotate Clipboard Counter-Clockwise"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, "editor-clipboard-flip-horizontal", N_("Flip Clipboard Horizontally"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL, "editor-clipboard-flip-vertical", N_("Flip Clipboard Vertically"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECTION_ROTATE, "editor-selection-rotate", N_("Rotate Selection"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FLIP, "editor-selection-flip", N_("Flip Selection"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FILL, "editor-selection-fill", N_("Fill Selection"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECTION_GENERATE, "editor-selection-generate", N_("Generate Tiles In Selection"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_SELECTION_RANDOMIZE, "editor-selection-randomize", N_("Randomize Tiles In Selection"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_RESIZE, "editor-map-resize", N_("Resize Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_ROTATE, "editor-map-rotate", N_("Rotate Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_GENERATE, "editor-map-generate", N_("Generate Map"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_APPLY_MASK, "editor-map-apply-mask", N_("Apply a Mask"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_MAP_CREATE_MASK_TO, "editor-map-create-mask-to", N_("Create Mask"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_REFRESH, "editor-refresh", N_("Refresh Display"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_UPDATE_TRANSITIONS, "editor-update-transitions", N_("Update Terrain Transitions"), false, hotkey::SCOPE_EDITOR, NULL },
		// This item is for binding in the preferences
	{ hotkey::HOTKEY_EDITOR_TOGGLE_TRANSITIONS, "editor-toggle-transitions", N_("Toggle Terrain Transition Update"), true, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, "editor-auto-update-transitions", N_("Auto-update Terrain Transitions"), false, hotkey::SCOPE_EDITOR, NULL },
		// The next three are for displaying the different states in the menu
	{ hotkey::HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS, "editor-no-update-transitions", N_("Auto-update Terrain Transitions: No"), true, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS, "editor-partial-update-transitions", N_("Auto-update Terrain Transitions: Partial"), true, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_REFRESH_IMAGE_CACHE, "editor-refresh-image-cache", N_("Refresh Image Cache"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_DRAW_COORDINATES, "editor-draw-coordinates", N_("Draw Hex Coordinates"), false, hotkey::SCOPE_EDITOR, NULL },
	{ hotkey::HOTKEY_EDITOR_DRAW_TERRAIN_CODES, "editor-draw-terrain-codes", N_("Draw Terrain Codes"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_EDITOR_AREA_DEFINE, "editor-define-area", N_("Define (Time) Area"), false, hotkey::SCOPE_EDITOR, NULL },

	{ hotkey::HOTKEY_DELAY_SHROUD, "delayshroud", N_("Delay Shroud Updates"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_UPDATE_SHROUD, "updateshroud", N_("Update Shroud Now"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CONTINUE_MOVE, "continue", N_("Continue Move"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SEARCH, "search", N_("Find Label or Unit"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SPEAK_ALLY, "speaktoally", N_("Speak to Ally"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_SPEAK_ALL, "speaktoall", N_("Speak to All"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_HELP, "help", N_("Help"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_CHAT_LOG, "chatlog", N_("View Chat Log"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_LANGUAGE, "changelanguage", N_("Change Language"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_USER_CMD, "command", N_("Enter User Command"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::HOTKEY_CUSTOM_CMD, "customcommand", N_("Custom Command"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_AI_FORMULA, "aiformula", N_("Run Formula"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::HOTKEY_CLEAR_MSG, "clearmessages", N_("Clear Messages"), false, hotkey::SCOPE_GAME, NULL },
	{ hotkey::TITLE_SCREEN__RELOAD_WML, "title_screen__reload_wml", N_("Refresh WML"), true ,hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__NEXT_TIP, "title_screen__next_tip", N_("Next Tip of the Day"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__PREVIOUS_TIP, "title_screen__previous_tip", N_("Previous Tip of the Day"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__TUTORIAL, "title_screen__tutorial", N_("Start Tutorial"), false	, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__CAMPAIGN, "title_screen__campaign", N_("Start Campaign"), false	, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__MULTIPLAYER, "title_screen__multiplayer", N_("Start Multiplayer Game"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__ADDONS, "title_screen__addons", N_("Manage Add-ons"), false	, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__EDITOR, "title_screen__editor", N_("Start Editor"), false, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::TITLE_SCREEN__CREDITS, "title_screen__credits", N_("Show Credits"), false	, hotkey::SCOPE_GENERAL, NULL },
	{ hotkey::GLOBAL__HELPTIP, "global__helptip", N_("Show Helptip"), false, hotkey::SCOPE_GENERAL, NULL },

	//This list item must stay at the end since it is used as terminator for iterating.
	{ hotkey::HOTKEY_NULL, "null", N_("Unrecognized Command"), true, hotkey::SCOPE_GENERAL, NULL }
};

const hotkey_command* get_hotkey_commands() {
	return hotkey_list_;
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

static void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor);
static void jbutton_event_execute(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor);
static void jhat_event_execute(display& disp, const SDL_JoyHatEvent& event, command_executor* executor);
static void mbutton_event_execute(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor);

const std::string CLEARED_TEXT = "__none__";

scope hotkey_item::get_scope() const {
	return hotkey_list_[command_map_[get_command()]].scope;
}


HOTKEY_COMMAND hotkey_item::get_id() const {
	return hotkey_list_[command_map_[get_command()]].id;
}

const std::string hotkey_item::get_description() const {
	return hotkey::get_description(get_command());
}

// There are two kinds of "key" values.
// One refers to actual keys, like F1 or SPACE.
// The other refers to characters produced, eg 'M' or ':'.
// For the latter, specifying shift+; doesn't make sense,
// because ; is already shifted on French keyboards, for example.
// You really want to say ':', however that is typed.
// However, when you say shift+SPACE,
// you're really referring to the space bar,
// as shift+SPACE usually just produces a SPACE character.
void hotkey_item::load_from_config(const config& cfg)
{
	command_ = cfg["command"].str();

	const std::string& mouse = cfg["mouse"];
	if (!mouse.empty()) {
			mouse_ = cfg["mouse"].to_int();
			button_ = cfg["button"].to_int();
	}
	const std::string& joystick = cfg["joystick"];
	if (!joystick.empty()) {
		joystick_ = cfg["joystick"].to_int();
	}
	const std::string& hat = cfg["hat"];
	if (!hat.empty()) {
		hat_ = cfg["hat"].to_int();
		value_ = cfg["value"].to_int();
	}

	const std::string& button = cfg["button"];
	if (!button.empty()) {
		button_ = cfg["button"].to_int();
	}

	shift_ = cfg["shift"].to_bool();
	ctrl_  = cfg["ctrl"].to_bool();
	cmd_   = cfg["cmd"].to_bool();
	alt_   = cfg["alt"].to_bool();

	const std::string& key = cfg["key"];
	if (key.empty()) {
		return;
	}

	wide_string wkey = utils::string_to_wstring(key);

	// They may really want a specific key on the keyboard:
	// we assume that any single character keyname is a character.
	if (wkey.size() > 1) {

		keycode_ = sdl_keysym_from_name(key);
		if (keycode_ == SDLK_UNKNOWN) {
			if (tolower(key[0]) != 'f') {
				ERR_CF << "hotkey key '" << key << "' invalid\n";
			} else {
				int num = lexical_cast_default<int>(key.c_str() + 1);
				keycode_ = num + SDLK_F1 - 1;
			}
		}
	} else if (key == " " || shift_
#ifdef __APPLE__
		|| alt_
#endif
		) {
		// Space must be treated as a key because shift-space
		// isn't a different character from space,
		// and control key makes it go weird.
		// shift=yes should never be specified on single characters
		// (eg. key=m, shift=yes would be key=M),
		// but we don't want to break old preferences files.
		keycode_ = wkey[0];
	} else {
		character_ = wkey[0];
	}
}

void hotkey_item::set_command(const std::string& command) {
	command_ = command;
}


std::string hotkey_item::get_name() const
{
	std::stringstream str;

	if (shift_) { str << "shift+"; }
	if (ctrl_)  { str << "ctrl+";  }
	if (cmd_)   { str << "cmd+";   }
	if (alt_)   { str << "alt+";   }

	if (mouse_     >=  0) { str << _("Mouse") << mouse_ << _("Button") << button_; }
	if (character_ != -1) { str << static_cast<char>(character_); }
	if (keycode_   != -1) { str << SDL_GetKeyName(SDLKey(keycode_)); }
	if (joystick_  >=  0) { str << _("Joystick") << joystick_ << _("Button") << button_; }

	if (value_ >= 0) {
		std::string direction;
		switch (value_) {
			case SDL_HAT_CENTERED:
				direction = _("Centered");
				break;
			case SDL_HAT_UP:
				direction = _("Up");
				break;
			case SDL_HAT_RIGHT:
				direction = _("Right");
				break;
			case SDL_HAT_DOWN:
				direction = _("Down");
				break;
			case SDL_HAT_LEFT:
				direction = _("Left");
				break;
			case SDL_HAT_RIGHTUP:
				direction = _("RightUp");
				break;
			case SDL_HAT_RIGHTDOWN:
				direction = _("RightDown");
				break;
			case SDL_HAT_LEFTUP:
				direction = _("LeftUp");
				break;
			case SDL_HAT_LEFTDOWN:
				direction = _("LeftDown");
				break;
			default:
				direction = _("Unknown");
				break;
		}
		str << _("Joystick") << joystick_ << _("Hat") << button_ << direction;
	}

	return str.str();
}

void hotkey_item::clear()
{
	command_ = "null";
}


void hotkey_item::save(config& item)
{
	if (get_button()    >= 0) item["button"]   = get_button();
	if (get_joystick()  >= 0) item["joystick"] = get_joystick();
	if (get_hat()       >= 0) item["hat"]      = get_hat();
	if (get_value()     >= 0) item["value"]    = get_value();
	if (get_keycode()   >= 0) item["key"]      = SDL_GetKeyName(SDLKey(get_keycode()));
	if (get_character() >= 0) item["key"]      = utils::wchar_to_string(get_character());
	if (get_mouse()     >= 0) item["mouse"]    = get_mouse();
	if (get_button()    >= 0) item["button"]   = get_button();

	item["command"] = get_command();
	if (get_shift()) item["shift"] = get_shift();
	if (get_ctrl() ) item["ctrl"]  = get_ctrl();
	if (get_cmd()  ) item["cmd"]   = get_cmd();
	if (get_alt()  ) item["alt"]   = get_alt();
}

void hotkey_item::set_jbutton(int joystick, int button,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	joystick_ = joystick;
	button_   = button;
	shift_    = shift;
	ctrl_     = ctrl;
	cmd_      = cmd;
	alt_      = alt;
}

void hotkey_item::set_jhat(int joystick, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	joystick_ = joystick;
	hat_      = hat;
	value_    = value;
	shift_    = shift;
	ctrl_     = ctrl;
	cmd_      = cmd;
	alt_      = alt;
}

void hotkey_item::set_mbutton(int mouse, int button,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	mouse_  = mouse;
	button_ = button;
	shift_  = shift;
	ctrl_   = ctrl;
	cmd_    = cmd;
	alt_    = alt;
}

void hotkey_item::set_key(int character, int keycode,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	LOG_G << "setting hotkey: char=" << lexical_cast<std::string>(character)
		   << " keycode="  << lexical_cast<std::string>(keycode) << " "
		   << (shift ? "shift," : "")
		   << (ctrl  ? "ctrl,"  : "")
		   << (cmd   ? "cmd,"   : "")
		   << (alt   ? "alt,"   : "")
		   << "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (character < 64 && ctrl) {
		if (shift) {
			character += 64; }
		else {
			character += 96; }
		LOG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift) {
		character -= 32; }

	// We handle simple cases by character, others by the actual key.
	if (isprint(character) && !isspace(character)) {
		character_ = character;
		ctrl_      = ctrl;
		cmd_       = cmd;
		alt_       = alt;
		LOG_G << "type = BY_CHARACTER\n";
	} else {
		keycode_ = keycode;
		shift_   = shift;
		ctrl_    = ctrl;
		cmd_     = cmd;
		alt_     = alt;
		LOG_G << "type = BY_KEYCODE\n";
	}
}

manager::manager()
{
	init();
}

void manager::init()
{
	size_t i;
	for (i = 0; hotkey_list_[i].id != hotkey::HOTKEY_NULL; ++i) {
		command_map_[hotkey_list_[i].command] = i;
	}
	//TODO find a clever way to extend the loop and remove the next line.
	command_map_[hotkey_list_[i].command] = i;
}

void manager::wipe()
{
	hotkeys_.clear();
	command_map_.clear();
}

manager::~manager()
{
	wipe();
}

scope_changer::scope_changer()
: prev_scope_active_(scope_active_)
{}

scope_changer::~scope_changer()
{
	scope_active_.swap(prev_scope_active_);
}

void clear_hotkeys(const std::string& command)
{
	BOOST_FOREACH(hotkey::hotkey_item& item, hotkeys_) {
		if (item.get_command() == command) {
			item.clear(); }
	}
}

void load_hotkeys(const config& cfg, bool set_as_default)
{
	hotkeys_.clear();
	BOOST_FOREACH(const config &hk, cfg.child_range("hotkey")) {
		hotkeys_.push_back(hotkey_item(hk));
	}

	if (hotkeys_.empty()) {
		reset_default_hotkeys();
	} else if (set_as_default) {
		default_hotkey_cfg_ = cfg;
	}
}

void reset_default_hotkeys()
{
	hotkeys_.clear();

	if(!default_hotkey_cfg_.empty()) {
		load_hotkeys(default_hotkey_cfg_, false);
	} else {
		ERR_G << "no default hotkeys set yet; all hotkeys are now unassigned!\n";
	}
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children("hotkey");

	for(std::vector<hotkey_item>::iterator i = hotkeys_.begin();
			i != hotkeys_.end(); ++i)
	{
		if (i->null()) {
			continue;
		}

		config& item = cfg.add_child("hotkey");
		i->save(item);
	}
}

const std::string get_description(const std::string& command) {
	std::string tmp(command);
	if (command_map_.find(tmp) == command_map_.end()) {
		tmp = "null";
	}
	return t_string(hotkey_list_[command_map_[tmp]].description,
			PACKAGE "-lib");
}

const std::string get_tooltip(const std::string& command) {
		if (command_map_.find(command) == command_map_.end()) {
		return std::string();
	}
		if (hotkey_list_[command_map_[command]].tooltip != NULL)
			return t_string(hotkey_list_[command_map_[command]].tooltip,
					PACKAGE "-lib");
		else return std::string();
}

HOTKEY_COMMAND get_id(const std::string& command) {
	if (command_map_.find(command) == command_map_.end()) {
		return HOTKEY_NULL;
	}

	return hotkey_list_[command_map_[command]].id;
}

std::string get_names(HOTKEY_COMMAND id) {

	std::vector<std::string> names;
	BOOST_FOREACH(const hotkey::hotkey_item& item, hotkeys_) {
		if (item.get_id() == id && (!item.null()) ) {
			names.push_back(item.get_name());
		}
	}

	return boost::algorithm::join(names, ", ");
}

std::string get_names(std::string id) {

	std::vector<std::string> names;
	BOOST_FOREACH(const hotkey::hotkey_item& item, hotkeys_) {
		if (item.get_command() == id && (!item.null()) ) {
			names.push_back(item.get_name());
		}
	}

	return boost::algorithm::join(names, ", ");
}


HOTKEY_COMMAND get_hotkey_command(const std::string& command)
{
	if (command_map_.find(command) != command_map_.end()) {
		return HOTKEY_NULL;
	}

	return hotkey_list_[command_map_[command]].id;
}

hotkey_item& get_hotkey(int mouse, int joystick, int button, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {

		if (!(itor->is_in_active_scope())) {
			continue;
		}

		if ( itor->get_shift() != shift || itor->get_ctrl() != ctrl
				|| itor->get_cmd() != cmd || itor->get_alt() != alt ) {
			continue;
		}

		if ( itor->get_joystick() == joystick && itor->get_button() == button
				&& itor->get_hat() == hat && itor->get_value() == value
				&& itor->get_mouse() == mouse ) {
			return *itor;
		}
	}

	return null_hotkey_;
}

hotkey_item& get_hotkey(int character, int keycode,
		bool shift, bool ctrl,	bool cmd, bool alt)
{
	std::vector<hotkey_item>::iterator itor;

	DBG_G << "getting hotkey: char=" << lexical_cast<std::string>(character)
		<< " keycode="  << lexical_cast<std::string>(keycode) << " "
		<< (shift ? "shift," : "")
		<< (ctrl  ? "ctrl,"  : "")
		<< (cmd   ? "cmd,"   : "")
		<< (alt   ? "alt,"   : "")
		<< "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (0 < character && character < 64 && ctrl) {
		if (shift) {
			character += 64;
		} else {
			character += 96;
		}
		/// @todo
		DBG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift) {
		character -= 32; }

	bool found = false;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_character() != -1) {
			if (character == itor->get_character()) {
				if (ctrl == itor->get_ctrl()
						&& cmd == itor->get_cmd()
						&& alt == itor->get_alt()) {
					if (itor->is_in_active_scope()) {
						DBG_G << "Could match by character..." << "yes\n";
						found = true;
						break;
					} else {
						DBG_G << "Could match by character..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by character..." << "but modifiers different\n";
			}
		} else if (itor->get_keycode() != -1) {
			if (keycode == itor->get_keycode()) {
				if (shift == itor->get_shift()
						&& ctrl == itor->get_ctrl()
						&& cmd  == itor->get_cmd()
						&& alt  == itor->get_alt()) {
					if (itor->is_in_active_scope()) {
						DBG_G << "Could match by keycode..." << "yes\n";
						found = true;
						break;
					} else {
						DBG_G << "Could match by keycode..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by keycode..." << "but modifiers different\n";
			}
		}
		if (found) { break; }
	}

	if (!found) {
		return null_hotkey_; }

	return *itor;
}

void add_hotkey(const hotkey_item& item) {
	hotkeys_.push_back(item);
}

hotkey_item& get_hotkey(const SDL_JoyButtonEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(-1, event.which, event.button, -1, -1, shift, ctrl, cmd, alt);
}

hotkey_item& get_hotkey(const SDL_JoyHatEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(-1, event.which, -1, event.hat, event.value, shift, ctrl, cmd, alt);
}

static hotkey_item& get_hotkey(const SDL_MouseButtonEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(event.which, -1, event.button, -1, -1, shift, ctrl, cmd, alt);
}

hotkey_item& get_hotkey(const SDL_KeyboardEvent& event)
{
	return get_hotkey(event.keysym.unicode, event.keysym.sym,
			(event.keysym.mod & KMOD_SHIFT) != 0,
			(event.keysym.mod & KMOD_CTRL)  != 0,
			(event.keysym.mod & KMOD_META)  != 0,
			(event.keysym.mod & KMOD_ALT)   != 0
			);
}

std::vector<hotkey_item>& get_hotkeys()
{
	return hotkeys_;
}

basic_handler::basic_handler(display* disp, command_executor* exec) : disp_(disp), exec_(exec) {}

void basic_handler::handle_event(const SDL_Event& event)
{
	//TODO this code path is never called?

	if (disp_ == NULL) {
		return;
	}

	switch (event.type) {
	case SDL_KEYDOWN:
		// If we're in a dialog we only want to handle items that are explicitly
		// handled by the executor.
		// If we're not in a dialog we can call the regular key event handler.
		if (!gui::in_dialog()) {
			key_event(*disp_, event.key,exec_);
		} else if (exec_ != NULL) {
			key_event_execute(*disp_, event.key,exec_);
		}
		break;
	case SDL_JOYBUTTONDOWN:
		if (!gui::in_dialog()) {
			jbutton_event(*disp_, event.jbutton,exec_);
		} else if (exec_ != NULL) {
			jbutton_event_execute(*disp_, event.jbutton,exec_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!gui::in_dialog()) {
			mbutton_event(*disp_, event.button,exec_);
		} else if (exec_ != NULL) {
			mbutton_event_execute(*disp_, event.button,exec_);
		}
		break;
	}
}

void mbutton_event(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor)
{
	mbutton_event_execute(disp, event, executor);
}


void jbutton_event(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor)
{
	jbutton_event_execute(disp, event, executor);
}

void jhat_event(display& disp, const SDL_JoyHatEvent& event, command_executor* executor)
{
	jhat_event_execute(disp, event, executor);
}

void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	if(event.keysym.sym == SDLK_ESCAPE && disp.in_game()) {
		LOG_G << "escape pressed..showing quit\n";
		const int res = gui2::show_message(disp.video(), _("Quit"),
				_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
		if(res != gui2::twindow::CANCEL) {
			throw end_level_exception(QUIT);
		} else {
			return;
		}
	}

	key_event_execute(disp, event,executor);
}

void mbutton_event_execute(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (hk->null()) {
		return;
	}

	execute_command(disp, hk->get_id(), executor);
	executor->set_button_state(disp);
}

void jbutton_event_execute(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (hk->null()) {
		return;
	}

	execute_command(disp, hk->get_id(), executor);
	executor->set_button_state(disp);
}

void jhat_event_execute(display& disp, const SDL_JoyHatEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (hk->null()) {
		return;
	}

	execute_command(disp, hk->get_id(), executor);
	executor->set_button_state(disp);
}


void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);

#if 0
	// This is not generally possible without knowing keyboard layout.
	if (hk->null()) {
		//no matching hotkey was found, but try an in-exact match.
		hk = &get_hotkey(event, true);
	}
#endif

	if (hk->null()) {
		return;
	}

	execute_command(disp, hk->get_id(), executor);
	executor->set_button_state(disp);
}

bool command_executor::execute_command(HOTKEY_COMMAND command, int /*index*/)
{
	switch(command) {
		case HOTKEY_CYCLE_UNITS:
			cycle_units();
			break;
		case HOTKEY_CYCLE_BACK_UNITS:
			cycle_back_units();
			break;
		case HOTKEY_ENDTURN:
			end_turn();
			break;
		case HOTKEY_UNIT_HOLD_POSITION:
			unit_hold_position();
			break;
		case HOTKEY_END_UNIT_TURN:
			end_unit_turn();
			break;
		case HOTKEY_LEADER:
			goto_leader();
			break;
		case HOTKEY_UNDO:
			undo();
			break;
		case HOTKEY_REDO:
			redo();
			break;
		case HOTKEY_UNIT_DESCRIPTION:
			unit_description();
			break;
		case HOTKEY_RENAME_UNIT:
			rename_unit();
			break;
		case HOTKEY_SAVE_GAME:
			save_game();
			break;
		case HOTKEY_SAVE_REPLAY:
			save_replay();
			break;
		case HOTKEY_SAVE_MAP:
			save_map();
			break;
		case HOTKEY_LOAD_GAME:
			load_game();
			break;
		case HOTKEY_TOGGLE_ELLIPSES:
			toggle_ellipses();
			break;
		case HOTKEY_TOGGLE_GRID:
			toggle_grid();
			break;
		case HOTKEY_STATUS_TABLE:
			status_table();
			break;
		case HOTKEY_RECALL:
			recall();
			break;
		case HOTKEY_RECRUIT:
			recruit();
			break;
		case hotkey::HOTKEY_REPEAT_RECRUIT:
			repeat_recruit();
			break;
		case HOTKEY_SPEAK:
			speak();
			break;
		case HOTKEY_SPEAK_ALLY:
			whisper();
			break;
		case HOTKEY_SPEAK_ALL:
			shout();
			break;
		case HOTKEY_CREATE_UNIT:
			create_unit();
			break;
		case HOTKEY_CHANGE_SIDE:
			change_side();
			break;
		case HOTKEY_PREFERENCES:
			preferences();
			break;
		case HOTKEY_OBJECTIVES:
			objectives();
			break;
		case HOTKEY_UNIT_LIST:
			unit_list();
			break;
		case HOTKEY_STATISTICS:
			show_statistics();
			break;
		case HOTKEY_STOP_NETWORK:
			stop_network();
			break;
		case HOTKEY_START_NETWORK:
			start_network();
			break;
		case HOTKEY_LABEL_TEAM_TERRAIN:
			label_terrain(true);
			break;
		case HOTKEY_LABEL_TERRAIN:
			label_terrain(false);
			break;
		case HOTKEY_CLEAR_LABELS:
			clear_labels();
			break;
		case HOTKEY_SHOW_ENEMY_MOVES:
			show_enemy_moves(false);
			break;
		case HOTKEY_BEST_ENEMY_MOVES:
			show_enemy_moves(true);
			break;
		case HOTKEY_DELAY_SHROUD:
			toggle_shroud_updates();
			break;
		case HOTKEY_UPDATE_SHROUD:
			update_shroud_now();
			break;
		case HOTKEY_CONTINUE_MOVE:
			continue_move();
			break;
		case HOTKEY_SEARCH:
			search();
			break;
		case HOTKEY_HELP:
			show_help();
			break;
		case HOTKEY_CHAT_LOG:
			show_chat_log();
			break;
		case HOTKEY_USER_CMD:
			user_command();
			break;
		case HOTKEY_CUSTOM_CMD:
			custom_command();
			break;
		case HOTKEY_AI_FORMULA:
			ai_formula();
			break;
		case HOTKEY_CLEAR_MSG:
			clear_messages();
			break;
		case HOTKEY_LANGUAGE:
			change_language();
			break;
		case HOTKEY_PLAY_REPLAY:
			play_replay();
			break;
		case HOTKEY_RESET_REPLAY:
			reset_replay();
			break;
		case HOTKEY_STOP_REPLAY:
			stop_replay();
			break;
		case HOTKEY_REPLAY_NEXT_TURN:
			replay_next_turn();
			break;
		case HOTKEY_REPLAY_NEXT_SIDE:
			replay_next_side();
			break;
		case HOTKEY_REPLAY_SHOW_EVERYTHING:
			replay_show_everything();
			break;
		case HOTKEY_REPLAY_SHOW_EACH:
			replay_show_each();
			break;
		case HOTKEY_REPLAY_SHOW_TEAM1:
			replay_show_team1();
			break;
		case HOTKEY_REPLAY_SKIP_ANIMATION:
			replay_skip_animation();
			break;
		case HOTKEY_WB_TOGGLE:
			whiteboard_toggle();
			break;
		case HOTKEY_WB_EXECUTE_ACTION:
			whiteboard_execute_action();
			break;
		case HOTKEY_WB_EXECUTE_ALL_ACTIONS:
			whiteboard_execute_all_actions();
			break;
		case HOTKEY_WB_DELETE_ACTION:
			whiteboard_delete_action();
			break;
		case HOTKEY_WB_BUMP_UP_ACTION:
			whiteboard_bump_up_action();
			break;
		case HOTKEY_WB_BUMP_DOWN_ACTION:
			whiteboard_bump_down_action();
			break;
		case HOTKEY_WB_SUPPOSE_DEAD:
			whiteboard_suppose_dead();
			break;
		case HOTKEY_LEFT_MOUSE_CLICK:
			left_mouse_click();
			break;
		case HOTKEY_RIGHT_MOUSE_CLICK:
			right_mouse_click();
			break;
		default:
			return false;
	}
	return true;
}

void execute_command(display& disp, HOTKEY_COMMAND command, command_executor* executor, int index)
{
	const int zoom_amount = 4;
	bool map_screenshot = false;

	if (executor != NULL) {
		if (!executor->can_execute_command(command, index)
				|| executor->execute_command(command, index)) {
			return;
		}
	}
	switch (command) {
		case HOTKEY_ZOOM_IN:
			disp.set_zoom(zoom_amount);
			break;
		case HOTKEY_ZOOM_OUT:
			disp.set_zoom(-zoom_amount);
			break;
		case HOTKEY_ZOOM_DEFAULT:
			disp.set_default_zoom();
			break;
		case HOTKEY_FULLSCREEN:
			preferences::set_fullscreen(!preferences::fullscreen());
			break;
		case HOTKEY_MAP_SCREENSHOT:
			if (!disp.in_game() && !disp.in_editor()) {
				break;
			}
			map_screenshot = true;
			/// @todo intentional fall through?
		case HOTKEY_SCREENSHOT: {
			std::string name = map_screenshot ? _("Map-Screenshot") : _("Screenshot");
			std::string filename = get_screenshot_dir() + "/" + name + "_";
			filename = get_next_filename(filename, ".bmp");
			int size = disp.screenshot(filename, map_screenshot);
			if (size > 0) {
				std::stringstream res;
				res << filename << " ( " << utils::si_string(size, true, _("unit_byte^B")) << " )";
				gui2::show_message(disp.video(), _("Screenshot done"), res.str());
			} else {
				gui2::show_message(disp.video(), _("Screenshot done"), "");
			}
			break;
		}
		case HOTKEY_ANIMATE_MAP:
			preferences::set_animate_map(!preferences::animate_map());
			break;
		case HOTKEY_MOUSE_SCROLL:
			preferences::enable_mouse_scroll(!preferences::mouse_scroll_enabled());
			break;
		case HOTKEY_ACCELERATED:
			preferences::set_turbo(!preferences::turbo());
			break;
		case HOTKEY_MUTE:
			{
				// look if both is not playing
				static struct before_muted_s
				{
					bool playing_sound,playing_music;
					before_muted_s() : playing_sound(false),playing_music(false){}
				} before_muted;
				if (preferences::music_on() || preferences::sound_on())
				{
					// then remember settings and mute both
					before_muted.playing_sound = preferences::sound_on();
					before_muted.playing_music = preferences::music_on();
					preferences::set_sound(false);
					preferences::set_music(false);
				}
				else
				{
					// then set settings before mute
					preferences::set_sound(before_muted.playing_sound);
					preferences::set_music(before_muted.playing_music);
				}
			}
			break;
		case HOTKEY_QUIT_GAME: {
			if (disp.in_game()) {
				DBG_G << "is in game -- showing quit message\n";
				const int res = gui2::show_message(disp.video(), _("Quit"),
						_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
				if (res != gui2::twindow::CANCEL) {
					throw end_level_exception(QUIT);
				}
			}
			break;
		}
		default:
			DBG_G << "command_executor: unknown command number " << command << ", ignoring.\n";
			break;
	}
}

void command_executor::set_button_state(display& disp) {

	BOOST_FOREACH(const theme::action& action, disp.get_theme().actions()) {

		gui::button* button = disp.find_action_button(action.get_id());
		bool enabled = false;
		BOOST_FOREACH(const std::string& command, action.items()) {

			hotkey::HOTKEY_COMMAND command_id = hotkey::get_id(command);
			std::string tooltip = action.tooltip();
			if (file_exists(game_config::path + "/images/icons/action/" + command + "_30.png" ))
				button->set_overlay("icons/action/" + command);
			if (!tooltip.empty())
				button->set_tooltip_string(tooltip);

			bool can_execute = can_execute_command(command_id);
			if (!can_execute) continue;
			enabled = true;


			ACTION_STATE state = get_action_state(command_id, -1);
			switch (state) {
			case ACTION_SELECTED:
			case ACTION_ON:
				button->set_check(true);
				break;
			case ACTION_OFF:
			case ACTION_DESELECTED:
				button->set_check(false);
				break;
			case ACTION_STATELESS:
				break;
			default:
				break;
			}

			break;
		}
		button->enable(enabled);
	}
}

void command_executor::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool /*context_menu*/, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (items.empty()) return;

	std::vector<std::string> menu = get_menu_images(gui, items);
	int res = 0;
	{
		gui::dialog mmenu = gui::dialog(gui,"","",
				gui::MESSAGE, gui::dialog::hotkeys_style);
		mmenu.set_menu(menu);
		res = mmenu.show(xloc, yloc);
	} // This will kill the dialog.
	if (res < 0 || size_t(res) >= items.size()) return;

	const theme::menu* submenu = gui.get_theme().get_menu_item(items[res]);
	if (submenu) {
		int y,x;
		SDL_GetMouseState(&x,&y);
		this->show_menu(submenu->items(), x, y, submenu->is_context(), gui);
	} else {
		const hotkey::HOTKEY_COMMAND cmd = hotkey::get_id(items[res]);
		hotkey::execute_command(gui,cmd,this,res);
		set_button_state(gui);
	}
}

void command_executor::execute_action(const std::vector<std::string>& items_arg, int /*xloc*/, int /*yloc*/, bool /*context_menu*/, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (items.empty()) {
		return;
	}

	hotkey::HOTKEY_COMMAND command;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		command = hotkey::get_id(*i);
		if (can_execute_command(command)) {
			hotkey::execute_command(gui, command, this);
			set_button_state(gui);
		}
		++i;
	}
}

std::string command_executor::get_menu_image(display& disp, const std::string& command, int index) const {

	const std::string base_image_name = "icons/action/" + command + "_25.png";
	const std::string pressed_image_name = "icons/action/" + command + "_25-pressed.png";

	const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(command);
	const hotkey::ACTION_STATE state = get_action_state(hk, index);

	const theme::menu* menu = disp.get_theme().get_menu_item(command);
	if (menu)
		return "buttons/fold-arrow.png";
	//if (hk == hotkey::HOTKEY_NULL)

	if (file_exists(game_config::path + "/images/" + base_image_name)) {
		switch (state) {
			case ACTION_ON:
			case ACTION_SELECTED:
				return pressed_image_name;
			default:
				return base_image_name;
		}
	}

	switch (get_action_state(hk, index)) {
		case ACTION_ON:
			return game_config::images::checked_menu;
		case ACTION_OFF:
			return game_config::images::unchecked_menu;
		case ACTION_SELECTED:
			return game_config::images::selected_menu;
		case ACTION_DESELECTED:
			return game_config::images::deselected_menu;
		default: return get_action_image(hk, index);
	}
}

std::vector<std::string> command_executor::get_menu_images(display& disp, const std::vector<std::string>& items) {
	std::vector<std::string> result;
	bool has_image = false;

	for (size_t i = 0; i < items.size(); ++i) {
		std::string const& item = items[i];
		const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(item);

		std::stringstream str;
		//see if this menu item has an associated image
		std::string img(get_menu_image(disp, item, i));
		if (img.empty() == false) {
			has_image = true;
			str << IMAGE_PREFIX << img << COLUMN_SEPARATOR;
		}

		if (hk == hotkey::HOTKEY_NULL) {
			const theme::menu* menu = disp.get_theme().get_menu_item(item);
			if (menu)
				str << menu->title();
			else
				str << item.substr(0, item.find_last_not_of(' ') + 1) << COLUMN_SEPARATOR;
		} else {
			std::string desc = hotkey::get_description(item);
			if (hk == HOTKEY_ENDTURN) {
				const theme::action *b = disp.get_theme().get_action_item("button-endturn");
				if (b) {
					desc = b->title();
				}
			}
			str << desc << COLUMN_SEPARATOR << hotkey::get_names(hk);
		}

		result.push_back(str.str());
	}
	//If any of the menu items have an image, create an image column
	if (has_image) {
		for (std::vector<std::string>::iterator i = result.begin(); i != result.end(); ++i) {
			if (*(i->begin()) != IMAGE_PREFIX) {
				i->insert(i->begin(), COLUMN_SEPARATOR);
			}
		}
	}
	return result;
}

}
