/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playmp_controller.hpp"

#include "dialogs.hpp"
#include "game_errors.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "playturn.hpp"
#include "sound.hpp"
#include "upload_log.hpp"

#include <cassert>

#define LOG_NG LOG_STREAM(info, engine)

unsigned int playmp_controller::replay_last_turn_ = 0;

playmp_controller::playmp_controller(const config& level,
	const game_data& gameinfo, game_state& state_of_game, const int ticks,
	const int num_turns, const config& game_config, CVideo& video,
	bool skip_replay, bool is_host)
	: playsingle_controller(level, gameinfo, state_of_game, ticks, num_turns,
		game_config, video, skip_replay)
{
	beep_warning_time_ = 0;
	turn_data_ = NULL;
	is_host_ = is_host;
	// We stop quick replay if play isn't yet past turn 1
	if ( replay_last_turn_ <= 1)
	{
		skip_replay_ = false;
	}
}

playmp_controller::~playmp_controller() {
	//halt and cancel the countdown timer
	if(beep_warning_time_ < 0) {
		sound::stop_bell();
	}
}

void playmp_controller::set_replay_last_turn(unsigned int turn){
	 replay_last_turn_ = turn;
}

void playmp_controller::clear_labels(){
	menu_handler_.clear_labels();
}

void playmp_controller::speak(){
	menu_handler_.speak();
}

void playmp_controller::whisper(){
	menu_handler_.whisper();
}

void playmp_controller::shout(){
	menu_handler_.shout();
}

void playmp_controller::play_side(const unsigned int team_index, bool save){
	do {
		player_type_changed_ = false;
		end_turn_ = false;

		statistics::reset_turn_stats(player_number_);

		// we can't call playsingle_controller::play_side because
		// we need to catch exception here
		if(current_team().is_human()) {
			LOG_NG << "is human...\n";

			// reset default state
			beep_warning_time_ = 0;

			try{
				before_human_turn(save);
				play_human_turn();
				after_human_turn();
			} catch(end_turn_exception& end_turn) {
				if (end_turn.redo == team_index) {
					player_type_changed_ = true;
					// if new controller is not human,
					// reset gui to prev human one
					if (!teams_[team_index-1].is_human()) {
						int t = find_human_team_before(team_index);
						
						if (t <= 0)
							t = gui_->get_playing_team() + 1;

						gui_->set_team(t-1);
						gui_->recalculate_minimap();
						gui_->invalidate_all();
						gui_->draw();
						gui_->update_display();
					}
				}
			}
			LOG_NG << "human finished turn...\n";
		} else if(current_team().is_ai()) {
			play_ai_turn();
		} else if(current_team().is_network()) {
			play_network_turn();
		}
	} while (player_type_changed_);
	//keep looping if the type of a team (human/ai/networked) has changed mid-turn
}

void playmp_controller::before_human_turn(bool save){
	playsingle_controller::before_human_turn(save);

	turn_data_ = new turn_info(gameinfo_,gamestate_,status_,
		*gui_,map_,teams_,player_number_,units_,replay_sender_, undo_stack_);
	turn_data_->replay_error().attach_handler(this);
	turn_data_->host_transfer().attach_handler(this);
}

bool playmp_controller::counting_down() {
	return beep_warning_time_ > 0;
}

namespace {
	const int WARNTIME = 20000; //start beeping when 20 seconds are left (20,000ms)
	unsigned timer_refresh = 0;
	const unsigned timer_refresh_rate = 50; // prevents calling SDL_GetTicks() too frequently
}

//make sure we think about countdown even while dialogs are open
void playmp_controller::process(events::pump_info &info) {
	if(playmp_controller::counting_down()) {
		if(info.ticks(&timer_refresh, timer_refresh_rate)) {
			playmp_controller::think_about_countdown(info.ticks());
		}
	}
}

//check if it is time to start playing the timer warning
void playmp_controller::think_about_countdown(int ticks) {
	if(ticks >= beep_warning_time_) {
		const bool bell_on = preferences::turn_bell();
		if(bell_on || preferences::sound_on() || preferences::UI_sound_on()) {
			const int loop_ticks = WARNTIME - (ticks - beep_warning_time_);
			const int fadein_ticks = (loop_ticks > WARNTIME / 2) ? loop_ticks - WARNTIME / 2 : 0;
			sound::play_timer(game_config::sounds::timer_bell, loop_ticks, fadein_ticks);
			beep_warning_time_ = -1;
		}
	}
}

