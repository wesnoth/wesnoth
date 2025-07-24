/*
	Copyright (C) 2003 - 2025
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

#include "hotkey_command.hpp"

#include <SDL2/SDL_events.h>

class display;

namespace hotkey
{
enum class action_state { stateless, on, off, selected, deselected };

/** Returns action_state::on if @a condition is true, else action_state::off. */
inline action_state on_if(bool condition)
{
	return condition ? action_state::on : action_state::off;
}

/** Returns action_state::selected if @a condition is true, else action_state::deselected. */
inline action_state selected_if(bool condition)
{
	return condition ? action_state::selected : action_state::deselected;
}

/// Used as the main parameter for can_execute_command/do_execute_command
/// These functions are used to execute hotkeys but also to execute menu items,
/// (Most menu items point to the same action as a hotkey but not all)
struct ui_command
{
	/// The hotkey::HOTKEY_COMMAND associated with this action, HOTKEY_NULL for actions that don't allow hotkey binding.
	/// different actions of the ame type might have the same HOTKEY_COMMAND (like different wml menu items that allow
	/// hotkey bindings.). This is preferred to be used for comparision over id (for example being an enum makes it
	/// impossible to make typos in the id and its faster, plus c++ unfortunately doesn't allow switch statements with
	/// strings)
	hotkey::HOTKEY_COMMAND hotkey_command;
	/// The string command, never empty, describes the action uniquely. when the action is the result of a menu click
	/// this matches the id element of the clicked item (the id parameter of show_menu)
	std::string id;
	/// When this action was the result of a menu click, this is the index of the clicked item in the menu.
	int index;
	ui_command(hotkey::HOTKEY_COMMAND hotkey_command, std::string_view id, int index = -1)
		: hotkey_command(hotkey_command)
		, id(id)
		, index(index)
	{
	}
	explicit ui_command(const hotkey::hotkey_command& cmd, int index = -1)
		: ui_command(cmd.command, cmd.id, index)
	{
	}
	explicit ui_command(std::string_view id, int index = -1)
		: ui_command(hotkey::HOTKEY_NULL, id, index)
	{
		// Only set the command from the associated hotkey_command, not the ID.
		// There are cases (specifically, autoload items) with which no command
		// is associated, yet whose ID should be stored unmodified.
		hotkey_command = hotkey::get_hotkey_command(id).command;
	}
};

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
	virtual void terrain_description() {}
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
	virtual void kill_unit() {}
	virtual void preferences() {}
	virtual void objectives() {}
	virtual void unit_list() {}
	virtual void show_statistics() {}
	virtual void stop_network() {}
	virtual void start_network() {}
	virtual void label_terrain(bool /*team_only*/) {}
	virtual void clear_labels() {}
	virtual void label_settings() {}
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
	virtual void play_replay() {  }
	virtual void reset_replay() {}
	virtual void stop_replay() {}
	virtual void replay_next_turn() {  }
	virtual void replay_next_side() {  }
	virtual void replay_next_move() {  }
	virtual void replay_show_everything() {}
	virtual void replay_show_each() {}
	virtual void replay_show_team1() {}
	virtual void replay_skip_animation() {}
	virtual void replay_exit() {}
	virtual void whiteboard_toggle() {}
	virtual void whiteboard_execute_action() {}
	virtual void whiteboard_execute_all_actions() {}
	virtual void whiteboard_delete_action() {}
	virtual void whiteboard_bump_up_action() {}
	virtual void whiteboard_bump_down_action() {}
	virtual void whiteboard_suppose_dead() {}
	virtual void select_hex() {}
	virtual void deselect_hex() {}
	virtual void move_action() {}
	virtual void select_and_action() {}
	virtual void select_teleport() {}
	virtual void touch_hex() {}
	virtual void toggle_accelerated_speed() {}
	virtual void scroll_up(bool /*on*/) {}
	virtual void scroll_down(bool /*on*/) {}
	virtual void scroll_left(bool /*on*/) {}
	virtual void scroll_right(bool /*on*/) {}
	virtual void lua_console();
	virtual void zoom_in() {}
	virtual void zoom_out() {}
	virtual void zoom_default() {}
	virtual void map_screenshot() {}
	virtual void surrender_quit_game() {}

	virtual void set_button_state() {}
	virtual void recalculate_minimap() {}

	void surrender_game();
	void execute_quit_command();

	// @a items_arg the items in the menus to be shows, each item can have the following attributes:
	//   'id':    The id describing the action, will be passed to do_execute_command and can_execute_command,
	//            If 'id' specifies a known hotkey command or theme item the other attributes can be generated from it.
	//   'label': The label for this menu entry.
	//   'icon':  The icon for this menu entry.
	virtual void show_menu(const std::vector<config>& items_arg, const point& menu_loc, bool context_menu);

	// @a items_arg the actions to be executed, executes all of the actions, it looks like the idea is to associate
	//  multiple actions with a single menu button, not sure whether it is actually used.
	void execute_action(const std::vector<std::string>& items_arg);

protected:
	virtual bool can_execute_command(const hotkey::ui_command& command) const = 0;
	virtual bool do_execute_command(const hotkey::ui_command& command, bool press = true, bool release = false);

	// Does the action control a toggle switch? If so, return the state of the action (on or off).
	virtual action_state get_action_state(const hotkey::ui_command&) const
	{
		return action_state::stateless;
	}

	/**
	 * Determines whether the command should be in the context menu or not.
	 * Independent of whether or not we can actually execute the command.
	 */
	virtual bool in_context_menu(const hotkey::ui_command&) const
	{
		return true;
	}

private:
	void populate_menu_controls(config& item, int index) const;
	void populate_menu_item_info(config& item, int index) const;

	/** If true, the menu will remain open after an item has been selected. */
	virtual bool keep_menu_open() const
	{
		return false;
	}

	struct queued_command
	{
		queued_command(const hotkey_command& command_, int index_, bool press_, bool release_)
			: command(&command_), index(index_), press(press_), release(release_)
		{}

		const hotkey_command* command;
		int index;
		bool press;
		bool release;
	};

	void execute_command_wrap(const queued_command& command);
	std::vector<queued_command> filter_command_queue();

	bool press_event_sent_ = false;
	std::vector<queued_command> command_queue_;

public:
	void queue_command(const SDL_Event& event, int index = -1);
	bool run_queued_commands();

	void handle_keyup()
	{
		press_event_sent_ = false;
	}
};

class command_executor_default : public command_executor
{
protected:
	virtual display& get_display() = 0;
public:
	void set_button_state();
	void recalculate_minimap();
	void lua_console();
	void zoom_in();
	void zoom_out();
	void zoom_default();
	void map_screenshot();
};

/* Functions to be called every time a event is intercepted.
 * Will call the relevant function in executor if the event is not nullptr.
 * Also handles some events in the function itself,
 * and so is still meaningful to call with executor=nullptr
 */
void jbutton_event(const SDL_Event& event, command_executor* executor);
void jhat_event(const SDL_Event& event, command_executor* executor);
void key_event(const SDL_Event& event, command_executor* executor);
void keyup_event(const SDL_Event& event, command_executor* executor);
void mbutton_event(const SDL_Event& event, command_executor* executor);
// Function to call to process the events.
void run_events(command_executor* executor);

}
