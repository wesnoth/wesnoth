/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
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
#include "SDL.h"

#include <string>
#include <vector>

class config;
class display;

//the hotkey system allows hotkey definitions to be loaded from
//configuration objects, and then detect if a keyboard event
//refers to a hotkey command being executed.
namespace hotkey {

enum HOTKEY_COMMAND {
	HOTKEY_CYCLE_UNITS,HOTKEY_CYCLE_BACK_UNITS, HOTKEY_UNIT_HOLD_POSITION,
	HOTKEY_END_UNIT_TURN, HOTKEY_LEADER,
	HOTKEY_UNDO, HOTKEY_REDO,
	HOTKEY_ZOOM_IN, HOTKEY_ZOOM_OUT, HOTKEY_ZOOM_DEFAULT,
	HOTKEY_FULLSCREEN, HOTKEY_SCREENSHOT, HOTKEY_ACCELERATED,
	HOTKEY_UNIT_DESCRIPTION, HOTKEY_RENAME_UNIT,
	HOTKEY_SAVE_GAME, HOTKEY_SAVE_REPLAY, HOTKEY_SAVE_MAP, HOTKEY_LOAD_GAME,
	HOTKEY_RECRUIT, HOTKEY_REPEAT_RECRUIT, HOTKEY_RECALL, HOTKEY_ENDTURN,
	HOTKEY_TOGGLE_GRID, HOTKEY_STATUS_TABLE, HOTKEY_MUTE, HOTKEY_MOUSE_SCROLL,
	HOTKEY_SPEAK, HOTKEY_CREATE_UNIT, HOTKEY_CHANGE_UNIT_SIDE, HOTKEY_PREFERENCES,
	HOTKEY_OBJECTIVES, HOTKEY_UNIT_LIST, HOTKEY_STATISTICS, HOTKEY_QUIT_GAME,
	HOTKEY_LABEL_TEAM_TERRAIN, HOTKEY_LABEL_TERRAIN, HOTKEY_CLEAR_LABELS,HOTKEY_SHOW_ENEMY_MOVES, HOTKEY_BEST_ENEMY_MOVES,
	HOTKEY_DELAY_SHROUD, HOTKEY_UPDATE_SHROUD, HOTKEY_CONTINUE_MOVE,
	HOTKEY_SEARCH, HOTKEY_SPEAK_ALLY, HOTKEY_SPEAK_ALL, HOTKEY_HELP,
	HOTKEY_CHAT_LOG, HOTKEY_LANGUAGE,
	HOTKEY_PLAY_REPLAY, HOTKEY_RESET_REPLAY, HOTKEY_STOP_REPLAY, HOTKEY_REPLAY_NEXT_TURN,
	HOTKEY_REPLAY_NEXT_SIDE, HOTKEY_REPLAY_SHOW_EVERYTHING,
	HOTKEY_REPLAY_SHOW_EACH, HOTKEY_REPLAY_SHOW_TEAM1,
	HOTKEY_REPLAY_SKIP_ANIMATION,

	//editing specific commands
	HOTKEY_EDIT_SET_TERRAIN,
	HOTKEY_EDIT_QUIT, HOTKEY_EDIT_SAVE_MAP,
	HOTKEY_EDIT_SAVE_AS, HOTKEY_EDIT_SET_START_POS,
	HOTKEY_EDIT_NEW_MAP, HOTKEY_EDIT_LOAD_MAP, HOTKEY_EDIT_FLOOD_FILL,
	HOTKEY_EDIT_FILL_SELECTION, HOTKEY_EDIT_ROTATE_SELECTION, HOTKEY_EDIT_CUT, HOTKEY_EDIT_COPY,
	HOTKEY_EDIT_PASTE, HOTKEY_EDIT_REVERT, HOTKEY_EDIT_RESIZE,
	HOTKEY_EDIT_FLIP, HOTKEY_EDIT_SELECT_ALL, HOTKEY_EDIT_DRAW,
	HOTKEY_EDIT_REFRESH, HOTKEY_EDIT_UPDATE, HOTKEY_EDIT_AUTO_UPDATE,

	//misc.
	HOTKEY_USER_CMD,
	HOTKEY_CLEAR_MSG,
#ifdef USRCMD2
	HOTKEY_USER_CMD_2,
	HOTKEY_USER_CMD_3,
#endif
	HOTKEY_NULL
};

class hotkey_item {
public:
	hotkey_item() : 
		id_(HOTKEY_NULL),
		command_(),
		description_(),
		type_(UNBOUND),
		character_(0),
		ctrl_(false),
		alt_(false),
		cmd_(false),
		keycode_(0),
		shift_(false),
		hidden_(false)
		{}

	hotkey_item(HOTKEY_COMMAND id, const std::string& command, const std::string& description, bool hidden=false);

	HOTKEY_COMMAND get_id() const { return id_; };
	const std::string& get_command() const { return command_; };
	const std::string& get_description() const { return description_; };

	void load_from_config(const config& cfg);

	void set_description(const std::string& description);
	void set_key(int character, int keycode, bool shift, bool ctrl, bool alt, bool cmd);

	enum type {
		UNBOUND,
		BY_KEYCODE,
		BY_CHARACTER
	};

	enum type get_type() const { return type_; }

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
	std::string description_;

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
	~manager();
};

void load_descriptions();

void load_hotkeys(const config& cfg);
void save_hotkeys(config& cfg);

hotkey_item& get_hotkey(HOTKEY_COMMAND id);
hotkey_item& get_hotkey(const std::string& command);

hotkey_item& get_hotkey(int character, int keycode, bool shift, bool ctrl, bool alt, bool cmd);
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
	virtual void toggle_grid() {}
	virtual void status_table() {}
	virtual void recall() {}
	virtual void recruit() {}
	virtual void repeat_recruit() {}
	virtual void speak() {}
	virtual void whisper() {}
	virtual void shout() {}
	virtual void create_unit() {}
	virtual void change_unit_side() {}
	virtual void preferences() {}
	virtual void objectives() {}
	virtual void unit_list() {}
	virtual void show_statistics() {}
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

	// Map editor stuff.
	virtual void edit_set_terrain() {}
	virtual void edit_quit() {}
	virtual void edit_new_map() {}
	virtual void edit_load_map() {}
	virtual void edit_save_map() {}
	virtual void edit_save_as() {}
	virtual void edit_set_start_pos() {}
	virtual void edit_flood_fill() {}
	virtual void edit_fill_selection() {}
	virtual void edit_rotate_selection() {}
	virtual void edit_cut() {}
	virtual void edit_copy() {}
	virtual void edit_paste() {}
	virtual void edit_revert() {}
	virtual void edit_resize() {}
	virtual void edit_flip() {}
	virtual void edit_select_all() {}
	virtual void edit_draw() {}
	virtual void edit_refresh() {}
	virtual void edit_update() {}
	virtual void edit_auto_update() {}

	//Gets the action's image (if any). Displayed left of the action text in menus.
	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND /*command*/, int /*index*/) const { return ""; }
	//Does the action control a toggle switch? If so, return the state of the action (on or off)
	virtual ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND /*command*/) const { return ACTION_STATELESS; }
	//Returns the appropriate menu image. Checkable items will get a checked/unchecked image.
	std::string get_menu_image(hotkey::HOTKEY_COMMAND command, int index=-1) const;
	//Returns a vector of images for a given menu
	std::vector<std::string> get_menu_images(const std::vector<std::string>& items_arg);

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
