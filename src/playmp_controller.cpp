/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "actions/undo.hpp"
#include "display_chat_manager.hpp"
#include "game_end_exceptions.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gettext.hpp"
#include "hotkey/hotkey_handler_mp.hpp"
#include "log.hpp"
#include "mp_ui_alerts.hpp"
#include "playturn.hpp"
#include "preferences/general.hpp"
#include "game_initialization/playcampaign.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "serialization/string_utils.hpp"
#include "whiteboard/manager.hpp"
#include "countdown_clock.hpp"
#include "synced_context.hpp"
#include "replay_helper.hpp"
#include "wesnothd_connection.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

playmp_controller::playmp_controller(const config& level,
		saved_game& state_of_game, const config& game_config,
		const ter_data_cache & tdata,
		mp_campaign_info* mp_info)
	: playsingle_controller(level, state_of_game, game_config, tdata, mp_info && mp_info->skip_replay)
	, network_processing_stopped_(false)
	, blindfold_(*gui_, mp_info && mp_info->skip_replay_blindfolded)
	, mp_info_(mp_info)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the mp (network enabled) version

	//turn_data_.set_host(is_host);
	turn_data_.host_transfer().attach_handler(this);
	if (!mp_info || mp_info->current_turn <= turn()) {
		skip_replay_ = false;
	}

	if (gui_->is_blindfolded() && gamestate().first_human_team_ != -1) {
		blindfold_.unblind();
	}
}

playmp_controller::~playmp_controller() {
	//halt and cancel the countdown timer
	try {
		turn_data_.host_transfer().detach_handler(this);
	} catch (...) {}
}

void playmp_controller::start_network(){
	network_processing_stopped_ = false;
	LOG_NG << "network processing activated again";
}

void playmp_controller::stop_network(){
	network_processing_stopped_ = true;
	LOG_NG << "network processing stopped";
}

void playmp_controller::play_side_impl()
{
	mp_ui_alerts::turn_changed(current_team().current_player());
	// Proceed with the parent function.
	return playsingle_controller::play_side_impl();
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
	}
}

void playmp_controller::play_human_turn()
{
	LOG_NG << "playmp::play_human_turn...\n";
	assert(!linger_);
	assert(gamestate_->init_side_done());
	assert(gamestate().gamedata_.phase() == game_data::PLAY);

	LOG_NG << "events::commands_disabled=" << events::commands_disabled <<"\n";

	remove_blindfold();
	const std::unique_ptr<countdown_clock> timer(saved_game_.mp_settings().mp_countdown
        ? new countdown_clock(current_team())
        : nullptr);
	show_turn_dialog();
	if(undo_stack().can_undo()) {
		// If we reload a networked mp game we cannot undo moves made before the save
		// because other players already received them
		if(!current_team().auto_shroud_updates()) {
			synced_context::run_and_store("update_shroud", replay_helper::get_update_shroud());
		}
		undo_stack().clear();
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
				if ( undo_stack().can_undo() )
				{
					font::floating_label flabel(_("Undoing moves not yet transmitted to the server."));

					color_t color {255,255,255,SDL_ALPHA_OPAQUE};
					flabel.set_color(color);
					SDL_Rect rect = gui_->map_area();
					flabel.set_position(rect.w/2, rect.h/2);
					flabel.set_lifetime(150);
					flabel.set_clip_rect(rect);

					font::add_floating_label(flabel);
				}

				while( undo_stack().can_undo() )
					undo_stack().undo();

			}
			check_objectives();
			play_slice_catch();
			if(timer)
			{
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
		std::shared_ptr<gui::button> btn_end = gui_->find_action_button("button-endturn");
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
	gui_->set_game_mode(game_display::LINGER);
	// End all unit moves
	gamestate().board_.set_all_units_user_end_turn();

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
			turn_data_.send_data();
			end_turn_ = END_TURN_NONE;
			play_linger_turn();
			after_human_turn();
			LOG_NG << "finished human turn" << std::endl;
		} catch (savegame::load_game_exception&) {
			LOG_NG << "caught load-game-exception" << std::endl;
			// this should not happen, the option to load a game is disabled
			throw;
		} catch (ingame_wesnothd_error&) {
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
			const bool res =
				mp_info_->connection.fetch_data_with_loading_screen(cfg, loading_stage::next_scenario);

			if(res) {
				if (turn_data_.process_network_data_from_reader() == turn_info::PROCESS_END_LINGER) {
					break;
				}
			}
			else
			{
				throw_quit_game_exception();
			}

		} catch(const quit_game_exception&) {
			network_reader_.set_source([this](config& cfg) { return receive_from_wesnothd(cfg);});
			turn_data_.send_data();
			throw;
		}
	}
	network_reader_.set_source([this](config& cfg) { return receive_from_wesnothd(cfg);});
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
		resources::recorder->add_countdown_update(new_time, current_side());
	}
	LOG_NG << "playmp::after_human_turn...\n";

	// Normal post-processing for human turns (clear undos, end the turn, etc.)
	playsingle_controller::after_human_turn();
	//send one more time to make sure network is up-to-date.
	turn_data_.send_data();

}

