#include "../config.hpp"
#include "../game_config.hpp"
#include "../network.hpp"

#include "SDL.h"

#include "game.hpp"
#include "player.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

config construct_error(const std::string& msg)
{
	config cfg;
	config* const err = new config();
	(*err)["message"] = msg;
	cfg.children["error"].push_back(err);
	return cfg;
}

int main()
{
	const network::manager net_manager;
	const network::server_manager server;

	config login_response;
	login_response.children["mustlogin"].push_back(new config());
	login_response["version"] = game_config::version;

	config initial_response;
	initial_response.children["gamelist"].push_back(new config());
	config& gamelist = *initial_response.children["gamelist"].back();

	typedef std::map<network::connection,player> player_map;
	player_map players;

	game not_logged_in;
	game lobby_players;
	std::vector<game> games;

	for(;;) {
		try {
			network::connection sock = network::accept_connection();
			if(sock) {
				network::send_data(login_response,sock);
				not_logged_in.add_player(sock);
			}

			config data;
			while((sock = network::receive_data(data)) != NULL) {

				//if someone who is not yet logged in is sending
				//login details
				if(not_logged_in.is_member(sock)) {
					const config* const login = data.child("login");

					//client must send a login first.
					if(login == NULL) {
						network::send_data(construct_error(
						                   "You must login first"),sock);
						continue;
					}

					//check the username is valid (all alnum)
					const std::string& username = (*login)["username"];
					if(std::count_if(username.begin(),username.end(),isalnum)
					   != username.size() || username.empty()) {
						network::send_data(construct_error(
						                   "This username is not valid"),sock);
						continue;
					}

					//check the username isn't already taken
					player_map::const_iterator p;
					for(p = players.begin(); p != players.end(); ++p) {
						if(p->second.name() == username) {
							break;
						}
					}

					if(p != players.end()) {
						network::send_data(construct_error(
						                   "This username is already taken"));
						continue;
					}
					
					config* const player_cfg = new config();
					const player new_player(username,*player_cfg);
					players.insert(std::pair<network::connection,player>(
					                            sock,new_player));
					initial_response.children["user"].push_back(player_cfg);

					//remove player from the not-logged-in list and place
					//the player in the lobby
					not_logged_in.remove_player(sock);
					lobby_players.add_player(sock);

					//currently update user list to all players who are in lobby
					//(later may optimize to only send new login information
					//to all but the new player)
					lobby_players.send_data(initial_response);

				} else if(lobby_players.is_member(sock)) {
					const config* const create_game = data.child("create_game");
					if(create_game != NULL) {

						std::cerr << "creating game...\n";

						//create the new game, remove the player from the
						//lobby and put him/her in the game they have created
						games.push_back(game());
						lobby_players.remove_player(sock);
						games.back().add_player(sock);

						//store the game data here at the moment
						games.back().level() = *create_game;
						std::stringstream converter;
						converter << games.back().id();
						games.back().level()["id"] = converter.str();

						continue;
					}

					//see if the player is joining a game
					const config* const join = data.child("join");
					if(join != NULL) {
						const std::string& id = (*join)["id"];
						const int nid = atoi(id.c_str());
						const std::vector<game>::iterator it =
						             std::find_if(games.begin(),games.end(),
						                          game_id_matches(nid));
						if(it == games.end()) {
							std::cerr << "attempt to join unknown game\n";
							continue;
						}

						lobby_players.remove_player(sock);
						it->add_player(sock);

						//send them the game data
						network::send_data(it->level(),sock);
					}

					//see if it's a message, in which case we add the name
					//of the sender, and forward it to all players in the lobby
					config* const message = data.child("message");
					if(message != NULL) {
						const player_map::const_iterator p = players.find(sock);
						assert(p != players.end());
						(*message)["sender"] = p->second.name();
						lobby_players.send_data(data);
					}

				} else {
					std::vector<game>::iterator g;
					for(g = games.begin(); g != games.end(); ++g) {
						if(g->is_member(sock))
							break;
					}

					if(g == games.end()) {
						std::cerr << "ERROR: unknown socket " << games.size() << "\n";
						continue;
					}

					//if this is data describing the level for a game
					if(data.child("side") != NULL) {

						//if this game is having its level-data initialized
						//for the first time, and is ready for players to join
						if(!g->level_init()) {
							
							//update our config object which describes the
							//open games
							gamelist.children["game"].push_back(
							                           new config(g->level()));
							g->set_description(
							          gamelist.children["game"].back());

							//send all players in the lobby the list of games
							lobby_players.send_data(initial_response);
						}

						//record the new level data, and send to all players
						//who are in the game
						g->level() = data;
					}

					const string_map::const_iterator side =
					                           data.values.find("side");
					if(side != data.values.end()) {
						const bool res = g->take_side(sock,side->second);
						config response;
						if(res) {
							response["side_secured"] = side->second;
						} else {
							response["failed"] = "yes";
						}

						network::send_data(response,sock);
						continue;
					}

					if(data.child("start_game")) {
						g->start_game();
					}

					//forward data to all players who are in the game,
					//except for the original data sender
					g->send_data(data,sock);
					g->record_data(data);
				}
			}
			
		} catch(network::error& e) {
			if(!e.socket) {
				std::cerr << "fatal network error: " << e.message << "\n";
				break;
			} else {
				std::cerr << "socket closed: " << e.message << "\n";

				const std::map<network::connection,player>::iterator pl_it =
				               players.find(e.socket);
				if(pl_it != players.end()) {
					std::vector<config*>& users =
					                   initial_response.children["user"];
					users.erase(std::find(users.begin(),users.end(),
					                      pl_it->second.config_address()));
					players.erase(pl_it);
				}
				
				not_logged_in.remove_player(e.socket);
				lobby_players.remove_player(e.socket);
				for(std::vector<game>::iterator i = games.begin();
				    i != games.end(); ++i) {
					if(i->is_needed(e.socket)) {

						//delete the game's configuration
						config* const gamelist =
						            initial_response.child("gamelist");
						assert(gamelist != NULL);
						std::vector<config*>& vg = gamelist->children["game"];
						std::vector<config*>::iterator g =
						    std::find(vg.begin(),vg.end(),i->description());
						if(g != vg.end()) {
							delete *g;
							vg.erase(g);
						}
						
						i->disconnect();
						games.erase(i);
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
				lobby_players.send_data(initial_response);
			}

			continue;
		} catch(config::error& e) {
			std::cerr << "error in received data: " << e.message << "\n";
			continue;
		}

		SDL_Delay(20);
	}
}
