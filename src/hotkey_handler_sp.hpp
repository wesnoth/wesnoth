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
 * An extension of play_controller::hotkey_handler, which has support for
 * SP wesnoth features like whiteboard, end turn, etc.
 */

#ifndef HOTKEY_HANDLER_SP_HPP_INCL_
#define HOTKEY_HANDLER_SP_HPP_INCL_

#include "playsingle_controller.hpp"

#include "hotkey_handler.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class playsingle_controller::hotkey_handler : public play_controller::hotkey_handler {

protected:
	playsingle_controller & playsingle_controller_;

	boost::shared_ptr<wb::manager> whiteboard_manager_;

	bool is_observer() const;

public:
	hotkey_handler(playsingle_controller &, saved_game &);
	~hotkey_handler();

	virtual void recruit();
	virtual void repeat_recruit();
	virtual void recall();
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
	virtual void toggle_shroud_updates();
	virtual void update_shroud_now();
	virtual void end_turn();
	virtual void rename_unit();
	virtual void create_unit();
	virtual void change_side();
	virtual void kill_unit();
	virtual void label_terrain(bool);
	virtual void clear_labels();
	virtual void label_settings();
	virtual void continue_move();
	virtual void unit_hold_position();
	virtual void end_unit_turn();
	virtual void user_command();
	virtual void custom_command();
	virtual void ai_formula();
	virtual void clear_messages();
	// Whiteboard hotkeys
	virtual void whiteboard_toggle();
	virtual void whiteboard_execute_action();
	virtual void whiteboard_execute_all_actions();
	virtual void whiteboard_delete_action();
	virtual void whiteboard_bump_up_action();
	virtual void whiteboard_bump_down_action();
	virtual void whiteboard_suppose_dead();

	//replay
	replay_controller& get_replay_controller()
	{
		assert(playsingle_controller_.get_replay_controller());
		return *playsingle_controller_.get_replay_controller();
	}
	virtual void stop_replay() OVERRIDE
	{ return get_replay_controller().stop_replay(); }
	virtual void play_replay() OVERRIDE
	{ return get_replay_controller().play_replay(); }
	virtual void replay_next_turn() OVERRIDE
	{ return get_replay_controller().replay_next_turn(); }
	virtual void replay_next_side() OVERRIDE
	{ return get_replay_controller().replay_next_side(); }
	virtual void replay_next_move() OVERRIDE
	{ return get_replay_controller().replay_next_move(); }
	virtual void replay_show_everything() OVERRIDE
	{ return get_replay_controller().replay_show_everything(); }
	virtual void replay_show_each() OVERRIDE
	{ return get_replay_controller().replay_show_each(); }
	virtual void replay_show_team1() OVERRIDE
	{ return get_replay_controller().replay_show_team1(); }
	virtual void reset_replay() OVERRIDE
	{ return playsingle_controller_.reset_replay(); }
	virtual void replay_exit() OVERRIDE;
	virtual void load_autosave(const std::string& filename);
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const;
};

#endif
