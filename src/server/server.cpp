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

	const network::manager net_manager_;
	const network::server_manager server_;

	config login_response_;

	config initial_response_;
	config old_initial_response_;

	config sync_initial_response();

	typedef std::map<network::connection,player> player_map;
	player_map players_;

	game not_logged_in_;
	game lobby_players_;
	std::vector<game> games_;
};

server::server(int port) : net_manager_(), server_(port)
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

void server::run()
{
	config& gamelist = initial_response_.add_child("gamelist");
	old_initial_response_ = initial_response_;

	for(;;) {
		try {
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

				} else if(lobby_players_.is_member(sock)) {
					const config* const create_game = data.child("create_game");
					if(create_game != NULL) {

						std::cerr << "creating game...\n";

						//create the new game, remove the player from the
						//lobby and put him/her in the game they have created
						games_.push_back(game());
						lobby_players_.remove_player(sock);
						games_.back().add_player(sock);

						//store the game data here at the moment
						games_.back().level() = *create_game;
						std::stringstream converter;
						converter << games_.back().id();
						games_.back().level()["id"] = converter.str();

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
							std::cerr << "attempt to join unknown game\n";
							continue;
						}

						lobby_players_.remove_player(sock);

						//send them the game data
						network::send_data(it->level(),sock);

						it->add_player(sock);
					}

					//see if it's a message, in which case we add the name
					//of the sender, and forward it to all players in the lobby
					config* const message = data.child("message");
					if(message != NULL) {
						const player_map::const_iterator p = players_.find(sock);
						assert(p != players_.end());
						(*message)["sender"] = p->second.name();
						lobby_players_.send_data(data);
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
					}

					//if this is data describing the level for a game
					else if(data.child("side") != NULL) {

						const bool is_init = g->level_init();

						//if this game is having its level-data initialized
						//for the first time, and is ready for players to join
						if(!is_init) {
							
							//update our config object which describes the
							//open games, and notifies the game of where its description
							//is located at
							config& desc = gamelist.add_child("game",g->level());
							g->set_description(&desc);

							//send all players in the lobby the update to the list of games
							lobby_players_.send_data(sync_initial_response());
						}

						//record the new level data, and send to all players
						//who are in the game
						g->level() = data;

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
						} else {
							response["failed"] = "yes";
						}

						network::send_data(response,sock);
						continue;
					}

					if(data.child("start_game")) {
						g->start_game();
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
								delete gamelist->remove_child("game",desc - vg.first);
							}

							//update the state of the lobby to players in it.
							//We have to sync the state of the lobby because we can
							//send it to the players leaving the game
							lobby_players_.send_data(sync_initial_response());

							//put the players back in the lobby and send
							//them the game list and user list again
							g->send_data(initial_response_);
							lobby_players_.add_players(*g);
							games_.erase(g);

							//now sync players in the lobby again, to remove the game
							lobby_players_.send_data(sync_initial_response());
						} else {

							g->remove_player(sock);
							lobby_players_.add_player(sock);

							//send the player who has quit the game list
							network::send_data(initial_response_,sock);
						}

						continue;
					} else if(data["side_secured"].empty() == false) {
						continue;
					} else if(data["failed"].empty() == false) {
						std::cerr << "ERROR: failure to get side\n";
						continue;
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
		delete *g;
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
