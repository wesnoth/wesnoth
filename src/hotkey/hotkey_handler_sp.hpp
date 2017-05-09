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
 * An extension of play_controller::hotkey_handler, which has support for
 * SP wesnoth features like whiteboard, end turn, etc.
 */

#pragma once

#include "playsingle_controller.hpp"

#include "hotkey/hotkey_handler.hpp"

class playsingle_controller::hotkey_handler : public play_controller::hotkey_handler {

protected:
	playsingle_controller & playsingle_controller_;

	std::shared_ptr<wb::manager> whiteboard_manager_;

	bool is_observer() const;

public:
	hotkey_handler(playsingle_controller &, saved_game &);
	~hotkey_handler();

	virtual void recruit() override;
	virtual void repeat_recruit() override;
	virtual void recall() override;
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const override;
	virtual void toggle_shroud_updates() override;
	virtual void update_shroud_now() override;
	virtual void end_turn() override;
	virtual void rename_unit() override;
	virtual void create_unit() override;
	virtual void change_side() override;
	virtual void kill_unit() override;
	virtual void label_terrain(bool) override;
	virtual void clear_labels() override;
	virtual void label_settings() override;
	virtual void continue_move() override;
	virtual void unit_hold_position() override;
	virtual void end_unit_turn() override;
	virtual void user_command() override;
	virtual void custom_command() override;
	virtual void ai_formula() override;
	virtual void clear_messages() override;
	// Whiteboard hotkeys
	virtual void whiteboard_toggle() override;
	virtual void whiteboard_execute_action() override;
	virtual void whiteboard_execute_all_actions() override;
	virtual void whiteboard_delete_action() override;
	virtual void whiteboard_bump_up_action() override;
	virtual void whiteboard_bump_down_action() override;
	virtual void whiteboard_suppose_dead() override;

	//replay
	replay_controller& get_replay_controller()
	{
		assert(playsingle_controller_.get_replay_controller());
		return *playsingle_controller_.get_replay_controller();
	}
	virtual void stop_replay() override
	{ return get_replay_controller().stop_replay(); }
	virtual void play_replay() override
	{ return get_replay_controller().play_replay(); }
	virtual void replay_next_turn() override
	{ return get_replay_controller().replay_next_turn(); }
	virtual void replay_next_side() override
	{ return get_replay_controller().replay_next_side(); }
	virtual void replay_next_move() override
	{ return get_replay_controller().replay_next_move(); }
	virtual void replay_show_everything() override
	{ return get_replay_controller().replay_show_everything(); }
	virtual void replay_show_each() override
	{ return get_replay_controller().replay_show_each(); }
	virtual void replay_show_team1() override
	{ return get_replay_controller().replay_show_team1(); }
	virtual void reset_replay() override
	{ return playsingle_controller_.reset_replay(); }
	virtual void replay_exit() override;
	virtual void load_autosave(const std::string& filename) override;
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const override;
};
