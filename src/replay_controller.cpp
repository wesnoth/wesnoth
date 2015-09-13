/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "replay_controller.hpp"

#include "carryover.hpp"
#include "actions/vision.hpp"
#include "display_chat_manager.hpp"
#include "game_end_exceptions.hpp"
#include "game_errors.hpp" //needed to be thrown
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "gettext.hpp"
#include "hotkey_handler_replay.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "mouse_handler_base.hpp"
#include "replay.hpp"
#include "random_new_deterministic.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "saved_game.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "statistics.hpp"
#include "synced_context.hpp"
#include "unit_id.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

class replay_at_end_exception : public std::exception {};

void play_replay_level_main_loop(replay_controller & replaycontroller, bool & is_unit_test);

LEVEL_RESULT play_replay_level(const config& game_config, const tdata_cache & tdata,
		CVideo& video, saved_game& state_of_game, bool is_unit_test)
{
	const int ticks = SDL_GetTicks();

	DBG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << std::endl;

	boost::scoped_ptr<replay_controller> rc;

	const events::command_disabler disable_commands;

	rc.reset(new replay_controller(state_of_game.get_replay_starting_pos(), state_of_game, ticks, game_config, tdata, video, is_unit_test));
	DBG_NG << "created objects... " << (SDL_GetTicks() - rc->get_ticks()) << std::endl;

	//replay event-loop
	rc->main_loop();
	if(rc->is_regular_game_end())
	{
	//	return rc->get_end_level_data_const().is_victory ? LEVEL_RESULT::VICTORY : LEVEL_RESULT::DEFEAT;
		//The replay contained the whole scenario, returns LEVEL_RESULT::VICTORY regardless of the original outcome.
		return LEVEL_RESULT::VICTORY;
	}
	else
	{
		//The replay was finished without reaching the scenario end.
		return LEVEL_RESULT::QUIT;
	}
}

struct replay_play_nostop : public replay_controller::replay_stop_condition
{
	replay_play_nostop() {}
	virtual bool should_stop() { return false; }
};

struct replay_play_moves_base : public replay_controller::replay_stop_condition
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
void replay_controller::main_loop()
{
	if (is_unit_test_) {
		//FIXME: return when at end.
		stop_condition_.reset(new replay_play_nostop());
	}
	//Quits by quit_level_exception
	for (;;) {
		try {
			while(true) {
				play_turn();
				if (is_regular_game_end()) {
					return;
				}
				gamestate_->player_number_ = 1;
			}
			while(true) {
				//lingering
				play_slice();
			}
		}
		catch(const reset_replay_exception&) {
			reset_replay_impl();
		}
		catch(const replay_at_end_exception&) {
			//For unit test
			return;
		}
	}

}

replay_controller::replay_controller(const config& level,
		saved_game& state_of_game, const int ticks,
		const config& game_config, 
		const tdata_cache & tdata, CVideo& video, bool is_unit_test)
	: play_controller(level, state_of_game, ticks, game_config, tdata, video, false)
	, gameboard_start_(gamestate().board_)
	, tod_manager_start_(level)
	, vision_(state_of_game.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER ? CURRENT_TEAM : HUMAN_TEAM)
	, stop_condition_(new replay_stop_condition())
	, level_(level)
	, is_unit_test_(is_unit_test)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the replay controller version

	// Our game_data correctly detects that we are loading a game. However,
	// we are not loading mid-game, so from here on, treat this as not loading
	// a game. (Allows turn_1 et al. events to fire at the correct time.)
	init();
	reset_replay_impl();
}

replay_controller::~replay_controller()
{
	//YogiHH
	//not absolutely sure if this is needed, but it makes me feel a lot better ;-)
	//feel free to delete this if it is not necessary
	gui_->get_theme().theme_reset_event().detach_handler(this);
	gui_->complete_redraw_event().detach_handler(this);
}

