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
#include "menu.hpp"
#include "multiplayer.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "replay.hpp"

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
	std::vector<config*>& sides_;
};

connection_acceptor::connection_acceptor(config& players)
                   : players_(players), sides_(players.children["side"])
{
	std::vector<config*>& sides = players.children["side"];
	for(std::vector<config*>::const_iterator i = sides.begin();
	    i != sides.end(); ++i) {
		if((*i)->values["controller"] == "network") {
			positions_[*i] = 0;
		}
	}
}

int connection_acceptor::do_action()
{
	network::connection sock = network::accept_connection();
	if(sock) {
		std::cerr << "Received connection\n";
		network::send_data(players_,sock);
	}

	config cfg;

	try {
		sock = network::receive_data(cfg);
	} catch(network::error& e) {

		sock = 0;

		//if the problem isn't related to any specific connection
		if(!e.socket) {
			throw e;
		}

		bool changes = false;

		//a socket has disconnected. Remove its positions.
		for(positions_map::iterator i = positions_.begin();
		    i != positions_.end(); ) {
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
		const int side_taken = atoi(cfg.values["side"].c_str())-1;
		if(side_taken >= 0 && side_taken < int(sides_.size())) {
			positions_map::iterator pos = positions_.find(sides_[side_taken]);
			if(pos != positions_.end()) {
				if(!pos->second) {
					std::cerr << "client has taken a valid position\n";

					//broadcast to everyone the new game status
					pos->first->values["taken"] = "yes";
					positions_[sides_[side_taken]] = sock;
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
			std::cerr << "tried to take unknown side: " << side_taken
			          << "\n";
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

void play_multiplayer_client(display& disp, game_data& units_data, config& cfg,
                             game_state& state)
{
	const network::manager net_manager;

	std::string host;
	const int res = gui::show_dialog(disp,NULL,"","",
	                                 gui::OK_CANCEL,NULL,NULL,
	                                 string_table["remote_host"] + ": ",&host);
	if(res != 0 || host.empty()) {
		return;
	}

	network::connection sock;

	sock = network::connect(host);
	config sides;

	network::connection data_res = network::receive_data(sides,0,3000);

	if(!data_res) {
		throw network::error(string_table["connection_timeout"]);
	}

	std::map<int,int> choice_map;
	std::vector<std::string> choices;

	std::vector<config*>& sides_list = sides.children["side"];
	for(std::vector<config*>::iterator s = sides_list.begin();
	    s != sides_list.end(); ++s) {
		if((*s)->values["controller"] == "network" &&
		   (*s)->values["taken"] != "yes") {
			choice_map[choices.size()] = 1 + s - sides_list.begin();
			choices.push_back((*s)->values["name"] + " - " +
			                  (*s)->values["type"]);
		}
	}

	const int choice = gui::show_dialog(disp,NULL,"","Choose side:",
	                                    gui::OK_CANCEL,&choices);
	if(choice < 0) {
		return;
	}

	const int team_num = choice_map[choice];

	//send our choice of team to the server
	{
		config response;
		std::stringstream stream;
		stream << team_num;
		response.values["side"] = stream.str();
		network::send_data(response);
	}

	bool got_side = false;

	for(;;) {
		config reply;
		data_res = network::receive_data(reply,0,100);
		if(data_res) {
			if(reply.values["failed"] == "yes") {
				got_side = false;
				break;
			} else if(reply.values["side_secured"].size() > 0) {
				got_side = true;
			} else if(reply.children["start_game"].empty() == false) {
				break;
			} else {
				sides = reply;
			}
		}
	}

	if(!got_side) {
		throw network::error("Choice of team unavailable.");
	}

	//we want to make the network/human players look right from our
	//perspective
	{
		std::vector<config*>& sides_list = sides.children["side"];
		for(std::vector<config*>::iterator side = sides_list.begin();
		    side != sides_list.end(); ++side) {
			string_map& values = (*side)->values;
			if(team_num-1 == side - sides_list.begin())
				values["controller"] = "human";
			else
				values["controller"] = "network";
		}
	}

	std::cerr << "starting game\n";

	state.starting_pos = sides;

	recorder.set_save_info(state);

	std::vector<config*> story;
	play_level(units_data,cfg,&sides,disp.video(),state,story);
	recorder.clear();
}

void play_multiplayer(display& disp, game_data& units_data, config& cfg,
                      game_state& state)
{
	std::vector<std::string> options;
	std::vector<config*>& levels = cfg.children["multiplayer"];
	for(std::vector<config*>::iterator i = levels.begin(); i!=levels.end();++i){
		const std::string& lang_name = string_table[(*i)->values["id"]];
		if(lang_name.empty() == false)
			options.push_back(lang_name);
		else
			options.push_back((*i)->values["name"]);
	}

	int res = gui::show_dialog(disp,NULL,"",
	                        string_table["choose_scenario"],gui::OK_CANCEL,
							&options);
	if(res == -1)
		return;

	config& level = *levels[res];
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
		(*sd)->values["name"] = possible_sides.front()->values["name"];
		(*sd)->values["type"] = possible_sides.front()->values["type"];
		(*sd)->values["recruit"] = possible_sides.front()->values["recruit"];
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
			}
		}
	}

	const network::manager net_manager;
	const network::server_manager server_man;

	const bool network_state = accept_network_connections(disp,level);
	if(network_state == false)
		return;

	state.starting_pos = level;

	recorder.set_save_info(state);

	std::vector<config*> story;
	play_level(units_data,cfg,&level,disp.video(),state,story);
	recorder.clear();
}
