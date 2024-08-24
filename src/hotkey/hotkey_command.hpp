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

#pragma once

#include "global.hpp"
#include "tooltips.hpp"
#include "tstring.hpp"

#include <bitset>
#include <functional>
#include <map>

class config;

namespace hotkey {

/**
 * Available hotkey scopes. The scope is used to allow command from
 * non-overlapping areas of the game share the same key
 */
enum scope {
	SCOPE_MAIN_MENU,
	SCOPE_GAME,
	SCOPE_EDITOR,
	SCOPE_COUNT,
};

// For some reason std::bitset::operator| is not constexpr, so we'll construct the bitset with these values
// FIXME: unify these with the enum above. Right now these are the proper bitmasks to initialize a bitset,
// while the values above are used as indices to access the bits of the bitset.
constexpr uint32_t scope_game   = 1 << SCOPE_GAME;
constexpr uint32_t scope_editor = 1 << SCOPE_EDITOR;
constexpr uint32_t scope_main   = 1 << SCOPE_MAIN_MENU;

enum HOTKEY_COMMAND {
	HOTKEY_CYCLE_UNITS, HOTKEY_CYCLE_BACK_UNITS,
	HOTKEY_UNIT_HOLD_POSITION,
	HOTKEY_END_UNIT_TURN, HOTKEY_LEADER,
	HOTKEY_UNDO, HOTKEY_REDO,
	HOTKEY_ZOOM_IN, HOTKEY_ZOOM_OUT, HOTKEY_ZOOM_DEFAULT,
	HOTKEY_FULLSCREEN, HOTKEY_SCREENSHOT, HOTKEY_MAP_SCREENSHOT, HOTKEY_ACCELERATED,
	HOTKEY_TERRAIN_DESCRIPTION,
	HOTKEY_UNIT_DESCRIPTION, HOTKEY_RENAME_UNIT, HOTKEY_DELETE_UNIT,
	HOTKEY_SAVE_GAME, HOTKEY_SAVE_REPLAY, HOTKEY_SAVE_MAP, HOTKEY_LOAD_GAME,
	HOTKEY_RECRUIT, HOTKEY_REPEAT_RECRUIT, HOTKEY_RECALL, HOTKEY_ENDTURN,
	HOTKEY_TOGGLE_ELLIPSES, HOTKEY_TOGGLE_GRID, HOTKEY_STATUS_TABLE, HOTKEY_MUTE, HOTKEY_MOUSE_SCROLL,
	HOTKEY_SPEAK, HOTKEY_CREATE_UNIT, HOTKEY_CHANGE_SIDE, HOTKEY_KILL_UNIT, HOTKEY_PREFERENCES, HOTKEY_TELEPORT_UNIT,
	HOTKEY_OBJECTIVES, HOTKEY_UNIT_LIST, HOTKEY_STATISTICS, HOTKEY_STOP_NETWORK, HOTKEY_START_NETWORK, HOTKEY_SURRENDER, HOTKEY_QUIT_GAME, HOTKEY_QUIT_TO_DESKTOP,
	HOTKEY_LABEL_TEAM_TERRAIN, HOTKEY_LABEL_TERRAIN, HOTKEY_CLEAR_LABELS,HOTKEY_SHOW_ENEMY_MOVES, HOTKEY_BEST_ENEMY_MOVES,
	HOTKEY_DELAY_SHROUD, HOTKEY_UPDATE_SHROUD, HOTKEY_CONTINUE_MOVE,
	HOTKEY_SEARCH, HOTKEY_SPEAK_ALLY, HOTKEY_SPEAK_ALL, HOTKEY_HELP,
	HOTKEY_HELP_ABOUT_SAVELOAD,
	HOTKEY_CHAT_LOG, HOTKEY_LANGUAGE, HOTKEY_ANIMATE_MAP,

	// Replay
	HOTKEY_REPLAY_PLAY, HOTKEY_REPLAY_RESET, HOTKEY_REPLAY_STOP, HOTKEY_REPLAY_NEXT_TURN,
	HOTKEY_REPLAY_NEXT_SIDE, HOTKEY_REPLAY_NEXT_MOVE, HOTKEY_REPLAY_SHOW_EVERYTHING,
	HOTKEY_REPLAY_SHOW_EACH, HOTKEY_REPLAY_SHOW_TEAM1,
	HOTKEY_REPLAY_SKIP_ANIMATION,
	HOTKEY_REPLAY_EXIT,