void replay_controller::init()
{
	DBG_REPLAY << "in replay_controller::init()...\n";
	
	last_replay_action = REPLAY_FOUND_END_MOVE;
	//guarantee the cursor goes back to 'normal' at the end of the level
	const cursor::setter cursor_setter(cursor::NORMAL);
	init_replay_display();
}

void replay_controller::init_gui()
{
	DBG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks()) << "\n";
	play_controller::init_gui();

	gui_->set_team(vision_ == HUMAN_TEAM ? gamestate().first_human_team_ : 0, vision_ == SHOW_ALL);
	gui_->scroll_to_leader(current_side(), display::WARP);
	update_locker lock_display((*gui_).video(),false);
	BOOST_FOREACH(const team & t, gamestate().board_.teams()) {
		t.reset_objectives_changed();
	}
	get_hotkey_command_executor()->set_button_state(*gui_);
	update_replay_ui();
}

void replay_controller::init_replay_display(){
	DBG_REPLAY << "initializing replay-display... " << (SDL_GetTicks() - ticks()) << "\n";

	rebuild_replay_theme();
	gui_->get_theme().theme_reset_event().attach_handler(this);
	gui_->complete_redraw_event().attach_handler(this);
	DBG_REPLAY << "done initializing replay-display... " << (SDL_GetTicks() - ticks()) << "\n";
}

void replay_controller::rebuild_replay_theme()
{
	const config &theme_cfg = controller_base::get_theme(game_config_, level_["theme"]);
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (const config &replay_theme_cfg = res.child("replay"))
			gui_->get_theme().modify(replay_theme_cfg);
		gui_->get_theme().modify_label("time-icon", _ ("current local time"));
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		gui_->invalidate_theme();
	}
}

gui::button* replay_controller::play_button()
{
	return gui_->find_action_button("button-playreplay");
}

gui::button* replay_controller::stop_button()
{
	return gui_->find_action_button("button-stopreplay");
}

gui::button* replay_controller::reset_button()
{
	return gui_->find_action_button("button-resetreplay");
}

gui::button* replay_controller::play_turn_button()
{
	return gui_->find_action_button("button-nextturn");
}

gui::button* replay_controller::play_side_button()
{
	return gui_->find_action_button("button-nextside");
}

gui::button* replay_controller::play_move_button()
{
	return gui_->find_action_button("button-nextmove");
}

void replay_controller::update_replay_ui()
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

void replay_controller::reset_replay()
{
	throw reset_replay_exception();
}

void replay_controller::reset_replay_impl()
{
	DBG_REPLAY << "replay_controller::reset_replay\n";

	gui_->get_chat_manager().clear_chat_messages();
	reset_gamestate(level_, 0);
#if 0
	gamestate_->player_number_ = level_["playing_team"].to_int() + 1;
	gamestate_->init_side_done() = level_["init_side_done"].to_bool(false);
	skip_replay_ = false;
	gamestate().tod_manager_= tod_manager_start_;
	gamestate().board_ = gameboard_start_;
	gui_->change_display_context(&gamestate().board_); //this doesn't change the pointer value, but it triggers the gui to update the internal terrain builder object,
						   //idk what the consequences of not doing that are, but its probably a good idea to do it, esp. if layout
						   //of game_board changes in the future


	/*if (events_manager_ ){
		// NOTE: this double reset is required so that the new
		// instance of game_events::manager isn't created before the
		// old manager is actually destroyed (triggering an assertion
		// failure)
		events_manager_.reset();
		events_manager_.reset(new game_events::manager(level_));
	}*/

	gamestate().events_manager_.reset();
	resources::game_events = NULL;
	gamestate().lua_kernel_.reset();
	resources::lua_kernel = NULL;
	gamestate().lua_kernel_.reset(new game_lua_kernel(&gui_->video(), gamestate(), *this, *gamestate().reports_));
	gamestate().lua_kernel_->set_game_display(gui_.get());
	resources::lua_kernel = gamestate().lua_kernel_.get();
	gamestate().game_events_resources_->lua_kernel = resources::lua_kernel;
	gamestate().events_manager_.reset(new game_events::manager(level_, gamestate().game_events_resources_));
	resources::game_events = gamestate().events_manager_.get();
	*resources::gamedata = game_data(level_);
#endif
	gui_->labels().read(level_);

	statistics::fresh_stats();

	gui_->needs_rebuild(true);
	gui_->maybe_rebuild();

	// Scenario initialization. (c.f. playsingle_controller::play_scenario())
	start_game(level_);
	update_gui();

	reset_replay_ui();
}

