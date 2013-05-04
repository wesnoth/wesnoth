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
#ifndef HOTKEYS_HPP_INCLUDED
#define HOTKEYS_HPP_INCLUDED

#include "events.hpp"
#include "tstring.hpp"

class config;
class display;

/* The hotkey system allows hotkey definitions to be loaded from
 * configuration objects, and then detect if a keyboard event
 * refers to a hotkey command being executed.
 */
namespace hotkey {

/**
 * Available hotkey scopes. The scope is used to allow command from
 * non-overlapping areas of the game share the same key
 */
enum scope {
	SCOPE_GENERAL,
	SCOPE_GAME,
	SCOPE_EDITOR,
	SCOPE_COUNT
};
void deactivate_all_scopes();
void set_scope_active(scope s, bool set = true);
bool is_scope_active(scope s);

enum INPUT_CONTROLL {
	INPUT_SCROLL_HORIZONTAL,
	INPUT_SCROLL_VERTICAL,
	INPUT_NULL
};

enum HOTKEY_COMMAND {
	HOTKEY_CYCLE_UNITS,HOTKEY_CYCLE_BACK_UNITS,
	HOTKEY_UNIT_HOLD_POSITION,
	HOTKEY_END_UNIT_TURN, HOTKEY_LEADER,
	HOTKEY_UNDO, HOTKEY_REDO,
	HOTKEY_ZOOM_IN, HOTKEY_ZOOM_OUT, HOTKEY_ZOOM_DEFAULT,
	HOTKEY_FULLSCREEN, HOTKEY_SCREENSHOT, HOTKEY_MAP_SCREENSHOT, HOTKEY_ACCELERATED,
	HOTKEY_UNIT_DESCRIPTION, HOTKEY_RENAME_UNIT, HOTKEY_DELETE_UNIT,
	HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT, HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE,
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
	HOTKEY_ANIMATE_MAP,
	HOTKEY_LEFT_MOUSE_CLICK, HOTKEY_RIGHT_MOUSE_CLICK,
	HOTKEY_CANCEL, HOTKEY_OKAY,

	// Whiteboard commands
	HOTKEY_WB_TOGGLE,
	HOTKEY_WB_EXECUTE_ACTION,
	HOTKEY_WB_EXECUTE_ALL_ACTIONS,
	HOTKEY_WB_DELETE_ACTION,
	HOTKEY_WB_BUMP_UP_ACTION,
	HOTKEY_WB_BUMP_DOWN_ACTION,
	HOTKEY_WB_SUPPOSE_DEAD,

	// Editor commands
	HOTKEY_EDITOR_QUIT_TO_DESKTOP,
	HOTKEY_EDITOR_CLOSE_MAP, HOTKEY_EDITOR_SWITCH_MAP,
	HOTKEY_EDITOR_SETTINGS,
	HOTKEY_EDITOR_PARTIAL_UNDO,
	HOTKEY_EDITOR_MAP_NEW, HOTKEY_EDITOR_MAP_LOAD, HOTKEY_EDITOR_MAP_SAVE,
	HOTKEY_EDITOR_MAP_SAVE_AS, HOTKEY_EDITOR_MAP_SAVE_ALL,
	HOTKEY_EDITOR_MAP_REVERT, HOTKEY_EDITOR_MAP_INFO,
	HOTKEY_EDITOR_PALETTE_ITEM_SWAP,
	HOTKEY_EDITOR_PALETTE_GROUPS, HOTKEY_EDITOR_PALETTE_UPSCROLL, HOTKEY_EDITOR_PALETTE_DOWNSCROLL,
	HOTKEY_EDITOR_TOOL_NEXT,
	HOTKEY_EDITOR_TOOL_PAINT, HOTKEY_EDITOR_TOOL_FILL,
	HOTKEY_EDITOR_TOOL_SELECT, HOTKEY_EDITOR_TOOL_STARTING_POSITION, HOTKEY_EDITOR_TOOL_LABEL,
	HOTKEY_EDITOR_TOOL_UNIT, HOTKEY_EDITOR_TOOL_VILLAGE, HOTKEY_EDITOR_TOOL_ITEM, HOTKEY_EDITOR_TOOL_SOUNDSOURCE,
	HOTKEY_EDITOR_BRUSH_NEXT, HOTKEY_EDITOR_BRUSH_DEFAULT,
	HOTKEY_EDITOR_BRUSH_1, HOTKEY_EDITOR_BRUSH_2, HOTKEY_EDITOR_BRUSH_3, HOTKEY_EDITOR_BRUSH_SW_NE, HOTKEY_EDITOR_BRUSH_NW_SE,
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
	HOTKEY_EDITOR_REFRESH,
	HOTKEY_EDITOR_UPDATE_TRANSITIONS, HOTKEY_EDITOR_TOGGLE_TRANSITIONS,
	HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS, HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS, HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS,
	HOTKEY_EDITOR_REFRESH_IMAGE_CACHE,
	HOTKEY_EDITOR_DRAW_COORDINATES, HOTKEY_EDITOR_DRAW_TERRAIN_CODES,
	HOTKEY_EDITOR_SIDE_NEW, HOTKEY_EDITOR_SIDE_SWITCH,

