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

#include "playmp_controller.hpp"

#include "dialogs.hpp"

#include "actions/undo.hpp"
#include "display_chat_manager.hpp"
#include "game_end_exceptions.hpp"
#include "gettext.hpp"
#include "hotkey_handler_mp.hpp"
#include "log.hpp"
#include "mp_ui_alerts.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "formula_string_utils.hpp"
#include "unit_animation.hpp"
#include "whiteboard/manager.hpp"
#include "countdown_clock.hpp"
#include "synced_context.hpp"
#include "replay_helper.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

unsigned int playmp_controller::replay_last_turn_ = 0;

playmp_controller::playmp_controller(const config& level,
		saved_game& state_of_game, const int ticks, const config& game_config, 
		const tdata_cache & tdata, CVideo& video,
		bool skip_replay, bool blindfold_replay_, bool is_host)
	: playsingle_controller(level, state_of_game, ticks,
		game_config, tdata, video, skip_replay || blindfold_replay_) //this || means that if blindfold is enabled, quick replays will be on.
	, network_processing_stopped_(false)
	, blindfold_(*gui_,blindfold_replay_)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the mp (network enabled) version

	turn_data_.set_host(is_host);
	turn_data_.host_transfer().attach_handler(this);
	// We stop quick replay if play isn't yet past turn 1
	if ( replay_last_turn_ <= 1)
	{
		skip_replay_ = false;
	}
	if (blindfold_replay_) {
		LOG_NG << "Putting on the blindfold now " << std::endl;
	}
}

playmp_controller::~playmp_controller() {
	//halt and cancel the countdown timer
	try {
		turn_data_.host_transfer().detach_handler(this);
	} catch (...) {}
}

void playmp_controller::set_replay_last_turn(unsigned int turn){
	 replay_last_turn_ = turn;
}

void playmp_controller::start_network(){
	network_processing_stopped_ = false;
	LOG_NG << "network processing activated again";
}

void playmp_controller::stop_network(){
	network_processing_stopped_ = true;
	LOG_NG << "network processing stopped";
}

void playmp_controller::play_side()
{
	mp_ui_alerts::turn_changed(current_team().current_player());
	// Proceed with the parent function.
	return playsingle_controller::play_side();
}

void playmp_controller::on_not_observer() {
	remove_blindfold();
}

void playmp_controller::remove_blindfold() {
	if (gui_->is_blindfolded()) {
		blindfold_.unblind();
		LOG_NG << "Taking off the blindfold now " << std::endl;
		gui_->redraw_everything();
	}
}

void playmp_controller::play_linger_turn()
{
	if (is_host()) {
		end_turn_enable(true);
	}
	
	while(end_turn_ == END_TURN_NONE) {
		config cfg;
		if(network_reader_.read(cfg)) {
			if(turn_data_.process_network_data(cfg) == turn_info::PROCESS_END_LINGER)
			{
				end_turn();
			}
		}
		play_slice();
		gui_->draw();
	}
}

void playmp_controller::play_human_turn()
{
	LOG_NG << "playmp::play_human_turn...\n";
	assert(!linger_);
	remove_blindfold();
	boost::scoped_ptr<countdown_clock> timer;
	if(saved_game_.mp_settings().mp_countdown) {
		timer.reset(new countdown_clock(current_team()));
	}
	show_turn_dialog();
	if(undo_stack_->can_undo()) {
		// If we reload a networked mp game we cannot undo moved made before the save
		// Becasue other players already received them
		synced_context::run_and_store("update_shroud", replay_helper::get_update_shroud());
		undo_stack_->clear();
	}
	if (!preferences::disable_auto_moves()) {
		execute_gotos();
	}

	end_turn_enable(true);
	while(!should_return_to_play_side()) {
		try {
			process_network_data();
			if (player_type_changed_)
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

			}

			play_slice_catch();
			if(timer)
			{
				SDL_Delay(1);
				bool time_left = timer->update();
				if(!time_left)
				{
					end_turn_ = END_TURN_REQUIRED;
				}
			}
		}
		catch(...)
		{
			turn_data_.send_data();
			throw;
		}
		turn_data_.send_data();

		gui_->draw();
	}
}

void playmp_controller::play_idle_loop()
{
	LOG_NG << "playmp::play_human_turn...\n";

	remove_blindfold();

	while (!should_return_to_play_side())
	{
		try
		{
			process_network_data();
			play_slice_catch();
			SDL_Delay(1);
			gui_->draw();
		}
		catch(...)
		{
			turn_data_.send_data();
			throw;
		}
		turn_data_.send_data();
	}
}

