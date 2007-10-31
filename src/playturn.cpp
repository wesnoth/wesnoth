/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playturn.hpp"

#include "config.hpp"
#include "construct_dialog.hpp"
#include "game_display.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "menu_events.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "team.hpp"
#include "unit.hpp"

turn_info::turn_info(const game_data& gameinfo, game_state& state_of_game,
                     const gamestatus& status, game_display& gui, gamemap& map,
		     std::vector<team>& teams, unsigned int team_num, unit_map& units,
			 replay_network_sender& replay_sender, undo_list& undo_stack)
  : gameinfo_(gameinfo), state_of_game_(state_of_game), status_(status),
    gui_(gui), map_(map), teams_(teams), team_num_(team_num),
    units_(units), undo_stack_(undo_stack),
	replay_sender_(replay_sender), replay_error_("network_replay_error")
{}

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

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg, network::connection from, std::deque<config>& backlog, bool skip_replay)
{
	if(cfg.child("whisper") != NULL && is_observer()){
		sound::play_UI_sound(game_config::sounds::receive_message);

		const config& cwhisper = *cfg.child("whisper");
		gui_.add_chat_message("whisper: "+cwhisper["sender"],0,cwhisper["message"], game_display::MESSAGE_PRIVATE, preferences::message_bell());
		}
	if(cfg.child("observer") != NULL) {
		const config::child_list& observers = cfg.get_children("observer");
		for(config::child_list::const_iterator ob = observers.begin(); ob != observers.end(); ++ob) {
			gui_.add_observer((**ob)["name"]);
		}
	}

	if(cfg.child("observer_quit") != NULL) {
		const config::child_list& observers = cfg.get_children("observer_quit");
		for(config::child_list::const_iterator ob = observers.begin(); ob != observers.end(); ++ob) {
			gui_.remove_observer((**ob)["name"]);
		}
	}

	if(cfg.child("leave_game") != NULL) {
		throw network::error("");
	}

	bool turn_end = false;

	const config::child_list& turns = cfg.get_children("turn");
	if(turns.empty() == false && from != network::null_connection) {
		//forward the data to other peers
		network::send_data_all_except(cfg,from);
	}

	for(config::child_list::const_iterator t = turns.begin(); t != turns.end(); ++t) {

		if(turn_end == false) {
			replay replay_obj(**t);
			replay_obj.set_skip(skip_replay);
			replay_obj.start_replay();

			try{
				turn_end = do_replay(gui_,map_,gameinfo_,units_,teams_,
				team_num_,status_,state_of_game_,&replay_obj);
			}
			catch (replay::error& e){
				//notify remote hosts of out of sync error
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "out of sync";
				network::send_data(cfg);

				replay::last_replay_error = e.message; //FIXME: some better way to pass this?
				replay_error_.notify_observers();
			}

			recorder.add_config(**t,replay::MARK_AS_SENT);
		} else {

			//this turn has finished, so push the remaining moves
			//into the backlog
			backlog.push_back(config());
			backlog.back().add_child("turn",**t);
		}
	}

	if(const config* change= cfg.child("change_controller")) {
		//don't use lexical_cast_default it's "safer" to end on error
		const int side = lexical_cast<int>((*change)["side"]);
		const size_t index = static_cast<size_t>(side-1);

		const std::string& controller = (*change)["controller"];
		const std::string& player = (*change)["player"];

		if(index < teams_.size()) {
			teams_[index].set_current_player(player);
			const unit_map::iterator leader = find_leader(units_, side);
			if(leader != units_.end())
				leader->second.rename(player);

			if ( (controller == "human") && (!teams_[index].is_human()) ) {
				teams_[index].make_human();
				gui_.set_team(index);
			} else if ( (controller == "network") && (!teams_[index].is_network()) ){
				teams_[index].make_network();
			} else if ( (controller == "ai") && (!teams_[index].is_ai()) ) {
				teams_[index].make_ai();
			}

			return PROCESS_RESTART_TURN;
		}
	}

	//if a side has dropped out of the game.
	if(cfg["side_drop"] != "") {
		const std::string controller = cfg["controller"];
		const std::string side_str = cfg["side_drop"];
		const size_t side = atoi(side_str.c_str());
		const size_t side_index = side-1;

		if(side_index >= teams_.size()) {
			LOG_STREAM(err, network) << "unknown side " << side_index << " is dropping game\n";
			throw network::error("");
		}

		const unit_map::iterator leader = find_leader(units_,side);
		const bool have_leader = (leader != units_.end());

		if (controller == "ai"){
			teams_[side_index].make_ai();
			teams_[side_index].set_current_player("ai"+side_str);
			if(have_leader)
				leader->second.rename("ai"+side_str);

			take_side(side_str, "ai");

			return PROCESS_RESTART_TURN;
		}

		int action = 0;

		std::vector<std::string> observers;
		std::vector<team*> allies;
		std::vector<std::string> options;

		//see if the side still has a leader alive. If they have
		//no leader, we assume they just want to be replaced by
		//the AI.
		if(have_leader) {
			utils::string_map t_vars;
			options.push_back(_("Replace with AI"));
			options.push_back(_("Replace with local player"));
			options.push_back(_("Abort game"));

			//get all observers in as options to transfer control
			for(std::set<std::string>::const_iterator ob = gui_.observers().begin(); ob != gui_.observers().end(); ++ob) {
				t_vars["player"] = *ob;
				options.push_back(vgettext("Replace with $player", t_vars));
				observers.push_back(*ob);
			}

			//get all allies in as options to transfer control
			for (std::vector<team>::iterator team = teams_.begin(); team != teams_.end(); team++){
				if ( (!team->is_enemy(side)) && (!team->is_human()) && (!team->is_ai()) && (!team->is_empty())
					&& (team->current_player() != teams_[side_index].current_player()) ){
					//if this is an ally of the dropping side and it is not us (choose local player
					//if you want that) and not ai or empty and if it is not the dropping side itself,
					//get this team in as well
					t_vars["player"] = team->current_player();
					options.push_back(vgettext("Replace with $player", t_vars));
					allies.push_back(&(*team));
				}
			}

			t_vars["player"] = teams_[side_index].current_player();
			const std::string msg =  vgettext("$player has left the game. What do you want to do?", t_vars);
			gui::dialog dlg(gui_, "", msg, gui::OK_ONLY);
			dlg.set_menu(options);
			action = dlg.show();
		}

		//make the player an AI, and redo this turn, in case
		//it was the current player's team who has just changed into
		//an AI.
		switch(action) {
			case 0:
				teams_[side_index].make_ai();
				teams_[side_index].set_current_player("ai"+side_str);
				if(have_leader)
					leader->second.rename("ai"+side_str);

				take_side(side_str, "ai");

				return PROCESS_RESTART_TURN;

			//we don't have to test have_leader as action > 0 mean have_leader == true
			case 1:
				teams_[side_index].make_human();
				teams_[side_index].set_current_player("human"+side_str);
				leader->second.rename("human"+side_str);

				take_side(side_str, "human");

				return PROCESS_RESTART_TURN;
			case 2:
				//The user pressed "end game". Don't throw a network error here or he will get
				//thrown back to the title screen.
				throw end_level_exception(QUIT);
			default:
				if (action > 2) {
					const size_t index = static_cast<size_t>(action - 3);
					if (index < observers.size()) {
						teams_[side_index].make_network();
						change_side_controller(side_str, observers[index], false /*not our own side*/);
					} else if (index < options.size() - 1) {
						size_t i = index - observers.size();
						allies[i]->make_network();
						change_side_controller(side_str, allies[i]->save_id(), false /*not our own side*/);
					} else {
						teams_[side_index].make_ai();
						teams_[side_index].set_current_player("ai"+side_str);
						leader->second.rename("ai"+side_str);

						take_side(side_str, "ai");
					}
					return PROCESS_RESTART_TURN;
				}
				break;
		}
		throw network::error("");
	}
	if (const config* cfg_notify = cfg.child("notify_next_scenario")){
		if ( (*cfg_notify)["is_host"] == "1"){
			gui::button* btn_end = gui_.find_button("button-endturn");
			btn_end->enable(true);
		}
	}

	return turn_end ? PROCESS_END_TURN : PROCESS_CONTINUE;
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

	network::send_data(cfg);
}

void turn_info::take_side(const std::string& side, const std::string& controller)
{
	config cfg;
	cfg.values["side"] = side;
	cfg.values["controller"] = controller;
	cfg.values["name"] = controller+side;
	network::send_data(cfg);
}