void playmp_controller::play_network_turn(){
	LOG_NG << "is networked...\n";

	end_turn_enable(false);
	turn_data_.send_data();

	while(end_turn_ != END_TURN_SYNCED && !is_regular_game_end() && !player_type_changed_)
	{
		if (!network_processing_stopped_) {
			process_network_data();
			if (!mp_info_ || mp_info_->current_turn == turn()) {
				skip_replay_ = false;
			}
		}

		play_slice_catch();
		if (!network_processing_stopped_){
			turn_data_.send_data();
		}
	}

	LOG_NG << "finished networked...\n";
}


void playmp_controller::process_oos(const std::string& err_msg) const {
	// Notify the server of the oos error.
	config cfg;
	config& info = cfg.add_child("info");
	info["type"] = "termination";
	info["condition"] = "out of sync";
	send_to_wesnothd(cfg);

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
	scoped_savegame_snapshot snapshot(*this);
	savegame::oos_savegame save(saved_game_, ignore_replay_errors_);
	save.save_game_interactive(temp_buf.str(), savegame::savegame::YES_NO);
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
		assert(mp_info_);
		mp_info_->is_host = true;
		if (linger_){
			end_turn_enable(true);
			gui_->invalidate_theme();
		}
	}
}
bool playmp_controller::is_host() const
{
	return !mp_info_ || mp_info_->is_host;
}

void playmp_controller::do_idle_notification()
{
	gui_->get_chat_manager().add_chat_message(time(nullptr), "", 0,
		_("This side is in an idle state. To proceed with the game, it must be assigned to another controller. You may use :droid, :control or :give_control for example."),
		events::chat_handler::MESSAGE_PUBLIC, false);
}

void playmp_controller::maybe_linger()
{
	// mouse_handler expects at least one team for linger mode to work.
	assert(is_regular_game_end());
	if (!get_end_level_data_const().transient.linger_mode || gamestate().board_.teams().empty() || gui_->video().faked()) {
		const bool has_next_scenario = !gamestate().gamedata_.next_scenario().empty() && gamestate().gamedata_.next_scenario() != "null";
		if(!is_host() && has_next_scenario) {
			// If we continue without lingering we need to
			// make sure the host uploads the next scenario
			// before we attempt to download it.
			wait_for_upload();
		}
	} else {
		linger();
	}
}

void playmp_controller::surrender(int side_number) {
	undo_stack().clear();
	resources::recorder->add_surrender(side_number);
	turn_data_.send_data();
}

void playmp_controller::pull_remote_choice()
{
	// when using a remote user choice undoing must be impossible because that network traffic cannot be undone
	// Also turn_data_.sync_network() (which calls turn_data_.send_data()) won't work if the undo stack isn't empty because undoable moves won't be sent
	// Also undo_stack()clear() must be called synced so we cannot do that here.
	assert(!current_team().is_local() || !undo_stack().can_undo());
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
	// when using a remote user choice undoing must be impossible because that network traffic cannot be undone
	// Also turn_data_.send_data() won't work if the undo stack isn't empty because undoable moves won't be sent
	// Also undo_stack()clear() must be called synced so we cannot do that here.
	assert(!undo_stack().can_undo());
	turn_data_.send_data();
}

void playmp_controller::process_network_data()
{
	if(end_turn_ == END_TURN_SYNCED || is_regular_game_end() || player_type_changed_) {
		return;
	}
	turn_info::PROCESS_DATA_RESULT res = turn_info::PROCESS_CONTINUE;
	config cfg;
	if(!resources::recorder->at_end()) {
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
bool playmp_controller::is_networked_mp() const
{
	return mp_info_ != nullptr;
}

void playmp_controller::send_to_wesnothd(const config& cfg, const std::string&) const
{
	if (mp_info_ != nullptr) {
		mp_info_->connection.send_data(cfg);
	}
}
bool playmp_controller::receive_from_wesnothd(config& cfg) const
{
	if (mp_info_ != nullptr) {
		return mp_info_->connection.receive_data(cfg);
	}
	else {
		return false;
	}
}
