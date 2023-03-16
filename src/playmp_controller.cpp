/*
	Copyright (C) 2006 - 2022
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "countdown_clock.hpp"
#include "display_chat_manager.hpp"
#include "floating_label.hpp"
#include "game_end_exceptions.hpp"
#include "game_initialization/playcampaign.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "hotkey/hotkey_handler_mp.hpp"
#include "log.hpp"
#include "mp_ui_alerts.hpp"
#include "playturn.hpp"
#include "preferences/game.hpp"
#include "preferences/general.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "serialization/string_utils.hpp"
#include "synced_context.hpp"
#include "video.hpp" // only for faked
#include "wesnothd_connection.hpp"
#include "whiteboard/manager.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

playmp_controller::playmp_controller(const config& level, saved_game& state_of_game, mp_game_metadata* mp_info)
	: playsingle_controller(level, state_of_game, mp_info && mp_info->skip_replay)
	, network_processing_stopped_(false)
	, blindfold_(*gui_, mp_info && mp_info->skip_replay_blindfolded)
	, mp_info_(mp_info)
{
	// upgrade hotkey handler to the mp (network enabled) version
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_));

	// turn_data_.set_host(is_host);
	turn_data_.host_transfer().attach_handler(this);
	if(!mp_info || mp_info->current_turn <= turn()) {
		skip_replay_ = false;
	}

	if(gui_->is_blindfolded() && gamestate().first_human_team_ != -1) {
		blindfold_.unblind();
	}
}

playmp_controller::~playmp_controller()
{
	// halt and cancel the countdown timer
	try {
		turn_data_.host_transfer().detach_handler(this);
	} catch(...) {
		DBG_NG << "Caught exception in playmp_controller destructor: " << utils::get_unknown_exception_type();
	}
}

void playmp_controller::start_network()
{
	network_processing_stopped_ = false;
	LOG_NG << "network processing activated again";
}

void playmp_controller::stop_network()
{
	network_processing_stopped_ = true;
	LOG_NG << "network processing stopped";
}

void playmp_controller::on_not_observer()
{
	remove_blindfold();
}

void playmp_controller::remove_blindfold()
{
	if(gui_->is_blindfolded()) {
		blindfold_.unblind();
		LOG_NG << "Taking off the blindfold now";
		gui_->queue_rerender();
	}
}

void playmp_controller::play_linger_turn()
{
	if(replay_controller_.get() != nullptr) {
		// We have probably been using the mp "back to turn" feature
		// We continue play since we have reached the end of the replay.
		replay_controller_.reset();
	}

	while( gamestate().in_phase(game_data::GAME_ENDED) && !end_turn_requested_) {
		config cfg;
		if(network_reader_.read(cfg)) {
			if(turn_data_.process_network_data(cfg) == turn_info::PROCESS_END_LINGER) {
				end_turn();
			}
		}

		play_slice();
	}
}

void playmp_controller::play_human_turn()
{
	LOG_NG << "playmp::play_human_turn...";
	assert(gamestate().in_phase(game_data::TURN_PLAYING));

	mp::ui_alerts::turn_changed(current_team().current_player());

	LOG_NG << "events::commands_disabled=" << events::commands_disabled;

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

	if(!preferences::disable_auto_moves()) {
		execute_gotos();
	}

	end_turn_enable(true);

	while(!should_return_to_play_side()) {
		try {
			process_network_data();
			check_objectives();
			play_slice_catch();
			if(player_type_changed_) {
				// Clean undo stack if turn has to be restarted (losing control)
				if(undo_stack().can_undo()) {
					font::floating_label flabel(_("Undoing moves not yet transmitted to the server."));

					color_t color{255, 255, 255, SDL_ALPHA_OPAQUE};
					flabel.set_color(color);
					SDL_Rect rect = gui_->map_area();
					flabel.set_position(rect.w / 2, rect.h / 2);
					flabel.set_lifetime(2500);
					flabel.set_clip_rect(rect);

					font::add_floating_label(flabel);
				}

				while(undo_stack().can_undo()) {
					undo_stack().undo();
				}
			}

			if(timer) {
				bool time_left = timer->update();
				if(!time_left) {
					end_turn_requested_ = true;
				}
			}
		} catch(...) {
			DBG_NG << "Caught exception while playing a side: " << utils::get_unknown_exception_type();
			turn_data_.send_data();
			throw;
		}

		turn_data_.send_data();
	}
}

void playmp_controller::play_idle_loop()
{
	LOG_NG << "playmp::play_human_turn...";

	remove_blindfold();

	while(!should_return_to_play_side()) {
		try {
			process_network_data();
			play_slice_catch();
			SDL_Delay(1);
		} catch(...) {
			DBG_NG << "Caught exception while playing idle loop: " << utils::get_unknown_exception_type();
			turn_data_.send_data();
			throw;
		}

		turn_data_.send_data();
	}
}

void playmp_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger";

	// End all unit moves
	gamestate().board_.set_all_units_user_end_turn();

	update_gui_linger();

	assert(is_regular_game_end());

	if(get_end_level_data().transient.reveal_map) {
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
			play_linger_turn();
			after_human_turn();
			LOG_NG << "finished human turn";
		} catch(const savegame::load_game_exception&) {
			LOG_NG << "caught load-game-exception";
			// this should not happen, the option to load a game is disabled
			throw;
		} catch(const leavegame_wesnothd_error& e) {
			scoped_savegame_snapshot snapshot(*this);
			savegame::ingame_savegame save(saved_game_, preferences::save_compression_format());

			if(e.message == "") {
				save.save_game_interactive(_("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"),
					savegame::savegame::YES_NO);
			} else {
				save.save_game_interactive(
					_("This game has been ended.\nReason: ") + e.message + _("\nDo you want to save the game?"),
					savegame::savegame::YES_NO);
			}
			throw;
		} catch(const ingame_wesnothd_error&) {
			LOG_NG << "caught network-error-exception";
			quit = false;
		}
	} while(!quit);

	LOG_NG << "ending end-of-scenario linger";
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
			bool res = false;
			gui2::dialogs::loading_screen::display([&]() {
				gui2::dialogs::loading_screen::progress(loading_stage::next_scenario);

				res = mp_info_->connection.wait_and_receive_data(cfg);
			});

			if(res && turn_data_.process_network_data_from_reader() == turn_info::PROCESS_END_LINGER) {
				break;
			} else {
				throw_quit_game_exception();
			}
		} catch(const quit_game_exception&) {
			network_reader_.set_source([this](config& cfg) { return receive_from_wesnothd(cfg); });
			turn_data_.send_data();
			throw;
		}
	}

	network_reader_.set_source([this](config& cfg) { return receive_from_wesnothd(cfg); });
}

void playmp_controller::after_human_turn()
{
	if(saved_game_.mp_settings().mp_countdown) {
		// time_left + turn_bonus + (action_bonus * number of actions done)
		const int new_time_in_secs = (current_team().countdown_time() / 1000)
			+ saved_game_.mp_settings().mp_countdown_turn_bonus
			+ saved_game_.mp_settings().mp_countdown_action_bonus * current_team().action_bonus_count();

		const int new_time
			= 1000 * std::min<int>(new_time_in_secs, saved_game_.mp_settings().mp_countdown_reservoir_time);

		current_team().set_action_bonus_count(0);
		current_team().set_countdown_time(new_time);

		resources::recorder->add_countdown_update(new_time, current_side());
	}

	LOG_NG << "playmp::after_human_turn...";

	// Normal post-processing for human turns (clear undos, end the turn, etc.)
	playsingle_controller::after_human_turn();

	// send one more time to make sure network is up-to-date.
	turn_data_.send_data();
}

void playmp_controller::play_network_turn()
{
	LOG_NG << "is networked...";

	end_turn_enable(false);
	turn_data_.send_data();

	while(!gamestate().in_phase(game_data::TURN_ENDED) && !is_regular_game_end() && !player_type_changed_) {
		if(!network_processing_stopped_) {
			process_network_data();
			if(!mp_info_ || mp_info_->current_turn == turn()) {
				skip_replay_ = false;
			}
		}

		play_slice_catch();
		if(!network_processing_stopped_) {
			turn_data_.send_data();
		}
	}

	LOG_NG << "finished networked...";
}

void playmp_controller::process_oos(const std::string& err_msg) const
{
	// Notify the server of the oos error.
	config cfg;
	config& info = cfg.add_child("info");
	info["type"] = "termination";
	info["condition"] = "out of sync";
	send_to_wesnothd(cfg);

	std::stringstream temp_buf;
	std::vector<std::string> err_lines = utils::split(err_msg, '\n');
	temp_buf << _("The game is out of sync, and cannot continue. There are a number of reasons this could happen: this "
				  "can occur if you or another player have modified their game settings. This may mean one of the "
				  "players is attempting to cheat. It could also be due to a bug in the game, but this is less "
				  "likely.\n\nDo you want to save an error log of your game?");

	if(!err_msg.empty()) {
		temp_buf << " \n \n"; // and now the "Details:"
		for(std::vector<std::string>::iterator i = err_lines.begin(); i != err_lines.end(); ++i) {
			temp_buf << *i << '\n';
		}
		temp_buf << " \n";
	}

	scoped_savegame_snapshot snapshot(*this);
	savegame::oos_savegame save(saved_game_, ignore_replay_errors_);

	save.save_game_interactive(temp_buf.str(), savegame::savegame::YES_NO);
}

void playmp_controller::handle_generic_event(const std::string& name)
{
	turn_data_.send_data();

	if(name == "ai_user_interact") {
		playsingle_controller::handle_generic_event(name);
		turn_data_.send_data();
	} else if(name == "ai_gamestate_changed") {
		turn_data_.send_data();
	} else if(name == "host_transfer") {
		assert(mp_info_);
		mp_info_->is_host = true;
		if(is_linger_mode()) {
			end_turn_enable(true);
		}
	}
}
bool playmp_controller::is_host() const
{
	return !mp_info_ || mp_info_->is_host;
}

void playmp_controller::do_idle_notification()
{
	gui_->get_chat_manager().add_chat_message(std::time(nullptr), "", 0,
		_("This side is in an idle state. To proceed with the game, it must be assigned to another controller. You may "
		  "use :droid, :control or :give_control for example."),
		events::chat_handler::MESSAGE_PUBLIC, false);
}

void playmp_controller::maybe_linger()
{
	// mouse_handler expects at least one team for linger mode to work.
	assert(is_regular_game_end());
	if(!get_end_level_data().transient.linger_mode || get_teams().empty() || video::headless()) {
		const bool has_next_scenario
			= !gamestate().gamedata_.next_scenario().empty() && gamestate().gamedata_.next_scenario() != "null";
		if(!is_host() && has_next_scenario) {
			// If we continue without lingering we need to
			// make sure the host uploads the next scenario
			// before we attempt to download it.
			wait_for_upload();
		}
	} else {
		linger();
	}
	end_turn_requested_ = true;
}

void playmp_controller::surrender(int side_number)
{
	undo_stack().clear();
	resources::recorder->add_surrender(side_number);
	turn_data_.send_data();
}

void playmp_controller::pull_remote_choice()
{
	turn_info::PROCESS_DATA_RESULT res = turn_data_.sync_network();
	assert(res != turn_info::PROCESS_END_TURN);

	if(res == turn_info::PROCESS_END_LINGER) {
		// Probably some bad OOS, but there is currently no way to recover from this.
		throw ingame_wesnothd_error("");
	}

	if(res == turn_info::PROCESS_RESTART_TURN) {
		player_type_changed_ = true;
	}
}

void playmp_controller::send_user_choice()
{
	turn_data_.send_data();
}

void playmp_controller::play_slice(bool is_delay_enabled)
{
	if(!is_linger_mode() && !is_replay()) {
		// receive chat during animations and delay
		process_network_data(true);
		// cannot use turn_data_.send_data() here.
		replay_sender_.sync_non_undoable();
	}

	playsingle_controller::play_slice(is_delay_enabled);
}

void playmp_controller::process_network_data(bool chat_only)
{
	if(gamestate().in_phase(game_data::TURN_ENDED)  || is_regular_game_end() || player_type_changed_) {
		return;
	}

	turn_info::PROCESS_DATA_RESULT res = turn_info::PROCESS_CONTINUE;
	config cfg;

	if(!resources::recorder->at_end()) {
		res = turn_info::replay_to_process_data_result(do_replay());
	} else if(network_reader_.read(cfg)) {
		res = turn_data_.process_network_data(cfg, chat_only);
	}

	if(res == turn_info::PROCESS_CANNOT_HANDLE) {
		network_reader_.push_front(std::move(cfg));
	} else if(res == turn_info::PROCESS_RESTART_TURN) {
		player_type_changed_ = true;
	} else if(res == turn_info::PROCESS_END_TURN) {
	} else if(res == turn_info::PROCESS_END_LEVEL) {
	} else if(res == turn_info::PROCESS_END_LINGER) {
		replay::process_error("Received unexpected next_scenario during the game");
	}
}

bool playmp_controller::is_networked_mp() const
{
	return mp_info_ != nullptr;
}

void playmp_controller::send_to_wesnothd(const config& cfg, const std::string&) const
{
	if(mp_info_ != nullptr) {
		mp_info_->connection.send_data(cfg);
	}
}

bool playmp_controller::receive_from_wesnothd(config& cfg) const
{
	if(mp_info_ != nullptr) {
		return mp_info_->connection.receive_data(cfg);
	} else {
		return false;
	}
}
