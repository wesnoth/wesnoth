#include "../config.hpp"
#include "../game_config.hpp"
#include "../network.hpp"
#include "../util.hpp"

#include "SDL.h"

#include "game.hpp"
#include "player.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <ctime>
#include <vector>

config construct_error(const std::string& msg)
{
	config cfg;
	cfg.add_child("error")["message"] = msg;
	return cfg;
}

class server
{
public:
	server(int port);
	void run();
private:
	void delete_game(std::vector<game>::iterator i);

	void dump_stats();

	const network::manager net_manager_;
	const network::server_manager server_;

	config login_response_;

	config initial_response_;
	config old_initial_response_;

	config sync_initial_response();

	player_map players_;

	game not_logged_in_;
	game lobby_players_;
	std::vector<game> games_;

	time_t last_stats_;
};

server::server(int port) : net_manager_(), server_(port), not_logged_in_(players_), lobby_players_(players_), last_stats_(time(NULL))
{
	login_response_.add_child("mustlogin");
	login_response_["version"] = game_config::version;
}

config server::sync_initial_response()
{
	config res;
	res.add_child("gamelist_diff",initial_response_.get_diff(old_initial_response_));
	old_initial_response_ = initial_response_;
	return res;
}

void server::dump_stats()
{
	config parent;
	config& stats = parent.add_child("statistics");

	stats["num_players"] = str_cast(players_.size());
	stats["lobby_players"] = str_cast(lobby_players_.nplayers());
	stats["num_games"] = str_cast(games_.size());
	stats["start_interval"] = str_cast(last_stats_);
	last_stats_ = time(NULL);
	stats["end_interval"] = str_cast(last_stats_);

	//write and flush the output stream
	std::cout << parent.write() << std::endl;
}

