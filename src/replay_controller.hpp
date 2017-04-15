/*
   Copyright (C) 2015 - 2017 by the Battle for Wesnoth Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "play_controller.hpp"
#include "replay.hpp"
#include "mouse_handler_base.hpp" //events::command_disabler

#include <vector>

class video;

class replay_controller : public events::observer
{
public:
	class replay_stop_condition
	{
	public:
		virtual void move_done() {}
		virtual void new_side_turn(int , int ) {}
		virtual bool should_stop() { return true; }
		virtual ~replay_stop_condition(){}
	};
	static void nop() {};
	replay_controller(play_controller& controller, bool control_view, const std::shared_ptr<config>& reset_state, const std::function<void()>& on_end_replay = nop);
	~replay_controller();

	// void reset_replay();
	void play_replay();
	void stop_replay();
	void replay_next_turn();
	void replay_next_side();
	void replay_next_move();
	REPLAY_RETURN play_side_impl();

	bool recorder_at_end() const;
	bool should_stop() const { return stop_condition_->should_stop(); }
	bool can_execute_command(const hotkey::hotkey_command& cmd, int index) const;
	bool is_controlling_view() const { return vision_.is_initialized(); }
	bool allow_reset_replay() const { return reset_state_.get() != nullptr; }
	const std::shared_ptr<config>& get_reset_state() const { return reset_state_; };
	void return_to_play_side(bool r = true) { return_to_play_side_ = r; }
	void replay_show_everything();
	void replay_show_each();
	void replay_show_team1();
	void update_teams();
	void update_viewing_player();
private:
	void add_replay_theme();
	void init();
	void update_gui();
	void rebuild_replay_theme();
	void handle_generic_event(const std::string& name) override;

	void reset_replay_ui();
	void update_replay_ui();

	void replay_ui_playback_should_start();
	void replay_ui_playback_should_stop();

	std::shared_ptr<gui::button> play_button();
	std::shared_ptr<gui::button> stop_button();
	std::shared_ptr<gui::button> reset_button();
	std::shared_ptr<gui::button> play_turn_button();
	std::shared_ptr<gui::button> play_side_button();
	std::shared_ptr<gui::button> play_move_button();

	bool replay_ui_has_all_buttons() {
		return play_button() && stop_button() && reset_button() &&
		       play_turn_button() && play_side_button();
	}
	play_controller& controller_;
	std::unique_ptr<replay_stop_condition> stop_condition_;
	events::command_disabler disabler_;

	enum REPLAY_VISION
	{
		HUMAN_TEAM,
		CURRENT_TEAM,
		SHOW_ALL,
	};
	boost::optional<REPLAY_VISION> vision_;
	std::shared_ptr<config> reset_state_;
	std::function<void()> on_end_replay_;
	bool return_to_play_side_;
};

