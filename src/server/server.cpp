#include "../config.hpp"
#include "../network.hpp"

#include "SDL.h"

#include "game.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

int main()
{
	const network::manager net_manager;
	const network::server_manager server;

	config initial_response;
	initial_response.children["gamelist"].push_back(new config());
	config& gamelist = *initial_response.children["gamelist"].back();

	game lobby_players;
	std::vector<game> games;

	for(;;) {
		try {
			network::connection sock = network::accept_connection();
			if(sock) {
				network::send_data(initial_response,sock);
				lobby_players.add_player(sock);
			}

			config data;
			while((sock = network::receive_data(data)) != NULL) {
				if(lobby_players.is_member(sock)) {
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

							//send all players in the lobby the list of games
							lobby_players.send_data(initial_response);
						}

						//record the new level data, and send to all players
						//who are in the game
						g->level() = data;

						std::cerr << "registered game: " << data.write() << "\n";
					}

					//forward data to all players who are in the game,
					//except for the original data sender
					g->send_data(data,sock);
				}
			}
			
		} catch(network::error& e) {
			if(!e.socket) {
				std::cerr << "fatal network error: " << e.message << "\n";
			} else {
				std::cerr << "socket closed: " << e.message << "\n";
				
				lobby_players.remove_player(e.socket);
				for(std::vector<game>::iterator i = games.begin();
				    i != games.end(); ++i) {
					if(i->is_member(e.socket)) {
						i->disconnect();
						games.erase(i);
						e.socket = 0;
						break;
					}
				}

				if(e.socket) {
					e.disconnect();
				}
			}

			continue;
		} catch(config::error& e) {
			std::cerr << "error in received data: " << e.message << "\n";
			continue;
		}

		SDL_Delay(20);
	}
}
