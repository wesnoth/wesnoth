/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playturn.hpp"

#include "construct_dialog.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "rng.hpp"
#include "formula_string_utils.hpp"
#include "play_controller.hpp"

#include <ctime>

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)

turn_info::turn_info(unsigned team_num, replay_network_sender &replay_sender, undo_list &undo_stack) :
	team_num_(team_num), undo_stack_(undo_stack),
	replay_sender_(replay_sender),
	host_transfer_("host_transfer"), replay_()
{
	/**
	 * We do network sync so [init_side] is transfered to network hosts
	 */
	if(network::nconnections() > 0)
		send_data();
}

turn_info::~turn_info(){
	undo_stack_.clear();
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
	if(undo_stack_.empty()) {
		replay_sender_.commit_and_sync();
	} else {
		replay_sender_.sync_non_undoable();
	}
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg,
		network::connection from, std::deque<config>& backlog, bool skip_replay)
{
	if (const config &rnd_seed = cfg.child("random_seed")) {
		rand_rng::set_seed(lexical_cast<rand_rng::seed_t>(rnd_seed["seed"]));
		//may call a callback function, see rand_rng::set_seed_callback
	}

	if (const config &msg = cfg.child("message"))
	{
		const int side = lexical_cast_default<int>(msg["side"],0);

		resources::screen->add_chat_message(time(NULL), msg["sender"], side,
				msg["message"], events::chat_handler::MESSAGE_PUBLIC,
				preferences::message_bell());
	}

	if (const config &msg = cfg.child("whisper") /*&& is_observer()*/)
	{
		resources::screen->add_chat_message(time(NULL), "whisper: " + msg["sender"], 0,
				msg["message"], events::chat_handler::MESSAGE_PRIVATE,
				preferences::message_bell());
	}

	BOOST_FOREACH (const config &ob, cfg.child_range("observer")) {
		resources::screen->add_observer(ob["name"]);
	}

	BOOST_FOREACH (const config &ob, cfg.child_range("observer_quit")) {
		resources::screen->remove_observer(ob["name"]);
	}

	if(cfg.child("leave_game") != NULL) {
		throw network::error("");
	}

	bool turn_end = false;

	config::const_child_itors turns = cfg.child_range("turn");
	if (turns.first != turns.second && from != network::null_connection) {
		//forward the data to other peers
		network::send_data_all_except(cfg, from, true);
	}

	BOOST_FOREACH (const config &t, turns)
	{
		if(turn_end == false) {
			/** @todo FIXME: Check what commands we execute when it's our turn! */
			replay_.append(t);
			replay_.set_skip(skip_replay);

			turn_end = do_replay(team_num_, &replay_);
			recorder.add_config(t, replay::MARK_AS_SENT);
		} else {

			//this turn has finished, so push the remaining moves
			//into the backlog
			backlog.push_back(config());
			backlog.back().add_child("turn", t);
		}
	}

	if (const config &change= cfg.child("change_controller"))
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
			bool restart = resources::screen->get_playing_team() == index;
			if (!player.empty() && leader.valid())
				leader->second.rename(player);

			if (controller == "human" && !tm.is_human()) {
				if (!(*resources::teams)[resources::screen->get_playing_team()].is_human())
				{
					resources::screen->set_team(index);
				}
				tm.make_human();
			} else if (controller == "human_ai" && !tm.is_human_ai()) {
				tm.make_human_ai();
			} else if (controller == "network" && !tm.is_network_human()) {
				tm.make_network();
			} else if (controller == "network_ai" && !tm.is_network_ai()) {
				tm.make_network_ai();
			} else if (controller == "ai" && !tm.is_ai()) {
				tm.make_ai();
			}
			else
			{
				restart = false;
			}

			resources::screen->labels().recalculate_labels();

			return restart ? PROCESS_RESTART_TURN : PROCESS_CONTINUE;
		}
	}

	//if a side has dropped out of the game.
	if(cfg["side_drop"] != "") {
		const std::string controller = cfg["controller"];
		const std::string side_str = cfg["side_drop"];
		const size_t side = atoi(side_str.c_str());
		const size_t side_index = side-1;

		bool restart = side_index == resources::screen->get_playing_team();

		if (side_index >= resources::teams->size()) {
			ERR_NW << "unknown side " << side_index << " is dropping game\n";
			throw network::error("");
		}

		team &tm = (*resources::teams)[side_index];
		unit_map::iterator leader = resources::units->find_leader(side);
		const bool have_leader = leader.valid();

		if (controller == "ai"){
			tm.make_ai();
			tm.set_current_player("ai" + side_str);
			if(have_leader) leader->second.rename("ai"+side_str);


			return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
		}

		int action = 0;

		std::vector<std::string> observers;
		std::vector<team*> allies;
		std::vector<std::string> options;

		// We want to give host chance to decide what to do for side
		{
			utils::string_map t_vars;
			options.push_back(_("Replace with AI"));
			options.push_back(_("Replace with local player"));
			options.push_back(_("Abort game"));

			//get all observers in as options to transfer control
			BOOST_FOREACH (const std::string &ob, resources::screen->observers())
			{
				t_vars["player"] = ob;
				options.push_back(vgettext("Replace with $player", t_vars));
				observers.push_back(ob);
			}

			//get all allies in as options to transfer control
			BOOST_FOREACH (team &t, *resources::teams)
			{
				if (!t.is_enemy(side) && !t.is_human() && !t.is_ai() && !t.is_empty()
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
			gui::dialog dlg(*resources::screen, "", msg, gui::OK_ONLY);
			dlg.set_menu(options);
			action = dlg.show();
		}

		//make the player an AI, and redo this turn, in case
		//it was the current player's team who has just changed into
		//an AI.
		switch(action) {
			case 0:
				tm.make_human_ai();
				tm.set_current_player("ai" + side_str);
				if(have_leader) leader->second.rename("ai"+side_str);
				change_controller(side_str, "human_ai");


				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;

			case 1:
				tm.make_human();
				tm.set_current_player("human" + side_str);
				if(have_leader) leader->second.rename("human"+side_str);


				return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
			case 2:
				//The user pressed "end game". Don't throw a network error here or he will get
				//thrown back to the title screen.
				throw end_level_exception(QUIT);
			default:
				if (action > 2) {

					{
						// Server thinks this side is ours now so in case of error transfering side we have to make local state to same as what server thinks it is.
						tm.make_human();
						tm.set_current_player("human"+side_str);
						if(have_leader) leader->second.rename("human"+side_str);
					}

					const size_t index = static_cast<size_t>(action - 3);
					if (index < observers.size()) {
						change_side_controller(side_str, observers[index], false /*not our own side*/);
					} else if (index < options.size() - 1) {
						size_t i = index - observers.size();
						change_side_controller(side_str, allies[i]->current_player(), false /*not our own side*/);
					} else {
						tm.make_human_ai();
						tm.set_current_player("ai"+side_str);
						if(have_leader) leader->second.rename("ai"+side_str);
						change_controller(side_str, "human_ai");
					}
					return restart?PROCESS_RESTART_TURN:PROCESS_CONTINUE;
				}
				break;
		}
		throw network::error("");
	}

	// The host has ended linger mode in a campaign -> enable the "End scenario" button
	// and tell we did get the notification.
	if (cfg.child("notify_next_scenario")) {
		gui::button* btn_end = resources::screen->find_button("button-endturn");
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

	network::send_data(cfg, 0, true);
}


void turn_info::change_side_controller(const std::string& side, const std::string& player, bool own_side)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;

	if(own_side) {
		change["own_side"] = "yes";
	}

	network::send_data(cfg, 0, true);
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
