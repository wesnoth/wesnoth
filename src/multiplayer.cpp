/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "language.hpp"
#include "log.hpp"
#include "multiplayer.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

class connection_acceptor : public gui::dialog_action
{
public:

	typedef std::map<config*,network::connection> positions_map;

	connection_acceptor(config& players);
	int do_action();

	bool is_complete() const;

	std::vector<std::string> get_positions_status() const;

	enum { CONNECTIONS_PART_FILLED=1, CONNECTIONS_FILLED=2 };

private:
	positions_map positions_;
	config& players_;
};

connection_acceptor::connection_acceptor(config& players)
                   : players_(players)
{
	std::vector<config*>& sides = players.children["side"];
	for(std::vector<config*>::const_iterator i = sides.begin();
	    i != sides.end(); ++i) {
		if((*i)->values["controller"] == "network") {
			positions_[*i] = 0;
		}
	}

	//if we have any connected players when we are created, send them the data
	network::send_data(players_);
}

int connection_acceptor::do_action()
{
	network::connection sock = network::accept_connection();
	if(sock) {
		std::cerr << "Received connection\n";
		network::send_data(players_,sock);
	}

	config cfg;

	std::vector<config*>& sides = players_.children["side"];

	try {
		sock = network::receive_data(cfg);
	} catch(network::error& e) {

		sock = 0;

		//if the problem isn't related to any specific connection,
		//it's a general error and we should just re-throw the error
		//likewise if we are not a server, we cannot afford any connection
		//to go down, so also re-throw the error
		if(!e.socket || !network::is_server()) {
			throw e;
		}

		bool changes = false;

		//a socket has disconnected. Remove its positions.
		for(positions_map::iterator i = positions_.begin();
		    i != positions_.end(); ++i) {
			if(i->second == e.socket) {
				changes = true;
				i->second = 0;
				i->first->values.erase("taken");
			}
		}

		//now disconnect the socket
		e.disconnect();

		//if there have been changes to the positions taken,
		//then notify other players
		if(changes) {
			network::send_data(players_);
		}
	}

	if(sock) {
		const int side_drop = atoi(cfg.values["side_drop"].c_str())-1;
		if(side_drop >= 0 && side_drop < int(sides.size())) {
			positions_map::iterator pos = positions_.find(sides[side_drop]);
			if(pos != positions_.end()) {
				pos->second = 0;
				pos->first->values.erase("taken");
				network::send_data(players_);
			}
		}

		const int side_taken = atoi(cfg.values["side"].c_str())-1;
		if(side_taken >= 0 && side_taken < int(sides.size())) {
			positions_map::iterator pos = positions_.find(sides[side_taken]);
			if(pos != positions_.end()) {
				if(!pos->second) {
					std::cerr << "client has taken a valid position\n";

					//broadcast to everyone the new game status
					pos->first->values["taken"] = "yes";
					positions_[sides[side_taken]] = sock;
					network::send_data(players_);

					std::cerr << "sent player data\n";

					//send a reply telling the client they have secured
					//the side they asked for
					std::stringstream side;
					side << (side_taken+1);
					config reply;
					reply.values["side_secured"] = side.str();
					std::cerr << "going to send data...\n";
					network::send_data(reply,sock);

					//see if all positions are now filled
					bool unclaimed = false;
					for(positions_map::const_iterator p = positions_.begin();
					    p != positions_.end(); ++p) {
						if(!p->second) {
							unclaimed = true;
							break;
						}
					}

					if(!unclaimed) {
						std::cerr << "starting game now...\n";
						config start_game;
						start_game.children["start_game"].
						                         push_back(new config());
						network::send_data(start_game);
						return CONNECTIONS_FILLED;
					}
				} else {
					config response;
					response.values["failed"] = "yes";
					network::send_data(response,sock);
				}
			} else {
				std::cerr << "tried to take illegal side: " << side_taken
				          << "\n";
			}
		} else {
			std::cerr << "tried to take unknown side: " << side_taken << "\n";
		}

		return CONNECTIONS_PART_FILLED;
	}

	return CONTINUE_DIALOG;
}

bool connection_acceptor::is_complete() const
{
	for(positions_map::const_iterator i = positions_.begin();
	    i != positions_.end(); ++i) {
		if(!i->second) {
			return false;
		}
	}

	return true;
}

std::vector<std::string> connection_acceptor::get_positions_status() const
{
	std::vector<std::string> result;
	for(positions_map::const_iterator i = positions_.begin();
	    i != positions_.end(); ++i) {
		result.push_back(i->first->values["name"] + "," +
		                 (i->second ? ("@" + string_table["position_taken"]) :
		                              string_table["position_vacant"]));
	}

	return result;
}

bool accept_network_connections(display& disp, config& players)
{
	connection_acceptor acceptor(players);

	while(acceptor.is_complete() == false) {
		const std::vector<std::string>& items = acceptor.get_positions_status();
		const int res = gui::show_dialog(disp,NULL,"",
		                                 string_table["awaiting_connections"],
		                       gui::CANCEL_ONLY,&items,NULL,"",NULL,&acceptor);
		if(res == 0) {
			return false;
		}
	}

	return true;
}

}

