/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "actions/undo.hpp"
#include "construct_dialog.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "whiteboard/manager.hpp"
#include "formula_string_utils.hpp"
#include "play_controller.hpp"
#include "savegame.hpp"

#include <boost/foreach.hpp>

#include <cassert>
#include <ctime>

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)

turn_info::turn_info(unsigned team_num, replay_network_sender &replay_sender) :
	team_num_(team_num),
	replay_sender_(replay_sender),
	host_transfer_("host_transfer"),
	replay_()
{
	/**
	 * We do network sync so [init_side] is transferred to network hosts
	 */
	if(network::nconnections() > 0)
		send_data();
}

turn_info::~turn_info(){
}

void turn_info::sync_network()
{
	if(network::nconnections() > 0) {

		//receive data first, and then send data. When we sent the end of
		//the AI's turn, we don't want there to be any chance where we
		//could get data back pertaining to the next turn.
		config cfg;
		while(network::connection res = network::receive_data(cfg)) {
			std::deque<config> backlog;
			process_network_data(cfg,res,backlog,false);
			cfg.clear();
		}

		send_data();
	}
}

void turn_info::send_data()
{
	if ( resources::undo_stack->can_undo() ) {
		replay_sender_.sync_non_undoable();
	} else {
		replay_sender_.commit_and_sync();
	}
}

void turn_info::handle_turn(
	bool& turn_end,
	const config& t,
	const bool skip_replay,
	std::deque<config>& backlog)
{
	if(turn_end == false) {
		/** @todo FIXME: Check what commands we execute when it's our turn! */
		//TODO: can we remove replay_ ? i think yes.
		replay_.append(t);
		replay_.set_skip(skip_replay);

		bool was_skipping = recorder.is_skipping();
		recorder.set_skip(skip_replay);
		//turn_end = do_replay(team_num_, &replay_);
		//note, that this cunfion might call itself recursively: do_replay -> ... -> persist_var -> ... -> handle_generic_event -> sync_network -> handle_turn
		recorder.add_config(t, replay::MARK_AS_SENT);
		turn_end = do_replay(team_num_);
		recorder.set_skip(was_skipping);
		
	} else {

		//this turn has finished, so push the remaining moves
		//into the backlog
		backlog.push_back(config());
		backlog.back().add_child("turn", t);
	}
}

