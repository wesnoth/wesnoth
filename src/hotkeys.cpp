/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "construct_dialog.hpp"
#include "display.hpp"
#include "foreach.hpp"
#include "hotkeys.hpp"
#include "game_end_exceptions.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "wesconfig.h"
#include "wml_separators.hpp"

static lg::log_domain log_config("config");
#define ERR_G LOG_STREAM(err, lg::general)
#define LOG_G LOG_STREAM(info, lg::general)
#define DBG_G LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err, log_config)

namespace {

const struct {
	hotkey::HOTKEY_COMMAND id;
	const char* command;
	const char* description;
	bool hidden;
	hotkey::scope scope;
} hotkey_list_[] = {
	{ hotkey::HOTKEY_CYCLE_UNITS, "cycle", N_("Next Unit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CYCLE_BACK_UNITS, "cycleback", N_("Previous Unit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_ADD_WAYPOINT, "addwaypoint", N_("Add waypoint"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_UNIT_HOLD_POSITION, "holdposition", N_("Hold Position"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_END_UNIT_TURN, "endunitturn", N_("End Unit Turn"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_LEADER, "leader", N_("Leader"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_UNDO, "undo", N_("Undo"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_REDO, "redo", N_("Redo"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_ZOOM_IN, "zoomin", N_("Zoom In"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_ZOOM_OUT, "zoomout", N_("Zoom Out"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_ZOOM_DEFAULT, "zoomdefault", N_("Default Zoom"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_FULLSCREEN, "fullscreen", N_("Toggle Full Screen"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_SCREENSHOT, "screenshot", N_("Screenshot"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_MAP_SCREENSHOT, "mapscreenshot", N_("Map Screenshot"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_ACCELERATED, "accelerated", N_("Accelerated"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_UNIT_DESCRIPTION, "describeunit", N_("Unit Description"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_RENAME_UNIT, "renameunit", N_("Rename Unit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SAVE_GAME, "save", N_("Save Game"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SAVE_REPLAY, "savereplay", N_("Save Replay"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SAVE_MAP, "savemap", N_("Save Map"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_LOAD_GAME, "load", N_("Load Game"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_RECRUIT, "recruit", N_("Recruit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPEAT_RECRUIT, "repeatrecruit", N_("Repeat Recruit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_RECALL, "recall", N_("Recall"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_ENDTURN, "endturn", N_("End Turn"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_TOGGLE_ELLIPSES, "toggleellipses", N_("Toggle Ellipses"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_TOGGLE_GRID, "togglegrid", N_("Toggle Grid"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_MOUSE_SCROLL, "mousescroll", N_("Mouse Scrolling"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_STATUS_TABLE, "statustable", N_("Status Table"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_MUTE, "mute", N_("Mute"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_SPEAK, "speak", N_("Speak"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CREATE_UNIT, "createunit", N_("Create Unit (Debug!)"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CHANGE_SIDE, "changeside", N_("Change Side (Debug!)"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_PREFERENCES, "preferences", N_("Preferences"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_OBJECTIVES, "objectives", N_("Scenario Objectives"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_UNIT_LIST, "unitlist", N_("Unit List"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_STATISTICS, "statistics", N_("Statistics"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_STOP_NETWORK, "stopnetwork", N_("Stop"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_START_NETWORK, "startnetwork", N_("Play"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_QUIT_GAME, "quit", N_("Quit Game"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_LABEL_TEAM_TERRAIN, "labelteamterrain", N_("Set Team Label"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_LABEL_TERRAIN, "labelterrain", N_("Set Label"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CLEAR_LABELS, "clearlabels", N_("Clear Labels"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", N_("Show Enemy Moves"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", N_("Best Possible Enemy Moves"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_PLAY_REPLAY, "playreplay", N_("Play"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_RESET_REPLAY, "resetreplay", N_("Reset"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_STOP_REPLAY, "stopreplay", N_("Stop"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_NEXT_TURN, "replaynextturn", N_("Next Turn"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_NEXT_SIDE, "replaynextside", N_("Next Side"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING, "replayshoweverything",
	  N_("Full map"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_SHOW_EACH, "replayshoweach",
	  N_("Each team"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_SHOW_TEAM1, "replayshowteam1",
	  N_("Team 1"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_REPLAY_SKIP_ANIMATION, "replayskipanimation", N_("Skip animation"), false, hotkey::SCOPE_GAME },

#ifndef DISABLE_EDITOR
	{ hotkey::HOTKEY_EDITOR_QUIT_TO_DESKTOP, "editor-quit-to-desktop", N_("Quit to Desktop"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CLOSE_MAP, "editor-close-map", N_("Close Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SWITCH_MAP, "editor-switch-map", N_("Switch Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SETTINGS, "editor-settings", N_("Editor Settings"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_PARTIAL_UNDO, "editor-partial-undo", N_("Partial Undo"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_NEW, "editor-map-new", N_("New Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_LOAD, "editor-map-load", N_("Load Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE, "editor-map-save", N_("Save Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_SAVE_AS, "editor-map-save-as", N_("Save Map As"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_REVERT, "editor-map-revert", N_("Revert All Changes"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_INFO, "editor-map-info", N_("Map Information"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP, "editor-terrain-palette-swap",
		N_("Swap fore- and background terrains"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TOOL_NEXT, "editor-tool-next", N_("Next Tool"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TOOL_PAINT, "editor-tool-paint", N_("Paint Tool"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TOOL_FILL, "editor-tool-fill", N_("Fill Tool"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TOOL_SELECT, "editor-tool-select", N_("Selection Tool"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION, "editor-tool-starting-position",
		N_("Set Starting Positions Tool"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_BRUSH_NEXT, "editor-brush-next", N_("Next Brush"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_BRUSH_DEFAULT, "editor-brush-default", N_("Default Brush"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CUT, "editor-cut", N_("Cut"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_COPY, "editor-copy", N_("Copy"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_PASTE, "editor-paste", N_("Paste"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_EXPORT_SELECTION_COORDS, "editor-export-selection-coords", N_("Export selected coordinates to system clipboard"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECT_ALL, "editor-select-all",
		 N_("Select All"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECT_INVERSE, "editor-select-inverse",
		 N_("Select Inverse"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECT_NONE, "editor-select-none",
		 N_("Select None"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, "editor-clipboard-rotate-cw",
		 N_("Rotate Clipboard Clockwise"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW, "editor-clipboard-rotate-ccw",
		 N_("Rotate Clipboard Counter-Clockwise"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, "editor-clipboard-flip-horizontal",
		N_("Flip Clipboard Horizontally"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL, "editor-clipboard-flip-vertical",
		N_("Flip Clipboard Vertically"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECTION_ROTATE, "editor-selection-rotate",
		N_("Rotate Selection"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FLIP, "editor-selection-flip",
		N_("Flip Selection"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECTION_FILL, "editor-selection-fill",
		N_("Fill Selection"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECTION_GENERATE, "editor-selection-generate",
		N_("Generate Tiles In Selection"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_SELECTION_RANDOMIZE, "editor-selection-randomize",
		N_("Randomize Tiles In Selection"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_RESIZE, "editor-map-resize",
		N_("Resize Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_ROTATE, "editor-map-rotate",
		N_("Rotate Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_GENERATE, "editor-map-generate",
		 N_("Generate Map"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_APPLY_MASK, "editor-map-apply-mask",
		 N_("Apply a Mask"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_MAP_CREATE_MASK_TO, "editor-map-create-mask-to",
		 N_("Create Mask"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_REFRESH, "editor-refresh",
		N_("Refresh Display"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_UPDATE_TRANSITIONS, "editor-update-transitions",
		N_("Update Terrain Transitions"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, "editor-auto-update-transitions",
		N_("Auto-update Terrain Transitions"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_REFRESH_IMAGE_CACHE, "editor-refresh-image-cache",
		N_("Refresh Image Cache"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_DRAW_COORDINATES, "editor-draw-coordinates",
		N_("Draw Hex Coordinates"), false, hotkey::SCOPE_EDITOR },
	{ hotkey::HOTKEY_EDITOR_DRAW_TERRAIN_CODES, "editor-draw-terrain-codes",
		N_("Draw Terrain Codes"), false, hotkey::SCOPE_EDITOR },


#endif

	{ hotkey::HOTKEY_DELAY_SHROUD, "delayshroud", N_("Delay Shroud Updates"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_UPDATE_SHROUD, "updateshroud", N_("Update Shroud Now"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CONTINUE_MOVE, "continue", N_("Continue Move"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SEARCH, "search", N_("Find Label or Unit"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SPEAK_ALLY, "speaktoally", N_("Speak to Ally"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_SPEAK_ALL, "speaktoall", N_("Speak to All"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_HELP, "help", N_("Help"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_CHAT_LOG, "chatlog", N_("View Chat Log"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_LANGUAGE, "changelanguage", N_("Change the language"), true, hotkey::SCOPE_GENERAL },

	{ hotkey::HOTKEY_USER_CMD, "command", N_("Enter user command"), false, hotkey::SCOPE_GENERAL },
	{ hotkey::HOTKEY_CUSTOM_CMD, "customcommand", N_("Custom command"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_AI_FORMULA, "aiformula", N_("Run AI formula"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_CLEAR_MSG, "clearmessages", N_("Clear messages"), false, hotkey::SCOPE_GAME },
#ifdef USRCMD2
	{ hotkey::HOTKEY_USER_CMD_2, "usercommand#2", N_("User-Command#2"), false, hotkey::SCOPE_GAME },
	{ hotkey::HOTKEY_USER_CMD_3, "usercommand#3", N_("User-Command#3"), false, hotkey::SCOPE_GAME },
#endif
	{ hotkey::HOTKEY_NULL, NULL, NULL, true, hotkey::SCOPE_GENERAL }
};

std::vector<hotkey::hotkey_item> hotkeys_;
hotkey::hotkey_item null_hotkey_;

std::string hotkey_tag_name = "hotkey";

std::vector<bool> scope_active_(hotkey::SCOPE_COUNT, false);
}

namespace hotkey {


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

const std::string CLEARED_TEXT = "__none__";

hotkey_item::hotkey_item(HOTKEY_COMMAND id,
		const std::string& command, const t_string &description, bool hidden,
		scope s) :
	id_(id),
	command_(command),
	description_(description),
	scope_(s),
	type_(UNBOUND),
	character_(0),
	ctrl_(false),
	alt_(false),
	cmd_(false),
	keycode_(0),
	shift_(false),
	hidden_(hidden)
{
}

// There are two kinds of "key" values.  One refers to actual keys, like
// F1 or SPACE.  The other refers to characters produced, eg 'M' or ':'.
// For the latter, specifying shift+; doesn't make sense, because ; is
// already shifted on French keyboards, for example.  You really want to
// say ':', however that is typed.  However, when you say shift+SPACE,
// you're really referring to the space bar, as shift+SPACE usually just
// produces a SPACE character.
void hotkey_item::load_from_config(const config& cfg)
{
	const std::string& key = cfg["key"];

	alt_ = utils::string_bool(cfg["alt"]);
	cmd_ = utils::string_bool(cfg["cmd"]);
	ctrl_ = utils::string_bool(cfg["ctrl"]);
	shift_ = utils::string_bool(cfg["shift"]);

	if (key.empty()) return;

	if (key == CLEARED_TEXT)
	{
		type_ = hotkey_item::CLEARED;
		return;
	}

	wide_string wkey = utils::string_to_wstring(key);

	// They may really want a specific key on the keyboard: we assume
	// that any single character keyname is a character.
	if (wkey.size() > 1) {
		type_ = BY_KEYCODE;

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
		// isn't a different character from space, and control key
		// makes it go weird.  shift=yes should never be specified
		// on single characters (eg. key=m, shift=yes would be
		// key=M), but we don't want to break old preferences
		// files.
		type_ = BY_KEYCODE;
		keycode_ = wkey[0];
	} else {
		type_ = BY_CHARACTER;
		character_ = wkey[0];
	}
}

std::string hotkey_item::get_name() const
{
	std::stringstream str;
	if (type_ == BY_CHARACTER) {
		if (alt_)
			str << "alt+";
		if (cmd_)
			str << "cmd+";
		if (ctrl_)
			str << "ctrl+";
		str << static_cast<char>(character_);
	} else if (type_ == BY_KEYCODE) {
		if (alt_)
			str << "alt+";
		if (ctrl_)
			str << "ctrl+";
		if (shift_)
			str << "shift+";
		if (cmd_)
			str << "cmd+";
		str << SDL_GetKeyName(SDLKey(keycode_));
	}
	return str.str();
}

void hotkey_item::set_description(const t_string &description)
{
	description_ = description;
}
void hotkey_item::clear_hotkey()
{
	type_ = CLEARED;
}

void hotkey_item::set_key(int character, int keycode, bool shift, bool ctrl, bool alt, bool cmd)
{
	const std::string keyname = SDL_GetKeyName(SDLKey(keycode));

	LOG_G << "setting hotkey: char=" << lexical_cast<std::string>(character)
		   << " keycode="  << lexical_cast<std::string>(keycode) << " "
		   << (shift ? "shift," : "")
		   << (ctrl ? "ctrl," : "")
		   << (alt ? "alt," : "")
		   << (cmd ? "cmd," : "")
		   << "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (character < 64 && ctrl) {
		if (shift)
			character += 64;
		else
			character += 96;
		LOG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift)
		character -= 32;

	// We handle simple cases by character, others by the actual key.
	if (isprint(character) && !isspace(character)) {
		type_ = BY_CHARACTER;
		character_ = character;
		ctrl_ = ctrl;
		alt_ = alt;
		cmd_ = cmd;
		LOG_G << "type = BY_CHARACTER\n";
	} else {
		type_ = BY_KEYCODE;
		keycode_ = keycode;
		shift_ = shift;
		ctrl_ = ctrl;
		alt_ = alt;
		cmd_ = cmd;
		LOG_G << "type = BY_KEYCODE\n";
	}
}

manager::manager()
{
	init();
}

void manager::init()
{
	for (int i = 0; hotkey_list_[i].command; ++i) {
		hotkeys_.push_back(hotkey_item(hotkey_list_[i].id, hotkey_list_[i].command,
				"", hotkey_list_[i].hidden, hotkey_list_[i].scope));
	}
}

void manager::wipe()
{
	hotkeys_.clear();
}

manager::~manager()
{
	wipe();
}

scope_changer::scope_changer(const config& cfg, const std::string& hotkey_tag)
: cfg_(cfg)
, prev_tag_name_(hotkey_tag_name)
, prev_scope_active_(scope_active_)
{
	manager::wipe();
	manager::init();
	hotkey::load_descriptions();
	load_hotkeys(cfg_);
	set_hotkey_tag_name(hotkey_tag);
}

scope_changer::~scope_changer()
{
	scope_active_.swap(prev_scope_active_);
	manager::wipe();
	manager::init();
	hotkey::load_descriptions();
	set_hotkey_tag_name(prev_tag_name_);
	load_hotkeys(cfg_);
}

void load_descriptions()
{
	for (size_t i = 0; hotkey_list_[i].command; ++i) {
		if (i >= hotkeys_.size()) {
			ERR_G << "Hotkey list too short: " << hotkeys_.size() << "\n";
		}
		hotkeys_[i].set_description(t_string(hotkey_list_[i].description, PACKAGE "-lib"));
	}
}

void set_hotkey_tag_name(const std::string& name)
{
	hotkey_tag_name = name;
}

void load_hotkeys(const config& cfg)
{
	BOOST_FOREACH (const config &hk, cfg.child_range(hotkey_tag_name))
	{
		hotkey_item& h = get_hotkey(hk["command"]);
		if(h.get_id() != HOTKEY_NULL) {
			h.load_from_config(hk);
		}
	}
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children(hotkey_tag_name);

	for(std::vector<hotkey_item>::iterator i = hotkeys_.begin(); i != hotkeys_.end(); ++i) {
		if (i->hidden() || i->get_type() == hotkey_item::UNBOUND || !i->is_in_active_scope())
			continue;

		config& item = cfg.add_child(hotkey_tag_name);
		item["command"] = i->get_command();
		if (i->get_type() == hotkey_item::CLEARED)
		{
			item["key"] = CLEARED_TEXT;
			continue;
		}

		if (i->get_type() == hotkey_item::BY_KEYCODE) {
			item["key"] = SDL_GetKeyName(SDLKey(i->get_keycode()));
			item["shift"] = i->get_shift() ? "yes" : "no";
		} else if (i->get_type() == hotkey_item::BY_CHARACTER) {
			item["key"] = utils::wchar_to_string(i->get_character());
		}
		item["alt"] = i->get_alt() ? "yes" : "no";
		item["ctrl"] = i->get_ctrl() ? "yes" : "no";
		item["cmd"] = i->get_cmd() ? "yes" : "no";
	}
}

hotkey_item& get_hotkey(HOTKEY_COMMAND id)
{
	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_id() == id)
			break;
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(const std::string& command)
{
	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_command() == command)
			break;
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(int character, int keycode, bool shift, bool ctrl,
	bool alt, bool cmd)
{
	std::vector<hotkey_item>::iterator itor;

	DBG_G << "getting hotkey: char=" << lexical_cast<std::string>(character)
		   << " keycode="  << lexical_cast<std::string>(keycode) << " "
		   << (shift ? "shift," : "")
		   << (ctrl ? "ctrl," : "")
		   << (alt ? "alt," : "")
		   << (cmd ? "cmd," : "")
		   << "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (0 < character && character < 64 && ctrl) {
		if (shift)
			character += 64;
		else
			character += 96;
		DBG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift)
		character -= 32;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_type() == hotkey_item::BY_CHARACTER) {
			if (character == itor->get_character()) {
				if (ctrl == itor->get_ctrl()
						&& alt == itor->get_alt()
						&& cmd == itor->get_cmd()) {
					if (itor->is_in_active_scope()) {
						DBG_G << "Could match by character..." << "yes\n";
						break;
					} else {
						DBG_G << "Could match by character..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by character..." << "but modifiers different\n";
			}
		} else if (itor->get_type() == hotkey_item::BY_KEYCODE) {
			if (keycode == itor->get_keycode()) {
				if (shift == itor->get_shift()
						&& ctrl == itor->get_ctrl()
						&& alt == itor->get_alt()
						&& cmd == itor->get_cmd()) {
					if (itor->is_in_active_scope()) {
						DBG_G << "Could match by keycode..." << "yes\n";
						break;
					} else {
						DBG_G << "Could match by keycode..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by keycode..." << "but modifiers different\n";
			}
		}
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(const SDL_KeyboardEvent& event)
{
	return get_hotkey(event.keysym.unicode, event.keysym.sym,
			(event.keysym.mod & KMOD_SHIFT) != 0,
			(event.keysym.mod & KMOD_CTRL) != 0,
			(event.keysym.mod & KMOD_ALT) != 0,
			(event.keysym.mod & KMOD_LMETA) != 0
#ifdef __APPLE__
			|| (event.keysym.mod & KMOD_RMETA) != 0
#endif
			);
}

static void _get_visible_hotkey_itor(int index, std::vector<hotkey_item>::iterator& itor)
{
	int counter = 0;
	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->hidden() || !itor->is_in_active_scope())
			continue;

		if (index == counter)
			break;

		counter++;
	}
}

hotkey_item& get_visible_hotkey(int index)
{

	std::vector<hotkey_item>::iterator itor;
	_get_visible_hotkey_itor(index, itor);
	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

std::vector<hotkey_item>& get_hotkeys()
{
	return hotkeys_;
}

basic_handler::basic_handler(display* disp, command_executor* exec) : disp_(disp), exec_(exec) {}

void basic_handler::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYDOWN && disp_ != NULL) {

		//if we're in a dialog we only want to handle things that are explicitly handled
		//by the executor. If we're not in a dialog we can call the regular key event handler
		if(!gui::in_dialog()) {
			key_event(*disp_,event.key,exec_);
		} else if(exec_ != NULL) {
			key_event_execute(*disp_,event.key,exec_);
		}
	}
}


void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	if(event.keysym.sym == SDLK_ESCAPE && disp.in_game()) {
		LOG_G << "escape pressed..showing quit\n";
		const int res = gui::dialog(disp,_("Quit"),_("Do you really want to quit?"),gui::YES_NO).show();
		if(res == 0) {
			throw end_level_exception(QUIT);
		} else {
			return;
		}
	}

	key_event_execute(disp,event,executor);
}

void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);

#if 0
	// This is not generally possible without knowing keyboard layout.
	if(hk->null()) {
		//no matching hotkey was found, but try an in-exact match.
		hk = &get_hotkey(event, true);
	}
#endif

	if(hk->null())
		return;

	execute_command(disp,hk->get_id(),executor);
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
		case HOTKEY_ADD_WAYPOINT:
			add_waypoint();
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
#ifdef USRCMD2
		case HOTKEY_USER_CMD_2:
			//user_command();
			user_command_2();
			break;
		case HOTKEY_USER_CMD_3:
			user_command_3();
			break;
#endif

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
		 default:
			 return false;
	}
	return true;
}

void execute_command(display& disp, HOTKEY_COMMAND command, command_executor* executor, int index)
{
	const int zoom_amount = 4;
	bool map_screenshot = false;

	if(executor != NULL) {
		if(!executor->can_execute_command(command, index) || executor->execute_command(command, index))
		return;
	}
	switch(command) {
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
			if (!disp.in_game() && !disp.in_editor())
				break;
			map_screenshot = true;
		case HOTKEY_SCREENSHOT: {
			std::string name = map_screenshot ? _("Map-Screenshot") : _("Screenshot");
			std::string filename = get_screenshot_dir() + "/" + name + "_";
			filename = get_next_filename(filename, ".bmp");
			int size = disp.screenshot(filename, map_screenshot);
			if (size > 0) {
				std::stringstream res;
				res << filename << " ( " << size/1000000 <<" "<< (size/1000)%1000 << " kB )";
				gui::dialog(disp,_("Screenshot done"),res.str(),gui::MESSAGE).show();
			} else
				gui::dialog(disp,_("Screenshot failed"),"",gui::MESSAGE).show();
			break;
		}
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
					//then remember settings and mute both
					before_muted.playing_sound = preferences::sound_on();
					before_muted.playing_music = preferences::music_on();
					preferences::set_sound(false);
					preferences::set_music(false);
				}
				else
				{
					//then set settings before mute
					preferences::set_sound(before_muted.playing_sound);
					preferences::set_music(before_muted.playing_music);
				}
			}
			break;
		case HOTKEY_QUIT_GAME: {
			if(disp.in_game()) {
				DBG_G << "is in game -- showing quit message\n";
				const int res = gui::dialog(disp,_("Quit"),_("Do you really want to quit?"),gui::YES_NO).show();
				if(res == 0) {
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

void command_executor::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (can_execute_command(hotkey::get_hotkey(items.front()).get_id(), 0)){
		//if just one item is passed in, that means we should execute that item
		if(!context_menu && items.size() == 1 && items_arg.size() == 1) {
			hotkey::execute_command(gui,hotkey::get_hotkey(items.front()).get_id(),this);
			return;
		}

		std::vector<std::string> menu = get_menu_images(gui, items);

		int res = 0;
		{
			gui::dialog mmenu = gui::dialog(gui,"","",
			gui::MESSAGE, gui::dialog::hotkeys_style);
			mmenu.set_menu(menu);
			res = mmenu.show(xloc, yloc);
		} // this will kill the dialog
		if (res < 0 || size_t(res) >= items.size())
			return;

		const hotkey::HOTKEY_COMMAND cmd = hotkey::get_hotkey(items[res]).get_id();
		hotkey::execute_command(gui,cmd,this,res);
	}
}

std::string command_executor::get_menu_image(hotkey::HOTKEY_COMMAND command, int index) const {
	switch(get_action_state(command, index)) {
		case ACTION_ON: return game_config::checked_menu_image;
		case ACTION_OFF: return game_config::unchecked_menu_image;
		default: return get_action_image(command, index);
	}
}

std::vector<std::string> command_executor::get_menu_images(display &disp, const std::vector<std::string>& items){
	std::vector<std::string> result;
	bool has_image = false;

	for(size_t i = 0; i < items.size(); ++i) {
		std::string const& item = items[i];
		const hotkey::hotkey_item hk = hotkey::get_hotkey(item);

		std::stringstream str;
		//see if this menu item has an associated image
		std::string img(get_menu_image(hk.get_id(), i));
		if(img.empty() == false) {
			has_image = true;
			str << IMAGE_PREFIX << img << COLUMN_SEPARATOR;
		}

		if (hk.get_id() == hotkey::HOTKEY_NULL) {
			str << item.substr(0, item.find_last_not_of(' ') + 1) << COLUMN_SEPARATOR;
		} else {
			std::string desc = hk.get_description();
			if (hk.get_id() == HOTKEY_ENDTURN) {
				const theme::menu *b = disp.get_theme().get_menu_item("button-endturn");
				assert(b);
				desc = b->title();
			}
			str << desc << COLUMN_SEPARATOR << hk.get_name();
		}

		result.push_back(str.str());
	}
	//If any of the menu items have an image, create an image column
	if(has_image)
		for(std::vector<std::string>::iterator i = result.begin(); i != result.end(); ++i)
			if(*(i->begin()) != IMAGE_PREFIX)
				i->insert(i->begin(), COLUMN_SEPARATOR);
	return result;
}

}