	// Controls
	HOTKEY_SELECT_HEX, HOTKEY_DESELECT_HEX,
	HOTKEY_MOVE_ACTION, HOTKEY_SELECT_AND_ACTION, HOTKEY_TOUCH_HEX,

	// Camera movement
	HOTKEY_SCROLL_UP, HOTKEY_SCROLL_DOWN, HOTKEY_SCROLL_LEFT, HOTKEY_SCROLL_RIGHT,

	// Dialog control
	HOTKEY_CANCEL, HOTKEY_OKAY,

	// Whiteboard commands
	HOTKEY_WB_TOGGLE,
	HOTKEY_WB_EXECUTE_ACTION, HOTKEY_WB_EXECUTE_ALL_ACTIONS,
	HOTKEY_WB_DELETE_ACTION,
	HOTKEY_WB_BUMP_UP_ACTION, HOTKEY_WB_BUMP_DOWN_ACTION,
	HOTKEY_WB_SUPPOSE_DEAD,

	// Misc.
	HOTKEY_USER_CMD,
	HOTKEY_CUSTOM_CMD,
	HOTKEY_AI_FORMULA,
	HOTKEY_CLEAR_MSG,
	HOTKEY_LABEL_SETTINGS,
	HOTKEY_ACHIEVEMENTS,

	// Minimap
	HOTKEY_MINIMAP_CODING_TERRAIN, HOTKEY_MINIMAP_CODING_UNIT,
	HOTKEY_MINIMAP_DRAW_UNITS, HOTKEY_MINIMAP_DRAW_VILLAGES, HOTKEY_MINIMAP_DRAW_TERRAIN,

	// Multiplayer
	HOTKEY_MP_START_GAME,

	/* Gui2 specific hotkeys. */
	TITLE_SCREEN__RELOAD_WML,
	TITLE_SCREEN__NEXT_TIP,
	TITLE_SCREEN__PREVIOUS_TIP,
	TITLE_SCREEN__CAMPAIGN,
	TITLE_SCREEN__MULTIPLAYER,
	TITLE_SCREEN__ADDONS,
	TITLE_SCREEN__CORES,
	TITLE_SCREEN__EDITOR,
	TITLE_SCREEN__CREDITS,
	TITLE_SCREEN__TEST,
	GLOBAL__HELPTIP,
	LUA_CONSOLE,

	HOTKEY_WML,

	/* Editor commands */
	HOTKEY_EDITOR_CUSTOM_TODS,
	HOTKEY_EDITOR_PARTIAL_UNDO,
	HOTKEY_EDITOR_EDIT_UNIT,

	// Palette
	HOTKEY_EDITOR_PALETTE_ITEM_SWAP, HOTKEY_EDITOR_PALETTE_ITEMS_CLEAR,
	HOTKEY_EDITOR_PALETTE_GROUPS, HOTKEY_EDITOR_PALETTE_UPSCROLL, HOTKEY_EDITOR_PALETTE_DOWNSCROLL,

	HOTKEY_EDITOR_REMOVE_LOCATION,
	HOTKEY_EDITOR_PLAYLIST,
	HOTKEY_EDITOR_SCHEDULE,
	HOTKEY_EDITOR_LOCAL_TIME,
	HOTKEY_EDITOR_UNIT_FACING,

	HOTKEY_EDITOR_HELP_TEXT_SHOWN,

	// Unit
	HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT, HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE,
	HOTKEY_EDITOR_UNIT_CHANGE_ID, HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL,

	// Brushes
	HOTKEY_EDITOR_BRUSH_NEXT, HOTKEY_EDITOR_BRUSH_DEFAULT,
	HOTKEY_EDITOR_BRUSH_1, HOTKEY_EDITOR_BRUSH_2, HOTKEY_EDITOR_BRUSH_3, HOTKEY_EDITOR_BRUSH_SW_NE, HOTKEY_EDITOR_BRUSH_NW_SE,

