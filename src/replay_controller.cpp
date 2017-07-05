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

#include "replay_controller.hpp"

#include "game_config_manager.hpp"
#include "gettext.hpp"
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
	controller_.get_display().create_buttons();
	controller_.get_display().redraw_everything();
}
replay_controller::~replay_controller()
{
	if(controller_.is_skipping_replay()) {
		controller_.toggle_skipping_replay();
	}
	controller_.get_display().get_theme().theme_reset_event().detach_handler(this);
	controller_.get_display().create_buttons();
	controller_.get_display().redraw_everything();
	controller_.get_display().create_buttons();
}
void replay_controller::add_replay_theme()
{
	const config &theme_cfg = controller_.get_theme(game_config_manager::get()->game_config(), controller_.theme());
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (const config &replay_theme_cfg = res.child("replay")) {
			controller_.get_display().get_theme().modify(replay_theme_cfg);
		}
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		controller_.get_display().invalidate_theme();
	}
}


void replay_controller::rebuild_replay_theme()
{
	const config &theme_cfg = controller_.get_theme(game_config_manager::get()->game_config(), controller_.theme());
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (const config &replay_theme_cfg = res.child("replay")) {
			controller_.get_display().get_theme().modify(replay_theme_cfg);
		}
		controller_.get_display().get_theme().modify_label("time-icon", _ ("current local time"));
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		controller_.get_display().invalidate_theme();
	}
}

std::shared_ptr<gui::button> replay_controller::play_button()
{
	return controller_.get_display().find_action_button("button-playreplay");
}

std::shared_ptr<gui::button> replay_controller::stop_button()
{
	return controller_.get_display().find_action_button("button-stopreplay");
}

std::shared_ptr<gui::button> replay_controller::reset_button()
{
	return controller_.get_display().find_action_button("button-resetreplay");
}

std::shared_ptr<gui::button> replay_controller::play_turn_button()
{
	return controller_.get_display().find_action_button("button-nextturn");
}

std::shared_ptr<gui::button> replay_controller::play_side_button()
{
	return controller_.get_display().find_action_button("button-nextside");
}

std::shared_ptr<gui::button> replay_controller::play_move_button()
{
	return controller_.get_display().find_action_button("button-nextmove");
}

void replay_controller::update_replay_ui()
{
	//check if we have all buttons - if someone messed with theme then some buttons may be missing
	//if any of the buttons is missing, we just disable every one
	if(!replay_ui_has_all_buttons()) {
		std::shared_ptr<gui::button> play_b = play_button(), stop_b = stop_button(),
		            reset_b = reset_button(), play_turn_b = play_turn_button(),
		            play_side_b = play_side_button(), play_move_b = play_move_button();

		if(play_b) {
			play_b->enable(false);
		}

		if(stop_b) {
			stop_b->enable(false);
		}

		if(reset_b) {
			reset_b->enable(false);
		}

		if(play_turn_b) {
			play_turn_b->enable(false);
		}

		if(play_side_b) {
			play_side_b->enable(false);
		}

		if (play_move_b) {
			play_move_b->enable(false);
		}
	}
}

void replay_controller::replay_ui_playback_should_start()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(false);
	reset_button()->enable(false);
	play_turn_button()->enable(false);
	play_side_button()->enable(false);
	play_move_button()->enable(false);
}

void replay_controller::replay_ui_playback_should_stop()
{
	if(!replay_ui_has_all_buttons())
		return;

	if(!resources::recorder->at_end()) {
		play_button()->enable(true);
		reset_button()->enable(true);
		play_turn_button()->enable(true);
		play_side_button()->enable(true);
		play_move_button()->enable(true);

		play_button()->release();
		play_turn_button()->release();
		play_side_button()->release();
		play_move_button()->release();
	} else {
		reset_button()->enable(true);
		stop_button()->enable(false);
	}

	if(stop_condition_->should_stop()) {
		//user interrupted
		stop_button()->release();
	}
}

void replay_controller::reset_replay_ui()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(true);
	stop_button()->enable(true);
	reset_button()->enable(true);
	play_turn_button()->enable(true);
	play_side_button()->enable(true);
}


void replay_controller::stop_replay()
{
	stop_condition_.reset(new replay_stop_condition());
}

void replay_controller::replay_next_turn()
{
	stop_condition_.reset(new replay_play_turn(controller_.gamestate().tod_manager_.turn()));
}

void replay_controller::replay_next_side()
{
	stop_condition_.reset(new replay_play_side());
}

void replay_controller::replay_next_move()
{
	stop_condition_.reset(new replay_play_moves(1));
}

//move all sides till stop/end
void replay_controller::play_replay()
{
	stop_condition_.reset(new replay_play_nostop());
}

void replay_controller::update_gui()
{
	controller_.get_display().recalculate_minimap();
	controller_.get_display().redraw_minimap();
	controller_.get_display().draw();
}

void replay_controller::handle_generic_event(const std::string& name)
{

	if( name == "completely_redrawn" ) {
		update_replay_ui();
	} else {
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

REPLAY_RETURN replay_controller::play_side_impl()
{
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
				if(res == REPLAY_FOUND_END_MOVE) {
					stop_condition_->move_done();
				}
				if(res == REPLAY_FOUND_END_TURN) {
					return res;
				}
				if(res == REPLAY_RETURN_AT_END) {
					stop_replay();
				}
				if(res == REPLAY_FOUND_INIT_TURN)
				{
					stop_condition_->new_side_turn(controller_.current_side(), controller_.gamestate().tod_manager_.turn());
				}
			}
			controller_.play_slice(false);
		}
		else
		{
			controller_.play_slice(true);
			replay_ui_playback_should_stop();
		}
	}
	return REPLAY_FOUND_END_MOVE;
}
bool replay_controller::can_execute_command(const hotkey::hotkey_command& cmd, int) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;

	switch(command) {
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
	update_gui();
}

void replay_controller::update_viewing_player()
{
	assert(vision_);
	controller_.update_gui_to_player(vision_ == HUMAN_TEAM ? controller_.gamestate().first_human_team_ : controller_.current_side() - 1, *vision_ == SHOW_ALL);
}