	// misc.
	HOTKEY_USER_CMD,
	HOTKEY_CUSTOM_CMD,
	HOTKEY_AI_FORMULA,
	HOTKEY_CLEAR_MSG,

	/* Gui2 specific hotkeys. */
	TITLE_SCREEN__RELOAD_WML,
	TITLE_SCREEN__NEXT_TIP,
	TITLE_SCREEN__PREVIOUS_TIP,
	TITLE_SCREEN__TUTORIAL,
	TITLE_SCREEN__CAMPAIGN,
	TITLE_SCREEN__MULTIPLAYER,
	TITLE_SCREEN__ADDONS,
	TITLE_SCREEN__EDITOR,
	TITLE_SCREEN__CREDITS,
	GLOBAL__HELPTIP,

	HOTKEY_NULL
};


struct input_controll {

	hotkey::INPUT_CONTROLL controll;

	const char* controll_str;

	const char* descriptipn;

	bool hidden;

	hotkey::scope scope;
};

/// Stores all static information related to hotkey functions.
struct hotkey_command {
	/// This binds the command to a function. Does not need to be unique.
	hotkey::HOTKEY_COMMAND id;
	/// The command is unique.
	const char* command;
	/// note: The description is untranslated.
	const char* description;
	/// If hidden then don't show the command in the hotkey preferences.
	bool hidden;
	/// The visibility scope of the command.
	hotkey::scope scope;
	///
	const char* tooltip;
};

const hotkey_command* get_hotkey_commands();

class hotkey_item {
public:

	explicit hotkey_item(const std::string& command) :
		command_(command),
		shift_(false), 	ctrl_(false), cmd_(false), alt_(false),
		character_(-1), keycode_(-1),
		joystick_(-1), mouse_(-1),
		button_(-1), hat_(-1), value_(-1),
		axis_joystick(-1), joystick_axis(-1),
		axis_mouse(-1)
	{}

	explicit hotkey_item(const config& cfg):
		command_("null"),
		shift_(false), 	ctrl_(false), cmd_(false), alt_(false),
		character_(-1), keycode_(-1),
		joystick_(-1), mouse_(-1),
		button_(-1), hat_(-1), value_(-1),
		axis_joystick(-1), joystick_axis(-1),
    	axis_mouse(-1)
	{
		load_from_config(cfg);
	}

	void load_from_config(const config& cfg);

	void set_command(const std::string& command);

	/** get the string name of the HOTKEY_COMMAND */
	const std::string get_command() const { return command_; };
	/** The translated description */
	const std::string get_description() const;
    /** @return if the item should appear in the hotkey preferences */
	bool hidden() const;
	/** @return the visibility scope of this hotkey */
	scope get_scope() const;

	/// Return "name" of hotkey. Example :"ctrl+alt+g"
	std::string get_name() const;

	void clear();

	bool null() const { return command_  == "null"; };

	void save(config& cfg);

	bool is_in_active_scope() const { return is_scope_active(get_scope()); }

	/// Return the actual key code.
	int get_keycode() const { return keycode_; }
	/// Returns unicode value of the pressed key.
	int get_character() const { return character_; }

	/** for buttons on devices */
	int get_button() const { return button_; }
	int get_joystick() const { return joystick_; }
	int get_hat() const { return hat_; }
	int get_mouse() const { return mouse_; }
	int get_value() const { return value_; }

	/** modifiers */
	bool get_shift() const { return shift_; }
	bool get_ctrl() const { return ctrl_; }
	bool get_cmd() const { return cmd_; }
	bool get_alt() const { return alt_; }

	HOTKEY_COMMAND get_id() const;