	// Tools
	HOTKEY_EDITOR_TOOL_NEXT,
	HOTKEY_EDITOR_TOOL_PAINT, HOTKEY_EDITOR_TOOL_FILL,
	HOTKEY_EDITOR_TOOL_SELECT, HOTKEY_EDITOR_TOOL_STARTING_POSITION, HOTKEY_EDITOR_TOOL_LABEL,
	HOTKEY_EDITOR_TOOL_UNIT, HOTKEY_EDITOR_TOOL_VILLAGE, HOTKEY_EDITOR_TOOL_ITEM,

	// Select
	HOTKEY_EDITOR_SELECT_ALL, HOTKEY_EDITOR_SELECT_INVERSE,	HOTKEY_EDITOR_SELECT_NONE,
	// Clipboard
	HOTKEY_EDITOR_CLIPBOARD_PASTE,
	HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW,
	HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL,
	// Selection
	HOTKEY_EDITOR_SELECTION_CUT, HOTKEY_EDITOR_SELECTION_COPY,
	HOTKEY_EDITOR_SELECTION_ROTATE, HOTKEY_EDITOR_SELECTION_FLIP,
	HOTKEY_EDITOR_SELECTION_FILL,
	HOTKEY_EDITOR_SELECTION_EXPORT,
	HOTKEY_EDITOR_SELECTION_RANDOMIZE,

	// Map
	HOTKEY_EDITOR_MAP_NEW, HOTKEY_EDITOR_MAP_LOAD, HOTKEY_EDITOR_MAP_SAVE,
	HOTKEY_EDITOR_MAP_SAVE_AS, HOTKEY_EDITOR_MAP_SAVE_ALL,
	HOTKEY_EDITOR_MAP_REVERT, HOTKEY_EDITOR_MAP_INFO,
	HOTKEY_EDITOR_MAP_CLOSE,
	HOTKEY_EDITOR_MAP_SWITCH,
	HOTKEY_EDITOR_MAP_RESIZE,
	HOTKEY_EDITOR_MAP_GENERATE, HOTKEY_EDITOR_MAP_APPLY_MASK,
	HOTKEY_EDITOR_MAP_CREATE_MASK_TO,

	// Transitions
	HOTKEY_EDITOR_UPDATE_TRANSITIONS, HOTKEY_EDITOR_TOGGLE_TRANSITIONS,
	HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS, HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS,

	// Refresh
	HOTKEY_EDITOR_REFRESH,
	HOTKEY_EDITOR_REFRESH_IMAGE_CACHE,

	// Draw
	HOTKEY_EDITOR_DRAW_COORDINATES, HOTKEY_EDITOR_DRAW_TERRAIN_CODES, HOTKEY_EDITOR_DRAW_NUM_OF_BITMAPS,

	// Side
	HOTKEY_EDITOR_SIDE_NEW,
	HOTKEY_EDITOR_SIDE_EDIT,
	HOTKEY_EDITOR_SIDE_REMOVE,

	// Area
	HOTKEY_EDITOR_AREA_REMOVE,
	HOTKEY_EDITOR_AREA_ADD,
	HOTKEY_EDITOR_AREA_SAVE,
	HOTKEY_EDITOR_AREA_RENAME,

	// Addons
	HOTKEY_EDITOR_PBL,
	HOTKEY_EDITOR_CHANGE_ADDON_ID,
	HOTKEY_EDITOR_SELECT_ADDON,
	HOTKEY_EDITOR_OPEN_ADDON,

	// Scenario
	HOTKEY_EDITOR_SCENARIO_EDIT,
	HOTKEY_EDITOR_SCENARIO_NEW,
	HOTKEY_EDITOR_SCENARIO_SAVE_AS,

