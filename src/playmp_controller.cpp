/*
   Copyright (C) 2006 - 2013 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "playmp_controller.hpp"

#include "dialogs.hpp"

#include "actions/undo.hpp"
#include "game_end_exceptions.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "formula_string_utils.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

unsigned int playmp_controller::replay_last_turn_ = 0;

playmp_controller::playmp_controller(const config& level,
		game_state& state_of_game, const int ticks,
		const int num_turns, const config& game_config, CVideo& video,
		bool skip_replay, bool is_host) :
	playsingle_controller(level, state_of_game, ticks, num_turns,
		game_config, video, skip_replay),
	turn_data_(NULL),
	beep_warning_time_(0),
	network_processing_stopped_(false)
{
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

void playmp_controller::speak(){
	menu_handler_.speak();
}

void playmp_controller::whisper(){
	menu_handler_.whisper();
}

void playmp_controller::shout(){
	menu_handler_.shout();
}

void playmp_controller::start_network(){
	network_processing_stopped_ = false;
	LOG_NG << "network processing activated again";
}

void playmp_controller::stop_network(){
	network_processing_stopped_ = true;
	LOG_NG << "network processing stopped";
}

void playmp_controller::play_side(const unsigned int side_number, bool save)
{
	utils::string_map player;
	player["name"] = current_team().current_player();
	std::string turn_notification_msg = _("$name has taken control");
	turn_notification_msg = utils::interpolate_variables_into_string(turn_notification_msg, &player);
	gui_->send_notification(_("Turn changed"), turn_notification_msg);

	// Proceed with the parent function.
	playsingle_controller::play_side(side_number, save);
}

void playmp_controller::before_human_turn(bool save){
	LOG_NG << "playmp::before_human_turn...\n";
	playsingle_controller::before_human_turn(save);

	init_turn_data();
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

void playmp_controller::reset_countdown()
{
	if (beep_warning_time_ < 0)
		sound::stop_bell();
	beep_warning_time_ = 0;
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

namespace {
	struct command_disabled_resetter
	{
		command_disabled_resetter() : val_(events::commands_disabled) {}
		~command_disabled_resetter() { events::commands_disabled = val_; }
	private:
		int val_;
	};
}

void playmp_controller::play_human_turn(){
	LOG_NG << "playmp::play_human_turn...\n";
	command_disabled_resetter reset_commands;
	int cur_ticks = SDL_GetTicks();
	show_turn_dialog();
	execute_gotos();

	if ((!linger_) || (is_host_))
		gui_->enable_menu("endturn", true);
	while(!end_turn_) {

		try {
			config cfg;
			const network::connection res = network::receive_data(cfg);
			std::deque<config> backlog;

			if(res != network::null_connection) {
				if (turn_data_->process_network_data(cfg, res, backlog, skip_replay_) == turn_info::PROCESS_RESTART_TURN)
				{
					// Clean undo stack if turn has to be restarted (losing control)
					if ( undo_stack_->can_undo() )
					{
						font::floating_label flabel(_("Undoing moves not yet transmitted to the server."));

						SDL_Color color = {255,255,255,255};
						flabel.set_color(color);
						SDL_Rect rect = gui_->map_area();
						flabel.set_position(rect.w/2, rect.h/2);
						flabel.set_lifetime(150);
						flabel.set_clip_rect(rect);

						font::add_floating_label(flabel);
					}

					while( undo_stack_->can_undo() )
						undo_stack_->undo();
					throw end_turn_exception(gui_->playing_side());
				}
			}

			play_slice();
			check_end_level();
			// give a chance to the whiteboard to continue an execute_all_actions
			resources::whiteboard->continue_execute_all();
		} catch(const end_level_exception&) {
			turn_data_->send_data();
			throw;
		}

		if (!linger_ && (current_team().countdown_time() > 0) && gamestate_.mp_settings().mp_countdown) {
			SDL_Delay(1);
			const int ticks = SDL_GetTicks();
			int new_time = current_team().countdown_time()-std::max<int>(1,(ticks - cur_ticks));
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
				const int action_increment = gamestate_.mp_settings().mp_countdown_action_bonus;
				if ( (gamestate_.mp_settings().mp_countdown_turn_bonus == 0 )
					&& (action_increment == 0 || current_team().action_bonus_count() == 0)) {
					// Not possible to end level in MP with throw end_level_exception(DEFEAT);
					// because remote players only notice network disconnection
					// Current solution end remaining turns automatically
					current_team().set_countdown_time(10);
				} else {
					const int maxtime = gamestate_.mp_settings().mp_countdown_reservoir_time;
					int secs = gamestate_.mp_settings().mp_countdown_turn_bonus;
					secs += action_increment  * current_team().action_bonus_count();
					current_team().set_action_bonus_count(0);
					secs = (secs > maxtime) ? maxtime : secs;
					current_team().set_countdown_time(1000 * secs);
				}
				turn_data_->send_data();

				if (!rand_rng::has_new_seed_callback()) {
					throw end_turn_exception();
				}
			}
		}

		gui_->draw();

		turn_data_->send_data();
	}
}

void playmp_controller::set_end_scenario_button()
{
	// Modify the end-turn button
	if (! is_host_) {
		gui::button* btn_end = gui_->find_action_button("button-endturn");
		btn_end->enable(false);
	}
	gui_->get_theme().refresh_title2("button-endturn", "title2");
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

void playmp_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger\n";
	browse_ = true;
	linger_ = true;
	// If we need to set the status depending on the completion state
	// we're needed here.
	gui_->set_game_mode(game_display::LINGER_MP);

	// this is actually for after linger mode is over -- we don't want to
	// stay stuck in linger state when the *next* scenario is over.
	gamestate_.classification().completion = "running";
	// End all unit moves
	BOOST_FOREACH(unit &u, units_) {
		u.set_user_end_turn(true);
	}
	//current_team().set_countdown_time(0);
	//halt and cancel the countdown timer
	reset_countdown();

	set_end_scenario_button();
	set_button_state(*gui_);

	if ( get_end_level_data_const().transient.reveal_map ) {
		// Change the view of all players and observers
		// to see the whole map regardless of shroud and fog.
		update_gui_to_player(gui_->viewing_team(), true);
	}
	bool quit;
	do {
		quit = true;
		try {
			// reimplement parts of play_side()
			player_number_ = first_player_;
			init_turn_data();

			end_turn_ = false;
			play_human_turn();
			turn_over_ = true;  // We don't want to linger mode to add end_turn to replay
			after_human_turn();
			LOG_NG << "finished human turn" << std::endl;
		} catch (game::load_game_exception&) {
			LOG_NG << "caught load-game-exception" << std::endl;
			// this should not happen, the option to load a game is disabled
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

void playmp_controller::wait_for_upload()
{
	// If the host is here we'll never leave since we wait for the host to
	// upload the next scenario.
	assert(!is_host_);

	const bool set_turn_data = (turn_data_ == NULL);
	if(set_turn_data) {
		init_turn_data();
	}

	while(true) {
		try {
			config cfg;
			const network::connection res = dialogs::network_receive_dialog(
				*gui_, _("Waiting for next scenario..."), cfg);

			std::deque<config> backlog;
			if(res != network::null_connection) {
				if (turn_data_->process_network_data(cfg, res, backlog, skip_replay_)
						== turn_info::PROCESS_END_LINGER) {
					break;
				}
			}

		} catch(const end_level_exception&) {
			turn_data_->send_data();
			throw;
		}
	}

	if(set_turn_data) {
		delete turn_data_;
		turn_data_ = NULL;
	}
}

void playmp_controller::after_human_turn(){
	if ( gamestate_.mp_settings().mp_countdown ){
		const int action_increment = gamestate_.mp_settings().mp_countdown_action_bonus;
		const int maxtime = gamestate_.mp_settings().mp_countdown_reservoir_time;
		int secs = (current_team().countdown_time() / 1000) + gamestate_.mp_settings().mp_countdown_turn_bonus;
		secs += action_increment  * current_team().action_bonus_count();
		current_team().set_action_bonus_count(0);
		secs = (secs > maxtime) ? maxtime : secs;
		current_team().set_countdown_time(1000 * secs);
		recorder.add_countdown_update(current_team().countdown_time(),player_number_);
	}
	LOG_NG << "playmp::after_human_turn...\n";

	//ensure that turn_data_ is constructed before it is used.
	if (turn_data_ == NULL) init_turn_data();

	// Normal post-processing for human turns (clear undos, end the turn, etc.)
	playsingle_controller::after_human_turn();
	//send one more time to make sure network is up-to-date.
	turn_data_->send_data();

	// Release turn_data_.
	turn_data_->host_transfer().detach_handler(this);
	delete turn_data_;
	turn_data_ = NULL;
}

void playmp_controller::finish_side_turn(){
	play_controller::finish_side_turn();

	//just in case due to an exception turn_data_ has not been deleted in after_human_turn
	delete turn_data_;
	turn_data_ = NULL;

	//halt and cancel the countdown timer
	reset_countdown();

	// avoid callback getting called in the wrong turn
	rand_rng::clear_new_seed_callback();
}

void playmp_controller::play_network_turn(){
	LOG_NG << "is networked...\n";

	gui_->enable_menu("endturn", false);
	turn_info turn_data(player_number_, replay_sender_);
	turn_data.host_transfer().attach_handler(this);

	for(;;) {

		if (!network_processing_stopped_){
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
				if (skip_replay_ && replay_last_turn_ <= turn()){
						skip_replay_ = false;
				}
				const turn_info::PROCESS_DATA_RESULT result = turn_data.process_network_data(cfg, from, data_backlog_, skip_replay_);
				if (result == turn_info::PROCESS_RESTART_TURN) {
					player_type_changed_ = true;
					return;
				} else if (result == turn_info::PROCESS_END_TURN) {
					break;
				}
			}
		}

		play_slice();
		check_end_level();

		if (!network_processing_stopped_){
			turn_data.send_data();
		}

		gui_->draw();
	}

	turn_data.host_transfer().detach_handler(this);
	LOG_NG << "finished networked...\n";
	return;
}

void playmp_controller::init_turn_data() {
	turn_data_ = new turn_info(player_number_, replay_sender_);
	turn_data_->host_transfer().attach_handler(this);
}

void playmp_controller::process_oos(const std::string& err_msg) const {
	// Notify the server of the oos error.
	config cfg;
	config& info = cfg.add_child("info");
	info["type"] = "termination";
	info["condition"] = "out of sync";
	network::send_data(cfg, 0);

	std::stringstream temp_buf;
	std::vector<std::string> err_lines = utils::split(err_msg,'\n');
	temp_buf << _("The game is out of sync, and cannot continue. There are a number of reasons this could happen: this can occur if you or another player have modified their game settings. This may mean one of the players is attempting to cheat. It could also be due to a bug in the game, but this is less likely.\n\nDo you want to save an error log of your game?");
	if(!err_msg.empty()) {
		temp_buf << " \n \n"; //and now the "Details:"
		for(std::vector<std::string>::iterator i=err_lines.begin(); i!=err_lines.end(); ++i)
		{
			temp_buf << *i << '\n';
		}
		temp_buf << " \n";
	}

	savegame::oos_savegame save(to_config());
	save.save_game_interactive(resources::screen->video(), temp_buf.str(), gui::YES_NO);
}

void playmp_controller::handle_generic_event(const std::string& name){
	turn_info turn_data(player_number_, replay_sender_);

	if (name == "ai_user_interact"){
		playsingle_controller::handle_generic_event(name);
		turn_data.send_data();
	}
	else if ((name == "ai_gamestate_changed") || (name == "ai_sync_network")){
		turn_data.sync_network();
	}
	else if (name == "host_transfer"){
		is_host_ = true;
		if (linger_){
			gui::button* btn_end = gui_->find_action_button("button-endturn");
			btn_end->enable(true);
			gui_->invalidate_theme();
		}
	}
	if (end_turn_) {
		throw end_turn_exception();
	}
}

bool playmp_controller::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	bool res = true;
	switch (command){
		case hotkey::HOTKEY_SPEAK:
		case hotkey::HOTKEY_SPEAK_ALLY:
		case hotkey::HOTKEY_SPEAK_ALL:
			res = network::nconnections() > 0;
			break;
		case hotkey::HOTKEY_START_NETWORK:
		case hotkey::HOTKEY_STOP_NETWORK:
			res = is_observer();
			break;
		case hotkey::HOTKEY_STOP_REPLAY:
			if (is_observer()){
				network_processing_stopped_ = true;
				LOG_NG << "network processing stopped";
			}
			break;
	    default:
			return playsingle_controller::can_execute_command(cmd, index);
	}
	return res;
}
