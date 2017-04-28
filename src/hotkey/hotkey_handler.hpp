/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * This file implements all the hotkey handling and menu details for
 * play controller.
 */

#ifndef HOTKEY_HANDLER_HPP_INCL_
#define HOTKEY_HANDLER_HPP_INCL_

#include "play_controller.hpp"

namespace events { class menu_handler; }
namespace events { class mouse_handler; }
namespace game_events { class wml_menu_item; }

class game_display;
class game_state;
class saved_game;

class team;

class play_controller::hotkey_handler : public hotkey::command_executor_default {

protected:
	display& get_display() override { return play_controller_.get_display(); }

	/** References to parent object / constituents */
	play_controller & play_controller_;

	events::menu_handler & menu_handler_;
	events::mouse_handler & mouse_handler_;
	game_display * gui() const;
	saved_game & saved_game_;
	game_state & gamestate();
	const game_state & gamestate() const;

private:
	//
	// Private data related to menu implementation (expansion of AUTOSAVES, WML entries)
	//

	/// A smart pointer used when retrieving menu items.
	typedef std::shared_ptr<const game_events::wml_menu_item> const_item_ptr;

	// Expand AUTOSAVES in the menu items, setting the real savenames.
	void expand_autosaves(std::vector<config>& items, int i);

	std::vector<std::string> savenames_;

	/**
	 * Replaces "wml" in @a items with all active WML menu items for the current field.
	 */
	void expand_wml_commands(std::vector<config>& items, int i);
	std::vector<const_item_ptr> wml_commands_;
	int last_context_menu_x_;
	int last_context_menu_y_;

protected:
	bool browse() const;
	bool linger() const;

	const team & viewing_team() const;
	bool viewing_team_is_playing() const;

public:
	hotkey_handler(play_controller &, saved_game &);
	~hotkey_handler();

	static const std::string wml_menu_hotkey_prefix;

	//event handlers, overridden from command_executor
	virtual void objectives() override;
	virtual void show_statistics() override;
	virtual void unit_list() override;
	virtual void left_mouse_click() override;
	virtual void move_action() override;
	virtual void select_and_action() override;
	virtual void select_hex() override;
	virtual void deselect_hex() override;
	virtual void right_mouse_click() override;
	virtual void status_table() override;
	virtual void save_game() override;
	virtual void save_replay() override;
	virtual void save_map() override;
	virtual void load_game() override;
	virtual void preferences() override;
	virtual void show_chat_log() override;
	virtual void show_help() override;
	virtual void cycle_units() override;
	virtual void cycle_back_units() override;
	virtual void undo() override;
	virtual void redo() override;
	virtual void show_enemy_moves(bool ignore_units) override;
	virtual void goto_leader() override;
	virtual void unit_description() override;
	virtual void terrain_description() override;
	virtual void toggle_ellipses() override;
	virtual void toggle_grid() override;
	virtual void search() override;
	virtual void toggle_accelerated_speed() override;
	virtual void scroll_up(bool on) override;
	virtual void scroll_down(bool on) override;
	virtual void scroll_left(bool on) override;
	virtual void scroll_right(bool on) override;
	virtual void replay_skip_animation() override
	{ return play_controller_.toggle_skipping_replay(); }

	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND, int index) const override;
	virtual void load_autosave(const std::string& filename);
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const override;
	/** Check if a command can be executed. */
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const override;
	virtual bool execute_command(const hotkey::hotkey_command& command, int index=-1, bool press=true) override;
	void show_menu(const std::vector<config>& items_arg, int xloc, int yloc, bool context_menu, display& disp) override;

	/**
	 *  Determines whether the command should be in the context menu or not.
	 *  Independent of whether or not we can actually execute the command.
	 */
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

};

#endif