	/* This item must stay at the end since it is used as terminator for iterating. */
	HOTKEY_NULL
};

enum HOTKEY_CATEGORY {
	HKCAT_GENERAL,
	HKCAT_SAVING,
	HKCAT_MAP,
	HKCAT_UNITS,
	HKCAT_CHAT,
	HKCAT_REPLAY,
	HKCAT_WHITEBOARD,
	HKCAT_SCENARIO,
	HKCAT_PALETTE,
	HKCAT_TOOLS,
	HKCAT_CLIPBOARD,
	HKCAT_DEBUG,
	HKCAT_CUSTOM,
	HKCAT_PLACEHOLDER // Keep this one last
};

/** Gets the display name for a given hotkey category. */
t_string get_translatable_category_name(HOTKEY_CATEGORY category);

typedef std::bitset<SCOPE_COUNT> hk_scopes;

/**
 * hotkey_command uses t_string which might cause bugs when used at program startup,
 * so use this for the master hotkey list (and only there).
 */
struct hotkey_command_temp;

/**
 * Stores all information related to functions that can be bound to hotkeys.
 * this is currently a semi struct: it haves a constructor, but only const-public members.
 */
struct hotkey_command
{
	hotkey_command() = delete;

	/** Constructs a new command from a temporary static hotkey object. */
	hotkey_command(const hotkey_command_temp& temp_command);

	/** @todo: see if we can remove this with c++20. Aggregate initialization with try_emplace?*/
	hotkey_command(HOTKEY_COMMAND cmd, const std::string& id, const t_string& desc, bool hidden, bool toggle, hk_scopes scope, HOTKEY_CATEGORY category, const t_string& tooltip);

	hotkey_command(const hotkey_command&) = default;
	hotkey_command& operator=(const hotkey_command&) = default;

	/** The command associated with this hotkey. Does not need to be unique. */
	HOTKEY_COMMAND command;

	/** The unique ID. */
	std::string id;

	// since the wml_menu hotkey_command s can have different textdomains we need t_string now.
	t_string description;

	/** If hidden then don't show the command in the hotkey preferences. */
	bool hidden;

	/**
	 * Toggle hotkeys have some restrictions on what can be bound to them.
	 * They require a binding that has two states, "pressed" and "released"
	 */
	bool toggle;

	/** The visibility scope of the command. */
	hk_scopes scope;

	/** The category of the command. */
	HOTKEY_CATEGORY category;

	t_string tooltip;

	/** checks weather this is the null hotkey_command */
	bool null() const;

	/** returns the command that is treated as null */
	static const hotkey_command& null_command();

	/**
	 * the execute_command argument was changed from HOTKEY_COMMAND to hotkey_command,
	 * to be able to call it with HOTKEY_COMMAND, this function was created
	 */
	static const hotkey_command& get_command_by_command(HOTKEY_COMMAND command);
};

class scope_changer
{
public:
	scope_changer();
	explicit scope_changer(hk_scopes new_scopes, bool restore = true);
	~scope_changer();
private:
	hk_scopes prev_scope_active_;
	const bool restore_;
};

/**
 * returns a container that contains all currently active hotkey_commands.
 * everything that wants a hotkey, must be in this container
 */
const std::map<std::string_view, hotkey::hotkey_command>& get_hotkey_commands();

/** returns the hotkey_command with the given name */
NOT_DANGLING const hotkey_command& get_hotkey_command(const std::string& command);

bool is_scope_active(scope s);
bool is_scope_active(hk_scopes s);

bool has_hotkey_command(const std::string& id);

/**
 * RAII helper class to control the lifetime of a WML hotkey_command.
 */
class wml_hotkey_record
{
public:
	wml_hotkey_record() = default;

	/** Don't allow copying so objects don't get erased early. */
	wml_hotkey_record(const wml_hotkey_record&) = delete;
	const wml_hotkey_record& operator=(const wml_hotkey_record&) = delete;

	/** But we *do* want move semantics. */
	wml_hotkey_record(wml_hotkey_record&&) = default;
	wml_hotkey_record& operator=(wml_hotkey_record&&) = default;

	/** Registers a hotkey_command for a WML hotkey with the given ID if one does not already exist. */
	wml_hotkey_record(const std::string& id, const t_string& description, const config& default_hotkey);

	~wml_hotkey_record();

private:
	/** Handles removing the associated hotkey_command on this object's destruction. */
	std::function<void()> cleanup_{};
};

void init_hotkey_commands();
}
