/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef HOTKEYS_HPP_INCLUDED
#define HOTKEYS_HPP_INCLUDED

#include "events.hpp"
#include "tstring.hpp"

#include <string>
#include <vector>

class config;
class display;

//the hotkey system allows hotkey definitions to be loaded from
//configuration objects, and then detect if a keyboard event
//refers to a hotkey command being executed.
namespace hotkey {

/** Available hotkey scopes. The scope is used to allow command from
 * non-overlapping areas of the game share the same key
 */
enum scope {
	SCOPE_GENERAL,
	SCOPE_GAME,
	SCOPE_EDITOR,
	SCOPE_COUNT
};


enum HOTKEY_COMMAND {
	HOTKEY_CYCLE_UNITS,HOTKEY_CYCLE_BACK_UNITS,
	HOTKEY_ADD_WAYPOINT, HOTKEY_UNIT_HOLD_POSITION,
	HOTKEY_END_UNIT_TURN, HOTKEY_LEADER,
	HOTKEY_UNDO, HOTKEY_REDO,
	HOTKEY_ZOOM_IN, HOTKEY_ZOOM_OUT, HOTKEY_ZOOM_DEFAULT,
	HOTKEY_FULLSCREEN, HOTKEY_SCREENSHOT, HOTKEY_MAP_SCREENSHOT, HOTKEY_ACCELERATED,
	HOTKEY_UNIT_DESCRIPTION, HOTKEY_RENAME_UNIT,
	HOTKEY_SAVE_GAME, HOTKEY_SAVE_REPLAY, HOTKEY_SAVE_MAP, HOTKEY_LOAD_GAME,
	HOTKEY_RECRUIT, HOTKEY_REPEAT_RECRUIT, HOTKEY_RECALL, HOTKEY_ENDTURN,
	HOTKEY_TOGGLE_ELLIPSES, HOTKEY_TOGGLE_GRID, HOTKEY_STATUS_TABLE, HOTKEY_MUTE, HOTKEY_MOUSE_SCROLL,
	HOTKEY_SPEAK, HOTKEY_CREATE_UNIT, HOTKEY_CHANGE_SIDE, HOTKEY_PREFERENCES,
	HOTKEY_OBJECTIVES, HOTKEY_UNIT_LIST, HOTKEY_STATISTICS, HOTKEY_STOP_NETWORK, HOTKEY_START_NETWORK, HOTKEY_QUIT_GAME,
	HOTKEY_LABEL_TEAM_TERRAIN, HOTKEY_LABEL_TERRAIN, HOTKEY_CLEAR_LABELS,HOTKEY_SHOW_ENEMY_MOVES, HOTKEY_BEST_ENEMY_MOVES,
	HOTKEY_DELAY_SHROUD, HOTKEY_UPDATE_SHROUD, HOTKEY_CONTINUE_MOVE,
	HOTKEY_SEARCH, HOTKEY_SPEAK_ALLY, HOTKEY_SPEAK_ALL, HOTKEY_HELP,
	HOTKEY_CHAT_LOG, HOTKEY_LANGUAGE,
	HOTKEY_PLAY_REPLAY, HOTKEY_RESET_REPLAY, HOTKEY_STOP_REPLAY, HOTKEY_REPLAY_NEXT_TURN,
	HOTKEY_REPLAY_NEXT_SIDE, HOTKEY_REPLAY_SHOW_EVERYTHING,
	HOTKEY_REPLAY_SHOW_EACH, HOTKEY_REPLAY_SHOW_TEAM1,
	HOTKEY_REPLAY_SKIP_ANIMATION,

#ifndef DISABLE_EDITOR
	HOTKEY_EDITOR_QUIT_TO_DESKTOP,
	HOTKEY_EDITOR_CLOSE_MAP, HOTKEY_EDITOR_SWITCH_MAP,
	HOTKEY_EDITOR_SETTINGS,
	HOTKEY_EDITOR_PARTIAL_UNDO,
	HOTKEY_EDITOR_MAP_NEW, HOTKEY_EDITOR_MAP_LOAD, HOTKEY_EDITOR_MAP_SAVE,
	HOTKEY_EDITOR_MAP_SAVE_AS, HOTKEY_EDITOR_MAP_REVERT, HOTKEY_EDITOR_MAP_INFO,
	HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP,
	HOTKEY_EDITOR_TOOL_NEXT, HOTKEY_EDITOR_TOOL_PAINT, HOTKEY_EDITOR_TOOL_FILL,
	HOTKEY_EDITOR_TOOL_SELECT, HOTKEY_EDITOR_TOOL_STARTING_POSITION,
	HOTKEY_EDITOR_BRUSH_NEXT, HOTKEY_EDITOR_BRUSH_DEFAULT,
	HOTKEY_EDITOR_CUT, HOTKEY_EDITOR_COPY, HOTKEY_EDITOR_PASTE,
	HOTKEY_EDITOR_EXPORT_SELECTION_COORDS,
	HOTKEY_EDITOR_SELECT_ALL, HOTKEY_EDITOR_SELECT_INVERSE,
	HOTKEY_EDITOR_SELECT_NONE,
	HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW, HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW,
	HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL, HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL,
	HOTKEY_EDITOR_SELECTION_ROTATE, HOTKEY_EDITOR_SELECTION_FLIP,
	HOTKEY_EDITOR_SELECTION_FILL,
	HOTKEY_EDITOR_SELECTION_GENERATE, HOTKEY_EDITOR_SELECTION_RANDOMIZE,
	HOTKEY_EDITOR_MAP_RESIZE, HOTKEY_EDITOR_MAP_ROTATE,
	HOTKEY_EDITOR_MAP_GENERATE, HOTKEY_EDITOR_MAP_APPLY_MASK,
	HOTKEY_EDITOR_MAP_CREATE_MASK_TO,
	HOTKEY_EDITOR_REFRESH, HOTKEY_EDITOR_UPDATE_TRANSITIONS,
	HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS,
	HOTKEY_EDITOR_REFRESH_IMAGE_CACHE,
	HOTKEY_EDITOR_DRAW_COORDINATES, HOTKEY_EDITOR_DRAW_TERRAIN_CODES,
#endif

