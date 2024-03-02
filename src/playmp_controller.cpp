/*
	Copyright (C) 2006 - 2024
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
#include "formula/string_utils.hpp"     // for VGETTEXT
#include "game_end_exceptions.hpp"
#include "game_initialization/playcampaign.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "hotkey/hotkey_handler_mp.hpp"
#include "log.hpp"
#include "map/label.hpp"
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
static lg::log_domain log_network("network");

#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)
#define ERR_NW LOG_STREAM(err, log_network)

playmp_controller::playmp_controller(const config& level, saved_game& state_of_game, mp_game_metadata* mp_info)
	: playsingle_controller(level, state_of_game, mp_info && mp_info->skip_replay)
	, network_processing_stopped_(false)
	, blindfold_(*gui_, mp_info && mp_info->skip_replay_blindfolded)
	, replay_sender_(*resources::recorder)
	, network_reader_([this](config& cfg) { return receive_from_wesnothd(cfg); })
	, mp_info_(mp_info)
{
	// upgrade hotkey handler to the mp (network enabled) version
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_));

	if(!mp_info || mp_info->current_turn <= turn()) {
		skip_replay_ = false;
	}

	if(gui_->is_blindfolded() && !is_observer()) {
		blindfold_.unblind();
	}
}

playmp_controller::~playmp_controller()
{
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
	send_actions();
	if(replay_controller_.get() != nullptr) {
		// We have probably been using the mp "back to turn" feature
		// We continue play since we have reached the end of the replay.
		replay_controller_.reset();
	}

	while( gamestate().in_phase(game_data::GAME_ENDED) && !end_turn_requested_) {
		config cfg;
		if(network_reader_.read(cfg)) {
			if(process_network_data_impl(cfg) == turn_info::PROCESS_END_LINGER) {
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
			throw;
		}

		send_actions();
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
			throw;
		}

		send_actions();
	}
}

void playmp_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger";

	// End all unit moves
	gamestate().board_.set_all_units_user_end_turn();

	update_gui_linger();

	assert(is_regular_game_end());

	bool quit;
	do {
		quit = true;
		{
			// reimplement parts of play_side()
			send_actions();
			play_linger_turn();
			send_actions();
			LOG_NG << "finished human turn";
		}
	} while(!quit);

	LOG_NG << "ending end-of-scenario linger";
}

void playmp_controller::wait_for_upload()
{
	send_actions();
	// If the host is here we'll never leave since we wait for the host to
	// upload the next scenario.
	assert(!is_host());
	// TODO: should we handle the case that we become the ho0st because the host disconnectes here?a

	gui2::dialogs::loading_screen::display([&]() {
		gui2::dialogs::loading_screen::progress(loading_stage::next_scenario);
		while(true) {
			auto res = process_network_data_from_reader();
			if(res == turn_info::PROCESS_END_LINGER) {
				return;
			} else if (res != turn_info::PROCESS_CONTINUE) {
				throw quit_game_exception();
			}
			SDL_Delay(10);
			gui2::dialogs::loading_screen::spin();
		}
	});
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
	send_actions();
}

void playmp_controller::play_network_turn()
{
	LOG_NG << "is networked...";

	end_turn_enable(false);
	send_actions();

	while(!gamestate().in_phase(game_data::TURN_ENDED) && !is_regular_game_end() && !player_type_changed_) {
		if(!network_processing_stopped_) {
			process_network_data();
			if(!mp_info_ || mp_info_->current_turn == turn()) {
				skip_replay_ = false;
			}
		}

		play_slice_catch();
		if(!network_processing_stopped_) {
			send_actions();
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
	if(name == "ai_user_interact") {
		playsingle_controller::handle_generic_event(name);
		send_actions();
	} else if(name == "ai_gamestate_changed") {
		send_actions();
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
	send_actions();
}

void playmp_controller::receive_actions()
{
	turn_info::PROCESS_DATA_RESULT res = sync_network();
	assert(res != turn_info::PROCESS_END_TURN);

	if(res == turn_info::PROCESS_END_LINGER) {
		// Probably some bad OOS, but there is currently no way to recover from this.
		throw ingame_wesnothd_error("");
	}

	if(res == turn_info::PROCESS_RESTART_TURN) {
		player_type_changed_ = true;
	}
}


void playmp_controller::play_slice(bool is_delay_enabled)
{
	if(!is_linger_mode() && !is_replay() && !network_processing_stopped_) {
		// receive chat during animations and delay
		process_network_data(true);
		// cannot use send_actions() here.
		// todo: why? The checks in send_actions() should be safe enouth.
		replay_sender_.sync_non_undoable();
	}

	playsingle_controller::play_slice(is_delay_enabled);
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

void playmp_controller::process_network_data(bool chat_only)
{
	if(gamestate().in_phase(game_data::TURN_ENDED)  || is_regular_game_end() || player_type_changed_) {
		return;
	}

	turn_info::PROCESS_DATA_RESULT res = turn_info::PROCESS_CONTINUE;
	config cfg;

	if(!resources::recorder->at_end()) {
		res = replay_to_process_data_result(do_replay());
	} else if(network_reader_.read(cfg)) {
		res = process_network_data_impl(cfg, chat_only);
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

turn_info::PROCESS_DATA_RESULT playmp_controller::process_network_data_impl(const config& cfg, bool chat_only)
{
	// the simple wesnothserver implementation in wesnoth was removed years ago.
	assert(cfg.all_children_count() == 1);
	assert(cfg.attribute_range().empty());
	if(!resources::recorder->at_end())
	{
		ERR_NW << "processing network data while still having data on the replay.";
	}

	if (const auto message = cfg.optional_child("message"))
	{
		game_display::get_singleton()->get_chat_manager().add_chat_message(std::time(nullptr), message.value()["sender"], message.value()["side"],
				message.value()["message"], events::chat_handler::MESSAGE_PUBLIC,
				preferences::message_bell());
	}
	else if (auto whisper = cfg.optional_child("whisper") /*&& is_observer()*/)
	{
		game_display::get_singleton()->get_chat_manager().add_chat_message(std::time(nullptr), "whisper: " + whisper["sender"].str(), 0,
				whisper["message"], events::chat_handler::MESSAGE_PRIVATE,
				preferences::message_bell());
	}
	else if (auto observer = cfg.optional_child("observer") )
	{
		game_display::get_singleton()->get_chat_manager().add_observer(observer["name"]);
	}
	else if (auto observer_quit = cfg.optional_child("observer_quit"))
	{
		game_display::get_singleton()->get_chat_manager().remove_observer(observer_quit["name"]);
	}
	else if (cfg.has_child("leave_game")) {
		const bool has_reason = cfg.mandatory_child("leave_game").has_attribute("reason");
		throw leavegame_wesnothd_error(has_reason ? cfg.mandatory_child("leave_game")["reason"].str() : "");
	}
	else if (auto turn = cfg.optional_child("turn"))
	{
		return process_network_turn_impl(*turn, chat_only);
	}
	else if (cfg.has_child("whiteboard"))
	{
		set_scontext_unsynced scontext;
		resources::whiteboard->process_network_data(cfg);
	}
	else if (auto change = cfg.optional_child("change_controller"))
	{
		return process_network_change_controller_impl(*change);
	}
	else if (auto side_drop_c = cfg.optional_child("side_drop"))
	{
		return process_network_side_drop_impl(*side_drop_c);
	}

	// The host has ended linger mode in a campaign -> enable the "End scenario" button
	// and tell we did get the notification.
	else if (cfg.has_child("notify_next_scenario")) {
		if(chat_only) {
			return turn_info::PROCESS_CANNOT_HANDLE;
		}
		return turn_info::PROCESS_END_LINGER;
	}

	//If this client becomes the new host, notify the play_controller object about it
	else if (cfg.has_child("host_transfer")) {
		mp_info_->is_host = true;
		if(is_linger_mode()) {
			end_turn_enable(true);
		}
	}
	else
	{
		ERR_NW << "found unknown command:\n" << cfg.debug();
	}

	return turn_info::PROCESS_CONTINUE;
}

