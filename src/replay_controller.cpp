/*
	Copyright (C) 2015 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "replay_controller.hpp"

#include "log.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "playsingle_controller.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

namespace
{
struct replay_play_nostop : public replay_controller::replay_stop_condition
{
	replay_play_nostop() {}
	virtual bool should_stop() { return false; }
};

struct replay_play_moves : public replay_controller::replay_stop_condition
{
	int moves_todo_;
	replay_play_moves(int moves_todo) : moves_todo_(moves_todo) {}
	virtual void move_done() { --moves_todo_; }
	virtual bool should_stop() { return moves_todo_ == 0; }
};

struct replay_play_turn : public replay_controller::replay_stop_condition
{
	int turn_begin_;
	int turn_current_;
	replay_play_turn(int turn_begin) : turn_begin_(turn_begin), turn_current_(turn_begin) {}
	virtual void new_side_turn(int , int turn) { turn_current_ = turn; }
	virtual bool should_stop() { return turn_begin_ != turn_current_; }
};

struct replay_play_side : public replay_controller::replay_stop_condition
{
	bool next_side_;
	replay_play_side() : next_side_(false) {}
	virtual void new_side_turn(int , int) { next_side_ = true; }
	virtual bool should_stop() { return next_side_; }
};
}

replay_controller::replay_controller(play_controller& controller, bool control_view, const std::shared_ptr<config>& reset_state, const std::function<void()>& on_end_replay)
	: controller_(controller)
	, stop_condition_(new replay_stop_condition())
	, disabler_()
	, vision_()
	, reset_state_(reset_state)
	, on_end_replay_(on_end_replay)
	, return_to_play_side_(false)
{
	if(control_view) {
		vision_ = HUMAN_TEAM;
	}
	controller_.get_display().get_theme().theme_reset_event().attach_handler(this);
	controller_.get_display().queue_rerender();
}
replay_controller::~replay_controller()
{
	if(controller_.is_skipping_replay()) {
		controller_.toggle_skipping_replay();
	}
	controller_.get_display().get_theme().theme_reset_event().detach_handler(this);
	controller_.get_display().queue_rerender();
}
void replay_controller::add_replay_theme()
{
	const config& theme_cfg = theme::get_theme_config(controller_.theme());
	if (const auto res = theme_cfg.optional_child("resolution"))
	{
		if (const auto replay_theme_cfg = res->optional_child("replay")) {
			controller_.get_display().get_theme().modify(replay_theme_cfg.value());
		}
	}
}

void replay_controller::stop_replay()
{
	stop_condition_.reset(new replay_stop_condition());
	update_enabled_buttons();
}

void replay_controller::replay_next_turn()
{
	stop_condition_.reset(new replay_play_turn(controller_.gamestate().tod_manager_.turn()));
	update_enabled_buttons();
}

void replay_controller::replay_next_side()
{
	stop_condition_.reset(new replay_play_side());
	update_enabled_buttons();
}

void replay_controller::replay_next_move()
{
	stop_condition_.reset(new replay_play_moves(1));
	update_enabled_buttons();
}

//move all sides till stop/end
void replay_controller::play_replay()
{
	stop_condition_.reset(new replay_play_nostop());
	update_enabled_buttons();
}

void replay_controller::update_gui()
{
	controller_.get_display().queue_rerender();
}

void replay_controller::update_enabled_buttons()
{
	controller_.get_display().queue_rerender();
}

void replay_controller::handle_generic_event(const std::string& name)
{
	// this is only attached to one event - the theme_reset_event
	if(name == "theme_reset") {
		add_replay_theme();
	}
	if(std::shared_ptr<gui::button> skip_animation_button = controller_.get_display().find_action_button("skip-animation")) {
		skip_animation_button->set_check(controller_.is_skipping_replay());
	}
}

bool replay_controller::recorder_at_end() const
{
	return resources::recorder->at_end();
}

void replay_controller::play_side_impl()
{
	update_enabled_buttons();
	while(!return_to_play_side_ && !static_cast<playsingle_controller&>(controller_).get_player_type_changed())
	{
		if(!stop_condition_->should_stop())
		{
			if(resources::recorder->at_end()) {
				//Gather more replay data
				on_end_replay_();
			}
			else {
				REPLAY_RETURN res = do_replay(true);
				if(controller_.is_regular_game_end()) {
					return;
				}
				if(res == REPLAY_FOUND_END_TURN) {
					return;
				}
				stop_condition_->move_done();
				if(res == REPLAY_FOUND_INIT_TURN)
				{
					stop_condition_->new_side_turn(controller_.current_side(), controller_.gamestate().tod_manager_.turn());
				}
			}
			controller_.play_slice();

			// Update the buttons once, on the transition from not-stopped to stopped.
			if(stop_condition_->should_stop()) {
				update_enabled_buttons();
			}
		}
		else
		{
			// Don't move the update_enabled_buttons() call here. This play_slice() should block
			// until the next event occurs, but on X11/Linux update_enabled_buttons() seems to put
			// an event in the queue, turning this into a busy loop.
			controller_.play_slice();
		}
	}
	return;
}
bool replay_controller::can_execute_command(const hotkey::ui_command& cmd) const
{
	switch(cmd.hotkey_command) {
	case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
		return true;
	case hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING:
	case hotkey::HOTKEY_REPLAY_SHOW_EACH:
	case hotkey::HOTKEY_REPLAY_SHOW_TEAM1:
		return is_controlling_view();
	//commands we only can do before the end of the replay
	case hotkey::HOTKEY_REPLAY_STOP:
		return !recorder_at_end();
	case hotkey::HOTKEY_REPLAY_PLAY:
	case hotkey::HOTKEY_REPLAY_NEXT_TURN:
	case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
	case hotkey::HOTKEY_REPLAY_NEXT_MOVE:
		//we have one events_disabler when starting the replay_controller and a second when entering the synced context.
		return should_stop() && (events::commands_disabled <= 1 ) && !recorder_at_end();
	case hotkey::HOTKEY_REPLAY_RESET:
		return allow_reset_replay() && events::commands_disabled <= 1;
	default:
		assert(false);
		return false;
	}
}

void replay_controller::replay_show_everything()
{
	vision_ = SHOW_ALL;
	update_teams();
}

void replay_controller::replay_show_each()
{
	vision_ = CURRENT_TEAM;
	update_teams();
}

void replay_controller::replay_show_team1()
{
	vision_ = HUMAN_TEAM;
	update_teams();
}

void replay_controller::update_teams()
{
	update_viewing_player();
	controller_.get_display().invalidate_all();
	update_gui();
}

void replay_controller::update_viewing_player()
{
	assert(vision_);
	int viewing_side_num = vision_ == HUMAN_TEAM ? controller_.find_viewing_side() : controller_.current_side();
	if(viewing_side_num != 0) {
		controller_.update_gui_to_player(viewing_side_num - 1, *vision_ == SHOW_ALL);
	}
}

bool replay_controller::see_all()
{
	return vision_ == SHOW_ALL;
}