	//misc.
	HOTKEY_USER_CMD,
	HOTKEY_CUSTOM_CMD,
	HOTKEY_AI_FORMULA,
	HOTKEY_CLEAR_MSG,
#ifdef USRCMD2
	HOTKEY_USER_CMD_2,
	HOTKEY_USER_CMD_3,
#endif
	HOTKEY_NULL
};

void deactivate_all_scopes();
void set_scope_active(scope s, bool set = true);
bool is_scope_active(scope s);

class hotkey_item {
public:
	hotkey_item() :
		id_(HOTKEY_NULL),
		command_(),
		description_(),
		scope_(SCOPE_GENERAL),
		type_(UNBOUND),
		character_(0),
		ctrl_(false),
		alt_(false),
		cmd_(false),
		keycode_(0),
		shift_(false),
		hidden_(false)
		{}

	hotkey_item(HOTKEY_COMMAND id, const std::string& command,
		const t_string &description, bool hidden = false,
		scope s=SCOPE_GENERAL);

	HOTKEY_COMMAND get_id() const { return id_; };
	const std::string& get_command() const { return command_; };
	const t_string &get_description() const { return description_; };

	void load_from_config(const config& cfg);

	void set_description(const t_string &description);
	void clear_hotkey();
	void set_key(int character, int keycode, bool shift, bool ctrl, bool alt, bool cmd);

	enum type {
		UNBOUND,
		BY_KEYCODE,
		BY_CHARACTER,
		CLEARED
	};

	enum type get_type() const { return type_; }


	/** @return the scope of this hotkey */
	scope get_scope() const { return scope_; }

	bool is_in_active_scope() const { return is_scope_active(get_scope()); }

	// Returns unicode value of keypress.
	int get_character() const { return character_; }
	bool get_alt() const { return alt_; }
	bool get_cmd() const { return cmd_; }
	bool get_ctrl() const { return ctrl_; }

	// Return the actual key code.
	int get_keycode() const { return keycode_; }
	bool get_shift() const { return shift_; }

	// Return "name" of hotkey for example :"ctrl+alt+g"
	std::string get_name() const;

	bool null() const { return id_ == HOTKEY_NULL; };
	bool hidden() const { return hidden_; };
private:
	HOTKEY_COMMAND id_;
	std::string command_;
	t_string description_;
	scope scope_;

	// UNBOUND means unset, CHARACTER means see character_, KEY means keycode_.
	enum type type_;

	// Actual unicode character
	int character_;
	bool ctrl_;
	bool alt_;
	bool cmd_;

	// These used for function keys (which don't have a unicode value) or
	// space (which doesn't have a distinct unicode value when shifted).
	int keycode_;
	bool shift_;