turn_info::PROCESS_DATA_RESULT playmp_controller::process_network_turn_impl(const config& t, bool chat_only)
{
	//t can contain a [command] or a [upload_log]
	assert(t.all_children_count() == 1);

	if(!t.child_or_empty("command").has_child("speak") && chat_only) {
		return turn_info::PROCESS_CANNOT_HANDLE;
	}
	/** @todo FIXME: Check what commands we execute when it's our turn! */

	//note, that this function might call itself recursively: do_replay -> ... -> get_user_choice -> ... -> playmp_controller::pull_remote_choice -> sync_network -> handle_turn
	resources::recorder->add_config(t, replay::MARK_AS_SENT);
	turn_info::PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	return retv;
}

turn_info::PROCESS_DATA_RESULT playmp_controller::process_network_side_drop_impl(const config& side_drop_c)
{
	// Only the host receives this message when a player leaves/disconnects.
	const int  side_drop = side_drop_c["side_num"].to_int(0);
	std::size_t index = side_drop -1;

	bool restart = side_drop == game_display::get_singleton()->playing_side();

	if (index >= gamestate().board_.teams().size()) {
		ERR_NW << "unknown side " << side_drop << " is dropping game";
		throw ingame_wesnothd_error("");
	}

	auto ctrl = side_controller::get_enum(side_drop_c["controller"].str());
	if(!ctrl) {
		ERR_NW << "unknown controller type issued from server on side drop: " << side_drop_c["controller"];
		throw ingame_wesnothd_error("");
	}

	if (ctrl == side_controller::type::ai) {
		gamestate().board_.side_drop_to(side_drop, *ctrl);
		return restart ? turn_info::PROCESS_RESTART_TURN : turn_info::PROCESS_CONTINUE;
	}
	//null controlled side cannot be dropped because they aren't controlled by anyone.
	else if (ctrl != side_controller::type::human) {
		ERR_NW << "unknown controller type issued from server on side drop: " << side_controller::get_string(*ctrl);
		throw ingame_wesnothd_error("");
	}

	int action = 0;
	int first_observer_option_idx = 0;
	int control_change_options = 0;
	bool has_next_scenario = !resources::gamedata->next_scenario().empty() && resources::gamedata->next_scenario() != "null";

	std::vector<std::string> observers;
	std::vector<const team *> allies;
	std::vector<std::string> options;

	const team &tm = gamestate().board_.teams()[index];

	for (const team &t : gamestate().board_.teams()) {
		if (!t.is_enemy(side_drop) && !t.is_local_human() && !t.is_local_ai() && !t.is_network_ai() && !t.is_empty()
			&& t.current_player() != tm.current_player()) {
			allies.push_back(&t);
		}
	}

	// We want to give host chance to decide what to do for side
	if (!is_linger_mode() || has_next_scenario) {
		utils::string_map t_vars;

		//get all allies in as options to transfer control
		for (const team *t : allies) {
			//if this is an ally of the dropping side and it is not us (choose local player
			//if you want that) and not ai or empty and if it is not the dropping side itself,
			//get this team in as well
			t_vars["player"] = t->current_player();
			options.emplace_back(VGETTEXT("Give control to their ally $player", t_vars));
			control_change_options++;
		}

		first_observer_option_idx = options.size();

		//get all observers in as options to transfer control
		for (const std::string &screen_observers : game_display::get_singleton()->observers()) {
			t_vars["player"] = screen_observers;
			options.emplace_back(VGETTEXT("Give control to observer $player", t_vars));
			observers.push_back(screen_observers);
			control_change_options++;
		}

		options.emplace_back(_("Replace with AI"));
		options.emplace_back(_("Replace with local player"));
		options.emplace_back(_("Set side to idle"));
		options.emplace_back(_("Save and abort game"));

		t_vars["player"] = tm.current_player();
		t_vars["side_drop"] = std::to_string(side_drop);
		const std::string gettext_message =  VGETTEXT("$player who controlled side $side_drop has left the game. What do you want to do?", t_vars);
		gui2::dialogs::simple_item_selector dlg("", gettext_message, options);
		dlg.set_single_button(true);
		dlg.show();
		action = dlg.selected_index();

		// If esc was pressed, default to setting side to idle
		if (action == -1) {
			action = control_change_options + 2;
		}
	} else {
		// Always set leaving side to idle if in linger mode and there is no next scenario
		action = 2;
	}

	if (action < control_change_options) {
		// Grant control to selected ally
		// Server thinks this side is ours now so in case of error transferring side we have to make local state to same as what server thinks it is.
		gamestate().board_.side_drop_to(side_drop, side_controller::type::human, side_proxy_controller::type::idle);

		if (action < first_observer_option_idx) {
			send_change_side_controller(side_drop, allies[action]->current_player());
		} else {
			send_change_side_controller(side_drop, observers[action - first_observer_option_idx]);
		}

		return restart ? turn_info::PROCESS_RESTART_TURN : turn_info::PROCESS_CONTINUE;
	} else {
		action -= control_change_options;

		//make the player an AI, and redo this turn, in case
		//it was the current player's team who has just changed into
		//an AI.
		switch(action) {
			case 0:
				on_not_observer();
				gamestate().board_.side_drop_to(side_drop, side_controller::type::human, side_proxy_controller::type::ai);

				return restart?turn_info::PROCESS_RESTART_TURN:turn_info::PROCESS_CONTINUE;

			case 1:
				on_not_observer();
				gamestate().board_.side_drop_to(side_drop, side_controller::type::human, side_proxy_controller::type::human);

				return restart?turn_info::PROCESS_RESTART_TURN:turn_info::PROCESS_CONTINUE;
			case 2:
				gamestate().board_.side_drop_to(side_drop, side_controller::type::human, side_proxy_controller::type::idle);

				return restart?turn_info::PROCESS_RESTART_TURN:turn_info::PROCESS_CONTINUE;

			case 3:
				//The user pressed "end game". Don't throw a network error here or he will get
				//thrown back to the title screen.
				do_autosave();
				throw_quit_game_exception();
			default:
				break;
		}
	}
	return turn_info::PROCESS_CONTINUE;
}