void playmp_controller::set_end_scenario_button()
{
	// Modify the end-turn button
	if (! is_host()) {
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
	linger_ = true;
	// If we need to set the status depending on the completion state
	// we're needed here.
	gui_->set_game_mode(game_display::LINGER_MP);
	// End all unit moves
	gamestate_.board_.set_all_units_user_end_turn();

	set_end_scenario_button();
	assert(is_regular_game_end());
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
			turn_data_.send_data();
			end_turn_ = END_TURN_NONE;
			play_linger_turn();
			after_human_turn();
			LOG_NG << "finished human turn" << std::endl;
		} catch (game::load_game_exception&) {
			LOG_NG << "caught load-game-exception" << std::endl;
			// this should not happen, the option to load a game is disabled
			throw;
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
	assert(!is_host());

	config cfg;
	network_reader_.set_source(playturn_network_adapter::get_source_from_config(cfg));
	while(true) {
		try {
			const network::connection res = dialogs::network_receive_dialog(
				*gui_, _("Waiting for next scenario..."), cfg);

			if(res != network::null_connection) {
				if (turn_data_.process_network_data_from_reader() == turn_info::PROCESS_END_LINGER) {
					break;
				}
			}
			else
			{
				throw_quit_game_exception();
			}

		} catch(const quit_game_exception&) {
			network_reader_.set_source(playturn_network_adapter::read_network);
			turn_data_.send_data();
			throw;
		}
	}
	network_reader_.set_source(playturn_network_adapter::read_network);
}

void playmp_controller::after_human_turn(){
	if(saved_game_.mp_settings().mp_countdown)
	{
		//time_left + turn_bonus + (action_bouns * number of actions done)
		const int new_time_in_secs = (current_team().countdown_time() / 1000) 
			+ saved_game_.mp_settings().mp_countdown_turn_bonus
			+ saved_game_.mp_settings().mp_countdown_action_bonus * current_team().action_bonus_count();
		const int new_time = 1000 * std::min<int>(new_time_in_secs, saved_game_.mp_settings().mp_countdown_reservoir_time);

		current_team().set_action_bonus_count(0);
		current_team().set_countdown_time(new_time);
		recorder.add_countdown_update(new_time, player_number_);
	}
	LOG_NG << "playmp::after_human_turn...\n";

	// Normal post-processing for human turns (clear undos, end the turn, etc.)
	playsingle_controller::after_human_turn();
	//send one more time to make sure network is up-to-date.
	turn_data_.send_data();

}

void playmp_controller::finish_side_turn(){
	play_controller::finish_side_turn();
}

void playmp_controller::play_network_turn(){
	LOG_NG << "is networked...\n";

	end_turn_enable(false);
	turn_data_.send_data();

	while(end_turn_ != END_TURN_SYNCED && !is_regular_game_end() && !player_type_changed_)
	{
		if (!network_processing_stopped_) {
			process_network_data();
			if (replay_last_turn_ <= turn()) {
				skip_replay_ = false;
			}
		}

		play_slice_catch();
		if (!network_processing_stopped_){
			turn_data_.send_data();
		}

		gui_->draw();
	}

	LOG_NG << "finished networked...\n";
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
	update_savegame_snapshot();
	savegame::oos_savegame save(saved_game_, *gui_);
	save.save_game_interactive(gui_->video(), temp_buf.str(), gui::YES_NO);
}

void playmp_controller::handle_generic_event(const std::string& name){
	turn_data_.send_data();

	if (name == "ai_user_interact")
	{
		playsingle_controller::handle_generic_event(name);
		turn_data_.send_data();
	}
	else if (name == "ai_gamestate_changed")
	{
		turn_data_.send_data();
	}
	else if (name == "host_transfer"){
		if (linger_){
			end_turn_enable(true);
			gui_->invalidate_theme();
		}
	}
}

void playmp_controller::do_idle_notification()
{
	gui_->get_chat_manager().add_chat_message(time(NULL), "", 0,
		_("This side is in an idle state. To proceed with the game, it must be assigned to another controller. You may use :droid, :control or :give_control for example."),
		events::chat_handler::MESSAGE_PUBLIC, false);
}

void playmp_controller::maybe_linger()
{
	// mouse_handler expects at least one team for linger mode to work.
	assert(is_regular_game_end());
	if (!get_end_level_data_const().transient.linger_mode || gamestate_.board_.teams().empty()) {
		if(!is_host()) {
			// If we continue without lingering we need to
			// make sure the host uploads the next scenario
			// before we attempt to download it.
			wait_for_upload();
		}
	} else {
		linger();
	}
}

void playmp_controller::pull_remote_choice()
{
	turn_info::PROCESS_DATA_RESULT res = turn_data_.sync_network();
	assert(res != turn_info::PROCESS_END_LINGER);
	assert(res != turn_info::PROCESS_END_TURN);
	if(res == turn_info::PROCESS_RESTART_TURN)
	{
		player_type_changed_ = true;
	}
}

void playmp_controller::send_user_choice()
{
	turn_data_.send_data();
}

void playmp_controller::process_network_data()
{
	if(should_return_to_play_side()) {
		return;
	}
	turn_info::PROCESS_DATA_RESULT res = turn_info::PROCESS_CONTINUE;
	config cfg;
	if(!recorder.at_end()) {
		res = turn_info::replay_to_process_data_result(do_replay()); 
	}
	else if(network_reader_.read(cfg)) {
		res = turn_data_.process_network_data(cfg);
	}

	if (res == turn_info::PROCESS_RESTART_TURN) {
		player_type_changed_ = true;
	}
	else if (res == turn_info::PROCESS_END_TURN) {
		end_turn_ = END_TURN_SYNCED;
	}
	else if (res == turn_info::PROCESS_END_LEVEL) {
	}
	else if (res == turn_info::PROCESS_END_LINGER) {
		replay::process_error("Received unexpected next_scenario during the game");
	}
}