void playmp_controller::play_human_turn(){
	int cur_ticks = SDL_GetTicks();

	if ((!linger_) || (is_host_))
		gui_->enable_menu("endturn", true);
	while(!end_turn_) {

		try {
			config cfg;
			const network::connection res = network::receive_data(cfg);
			std::deque<config> backlog;

			if(res != network::null_connection) {
				try{
					if (turn_data_->process_network_data(cfg,res,backlog,skip_replay_) == turn_info::PROCESS_RESTART_TURN)
					{
						throw end_turn_exception(gui_->get_playing_team() + 1);
					}
				}
				catch (replay::error& e){
					process_oos(e.message);
					throw e;
				}
			}

			play_slice();
		} catch(end_level_exception& e) {
			turn_data_->send_data();
			throw e;
		}

		if (!linger_ && (current_team().countdown_time() > 0) && (level_["mp_countdown"] == "yes")) {
			SDL_Delay(1);
			const int ticks = SDL_GetTicks();
			int new_time = current_team().countdown_time()-maximum<int>(1,(ticks - cur_ticks));
			if (new_time > 0 ){
				current_team().set_countdown_time(new_time);
				cur_ticks = ticks;
				if(current_team().is_human() && !beep_warning_time_) {
					beep_warning_time_ = new_time - WARNTIME + ticks;
				}
				if(counting_down()) {
					think_about_countdown(ticks);
				}
			} else {
				// Clock time ended
				// If no turn bonus or action bonus -> defeat
				const int action_increment = lexical_cast_default<int>(level_["mp_countdown_action_bonus"],0);
				if ( lexical_cast_default<int>(level_["mp_countdown_turn_bonus"],0) == 0
					&& (action_increment == 0 || current_team().action_bonus_count() == 0)) {
					// Not possible to end level in MP with throw end_level_exception(DEFEAT);
					// because remote players only notice network disconnection
					// Current solution end remaining turns automatically
					current_team().set_countdown_time(10);
				} else {
					const int maxtime = lexical_cast_default<int>(level_["mp_countdown_reservoir_time"],0);
					int secs = lexical_cast_default<int>(level_["mp_countdown_turn_bonus"],0);
					secs += action_increment  * current_team().action_bonus_count();
					current_team().set_action_bonus_count(0);
					secs = (secs > maxtime) ? maxtime : secs;
					current_team().set_countdown_time(1000 * secs);
				}
				recorder.add_countdown_update(current_team().countdown_time(),player_number_);
				recorder.end_turn();
				turn_data_->send_data();

				throw end_turn_exception();
			}
		}

		gui_->draw();

		turn_data_->send_data();
	}
	menu_handler_.clear_undo_stack(player_number_);
}

void playmp_controller::set_end_scenario_button()
{
	// Modify the end-turn button
	if (! is_host_) {
		gui::button* btn_end = gui_->find_button("button-endturn");
		btn_end->enable(false);
	}
	gui_->get_theme().refresh_title("button-endturn", _("End scenario"));
	gui_->invalidate_theme();
	gui_->redraw_everything();
}

void playmp_controller::reset_end_scenario_button()
{
	// revert the end-turn button text to its normal label
	gui_->get_theme().refresh_title2("button-endturn", "title");
	gui_->invalidate_theme();
	gui_->redraw_everything();
	gui_->set_game_mode(game_display::RUNNING);
}

