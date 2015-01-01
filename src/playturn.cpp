/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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
#include "global.hpp"

#include "actions/undo.hpp"             // for undo_list
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"	// for add_chat_message, add_observer, etc
#include "formula_string_utils.hpp"     // for vgettext
#include "game_board.hpp"               // for game_board
#include "game_display.hpp"             // for game_display
#include "game_end_exceptions.hpp"      // for end_level_exception, etc
#include "gettext.hpp"                  // for _
#include "gui/dialogs/simple_item_selector.hpp"
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "make_enum.hpp"                // for bad_enum_cast
#include "map_label.hpp"
#include "network.hpp"                  // for error
#include "play_controller.hpp"          // for play_controller
#include "playturn_network_adapter.hpp"  // for playturn_network_adapter
#include "preferences.hpp"              // for message_bell
#include "replay.hpp"                   // for replay, recorder, do_replay, etc
#include "resources.hpp"                // for gameboard, screen, etc
#include "serialization/string_utils.hpp"  // for string_map
#include "team.hpp"                     // for team, team::CONTROLLER::AI, etc
#include "tstring.hpp"                  // for operator==
#include "util.hpp"                     // for lexical_cast
#include "whiteboard/manager.hpp"       // for manager
#include "widgets/button.hpp"           // for button

#include <boost/foreach.hpp>            // for auto_any_base, etc
#include <boost/shared_ptr.hpp>         // for shared_ptr
#include <cassert>                      // for assert
#include <cstdlib>                     // for atoi
#include <ctime>                        // for time, NULL
#include <ostream>                      // for operator<<, basic_ostream, etc
#include <vector>                       // for vector

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)

turn_info::turn_info(replay_network_sender &replay_sender,playturn_network_adapter &network_reader) :
	replay_sender_(replay_sender),
	host_transfer_("host_transfer"),
	network_reader_(network_reader),
	is_host_(true)
{
}

turn_info::~turn_info()
{
}