turn_info::PROCESS_DATA_RESULT playmp_controller::process_network_change_controller_impl(const config& change)
{

	if(change.empty()) {
		ERR_NW << "Bad [change_controller] signal from server, [change_controller] tag was empty.";
		return turn_info::PROCESS_CONTINUE;
	}

	const int side = change["side"].to_int();
	const bool is_local = change["is_local"].to_bool();
	const std::string player = change["player"];
	const std::string controller_type = change["controller"];
	const std::size_t index = side - 1;
	if(index >= gamestate().board_.teams().size()) {
		ERR_NW << "Bad [change_controller] signal from server, side out of bounds: " << change.debug();
		return turn_info::PROCESS_CONTINUE;
	}

	const team & tm = gamestate().board_.teams().at(index);
	const bool was_local = tm.is_local();

	gamestate().board_.side_change_controller(side, is_local, player, controller_type);

	if (!was_local && tm.is_local()) {
		on_not_observer();
	}

	// TODO: can we replace this with just a call to play_controller::update_viewing_player() ?
	auto disp_set_team = [](int side_index) {
		const bool side_changed = static_cast<int>(display::get_singleton()->viewing_team()) != side_index;
		display::get_singleton()->set_team(side_index);

		if(side_changed) {
			display::get_singleton()->queue_rerender();
		}
	};

	if (gamestate().board_.is_observer() || (gamestate().board_.teams())[display::get_singleton()->playing_team()].is_local_human()) {
		disp_set_team(display::get_singleton()->playing_team());
	} else if (tm.is_local_human()) {
		disp_set_team(side - 1);
	}

	resources::whiteboard->on_change_controller(side,tm);

	display::get_singleton()->labels().recalculate_labels();

	const bool restart = game_display::get_singleton()->playing_side() == side && (was_local || tm.is_local());
	return restart ? turn_info::PROCESS_RESTART_TURN : turn_info::PROCESS_CONTINUE;
}