void playmp_controller::linger(upload_log& log)
{
	LOG_NG << "beginning end-of-scenario linger\n";
	browse_ = true;
	linger_ = true;
	// If we need to set the status depending on the completion state
	// we're needed here.
	gui_->set_game_mode(game_display::LINGER_MP);

	// this is actually for after linger mode is over -- we don't want to
	// stay stuck in linger state when the *next* scenario is over.
	gamestate_.completion = "running";
	// End all unit moves
	for (unit_map::iterator u = units_.begin(); u != units_.end(); u++) {
		u->second.set_user_end_turn(true);
	}
	//current_team().set_countdown_time(0);
	//halt and cancel the countdown timer
	if(beep_warning_time_ < 0) {
		sound::stop_bell();
	}
	beep_warning_time_=-1;

	set_end_scenario_button();

	// switch to observer viewpoint
	gui_->set_team(0,true);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
	gui_->draw();
	gui_->update_display();

	bool quit;
	do {
		quit = true;
		try {
			// reimplement parts of play_side()
			turn_data_ = new turn_info(gameinfo_, gamestate_, status_,
			                           *gui_,map_, teams_, player_number_,
			                           units_, replay_sender_, undo_stack_);
			turn_data_->replay_error().attach_handler(this);
			turn_data_->host_transfer().attach_handler(this);

			play_human_turn();
			after_human_turn();
			LOG_NG << "finished human turn" << std::endl;
		} catch (game::load_game_exception&) {
			LOG_NG << "caught load-game-exception" << std::endl;
			// this should not happen, the option to load a game is disabled
			log.quit(status_.turn());
			throw;
		} catch (end_level_exception&) {
			// thrown if the host ends the scenario and let us advance
			// to the next level
			LOG_NG << "caught end-level-exception" << std::endl;
			reset_end_scenario_button();
			throw;
		} catch (end_turn_exception&) {
			// thrown if the host leaves the game (sends [leave_game]), we need
			// to stay in this loop to stay in linger mode, otherwise the game
			// gets aborted
			LOG_NG << "caught end-turn-exception" << std::endl;
			quit = false;
		} catch (network::error&) {
			LOG_NG << "caught network-error-exception" << std::endl;
			quit = false;
		}
	} while (!quit);

	reset_end_scenario_button();

	LOG_NG << "ending end-of-scenario linger\n";
}

//! Wait for the host to upload the next scenario.
void playmp_controller::wait_for_upload()
{
	// If the host is here we'll never leave since we wait for the host to
	// upload the next scenario.
	assert(!is_host_);

	const bool set_turn_data = (turn_data_ == 0);
	if(set_turn_data) {
		turn_data_ = new turn_info(gameinfo_,gamestate_,status_,
						*gui_,map_,teams_,player_number_,units_,replay_sender_, undo_stack_);
		turn_data_->replay_error().attach_handler(this);
		turn_data_->host_transfer().attach_handler(this);
	}

	while(true) {
		try {
			config cfg;
			const network::connection res = dialogs::network_receive_dialog(
				*gui_, _("Waiting for next scenario..."), cfg);

			std::deque<config> backlog;
			if(res != network::null_connection) {
				try{
					if(turn_data_->process_network_data(cfg,res,backlog,skip_replay_) 
							== turn_info::PROCESS_END_LINGER) {
						break;
					}
				}
				catch (replay::error& e){
					process_oos(e.message);
					throw e;
				}
			}

		} catch(end_level_exception& e) {
			turn_data_->send_data();
			throw e;
		}
	}

	if(set_turn_data) {
		delete turn_data_;
		turn_data_ = 0;
	}
}

void playmp_controller::after_human_turn(){
	if ( level_["mp_countdown"] == "yes" ){
		const int action_increment = lexical_cast_default<int>(level_["mp_countdown_action_bonus"],0);
		const int maxtime = lexical_cast_default<int>(level_["mp_countdown_reservoir_time"],0);
		int secs = (current_team().countdown_time() / 1000) + lexical_cast_default<int>(level_["mp_countdown_turn_bonus"],0);
		secs += action_increment  * current_team().action_bonus_count();
		current_team().set_action_bonus_count(0);
		secs = (secs > maxtime) ? maxtime : secs;
		current_team().set_countdown_time(1000 * secs);
		recorder.add_countdown_update(current_team().countdown_time(),player_number_);
	}
	end_turn_record();

	//send one more time to make sure network is up-to-date.
	turn_data_->send_data();
	if (turn_data_ != NULL){
		turn_data_->replay_error().detach_handler(this);
		turn_data_->host_transfer().detach_handler(this);
		delete turn_data_;
		turn_data_ = NULL;
	}

	playsingle_controller::after_human_turn();
}