void play_multiplayer(display& disp, game_data& units_data, config cfg,
                      game_state& state, bool server)
{
	log_scope("play multiplayer");

	std::vector<std::string> options;
	std::vector<config*>& levels = cfg.children["multiplayer"];
	for(std::vector<config*>::iterator i = levels.begin(); i!=levels.end();++i){
		const std::string& lang_name = string_table[(*i)->values["id"]];
		if(lang_name.empty() == false)
			options.push_back(lang_name);
		else
			options.push_back((*i)->values["name"]);
	}

	options.push_back("Load game...");

	int res = gui::show_dialog(disp,NULL,"",
	                        string_table["choose_scenario"],gui::OK_CANCEL,
							&options);
	if(res == -1)
		return;

	config* level_ptr = NULL;
	config loaded_level;
	if(size_t(res) == options.size()-1) {
		const std::vector<std::string>& games = get_saves_list();
		if(games.empty()) {
			gui::show_dialog(disp,NULL,
			                 string_table["no_saves_heading"],
							 string_table["no_saves_message"],
			                 gui::OK_ONLY);
			return;
		}

		const int res = gui::show_dialog(disp,NULL,
		                                 string_table["load_game_heading"],
		                                 string_table["load_game_message"],
		                                 gui::OK_CANCEL, &games);
		if(res == -1)
			return;

		load_game(units_data,games[res],state);

		if(state.campaign_type != "multiplayer") {
			gui::show_dialog(disp,NULL,"",
			                 "This is not a multiplayer save",gui::OK_ONLY);
			return;
		}

		if(state.version != game_config::version) {
			const int res = gui::show_dialog(disp,NULL,"",
			                      string_table["version_save_message"],
			                      gui::YES_NO);
			if(res == 1)
				return;
		}

		loaded_level = state.starting_pos;
		level_ptr = &loaded_level;

		//make all sides untaken
		for(config::child_itors i = level_ptr->child_range("side");
		    i.first != i.second; ++i.first) {
			(**i.first)["taken"].clear();
		}

		recorder = replay(state.replay_data);

		level_ptr->add_child("replay") = state.replay_data;

	} else {
		level_ptr = levels[res];
	}

	assert(level_ptr != NULL);

	config& level = *level_ptr;
	state.label = level.values["name"];

	state.scenario = res;

	std::vector<config*>& sides = level.children["side"];
	std::vector<config*>& possible_sides = cfg.children["multiplayer_side"];
	if(sides.empty() || possible_sides.empty()) {
		std::cerr << "no multiplayer sides found\n";
		return;
	}

	for(std::vector<config*>::iterator sd = sides.begin();
	    sd != sides.end(); ++sd) {
		if((*sd)->values["name"].empty())
			(*sd)->values["name"] = possible_sides.front()->values["name"];
		if((*sd)->values["type"].empty())
			(*sd)->values["type"] = possible_sides.front()->values["type"];
		if((*sd)->values["recruit"].empty())
			(*sd)->values["recruit"]=possible_sides.front()->values["recruit"];
		if((*sd)->values["recruitment_pattern"].empty())
			(*sd)->values["recruitment_pattern"] =
			        possible_sides.front()->values["recruitment_pattern"];
	}

	res = 0;
	while(size_t(res) != sides.size()) {
		std::vector<std::string> sides_list;
		for(std::vector<config*>::iterator sd = sides.begin();
		    sd != sides.end(); ++sd) {
			std::stringstream details;
			details << (*sd)->values["side"] << ","
					<< (*sd)->values["name"] << ",";

			const std::string& controller = (*sd)->values["controller"];
			if(controller == "human")
				details << string_table["human_controlled"];
			else if(controller == "network")
				details << string_table["network_controlled"];
			else
				details << string_table["ai_controlled"];

			sides_list.push_back(details.str());
		}

		sides_list.push_back(string_table["start_game"]);

		res = gui::show_dialog(disp,NULL,"",string_table["configure_sides"],
		                       gui::MESSAGE,&sides_list);

		if(size_t(res) < sides.size()) {
			std::vector<std::string> choices;

			for(int n = 0; n != 3; ++n) {
				for(std::vector<config*>::iterator i = possible_sides.begin();
				    i != possible_sides.end(); ++i) {
					std::stringstream choice;
					choice << (*i)->values["name"] << " - ";
					switch(n) {
						case 0: choice << string_table["human_controlled"];
						        break;
						case 1: choice << string_table["ai_controlled"];
						        break;
						case 2: choice << string_table["network_controlled"];
						        break;
						default: assert(false);
					}
						
					choices.push_back(choice.str());
				}
			}

			int result = gui::show_dialog(disp,NULL,"",
			                               string_table["choose_side"],
										   gui::MESSAGE,&choices);
			if(result >= 0) {
				std::string controller = "network";
				if(result < int(choices.size())/3) {
					controller = "human";
				} else if(result < int(choices.size()/3)*2) {
					controller = "ai";
					result -= choices.size()/3;
				} else {
					controller = "network";
					result -= (choices.size()/3)*2;
				}

				sides[res]->values["controller"] = controller;

				assert(result < int(possible_sides.size()));

				std::map<std::string,std::string>& values =
				                                possible_sides[result]->values;
				sides[res]->values["name"] = values["name"];
				sides[res]->values["type"] = values["type"];
				sides[res]->values["recruit"] = values["recruit"];
				sides[res]->values["recruitment_pattern"] =
				                          values["recruitment_pattern"];
			}
		}
	}

	const network::manager net_manager;
	const network::server_manager server_man(15000,server);

	const bool network_state = accept_network_connections(disp,level);
	if(network_state == false)
		return;

	state.starting_pos = level;

	recorder.set_save_info(state);

	if(!recorder.empty()) {
		const int res = gui::show_dialog(disp,NULL,
               "", string_table["replay_game_message"],
			   gui::YES_NO);
		//if yes, then show the replay, otherwise
		//skip showing the replay
		if(res == 0) {
			recorder.set_skip(0);
		} else {
			std::cerr << "skipping...\n";
			recorder.set_skip(-1);
		}
	}

	std::vector<config*> story;
	play_level(units_data,cfg,&level,disp.video(),state,story);
	recorder.clear();
}