turn_info::PROCESS_DATA_RESULT playmp_controller::sync_network()
{
	//there should be nothing left on the replay and we should get turn_info::PROCESS_CONTINUE back.
	turn_info::PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	if(is_networked_mp()) {

		//receive data first, and then send data. When we sent the end of
		//the AI's turn, we don't want there to be any chance where we
		//could get data back pertaining to the next turn.
		config cfg;
		while( (retv == turn_info::PROCESS_CONTINUE) &&  network_reader_.read(cfg)) {
			retv = process_network_data_impl(cfg);
			cfg.clear();
		}
		send_actions();
	}
	return retv;
}

void playmp_controller::send_actions()
{
	const bool send_everything = synced_context::is_unsynced() ? !resources::undo_stack->can_undo() : synced_context::undo_blocked();
	if ( !send_everything ) {
		replay_sender_.sync_non_undoable();
	} else {
		replay_sender_.commit_and_sync();
	}
}

turn_info::PROCESS_DATA_RESULT playmp_controller::process_network_data_from_reader()
{
	config cfg;
	while(this->network_reader_.read(cfg))
	{
		turn_info::PROCESS_DATA_RESULT res = process_network_data_impl(cfg);
		if(res != turn_info::PROCESS_CONTINUE)
		{
			return res;
		}
		cfg.clear();
	}
	return turn_info::PROCESS_CONTINUE;
}



void playmp_controller::send_change_side_controller(int side, const std::string& player)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;
	send_to_wesnothd(cfg);
}

turn_info::PROCESS_DATA_RESULT playmp_controller::replay_to_process_data_result(REPLAY_RETURN replayreturn)
{
	switch(replayreturn)
	{
	case REPLAY_RETURN_AT_END:
		return turn_info::PROCESS_CONTINUE;
	case REPLAY_FOUND_DEPENDENT:
		return turn_info::PROCESS_FOUND_DEPENDENT;
	case REPLAY_FOUND_END_TURN:
		return turn_info::PROCESS_END_TURN;
	case REPLAY_FOUND_END_LEVEL:
		return turn_info::PROCESS_END_LEVEL;
	default:
		assert(false);
		throw "found invalid REPLAY_RETURN";
	}
}