	bool hidden_;

};



class manager {
public:
	manager();
	static void init();
	static void wipe();
	~manager();
};

class scope_changer {
public:
	scope_changer(const config& cfg, const std::string& hotkey_tag);
	~scope_changer();
private:
	const config& cfg_;
	std::string prev_tag_name_;
	std::vector<bool> prev_scope_active_;
};

void load_descriptions();

void set_hotkey_tag_name(const std::string& name);
void load_hotkeys(const config& cfg);
void save_hotkeys(config& cfg);

hotkey_item& get_hotkey(HOTKEY_COMMAND id);
hotkey_item& get_hotkey(const std::string& command);

hotkey_item& get_hotkey(int character, int keycode, bool shift, bool ctrl,
	bool alt, bool cmd);
hotkey_item& get_hotkey(const SDL_KeyboardEvent& event);

hotkey_item& get_visible_hotkey(int index);

std::vector<hotkey_item>& get_hotkeys();

enum ACTION_STATE { ACTION_STATELESS, ACTION_ON, ACTION_OFF };

//abstract base class for objects that implement the ability
//to execute hotkey commands.
class command_executor
{
protected:
	virtual ~command_executor() {}
public:
	virtual void cycle_units() {}
	virtual void cycle_back_units() {}
	virtual void end_turn() {}
	virtual void goto_leader() {}
	virtual void add_waypoint() {}
	virtual void unit_hold_position() {}
	virtual void end_unit_turn() {}
	virtual void undo() {}
	virtual void redo() {}
	virtual void unit_description() {}
	virtual void rename_unit() {}
	virtual void save_game() {}
	virtual void save_replay() {}
	virtual void save_map() {}
	virtual void load_game() {}
	virtual void toggle_ellipses() {}
	virtual void toggle_grid() {}
	virtual void status_table() {}
	virtual void recall() {}
	virtual void recruit() {}
	virtual void repeat_recruit() {}
	virtual void speak() {}
	virtual void whisper() {}
	virtual void shout() {}
	virtual void create_unit() {}
	virtual void change_side() {}
	virtual void preferences() {}
	virtual void objectives() {}
	virtual void unit_list() {}
	virtual void show_statistics() {}
	virtual void stop_network() {}
	virtual void start_network() {}
	virtual void label_terrain(bool /*team_only*/) {}
	virtual void clear_labels() {}
	virtual void show_enemy_moves(bool /*ignore_units*/) {}
	virtual void toggle_shroud_updates() {}
	virtual void update_shroud_now() {}
	virtual void continue_move() {}
	virtual void search() {}
	virtual void show_help() {}
	virtual void show_chat_log() {}
	virtual void user_command() {}
	virtual void custom_command() {}
	virtual void ai_formula() {}
	virtual void clear_messages() {}
#ifdef USRCMD2
	virtual void user_command_2() {}
	virtual void user_command_3() {}
#endif
	virtual void change_language() {}
	virtual void play_replay() {}
	virtual void reset_replay() {}
	virtual void stop_replay() {}
	virtual void replay_next_turn() {}
	virtual void replay_next_side() {}
	virtual void replay_show_everything() {}
	virtual void replay_show_each() {}
	virtual void replay_show_team1() {}
	virtual void replay_skip_animation() {}

	//Gets the action's image (if any). Displayed left of the action text in menus.
	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND /*command*/, int /*index*/) const { return ""; }
	//Does the action control a toggle switch? If so, return the state of the action (on or off)
	virtual ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND /*command*/, int /*index*/) const { return ACTION_STATELESS; }
	//Returns the appropriate menu image. Checkable items will get a checked/unchecked image.
	std::string get_menu_image(hotkey::HOTKEY_COMMAND command, int index=-1) const;
	//Returns a vector of images for a given menu
	std::vector<std::string> get_menu_images(display &, const std::vector<std::string>& items_arg);

	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& gui);

	virtual bool can_execute_command(HOTKEY_COMMAND command, int index=-1) const = 0;
	virtual bool execute_command(HOTKEY_COMMAND command, int index=-1);
};

//function to be called every time a key event is intercepted. Will
//call the relevant function in executor if the keyboard event is
//not NULL. Also handles some events in the function itself, and so
//is still meaningful to call with executor=NULL
void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor);

void execute_command(display& disp, HOTKEY_COMMAND command, command_executor* executor, int index=-1);

//object which will ensure that basic keyboard events like escape
//are handled properly for the duration of its lifetime
struct basic_handler : public events::handler {
	basic_handler(display* disp, command_executor* exec=NULL);

	void handle_event(const SDL_Event& event);

private:
	display* disp_;
	command_executor* exec_;
};

}

#endif