void playmp_controller::finish_side_turn(){
	play_controller::finish_side_turn();

	//just in case due to an exception turn_data_ has not been deleted in after_human_turn
	delete turn_data_;
	turn_data_ = NULL;

	//halt and cancel the countdown timer
	if(beep_warning_time_ < 0) {
		sound::stop_bell();
	}
}

void playmp_controller::play_network_turn(){
	LOG_NG << "is networked...\n";

	browse_ = true;
	gui_->enable_menu("endturn", false);
	turn_info turn_data(gameinfo_,gamestate_,status_,*gui_,
				map_,teams_,player_number_,units_, replay_sender_, undo_stack_);
	turn_data.replay_error().attach_handler(this);
	turn_data.host_transfer().attach_handler(this);

	for(;;) {

		bool have_data = false;
		config cfg;

		network::connection from = network::null_connection;

		if(data_backlog_.empty() == false) {
			have_data = true;
			cfg = data_backlog_.front();
			data_backlog_.pop_front();
		} else {
			from = network::receive_data(cfg);
			have_data = from != network::null_connection;
		}

		if(have_data) {
			if (skip_replay_ && replay_last_turn_ <= status_.turn()){
					skip_replay_ = false;
			}
			try{
				const turn_info::PROCESS_DATA_RESULT result = turn_data.process_network_data(cfg,from,data_backlog_,skip_replay_);
				if(result == turn_info::PROCESS_RESTART_TURN) {
					player_type_changed_ = true;
					return;
				} else if(result == turn_info::PROCESS_END_TURN) {
					break;
				}
			}
			catch (replay::error e){
				process_oos(e.message);
				throw e;
			}

		}

		play_slice();
		turn_data.send_data();
		gui_->draw();
	}

	turn_data.replay_error().detach_handler(this);
	turn_data.host_transfer().detach_handler(this);
	LOG_NG << "finished networked...\n";
	return;
}

void playmp_controller::process_oos(const std::string& err_msg){
	std::stringstream temp_buf;
	std::vector<std::string> err_lines = utils::split(err_msg,'\n');
	temp_buf << _("The game is out of sync, and cannot continue. There are a number of reasons this could happen: this can occur if you or another player have modified their game settings. This may mean one of the players is attempting to cheat. It could also be due to a bug in the game, but this is less likely.\n\nDo you want to save an error log of your game?");
	if(!err_msg.empty()) {
		temp_buf << " \n \n"; //and now the "Details:"
		for(std::vector<std::string>::iterator i=err_lines.begin(); i!=err_lines.end(); i++)
		{
			temp_buf << "`#" << *i << '\n';
		}
		temp_buf << " \n";
	}
	menu_handler_.save_game(temp_buf.str(),gui::YES_NO, true);
}

void playmp_controller::handle_generic_event(const std::string& name){
	turn_info turn_data(gameinfo_,gamestate_,status_,*gui_,
						map_,teams_,player_number_,units_, replay_sender_, undo_stack_);

	if (name == "ai_user_interact"){
		playsingle_controller::handle_generic_event(name);
		turn_data.send_data();
	}
	else if ((name == "ai_unit_recruited") || (name == "ai_unit_moved")
		|| (name == "ai_enemy_attacked")){
		turn_data.sync_network();
	}
	else if (name == "network_replay_error"){
		process_oos(replay::last_replay_error);
	}
	else if (name == "host_transfer"){
		is_host_ = true;
		if (linger_){
			gui::button* btn_end = gui_->find_button("button-endturn");
			btn_end->enable(true);
			gui_->invalidate_theme();
		}
	}
}

bool playmp_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int index) const
{
	bool res = true;
	switch (command){
		case hotkey::HOTKEY_CLEAR_LABELS:
			res = !is_observer();
		case hotkey::HOTKEY_SPEAK:
		case hotkey::HOTKEY_SPEAK_ALLY:
		case hotkey::HOTKEY_SPEAK_ALL:
			res = res && network::nconnections() > 0;
			break;
	    default:
			return playsingle_controller::can_execute_command(command, index);
	}
	return res;
}