void server::run()
{
	config& gamelist = initial_response_.add_child("gamelist");
	old_initial_response_ = initial_response_;

	for(int loop = 0;; ++loop) {
		try {
			//make sure we log stats every 5 minutes
			if((loop%100) == 0 && last_stats_+5*60 < time(NULL)) {
				dump_stats();
			}


			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if(sock) {
				network::send_data(login_response_,sock);
				not_logged_in_.add_player(sock);
			}

			config data;
			while((sock = network::receive_data(data)) != NULL) {

				//if someone who is not yet logged in is sending
				//login details
				if(not_logged_in_.is_member(sock)) {
					const config* const login = data.child("login");

					//client must send a login first.
					if(login == NULL) {
						network::send_data(construct_error(
						                   "You must login first"),sock);
						continue;
					}

					//check the username is valid (all alpha-numeric)
					const std::string& username = (*login)["username"];
					if(std::count_if(username.begin(),username.end(),isalnum)
					   != username.size() || username.empty()) {
						network::send_data(construct_error(
						                   "This username is not valid"),sock);
						continue;
					}

					//check the username isn't already taken
					player_map::const_iterator p;
					for(p = players_.begin(); p != players_.end(); ++p) {
						if(p->second.name() == username) {
							break;
						}
					}

					if(p != players_.end()) {
						network::send_data(construct_error(
						              "This username is already taken"),sock);
						continue;
					}
					
					config* const player_cfg = &initial_response_.add_child("user");

					const player new_player(username,*player_cfg);

					players_.insert(std::pair<network::connection,player>(sock,new_player));

					//remove player from the not-logged-in list and place
					//the player in the lobby
					not_logged_in_.remove_player(sock);
					lobby_players_.add_player(sock);

					//send the new player the entire list of games and players
					network::send_data(initial_response_,sock);

					//send other players in the lobby the update that the player has joined
					lobby_players_.send_data(sync_initial_response(),sock);

					std::cerr << "'" << username << "' (" << network::ip_address(sock) << ") has logged on\n";

				} else if(lobby_players_.is_member(sock)) {
					const config* const create_game = data.child("create_game");
					if(create_game != NULL) {

						std::cerr << "creating game...\n";

						//create the new game, remove the player from the
						//lobby and put him/her in the game they have created
						games_.push_back(game(players_));
						lobby_players_.remove_player(sock);
						games_.back().add_player(sock);

						//store the game data here at the moment
						games_.back().level() = *create_game;
						std::stringstream converter;
						converter << games_.back().id();
						games_.back().level()["id"] = converter.str();

						//mark the player as unavailable in the lobby
						const player_map::iterator pl = players_.find(sock);
						if(pl != players_.end()) {
							pl->second.mark_available(false);

							lobby_players_.send_data(sync_initial_response());
						} else {
							std::cerr << "ERROR: Could not find player in map\n";
						}

						continue;
					}

					//see if the player is joining a game
					const config* const join = data.child("join");
					if(join != NULL) {
						const std::string& id = (*join)["id"];
						const int nid = atoi(id.c_str());
						const std::vector<game>::iterator it =
						             std::find_if(games_.begin(),games_.end(),
						                          game_id_matches(nid));
						if(it == games_.end()) {
							//send a response saying the game has been cancelled
							config cfg;
							cfg.add_child("leave_game");
							network::send_data(cfg,sock);

							std::cerr << "attempt to join unknown game\n";
							continue;
						}

						lobby_players_.remove_player(sock);

						//send them the game data
						network::send_data(it->level(),sock);

						it->add_player(sock);

						//mark the player as unavailable in the lobby
						const player_map::iterator pl = players_.find(sock);
						if(pl != players_.end()) {
							pl->second.mark_available(false);

							lobby_players_.send_data(sync_initial_response());
						} else {
							std::cerr << "ERROR: Could not find player in map\n";
						}
					}

					//see if it's a message, in which case we add the name
					//of the sender, and forward it to all players in the lobby
					config* const message = data.child("message");
					if(message != NULL) {
						const player_map::const_iterator p = players_.find(sock);
						assert(p != players_.end());
						(*message)["sender"] = p->second.name();
						lobby_players_.send_data(data,sock);
					}
				} else {
					std::vector<game>::iterator g;
					for(g = games_.begin(); g != games_.end(); ++g) {
						if(g->is_member(sock))
							break;
					}

					if(g == games_.end()) {
						std::cerr << "ERROR: unknown socket " << games_.size() << "\n";
						continue;
					}

					//if this is data describing changes to a game.
					if(data.child("scenario_diff")) {
						g->level().apply_diff(*data.child("scenario_diff"));
						g->send_data(data,sock);

						const bool lobby_changes = g->describe_slots();
						if(lobby_changes) {
							lobby_players_.send_data(sync_initial_response());
						}
					}

					//if this is data describing the level for a game.
					else if(data.child("side") != NULL) {

						const bool is_init = g->level_init();

						//if this game is having its level data initialized
						//for the first time, and is ready for players to join.
						//We should currently have a summary of the game in g->level().
						//we want to move this summary to the initial_response_, and
						//place a pointer to that summary in the game's description.
						//g->level() should then receive the full data for the game.
						if(!is_init) {

							//if there is no shroud, then tell players in the lobby what the map looks like
							if((*data.child("side"))["shroud"] != "yes") {
								g->level().values["map_data"] = data["map_data"];
								g->level().values["map"] = data["map"];
							}
							
							//update our config object which describes the
							//open games, and notifies the game of where its description
							//is located at
							config& desc = gamelist.add_child("game",g->level());
							g->set_description(&desc);

							//record the full description of the scenario in g->level()
							g->level() = data;
							g->describe_slots();

							//send all players in the lobby the update to the list of games
							lobby_players_.send_data(sync_initial_response());
						} else {

							//we've already initialized this scenario, but clobber its old
							//contents with the new ones given here
							g->level() = data;
						}

						//send all players in the level, except the sender, the new data
						g->send_data(data,sock);
						continue;
					}

					const string_map::const_iterator side = data.values.find("side");
					if(side != data.values.end()) {
						const bool res = g->take_side(sock,data);
						config response;
						if(res) {
							std::cerr << "played joined side\n";
							response["side_secured"] = side->second;

							//update the number of available slots
							const bool res = g->describe_slots();
							if(res) {
								lobby_players_.send_data(sync_initial_response());
							}
						} else {
							response["failed"] = "yes";
						}

						network::send_data(response,sock);
						continue;
					}

					if(data.child("start_game")) {
						//send notification of the game starting immediately.
						//g->start_game() will send data that assumes the [start_game]
						//message has been sent
						g->send_data(data,sock);

						g->start_game();
						lobby_players_.send_data(sync_initial_response());
						continue;
					} else if(data.child("leave_game")) {
						const bool needed = g->is_needed(sock);

						if(needed) {

							//tell all other players the game is over,
							//because a needed player has left
							config cfg;
							cfg.add_child("leave_game");
							g->send_data(cfg);

							//delete the game's description
							config* const gamelist = initial_response_.child("gamelist");
							assert(gamelist != NULL);
							const config::child_itors vg = gamelist->child_range("game");

							const config::child_iterator desc = std::find(vg.first,vg.second,g->description());
							if(desc != vg.second) {
								gamelist->remove_child("game",desc - vg.first);
							}

							//update the state of the lobby to players in it.
							//We have to sync the state of the lobby so we can
							//send it to the players leaving the game
							lobby_players_.send_data(sync_initial_response());

							//set the availability status for all quitting players
							for(player_map::iterator pl = players_.begin(); pl != players_.end(); ++pl) {
								if(g->is_member(pl->first)) {
									pl->second.mark_available(true);
								}
							}

							//put the players back in the lobby and send
							//them the game list and user list again
							g->send_data(initial_response_);
							lobby_players_.add_players(*g);
							games_.erase(g);

							//now sync players in the lobby again, to remove the game
							lobby_players_.send_data(sync_initial_response());
						} else {

							g->remove_player(sock);
							g->describe_slots();
							lobby_players_.add_player(sock);

							//mark the player as available in the lobby
							const player_map::iterator pl = players_.find(sock);
							if(pl != players_.end()) {
								pl->second.mark_available(true);
							} else {
								std::cerr << "ERROR: Could not find player in map\n";
							}

							//send the player who has quit the game list
							network::send_data(initial_response_,sock);

							//send all other players in the lobby the update to the lobby
							lobby_players_.send_data(sync_initial_response(),sock);
						}

						continue;
					} else if(data["side_secured"].empty() == false) {
						continue;
					} else if(data["failed"].empty() == false) {
						std::cerr << "ERROR: failure to get side\n";
						continue;
					}

					config* const turn = data.child("turn");
					if(turn != NULL) {
						g->filter_commands(sock,*turn);

						//notify the game of the commands, and if it changes
						//the description, then sync the new description
						//to players in the lobby
						const bool res = g->process_commands(*turn);
						if(res) {
							lobby_players_.send_data(sync_initial_response());
						}

						//any private 'speak' commands must be repackaged seperate
						//to other commands, and re-sent, since they should only go
						//to some clients.
						const config::child_itors speaks = turn->child_range("command");
						int npublic = 0, nprivate = 0, nother = 0;
						std::string team_name;
						for(config::child_iterator i = speaks.first; i != speaks.second; ++i) {
							config* const speak = (*i)->child("speak");
							if(speak == NULL) {
								++nother;
								continue;
							}

							//force the description to be correct to prevent
							//spoofing of messages
							const player_map::const_iterator pl = players_.find(sock);
							assert(pl != players_.end());
							(*speak)["description"] = pl->second.name();

							if((*speak)["team_name"] == "") {
								++npublic;
							} else {
								++nprivate;
								team_name = (*speak)["team_name"];
							}
						}

						//if all there are are messages and they're all private, then
						//just forward them on to the client that should receive them.
						if(nprivate > 0 && npublic == 0 && nother == 0) {
							g->send_data_team(data,team_name,sock);
							continue;
						}

						//at the moment, if private messages are mixed in with other
						//data, then let them go through. It's exceedingly unlikely that
						//this will happen anyway, and if it does, the client should
						//respect not displaying the message.
					}

					//forward data to all players who are in the game,
					//except for the original data sender
					g->send_data(data,sock);

					if(g->started()) {
						g->record_data(data);
					}
				}
			}
			
		} catch(network::error& e) {
			if(!e.socket) {
				std::cerr << "fatal network error: " << e.message << "\n";
				break;
			} else {
				std::cerr << "socket closed: " << e.message << "\n";

				const std::map<network::connection,player>::iterator pl_it = players_.find(e.socket);
				if(pl_it != players_.end()) {
					const config::child_list& users = initial_response_.get_children("user");
					const size_t index = std::find(users.begin(),users.end(),pl_it->second.config_address()) - users.begin();
					if(index < users.size())
						initial_response_.remove_child("user",index);

					players_.erase(pl_it);
				}
				
				not_logged_in_.remove_player(e.socket);
				lobby_players_.remove_player(e.socket);
				for(std::vector<game>::iterator i = games_.begin(); i != games_.end(); ++i) {
					if(i->is_needed(e.socket)) {
						delete_game(i);
						e.socket = 0;
						break;
					} else {
						i->remove_player(e.socket);
					}
				}

				if(e.socket) {
					e.disconnect();
				}

				//send all players the information that a player has logged
				//out of the system
				lobby_players_.send_data(sync_initial_response());

				std::cerr << "done closing socket...\n";
			}

			continue;
		} catch(config::error& e) {
			std::cerr << "error in received data: " << e.message << "\n";
			continue;
		}

		SDL_Delay(20);
	}
}