	void set_jbutton(int button, int joystick, bool shift, bool ctrl, bool cmd, bool alt);
	void set_jhat(int joystick, int hat, int value, bool shift, bool ctrl, bool cmd, bool alt);
	void set_key(int character, int keycode, bool shift, bool ctrl, bool cmd, bool alt);
	void set_mbutton(int device, int button, bool shift, bool ctrl, bool cmd, bool alt);

protected:

	// The unique command associated with this item.
	// Used to bind to a hotkey_command struct.
	std::string command_;

	// modifier keys
	bool shift_, ctrl_, cmd_, alt_;

	// Actual unicode character
	int character_;

	// These used for function keys (which don't have a unicode value) or
	// space (which doesn't have a distinct unicode value when shifted).
	int keycode_;

	int joystick_, mouse_;
	int button_;
	int hat_, value_;

	int axis_joystick, joystick_axis;
	int axis_mouse;

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
	scope_changer();
	~scope_changer();
private:
	std::vector<bool> prev_scope_active_;
};

void load_hotkeys(const config& cfg, bool set_as_default = false);
void reset_default_hotkeys();
void save_hotkeys(config& cfg);

//TODO they do the same?
HOTKEY_COMMAND get_id(const std::string& command);
HOTKEY_COMMAND get_hotkey_command(const std::string& command);
const std::string get_description(const std::string& command);
const std::string get_tooltip(const std::string& command);

std::string get_names(hotkey::HOTKEY_COMMAND id);
std::string get_names(std::string id);
void add_hotkey(const hotkey_item& item);
void clear_hotkeys(const std::string& command);

hotkey_item& get_hotkey(int mouse, int joystick,
		int button, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt);
hotkey_item& get_hotkey(int character, int keycode,
		bool shift, bool ctrl, bool cmd, bool alt);

hotkey_item& get_hotkey(const SDL_JoyButtonEvent& event);
hotkey_item& get_hotkey(const SDL_JoyHatEvent& event);
hotkey_item& get_hotkey(const SDL_KeyboardEvent& event);


std::vector<hotkey_item>& get_hotkeys();

enum ACTION_STATE { ACTION_STATELESS, ACTION_ON, ACTION_OFF, ACTION_SELECTED, ACTION_DESELECTED };

// Abstract base class for objects that implement the ability
// to execute hotkey commands.
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
	virtual void whiteboard_toggle() {}
	virtual void whiteboard_execute_action() {}
	virtual void whiteboard_execute_all_actions() {}
	virtual void whiteboard_delete_action() {}
	virtual void whiteboard_bump_up_action() {}
	virtual void whiteboard_bump_down_action() {}
	virtual void whiteboard_suppose_dead() {}
	virtual void left_mouse_click() {}
	virtual void right_mouse_click() {}

	// Gets the action's image (if any). Displayed left of the action text in menus.
	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND /*command*/, int /*index*/) const { return ""; }
	// Does the action control a toggle switch? If so, return the state of the action (on or off).
	virtual ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND /*command*/, int /*index*/) const { return ACTION_STATELESS; }
	// Returns the appropriate menu image. Checkable items will get a checked/unchecked image.
	std::string get_menu_image(const std::string& command, int index=-1) const;
	// Returns a vector of images for a given menu.
	std::vector<std::string> get_menu_images(display &, const std::vector<std::string>& items_arg);

	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& gui);
	void execute_action(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& gui);

	/**
	 * Adjusts the state of those theme menu buttons which trigger hotkey events.
	 * \param command The command whose linked buttons are adjusted */
	void set_button_state(display& disp);

	virtual bool can_execute_command(HOTKEY_COMMAND command, int index=-1) const = 0;
	virtual bool execute_command(HOTKEY_COMMAND command, int index=-1);
};

/* Functions to be called every time a event is intercepted.
 * Will call the relevant function in executor if the event is not NULL.
 * Also handles some events in the function itself,
 * and so is still meaningful to call with executor=NULL
 */
void jbutton_event(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor);
void jhat_event(display& disp, const SDL_JoyHatEvent& event, command_executor* executor);
void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor);
void mbutton_event(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor);

//TODO
void execute_command(display& disp, HOTKEY_COMMAND command, command_executor* executor, int index=-1);

// Object which will ensure that basic keyboard events like escape
// are handled properly for the duration of its lifetime.
struct basic_handler : public events::handler {
	basic_handler(display* disp, command_executor* exec=NULL);

	void handle_event(const SDL_Event& event);

private:
	display* disp_;
	command_executor* exec_;
};

}

#endif