void replay_controller::stop_replay()
{
	stop_condition_.reset(new replay_stop_condition());
}

void replay_controller::replay_next_turn()
{
	stop_condition_.reset(new replay_play_turn(gamestate().tod_manager_.turn()));
}

void replay_controller::replay_next_side()
{
	stop_condition_.reset(new replay_play_side(gamestate().tod_manager_.turn(), current_side()));
}

void replay_controller::replay_next_move()
{
	stop_condition_.reset(new replay_play_moves(1));
}


void replay_controller::process_oos(const std::string& msg) const
{
	if (game_config::ignore_replay_errors) {
		return;
	}

	std::stringstream message;
	message << _("The replay is corrupt/out of sync. It might not make much sense to continue. Do you want to save the game?");
	message << "\n\n" << _("Error details:") << "\n\n" << msg;

	if (non_interactive()) {
		throw game::game_error(message.str());
	} else {
		scoped_savegame_snapshot snapshot(*this);
		savegame::oos_savegame save(saved_game_, *gui_);
		save.save_game_interactive(resources::screen->video(), message.str(), gui::YES_NO); // can throw end_level_exception
	}
}

void replay_controller::replay_show_everything()
{
	vision_ = SHOW_ALL;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_each()
{
	vision_ = CURRENT_TEAM;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_team1()
{
	vision_ = HUMAN_TEAM;
	update_teams();
	update_gui();
}

void replay_controller::replay_skip_animation(){
	skip_replay_ = !skip_replay_;
}

//move all sides till stop/end
void replay_controller::play_replay()
{
	stop_condition_.reset(new replay_play_nostop());
}


void replay_controller::update_teams()
{
	gui_->set_team(vision_ == HUMAN_TEAM ? gamestate().first_human_team_ : current_side() - 1, vision_ == SHOW_ALL);
	gui_->invalidate_all();
}

void replay_controller::update_gui()
{
	(*gui_).recalculate_minimap();
	(*gui_).redraw_minimap();
	(*gui_).invalidate_all();
	events::raise_draw_event();
	(*gui_).draw();
}

void replay_controller::handle_generic_event(const std::string& name)
{

	if( name == "completely_redrawn" ) {
		update_replay_ui();

		gui::button* skip_animation_button = gui_->find_action_button("skip-animation");
		if(skip_animation_button) {
			skip_animation_button->set_check(is_skipping_replay());
		}
	} else {
		rebuild_replay_theme();
	}
}

bool replay_controller::recorder_at_end() {
	return resources::recorder->at_end();
}


void replay_controller::play_side_impl()
{
	stop_condition_->new_side_turn(current_side(), gamestate().tod_manager_.turn());
	while(true)
	{
		if(!stop_condition_->should_stop())
		{
			last_replay_action = do_replay(true);
			if(last_replay_action == REPLAY_FOUND_END_MOVE) {
				stop_condition_->move_done();
			}
			if(last_replay_action == REPLAY_FOUND_END_TURN) {
				return;
			}
			if(last_replay_action == REPLAY_RETURN_AT_END) {
				replay_ui_playback_should_stop();
				if(is_unit_test_) {
					throw replay_at_end_exception();
				}
			}
			play_slice(false);
		}
		else
		{
			play_slice(true);
			replay_ui_playback_should_stop();
		}
		
	}
}

void replay_controller::update_viewing_player()
{
	update_gui_to_player(vision_ == HUMAN_TEAM ? gamestate().first_human_team_ : current_side() - 1, vision_ == SHOW_ALL);

}
