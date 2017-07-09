/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playturn.hpp"

#include "actions/undo.hpp"             // for undo_list
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"	// for add_chat_message, add_observer, etc
#include "formula/string_utils.hpp"     // for vgettext
#include "game_board.hpp"               // for game_board
#include "game_display.hpp"             // for game_display
#include "game_end_exceptions.hpp"      // for end_level_exception, etc
#include "gettext.hpp"                  // for _
#include "gui/dialogs/simple_item_selector.hpp"
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "utils/make_enum.hpp"                // for bad_enum_cast
#include "map/label.hpp"
#include "play_controller.hpp"          // for play_controller
#include "playturn_network_adapter.hpp"  // for playturn_network_adapter
#include "preferences/general.hpp"              // for message_bell
#include "replay.hpp"                   // for replay, recorder, do_replay, etc
#include "resources.hpp"                // for gameboard, screen, etc
#include "serialization/string_utils.hpp"  // for string_map
#include "team.hpp"                     // for team, team::CONTROLLER::AI, etc
#include "tstring.hpp"                  // for operator==
#include "wesnothd_connection_error.hpp"
#include "whiteboard/manager.hpp"       // for manager
#include "widgets/button.hpp"           // for button

#include <cassert>                      // for assert
#include <ctime>                        // for time
#include <ostream>                      // for operator<<, basic_ostream, etc
#include <vector>                       // for vector

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)

turn_info::turn_info(replay_network_sender &replay_sender,playturn_network_adapter &network_reader) :
	replay_sender_(replay_sender),
	host_transfer_("host_transfer"),
	network_reader_(network_reader)
{
}

turn_info::~turn_info()
{
}

turn_info::PROCESS_DATA_RESULT turn_info::sync_network()
{
	//there should be nothing left on the replay and we should get turn_info::PROCESS_CONTINUE back.
	turn_info::PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	if(resources::controller->is_networked_mp()) {

		//receive data first, and then send data. When we sent the end of
		//the AI's turn, we don't want there to be any chance where we
		//could get data back pertaining to the next turn.
		config cfg;
		while( (retv == turn_info::PROCESS_CONTINUE) &&  network_reader_.read(cfg)) {
			retv = process_network_data(cfg);
			cfg.clear();
		}
		send_data();
	}
	return retv;
}

void turn_info::send_data()
{
	if ( resources::undo_stack->can_undo() ) {
		replay_sender_.sync_non_undoable();
	} else {
		replay_sender_.commit_and_sync();
	}
}

turn_info::PROCESS_DATA_RESULT turn_info::handle_turn(const config& t)
{
	//t can contain a [command] or a [upload_log]
	assert(t.all_children_count() == 1);
	/** @todo FIXME: Check what commands we execute when it's our turn! */

	//note, that this function might call itself recursively: do_replay -> ... -> get_user_choice -> ... -> playmp_controller::pull_remote_choice -> sync_network -> handle_turn
	resources::recorder->add_config(t, replay::MARK_AS_SENT);
	PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	return retv;
}