void turn_info::do_save()
{
	if ((resources::state_of_game != NULL) && (resources::screen != NULL) && (resources::controller != NULL)) {
		savegame::autosave_savegame save(*resources::state_of_game, *resources::screen, resources::controller->to_config(), preferences::save_compression_format());
		save.autosave(false, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
	}
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg,
		network::connection /*from*/, std::deque<config>& backlog, bool skip_replay)
{
	// we cannot be connected to multiple peers anymore because 
	// the simple wesnothserver implementation in wesnoth was removed years ago. 
	assert(network::nconnections() <= 1);
	
	if (const config &msg = cfg.child("message"))
	{
		resources::screen->add_chat_message(time(NULL), msg["sender"], msg["side"],
				msg["message"], events::chat_handler::MESSAGE_PUBLIC,
				preferences::message_bell());
	}

	if (const config &msg = cfg.child("whisper") /*&& is_observer()*/)
	{
		resources::screen->add_chat_message(time(NULL), "whisper: " + msg["sender"].str(), 0,
				msg["message"], events::chat_handler::MESSAGE_PRIVATE,
				preferences::message_bell());
	}

	BOOST_FOREACH(const config &ob, cfg.child_range("observer")) {
		resources::screen->add_observer(ob["name"]);
	}

	BOOST_FOREACH(const config &ob, cfg.child_range("observer_quit")) {
		resources::screen->remove_observer(ob["name"]);
	}

	if (cfg.child("leave_game")) {
		throw network::error("");
	}

	bool turn_end = false;

	config::const_child_itors turns = cfg.child_range("turn");

	const config& change = cfg.child_or_empty("change_controller");
	const std::string& side_drop = cfg["side_drop"].str();

	BOOST_FOREACH(const config &t, turns)
	{
		recorder.add_config(t, replay::MARK_AS_SENT);
	}
	handle_turn(turn_end, config(), skip_replay, backlog);

	resources::whiteboard->process_network_data(cfg);

	if (!change.empty())
	{
		//don't use lexical_cast_default it's "safer" to end on error
		const int side = lexical_cast<int>(change["side"]);
		const size_t index = static_cast<size_t>(side-1);

		const std::string &controller = change["controller"];
		const std::string &player = change["player"];

		if(index < resources::teams->size()) {
			team &tm = (*resources::teams)[index];
			if (!player.empty())
				tm.set_current_player(player);
			unit_map::iterator leader = resources::units->find_leader(side);
			bool restart = resources::screen->playing_side() == side;
			if (!player.empty() && leader.valid())
				leader->rename(player);

			if (controller == "human" && !tm.is_human()) {
				tm.make_human();
				resources::controller->on_not_observer();
			} else if (controller == "network" && !tm.is_network_human()) {
				tm.make_network();
			} else if (controller == "network_ai" && !tm.is_network_ai()) {
				tm.make_network_ai();
			} else if (controller == "ai" && !tm.is_ai()) {
				tm.make_ai();
				resources::controller->on_not_observer();
			} else if (controller == "idle" && !tm.is_idle()) {
				tm.make_idle();
			}
			else
			{
				restart = false;
			}

			if (is_observer() || (*resources::teams)[resources::screen->playing_team()].is_human()) {
				resources::screen->set_team(resources::screen->playing_team());
				resources::screen->redraw_everything();
				resources::screen->recalculate_minimap();
			} else if (controller == "human") {
				resources::screen->set_team(index);
				resources::screen->redraw_everything();
				resources::screen->recalculate_minimap();
			}

			resources::controller->maybe_do_init_side(index);

			resources::whiteboard->on_change_controller(side,tm);

			resources::screen->labels().recalculate_labels();

			return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
		}
	}

	//if a side has dropped out of the game.
	if(!side_drop.empty()) {
		const std::string controller = cfg["controller"];
		int side = atoi(side_drop.c_str());
		const size_t side_index = side-1;

		bool restart = side == resources::screen->playing_side();

		if (side_index >= resources::teams->size()) {
			ERR_NW << "unknown side " << side_index << " is dropping game\n";
			throw network::error("");
		}

		team &tm = (*resources::teams)[side_index];
		unit_map::iterator leader = resources::units->find_leader(side);
		const bool have_leader = leader.valid();

		if (controller == "ai"){
			tm.make_ai();
			tm.set_current_player("ai" + side_drop);
			if (have_leader) leader->rename("ai" + side_drop);
			return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
		}

		int action = 0;
		int first_observer_option_idx = 0;

		std::vector<std::string> observers;
		std::vector<team*> allies;
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

			//get all allies in as options to transfer control
			BOOST_FOREACH(team &t, *resources::teams)
			{
				if (!t.is_enemy(side) && !t.is_human() && !t.is_ai() && !t.is_network_ai() && !t.is_empty()
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
				tm.make_ai();
				resources::controller->on_not_observer();
				tm.set_current_player("ai" + side_drop);
				if (have_leader) leader->rename("ai" + side_drop);
				change_controller(side_drop, "ai");
				resources::controller->maybe_do_init_side(side_index);

				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

			case 1:
				tm.make_human();
				resources::controller->on_not_observer();
				tm.set_current_player("human" + side_drop);
				if (have_leader) leader->rename("human" + side_drop);
				change_controller(side_drop, "human");

				resources::controller->maybe_do_init_side(side_index);

				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
			case 2:
				tm.make_idle();
				tm.set_current_player("idle" + side_drop);
				if (have_leader) leader->rename("idle" + side_drop);

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
						tm.make_human();
						tm.set_current_player("human"+side_drop);
						if (have_leader) leader->rename("human"+side_drop);
					}

					const size_t index = static_cast<size_t>(action - first_observer_option_idx);
					if (index < observers.size()) {
						change_side_controller(side_drop, observers[index]);
					} else if (index < options.size() - 1) {
						size_t i = index - observers.size();
						change_side_controller(side_drop, allies[i]->current_player());
					} else {
						tm.make_ai();
						tm.set_current_player("ai"+side_drop);
						if (have_leader) leader->rename("ai" + side_drop);
						change_controller(side_drop, "ai");
					}
					return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
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
	if (cfg.child("notify_next_scenario")) {
		gui::button* btn_end = resources::screen->find_action_button("button-endturn");
		if(btn_end) {
			btn_end->enable(true);
		}
		return PROCESS_END_LINGER;
	}

	//If this client becomes the new host, notify the play_controller object about it
	if (const config &cfg_host_transfer = cfg.child("host_transfer")){
		if (cfg_host_transfer["value"] == "1") {
			host_transfer_.notify_observers();
		}
	}

	return turn_end ? PROCESS_END_TURN : PROCESS_CONTINUE;
}

void turn_info::change_controller(const std::string& side, const std::string& controller)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["controller"] = controller;

	network::send_data(cfg, 0);
}


void turn_info::change_side_controller(const std::string& side, const std::string& player)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;
	network::send_data(cfg, 0);
}

#if 0
void turn_info::take_side(const std::string& side, const std::string& controller)
{
	config cfg;
	cfg["side"] = side;
	cfg["controller"] = controller;
	cfg["name"] = controller+side;
	network::send_data(cfg, 0, true);
}
#endif
