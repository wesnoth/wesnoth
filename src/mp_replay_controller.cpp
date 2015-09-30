/*
   Copyright (C) 2015 by the Battle for Wesnoth Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "mp_replay_controller.hpp"

#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "config_assign.hpp"
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

namespace
{
struct replay_play_nostop : public mp_replay_controller::replay_stop_condition
{
	replay_play_nostop() {}
	virtual bool should_stop() { return false; }
};

struct replay_play_moves_base : public mp_replay_controller::replay_stop_condition
{
	int moves_todo_;
	bool started_;
	replay_play_moves_base(int moves_todo, bool started = true) : moves_todo_(moves_todo), started_(started) {}
	virtual void move_done() { if(started_) { --moves_todo_; } }
	virtual bool should_stop() { return moves_todo_ == 0; }
	void start() { started_ = true; }
};

struct replay_play_moves : public replay_play_moves_base
{
	replay_play_moves(int moves_todo) : replay_play_moves_base(moves_todo, true) {}
};

struct replay_play_turn : public replay_play_moves_base
{
	int turn_begin_;
	replay_play_turn(int turn_begin) : replay_play_moves_base(1, false), turn_begin_(turn_begin) {}
	virtual void new_side_turn(int , int turn)
	{ 
		if (turn != turn_begin_) {
			start();
		}
	}
};

struct replay_play_side : public replay_play_moves_base
{
	int turn_begin_;
	int side_begin_;
	replay_play_side(int turn_begin, int side_begin) : replay_play_moves_base(1, false), turn_begin_(turn_begin), side_begin_(side_begin) {}
	virtual void new_side_turn(int side , int turn)
	{ 
		if (turn != turn_begin_ || side != side_begin_) {
			start();
		}
	}
};
}
namespace
{
	config get_theme_removal_diff(const config& theme_cfg)
	{
		config res;
		BOOST_FOREACH(const config::any_child& pair, theme_cfg.child_or_empty("replay").all_children_range())
		{
			if(pair.key == "add") {
				BOOST_FOREACH(const config::any_child& inner, pair.cfg.all_children_range())
				{
					res.add_child("remove", config_of("id", inner.cfg["id"]));
				}
			}
			if(pair.key == "change") {
				std::string id = pair.cfg["id"];
				BOOST_FOREACH(const config::any_child& inner, theme_cfg.all_children_range())
				{
					if(inner.cfg["id"] == id) {
						
						res.add_child("change", config_of("id", id)("rect", inner.cfg["rect"]));
					}
					break;
				}
			}
		}
		return res;
	}
}

mp_replay_controller::mp_replay_controller(play_controller& controller, bool control_view, const boost::shared_ptr<config>& reset_state, const boost::function<void()>& on_end_replay)
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
	controller_.get_display().redraw_everything();
	//add_replay_theme();
}
mp_replay_controller::~mp_replay_controller()
{
	if(controller_.is_skipping_replay()) {
		controller_.toggle_skipping_replay();
	}
	controller_.get_display().get_theme().theme_reset_event().detach_handler(this);
	controller_.get_display().redraw_everything();
	//remove_replay_theme();
}
void mp_replay_controller::add_replay_theme()
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
void mp_replay_controller::remove_replay_theme()
{
	const config &theme_cfg = controller_.get_theme(game_config_manager::get()->game_config(), controller_.theme());
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (res.has_child("replay")) {
			controller_.get_display().get_theme().modify(get_theme_removal_diff(res));
		}
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		controller_.get_display().invalidate_theme();
	}
}


void mp_replay_controller::rebuild_replay_theme()
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

gui::button* mp_replay_controller::play_button()
{
	return controller_.get_display().find_action_button("button-playreplay");
}

gui::button* mp_replay_controller::stop_button()
{
	return controller_.get_display().find_action_button("button-stopreplay");
}

gui::button* mp_replay_controller::reset_button()
{
	return controller_.get_display().find_action_button("button-resetreplay");
}

gui::button* mp_replay_controller::play_turn_button()
{
	return controller_.get_display().find_action_button("button-nextturn");
}

gui::button* mp_replay_controller::play_side_button()
{
	return controller_.get_display().find_action_button("button-nextside");
}

gui::button* mp_replay_controller::play_move_button()
{
	return controller_.get_display().find_action_button("button-nextmove");
}

void mp_replay_controller::update_replay_ui()
{
	//check if we have all buttons - if someone messed with theme then some buttons may be missing
	//if any of the buttons is missing, we just disable every one
	if(!replay_ui_has_all_buttons()) {
		gui::button *play_b = play_button(), *stop_b = stop_button(),
		            *reset_b = reset_button(), *play_turn_b = play_turn_button(),
		            *play_side_b = play_side_button(), *play_move_b = play_move_button();

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

void mp_replay_controller::replay_ui_playback_should_start()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(false);
	reset_button()->enable(false);
	play_turn_button()->enable(false);
	play_side_button()->enable(false);
	play_move_button()->enable(false);
}

void mp_replay_controller::replay_ui_playback_should_stop()
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

void mp_replay_controller::reset_replay_ui()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(true);
	stop_button()->enable(true);
	reset_button()->enable(true);
	play_turn_button()->enable(true);
	play_side_button()->enable(true);
}


void mp_replay_controller::stop_replay()
{
	stop_condition_.reset(new replay_stop_condition());
}

void mp_replay_controller::replay_next_turn()
{
	stop_condition_.reset(new replay_play_turn(controller_.gamestate().tod_manager_.turn()));
}

void mp_replay_controller::replay_next_side()
{
	stop_condition_.reset(new replay_play_side(controller_.gamestate().tod_manager_.turn(), controller_.current_side()));
}

void mp_replay_controller::replay_next_move()
{
	stop_condition_.reset(new replay_play_moves(1));
}

//move all sides till stop/end
void mp_replay_controller::play_replay()
{
	stop_condition_.reset(new replay_play_nostop());
}

void mp_replay_controller::update_gui()
{
	controller_.get_display().recalculate_minimap();
	controller_.get_display().redraw_minimap();
	controller_.get_display().invalidate_all();
	events::raise_draw_event();
	controller_.get_display().draw();
}

void mp_replay_controller::handle_generic_event(const std::string& name)
{

	if( name == "completely_redrawn" ) {
		update_replay_ui();
	} else {
		add_replay_theme();
	}
	if(gui::button* skip_animation_button = controller_.get_display().find_action_button("skip-animation")) {
		skip_animation_button->set_check(controller_.is_skipping_replay());
	}
}

bool mp_replay_controller::recorder_at_end() const
{
	return resources::recorder->at_end();
}

#include "playsingle_controller.hpp"
REPLAY_RETURN mp_replay_controller::play_side_impl()
{
	stop_condition_->new_side_turn(controller_.current_side(), controller_.gamestate().tod_manager_.turn());
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
					new replay_stop_condition(); 
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
bool mp_replay_controller::can_execute_command(const hotkey::hotkey_command& cmd, int) const
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

void mp_replay_controller::replay_show_everything()
{
	vision_ = SHOW_ALL;
	update_teams();
}

void mp_replay_controller::replay_show_each()
{
	vision_ = CURRENT_TEAM;
	update_teams();
}

void mp_replay_controller::replay_show_team1()
{
	vision_ = HUMAN_TEAM;
	update_teams();
}

void mp_replay_controller::update_teams()
{
	update_viewing_player();
	controller_.get_display().invalidate_all();
	update_gui();
}

void mp_replay_controller::update_viewing_player()
{
	assert(vision_);
	controller_.update_gui_to_player(vision_ == HUMAN_TEAM ? controller_.gamestate().first_human_team_ : controller_.current_side() - 1, *vision_ == SHOW_ALL);
}
