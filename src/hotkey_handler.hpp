/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace events { class menu_handler; }
namespace events { class mouse_handler; }
namespace game_events { class wml_menu_item; }

class game_display;
class game_state;
class saved_game;
class wmi_pager;

class team;

class play_controller::hotkey_handler : public hotkey::command_executor {

protected:
	/** References to parent object / constituents */
	play_controller & play_controller_;

	events::menu_handler & menu_handler_;
	events::mouse_handler & mouse_handler_;
	game_display * gui() const;
	saved_game & saved_game_;
	game_state & gamestate();
	const game_state & gamestate() const;

private:
	/** Private data related to menu implementation (expansion of AUTOSAVES, WML entries) */

	/// A smart pointer used when retrieving menu items.
	typedef boost::shared_ptr<const game_events::wml_menu_item> const_item_ptr;

	// Expand AUTOSAVES in the menu items, setting the real savenames.
	void expand_autosaves(std::vector<std::string>& items);

	std::vector<std::string> savenames_;

	void expand_wml_commands(std::vector<std::string>& items);
	std::vector<const_item_ptr> wml_commands_;
	boost::scoped_ptr<wmi_pager> wml_command_pager_;
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
	virtual void objectives();
	virtual void show_statistics();
	virtual void unit_list();
	virtual void left_mouse_click();
	virtual void move_action();
	virtual void select_and_action();
	virtual void select_hex();
	virtual void deselect_hex();
	virtual void right_mouse_click();
	virtual void status_table();
	virtual void save_game();
	virtual void save_replay();
	virtual void save_map();
	virtual void load_game();
	virtual void preferences();
	virtual void show_chat_log();
	virtual void show_help();
	virtual void cycle_units();
	virtual void cycle_back_units();
	virtual void undo();
	virtual void redo();
	virtual void show_enemy_moves(bool ignore_units);
	virtual void goto_leader();
	virtual void unit_description();
	virtual void terrain_description();
	virtual void toggle_ellipses();
	virtual void toggle_grid();
	virtual void search();
	virtual void toggle_accelerated_speed();

	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND, int index) const;
	virtual void load_autosave(const std::string& filename);
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const;
	/** Check if a command can be executed. */
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
	virtual bool execute_command(const hotkey::hotkey_command& command, int index=-1);
	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp);

	/**
	 *  Determines whether the command should be in the context menu or not.
	 *  Independent of whether or not we can actually execute the command.
	 */
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

};

#endif