void turn_info::do_save()
{
	if (resources::controller != nullptr) {
		resources::controller->do_autosave();
	}
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data_from_reader()
{
	config cfg;
	while(this->network_reader_.read(cfg))
	{
		PROCESS_DATA_RESULT res = process_network_data(cfg);
		if(res != PROCESS_CONTINUE)
		{
			return res;
		}
		cfg.clear();
	}
	return PROCESS_CONTINUE;
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg)
{
	// the simple wesnothserver implementation in wesnoth was removed years ago.
	assert(cfg.all_children_count() == 1);
	assert(cfg.attribute_range().empty());
	if(!resources::recorder->at_end())
	{
		ERR_NW << "processing network data while still having data on the replay." << std::endl;
	}

	if (const config &message = cfg.child("message"))
	{
		resources::screen->get_chat_manager().add_chat_message(time(nullptr), message["sender"], message["side"],
				message["message"], events::chat_handler::MESSAGE_PUBLIC,
				preferences::message_bell());
	}
	else if (const config &whisper = cfg.child("whisper") /*&& is_observer()*/)
	{
		resources::screen->get_chat_manager().add_chat_message(time(nullptr), "whisper: " + whisper["sender"].str(), 0,
				whisper["message"], events::chat_handler::MESSAGE_PRIVATE,
				preferences::message_bell());
	}
	else if (const config &observer = cfg.child("observer") )
	{
		resources::screen->get_chat_manager().add_observer(observer["name"]);
	}
	else if (const config &observer_quit = cfg.child("observer_quit"))
	{
		resources::screen->get_chat_manager().remove_observer(observer_quit["name"]);
	}
	else if (cfg.child("leave_game")) {
		throw ingame_wesnothd_error("");
	}
	else if (const config &turn = cfg.child("turn"))
	{
		return handle_turn(turn);
	}
	else if (cfg.has_child("whiteboard"))
	{
		resources::whiteboard->process_network_data(cfg);
	}
	else if (const config &change = cfg.child("change_controller"))
	{
		if(change.empty()) {
			ERR_NW << "Bad [change_controller] signal from server, [change_controller] tag was empty." << std::endl;
			return PROCESS_CONTINUE;
		}

		const int side = change["side"].to_int();
		const bool is_local = change["is_local"].to_bool();
		const std::string player = change["player"];
		const size_t index = side - 1;
		if(index >= resources::gameboard->teams().size()) {
			ERR_NW << "Bad [change_controller] signal from server, side out of bounds: " << change.debug() << std::endl;
			return PROCESS_CONTINUE;
		}

		const team & tm = resources::gameboard->teams().at(index);
		const bool was_local = tm.is_local();

		resources::gameboard->side_change_controller(side, is_local, player);

		if (!was_local && tm.is_local()) {
			resources::controller->on_not_observer();
		}

		if (resources::gameboard->is_observer() || (resources::gameboard->teams())[resources::screen->playing_team()].is_local_human()) {
			resources::screen->set_team(resources::screen->playing_team());
			resources::screen->recalculate_minimap();
		} else if (tm.is_local_human()) {
			resources::screen->set_team(side - 1);
			resources::screen->recalculate_minimap();
		}

		resources::whiteboard->on_change_controller(side,tm);

		resources::screen->labels().recalculate_labels();

		const bool restart = resources::screen->playing_side() == side && (was_local || tm.is_local());
		return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
	}

	else if (const config &side_drop_c = cfg.child("side_drop"))
	{
		const int  side_drop = side_drop_c["side_num"].to_int(0);
		size_t index = side_drop -1;

		bool restart = side_drop == resources::screen->playing_side();

		if (index >= resources::gameboard->teams().size()) {
			ERR_NW << "unknown side " << side_drop << " is dropping game" << std::endl;
			throw ingame_wesnothd_error("");
		}

		team::CONTROLLER ctrl;
		if(!ctrl.parse(side_drop_c["controller"])) {
			ERR_NW << "unknown controller type issued from server on side drop: " << side_drop_c["controller"] << std::endl;
			throw ingame_wesnothd_error("");
		}

		if (ctrl == team::CONTROLLER::AI) {
			resources::gameboard->side_drop_to(side_drop, ctrl);
			return restart ? PROCESS_RESTART_TURN:PROCESS_CONTINUE;
		}
		//null controlled side cannot be dropped becasue they aren't controlled by anyone.
		else if (ctrl != team::CONTROLLER::HUMAN) {
			ERR_NW << "unknown controller type issued from server on side drop: " << ctrl.to_cstring() << std::endl;
			throw ingame_wesnothd_error("");
		}

		int action = 0;
		int first_observer_option_idx = 0;
		int control_change_options = 0;
		bool has_next_scenario = !resources::gamedata->next_scenario().empty() && resources::gamedata->next_scenario() != "null";

		std::vector<std::string> observers;
		std::vector<const team *> allies;
		std::vector<std::string> options;

		const team &tm = resources::gameboard->teams()[index];

		for (const team &t : resources::gameboard->teams()) {
			if (!t.is_enemy(side_drop) && !t.is_local_human() && !t.is_local_ai() && !t.is_network_ai() && !t.is_empty()
				&& t.current_player() != tm.current_player()) {
				allies.push_back(&t);
			}
		}

		// We want to give host chance to decide what to do for side
		if (!resources::controller->is_linger_mode() || has_next_scenario) {
			utils::string_map t_vars;

			//get all allies in as options to transfer control
			for (const team *t : allies) {
				//if this is an ally of the dropping side and it is not us (choose local player
				//if you want that) and not ai or empty and if it is not the dropping side itself,
				//get this team in as well
				t_vars["player"] = t->current_player();
				options.emplace_back(vgettext("Give control to their ally $player", t_vars));
				control_change_options++;
			}

			first_observer_option_idx = options.size();

			//get all observers in as options to transfer control
			for (const std::string &screen_observers : resources::screen->observers()) {
				t_vars["player"] = screen_observers;
				options.emplace_back(vgettext("Give control to observer $player", t_vars));
				observers.push_back(screen_observers);
				control_change_options++;
			}

			options.emplace_back(_("Replace with AI"));
			options.emplace_back(_("Replace with local player"));
			options.emplace_back(_("Set side to idle"));
			options.emplace_back(_("Save and abort game"));

			t_vars["player"] = tm.current_player();
			const std::string gettext_message =  vgettext("$player has left the game. What do you want to do?", t_vars);
			gui2::dialogs::simple_item_selector dlg("", gettext_message, options);
			dlg.set_single_button(true);
			dlg.show(CVideo::get_singleton());
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

			{
				// Server thinks this side is ours now so in case of error transferring side we have to make local state to same as what server thinks it is.
				resources::gameboard->side_drop_to(side_drop, team::CONTROLLER::HUMAN, team::PROXY_CONTROLLER::PROXY_IDLE);
			}

			if (action < first_observer_option_idx) {
				change_side_controller(side_drop, allies[action]->current_player());
			} else {
				change_side_controller(side_drop, observers[action - first_observer_option_idx]);
			}

			return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
		} else {
			action -= control_change_options;

			//make the player an AI, and redo this turn, in case
			//it was the current player's team who has just changed into
			//an AI.
			switch(action) {
				case 0:
					resources::controller->on_not_observer();
					resources::gameboard->side_drop_to(side_drop, team::CONTROLLER::HUMAN, team::PROXY_CONTROLLER::PROXY_AI);

					return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

				case 1:
					resources::controller->on_not_observer();
					resources::gameboard->side_drop_to(side_drop, team::CONTROLLER::HUMAN, team::PROXY_CONTROLLER::PROXY_HUMAN);

					return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
				case 2:
					resources::gameboard->side_drop_to(side_drop, team::CONTROLLER::HUMAN, team::PROXY_CONTROLLER::PROXY_IDLE);

					return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

				case 3:
					//The user pressed "end game". Don't throw a network error here or he will get
					//thrown back to the title screen.
					do_save();
					throw_quit_game_exception();
				default:
					break;
			}
		}
	}

	// The host has ended linger mode in a campaign -> enable the "End scenario" button
	// and tell we did get the notification.
	else if (cfg.child("notify_next_scenario")) {
		std::shared_ptr<gui::button> btn_end = resources::screen->find_action_button("button-endturn");
		if(btn_end) {
			btn_end->enable(true);
		}
		return PROCESS_END_LINGER;
	}

	//If this client becomes the new host, notify the play_controller object about it
	else if (cfg.child("host_transfer")){
		host_transfer_.notify_observers();
	}
	else
	{
		ERR_NW << "found unknown command:\n" << cfg.debug() << std::endl;
	}

	return PROCESS_CONTINUE;
}


void turn_info::change_side_controller(int side, const std::string& player)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;
	resources::controller->send_to_wesnothd(cfg);
}

turn_info::PROCESS_DATA_RESULT turn_info::replay_to_process_data_result(REPLAY_RETURN replayreturn)
{
	switch(replayreturn)
	{
	case REPLAY_RETURN_AT_END:
		return PROCESS_CONTINUE;
	case REPLAY_FOUND_DEPENDENT:
		return PROCESS_FOUND_DEPENDENT;
	case REPLAY_FOUND_END_TURN:
		return PROCESS_END_TURN;
	case REPLAY_FOUND_END_LEVEL:
		return PROCESS_END_LEVEL;
	default:
		assert(false);
		throw "found invalid REPLAY_RETURN";
	}
}