void server::delete_game(std::vector<game>::iterator i)
{
	//delete the game's configuration
	config* const gamelist = initial_response_.child("gamelist");
	assert(gamelist != NULL);
	const config::child_itors vg = gamelist->child_range("game");
	const config::child_list::iterator g = std::find(vg.first,vg.second,i->description());
	if(g != vg.second) {
		const size_t index = g - vg.first;
		gamelist->remove_child("game",index);
	}
	
	i->disconnect();
	games_.erase(i);
}

int main(int argc, char** argv)
{
	int port = 15000;

	network::set_default_send_size(4096);

	for(int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if((val == "--max_packet_size" || val == "-m") && arg+1 != argc) {
			network::set_default_send_size(size_t(atoi(argv[++arg])));
		}
		else if((val == "--port" || val == "-p") && arg+1 != argc) {
			port = atoi(argv[++arg]);
		} else if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
				<< " [options]\n"
				<< "  -p, --port     Binds the server to the specified port\n";
			return 0;
		} else if(val == "--version" || val == "-v") {
			std::cout << "Battle for Wesnoth server " << game_config::version
				<< "\n";
			return 0;
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << "\n";
			return 0;
		} else {
			port = atoi(argv[arg]);
		}
	}
			
	try {
		server(port).run();
	} catch(network::error& e) {
		std::cerr << "error starting server: " << e.message << "\n";
		return -1;
	}

	return 0;
}