turn_info::PROCESS_DATA_RESULT turn_info::sync_network()
{
	//there should be nothing left on the replay and we should get turn_info::PROCESS_CONTINUE back.
	turn_info::PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	if(network::nconnections() > 0) {

		//receive data first, and then send data. When we sent the end of
		//the AI's turn, we don't want there to be any chance where we
		//could get data back pertaining to the next turn.
		config cfg;
		while( (retv == turn_info::PROCESS_CONTINUE) &&  network_reader_.read(cfg)) {
			retv = process_network_data(cfg,false);
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

turn_info::PROCESS_DATA_RESULT turn_info::handle_turn(
	const config& t,
	const bool skip_replay)
{
	//t can contain a [command] or a [upload_log]
	assert(t.all_children_count() == 1);
	/** @todo FIXME: Check what commands we execute when it's our turn! */

	bool was_skipping = recorder.is_skipping();
	recorder.set_skip(skip_replay);
	//note, that this function might call itself recursively: do_replay -> ... -> persist_var -> ... -> handle_generic_event -> sync_network -> handle_turn
	recorder.add_config(t, replay::MARK_AS_SENT);
	PROCESS_DATA_RESULT retv = replay_to_process_data_result(do_replay());
	recorder.set_skip(was_skipping);
	return retv;
}

void turn_info::do_save()
{
	if (resources::controller != NULL) {
		resources::controller->do_autosave();
	}
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data_from_reader(bool skip_replay)
{
	config cfg;
	while(this->network_reader_.read(cfg))
	{
		PROCESS_DATA_RESULT res = process_network_data(cfg, skip_replay);
		if(res != PROCESS_CONTINUE)
		{
			return res;
		}
	}
	return PROCESS_CONTINUE;
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg, bool skip_replay)
{
	// we cannot be connected to multiple peers anymore because
	// the simple wesnothserver implementation in wesnoth was removed years ago.
	assert(network::nconnections() <= 1);
	assert(cfg.all_children_count() == 1);
	assert(cfg.attribute_range().first == cfg.attribute_range().second);
	if(!recorder.at_end())
	{
		ERR_NW << "processing network data while still having data on the replay." << std::endl;
	}

	if (const config &msg = cfg.child("message"))
	{
		resources::screen->get_chat_manager().add_chat_message(time(NULL), msg["sender"], msg["side"],
				msg["message"], events::chat_handler::MESSAGE_PUBLIC,
				preferences::message_bell());
	}
	else if (const config &msg = cfg.child("whisper") /*&& is_observer()*/)
	{
		resources::screen->get_chat_manager().add_chat_message(time(NULL), "whisper: " + msg["sender"].str(), 0,
				msg["message"], events::chat_handler::MESSAGE_PRIVATE,
				preferences::message_bell());
	}
	else if (const config &ob = cfg.child("observer") )
	{
		resources::screen->get_chat_manager().add_observer(ob["name"]);
	}
	else if (const config &ob = cfg.child("observer_quit"))
	{
		resources::screen->get_chat_manager().remove_observer(ob["name"]);
	}
	else if (cfg.child("leave_game")) {
		throw network::error("");
	}
	else if (const config &turn = cfg.child("turn"))
	{
		return handle_turn(turn, skip_replay);
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
		//don't use lexical_cast_default it's "safer" to end on error
		const int side = lexical_cast<int>(change["side"]);
		const size_t index = side - 1;

		if(index >= resources::gameboard->teams().size()) {
			ERR_NW << "Bad [change_controller] signal from server, side out of bounds: " << change.debug() << std::endl;
			return PROCESS_CONTINUE;
		}

		assert(resources::gameboard);
		const team & tm = resources::gameboard->teams().at(index);
		const std::string &player = change["player"];
		const bool was_local = tm.is_local();

		team::CONTROLLER new_controller = team::CONTROLLER();
		try {
			new_controller = team::string_to_CONTROLLER (change["controller"].str());
		} catch (bad_enum_cast & e) {
			ERR_NW << "Bad [change_controller] message from server:\n" << e.what() << std::endl << change.debug() << std::endl;
			return PROCESS_CONTINUE;
		}

		resources::gameboard->side_change_controller(side, new_controller, player);

		if (!was_local && tm.is_local()) {
			resources::controller->on_not_observer();
		}

		if (resources::gameboard->is_observer() || (resources::gameboard->teams())[resources::screen->playing_team()].is_local_human()) {
			resources::screen->set_team(resources::screen->playing_team());
			resources::screen->redraw_everything();
			resources::screen->recalculate_minimap();
		} else if (tm.is_local_human()) {
			resources::screen->set_team(side - 1);
			resources::screen->redraw_everything();
			resources::screen->recalculate_minimap();
		}

		resources::controller->maybe_do_init_side();

		resources::whiteboard->on_change_controller(side,tm);

		resources::screen->labels().recalculate_labels();

		const bool restart = resources::screen->playing_side() == side && (was_local || tm.is_local());
		return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
	}

	else if (const config &side_drop_c = cfg.child("side_drop"))
	{
		const int  side_drop = side_drop_c["side_num"].to_int(0);
		const std::string controller = side_drop_c["controller"];
		size_t index = side_drop -1;

		bool restart = side_drop == resources::screen->playing_side();

		if (index >= resources::teams->size()) {
			ERR_NW << "unknown side " << side_drop << " is dropping game" << std::endl;
			throw network::error("");
		}

		team::CONTROLLER ctrl = team::CONTROLLER();
		try {
			ctrl = team::string_to_CONTROLLER(controller);
		} catch (bad_enum_cast & e) {
			ERR_NW << "unknown controller type issued from server on side drop: " << e.what() << std::endl;
			throw network::error("");
		}

		if (ctrl == team::AI){
			resources::gameboard->side_drop_to(side_drop, ctrl);
			return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
		}

		int action = 0;
		int first_observer_option_idx = 0;

		std::vector<std::string> observers;
		std::vector<const team *> allies;
		std::vector<std::string> options;

		// We want to give host chance to decide what to do for side
		{
			utils::string_map t_vars;
			options.push_back(_("Replace with AI"));
			options.push_back(_("Replace with local player"));
			options.push_back(_("Set side to idle"));
			options.push_back(_("Save and abort game"));

			first_observer_option_idx = options.size();

			//get all observers in as options to transfer control
			BOOST_FOREACH(const std::string &ob, resources::screen->observers())
			{
				t_vars["player"] = ob;
				options.push_back(vgettext("Replace with $player", t_vars));
				observers.push_back(ob);
			}

			const team &tm = resources::gameboard->teams()[index];

			//get all allies in as options to transfer control
			BOOST_FOREACH(const team &t, resources::gameboard->teams())
			{
				if (!t.is_enemy(side_drop) && !t.is_local_human() && !t.is_local_ai() && !t.is_network_ai() && !t.is_empty()
					&& t.current_player() != tm.current_player())
				{
					//if this is an ally of the dropping side and it is not us (choose local player
					//if you want that) and not ai or empty and if it is not the dropping side itself,
					//get this team in as well
					t_vars["player"] = t.current_player();
					options.push_back(vgettext("Replace with $player", t_vars));
					allies.push_back(&t);
				}
			}

			t_vars["player"] = tm.current_player();
			const std::string msg =  vgettext("$player has left the game. What do you want to do?", t_vars);
			gui2::tsimple_item_selector dlg("", msg, options);
			dlg.set_single_button(true);
			dlg.show(resources::screen->video());
			action = dlg.selected_index();
		}

		//make the player an AI, and redo this turn, in case
		//it was the current player's team who has just changed into
		//an AI.
		switch(action) {
			case 0:
				resources::controller->on_not_observer();
				resources::gameboard->side_drop_to(side_drop, team::HUMAN, team::PROXY_AI);
				change_controller(side_drop, team::CONTROLLER_to_string(team::HUMAN));

				resources::controller->maybe_do_init_side();

				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

			case 1:
				resources::controller->on_not_observer();
				resources::gameboard->side_drop_to(side_drop, team::HUMAN, team::PROXY_HUMAN);
				change_controller(side_drop, team::CONTROLLER_to_string(team::HUMAN));

				resources::controller->maybe_do_init_side();

				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
			case 2:
				resources::gameboard->side_drop_to(side_drop, team::HUMAN, team::PROXY_IDLE);
				change_controller(side_drop, team::CONTROLLER_to_string(team::HUMAN));

				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

			case 3:
				//The user pressed "end game". Don't throw a network error here or he will get
				//thrown back to the title screen.
				do_save();
				throw end_level_exception(QUIT);
			default:
				if (action > 3) {

					{
						// Server thinks this side is ours now so in case of error transferring side we have to make local state to same as what server thinks it is.
						resources::gameboard->side_drop_to(side_drop, team::HUMAN, team::PROXY_IDLE);
					}

					const size_t index = static_cast<size_t>(action - first_observer_option_idx);
					if (index < observers.size()) {
						change_side_controller(side_drop, observers[index]);
					} else if (index < options.size() - 1) {
						size_t i = index - observers.size();
						change_side_controller(side_drop, allies[i]->current_player());
					} else {
						resources::gameboard->side_drop_to(side_drop, team::HUMAN, team::PROXY_AI);
						change_controller(side_drop, team::CONTROLLER_to_string(team::HUMAN));
					}
					return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
				}
				break;
		}
		//Lua code currently catches this exception if this function was called from lua code
		// in that case network::error doesn't end the game.
		// but at least he sees this error message.
		throw network::error("Network Error: A player left and you pressed Escape.");
	}

	// The host has ended linger mode in a campaign -> enable the "End scenario" button
	// and tell we did get the notification.
	else if (cfg.child("notify_next_scenario")) {
		gui::button* btn_end = resources::screen->find_action_button("button-endturn");
		if(btn_end) {
			btn_end->enable(true);
		}
		return PROCESS_END_LINGER;
	}

	//If this client becomes the new host, notify the play_controller object about it
	else if (const config &cfg_host_transfer = cfg.child("host_transfer")){
		if (cfg_host_transfer["value"] == "1") {
			is_host_ = true;
			host_transfer_.notify_observers();
		}
	}
	else
	{
		ERR_NW << "found unknown command:\n" << cfg.debug() << std::endl;
	}

	return PROCESS_CONTINUE;
}

void turn_info::change_controller(int side, const std::string& controller)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["controller"] = controller;

	network::send_data(cfg, 0);
}


void turn_info::change_side_controller(int side, const std::string& player)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;
	network::send_data(cfg, 0);
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
	default:
		assert(false);
		throw "found invalid REPLAY_RETURN";
	}
}
