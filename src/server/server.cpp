/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "../config.hpp"
#include "../game_config.hpp"
#include "../log.hpp"
#include "../network.hpp"
#include "../util.hpp"
#include "../wassert.hpp"
#include "serialization/string_utils.hpp"

#include "SDL.h"

#include "game.hpp"
#include "filesystem.hpp"
#include "input_stream.hpp"
#include "metrics.hpp"
#include "serialization/parser.hpp"
#include "player.hpp"
#include "proxy.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <ctime>
#include <vector>

#include <signal.h>

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

namespace {

config construct_error(const std::string& msg)
{
	config cfg;
	cfg.add_child("error")["message"] = msg;
	return cfg;
}

config construct_server_message(const std::string& message, const game& g)
{
	config turn;
	if(g.started()) {
		config& cmd = turn.add_child("turn");
		config& cfg = cmd.add_child("command");
		config& msg = cfg.add_child("speak");
		msg["description"] = "server";
		msg["message"] = message;
	} else {
		config& msg = turn.add_child("message");
		msg["sender"] = "server";
		msg["message"] = message;
	}

	return turn;
}

void truncate_message(t_string& str)
{
	const size_t max_message_length = 240;
	std::string newstr = str.str();
	newstr.resize(minimum<size_t>(str.size(),max_message_length));
	str = newstr;
}

}

class server
{
public:
	server(int port, input_stream& input, const config& cfg, size_t nthreads);
	void run();
private:
	void process_data(network::connection sock, config& data, config& gamelist);

	void process_login(network::connection sock, const config& data, config& gamelist);
	void process_query(network::connection sock, const config& query, config& gamelist);
	void process_data_from_player_in_lobby(network::connection sock, config& data, config& gamelist);
	void process_data_from_player_in_game(network::connection sock, config& data, config& gamelist);

	std::string process_command(const std::string& cmd);

	void delete_game(std::vector<game>::iterator i);

	void dump_stats();

	const network::manager net_manager_;
	const network::server_manager server_;

	config version_query_response_;
	config login_response_;
	config join_lobby_response_;

	config initial_response_;
	config old_initial_response_;

	config sync_initial_response();

	player_map players_;

	game not_logged_in_;
	game lobby_players_;
	std::vector<game> games_;

	time_t last_stats_;

	input_stream& input_;

	metrics metrics_;

	const config& cfg_;

	std::set<std::string> accepted_versions_;
	std::map<std::string,config> redirected_versions_;
	std::map<std::string,config> proxy_versions_;

	bool ip_exceeds_connection_limit(const std::string& ip);

	bool is_ip_banned(const std::string& ip);
	std::string ban_ip(const std::string& mask);
	std::vector<std::string> bans_;

	std::string admin_passwd_;
	std::set<network::connection> admins_;

	std::string motd_;
};

server::server(int port, input_stream& input, const config& cfg, size_t nthreads) : net_manager_(nthreads), server_(port),
    not_logged_in_(players_), lobby_players_(players_), last_stats_(time(NULL)), input_(input), cfg_(cfg), admin_passwd_(cfg["passwd"]), motd_(cfg["motd"])
{
	version_query_response_.add_child("version");

	login_response_.add_child("mustlogin");

	const std::string& versions = cfg_["versions_accepted"];
	if(versions.empty() == false) {
		const std::vector<std::string> accepted(utils::split(versions));
		for(std::vector<std::string>::const_iterator i = accepted.begin(); i != accepted.end(); ++i) {
			accepted_versions_.insert(*i);
		}
	} else {
		accepted_versions_.insert(game_config::version);
	}

	const config::child_list& redirects = cfg_.get_children("redirect");
	for(config::child_list::const_iterator i = redirects.begin(); i != redirects.end(); ++i) {
		const std::vector<std::string> versions(utils::split((**i)["version"]));
		for(std::vector<std::string>::const_iterator j = versions.begin(); j != versions.end(); ++j) {
			redirected_versions_[*j] = **i;
		}
	}

	const config::child_list& proxies = cfg_.get_children("proxy");
	for(config::child_list::const_iterator p = proxies.begin(); p != proxies.end(); ++p) {
		const std::vector<std::string> versions(utils::split((**p)["version"]));
		for(std::vector<std::string>::const_iterator j = versions.begin(); j != versions.end(); ++j) {
			proxy_versions_[*j] = **p;
		}
	}

	join_lobby_response_.add_child("join_lobby");
}

bool server::ip_exceeds_connection_limit(const std::string& ip)
{
	const size_t MaxConnections = 5;
	size_t connections = 0;
	for(player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(network::ip_address(i->first) == ip) {
			++connections;
		}
	}

	return connections > MaxConnections;
}

bool server::is_ip_banned(const std::string& ip)
{
	for(std::vector<std::string>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
		std::cerr << "comparing for ban '" << *i << "' vs '" << ip << "'\n";
		const std::string::const_iterator itor = std::mismatch(i->begin(),i->end(),ip.c_str()).first;
		if(itor == i->end() && i->size() == ip.size() || itor != i->end() && *itor == '*') {
			std::cerr << "is banned\n";
			return true;
		}

		std::cerr << "not banned\n";
	}

	return false;
}

std::string server::ban_ip(const std::string& mask)
{
	const std::string::const_iterator asterisk = std::find(mask.begin(),mask.end(),'*');
	if(asterisk != mask.end() && asterisk != mask.end()-1) {
		return "'*' may only appear at the end of the mask";
	}

	bans_.push_back(mask);

	return "";
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
	time_t old_stats = last_stats_;
	last_stats_ = time(NULL);

	std::cout <<
		"Statistics\n"
		"\tnum_players = " << players_.size() << "\n"
		"\tlobby_players = " << lobby_players_.nplayers() << "\n"
		"\tstart_interval = " << old_stats << "\n"
		"\tend_interval = " << last_stats_ << std::endl;
}

std::string server::process_command(const std::string& cmd)
{
	std::ostringstream out;
	const std::string::const_iterator i = std::find(cmd.begin(),cmd.end(),' ');
	const std::string command(cmd.begin(),i);
	if(command == "msg") {
		if(i == cmd.end()) {
			return "you must type a message";
		}

		const std::string msg(i+1,cmd.end()); lobby_players_.send_data(construct_server_message(msg,lobby_players_));
		for(std::vector<game>::iterator g = games_.begin(); g != games_.end(); ++g) {
			g->send_data(construct_server_message(msg,*g));
		}

		out << "message '" << msg << "' relayed to players\n";
	} else if(command == "status") {
		out << "STATUS REPORT\n---\n";
		for(player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
			const network::connection_stats& stats = network::get_connection_stats(i->first);
			const int time_connected = stats.time_connected/1000;
			const int seconds = time_connected%60;
			const int minutes = (time_connected/60)%60;
			const int hours = time_connected/(60*60);
			out << "'" << i->second.name() << "' @ " << network::ip_address(i->first) << " connected for " << hours << ":" << minutes << ":" << seconds << " sent " << stats.bytes_sent << " bytes, received " << stats.bytes_received << " bytes\n";
		}

		out << "---";
	} else if(command == "metrics") {
		out << metrics_;
	} else if(command == "ban" || command == "kban") {

		if(i == cmd.end()) {
			out << "BAN LIST\n---\n";
			for(std::vector<std::string>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
				out << *i << "\n";
			}
			out << "---";
		} else {
			std::string mask(i+1,cmd.end());

			for(player_map::const_iterator j = players_.begin(); j != players_.end(); ++j) {
				if(j->second.name() == mask) {
					const std::string nick = mask;
					mask = network::ip_address(j->first);
					if(command == "kban") {
						network::queue_disconnect(j->first);
						out << "Kicked " << nick << ", ";
					}
					break;
				}
			}

			const std::string& diagnostic = ban_ip(mask);
			if(diagnostic != "") {
				out << "Could not ban '" << mask << "': " << diagnostic;
			} else {
				out << "Set ban on '" << mask << "'\n";
			}
		}
	} else if(command == "unban") {
		if(i == cmd.end()) {
			return "You must enter a mask to unban";
		}

		const std::string mask(i+1,cmd.end());

		const std::vector<std::string>::iterator itor = std::remove(bans_.begin(),bans_.end(),mask);
		if(itor == bans_.end()) {
			out << "there is no ban on '" << mask << "'";
		} else {
			bans_.erase(itor,bans_.end());
			out << "ban removed on '" << mask << "'";
		}

	} else if(command == "kick") {
		if(i == cmd.end()) {
			return "you must enter a nick to kick";
		}

		const std::string nick(i+1,cmd.end());

		for(player_map::const_iterator j = players_.begin(); j != players_.end(); ++j) {
			if(j->second.name() == nick) {
				network::queue_disconnect(j->first);
				return "kicked " + nick;
			}
		}

		out << "could not find user '" << nick << "'\n";
	} else if(command == "motd") {
		if(i == cmd.end()) {
			if(motd_ != "") {
				out << "message of the day: " << motd_;
				return out.str();
			} else {
				return "no message of the day set";
			}
		}

		const std::string motd(i+1,cmd.end());
		motd_ = motd;
		out << "message of the day set: " << motd_;

	} else {
		out << "command '" << command << "' is not recognized";
		out << "available commands are: msg <message>, motd <message>, status, metrics, ban [<nick>], unban <nick>, kick <nick>, kban <nick> ";
	}

	return out.str();
}

void server::run()
{
	config& gamelist = initial_response_.add_child("gamelist");
	old_initial_response_ = initial_response_;

	bool sync_scheduled = false;
	for(int loop = 0;; ++loop) {
		try {
			if(sync_scheduled) {
				//send all players the information that a player has logged
				//out of the system
				lobby_players_.send_data(sync_initial_response());
				sync_scheduled = false;
			}

			//process admin commands
			std::string admin_cmd;
			if(input_.read_line(admin_cmd)) {
				std::cout << process_command(admin_cmd) << std::endl;
			}

			//make sure we log stats every 5 minutes
			if((loop%100) == 0 && last_stats_+5*60 < time(NULL)) {
				dump_stats();
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if(sock) {
				const std::string& ip = network::ip_address(sock);
				if(is_ip_banned(ip)) {
					std::cerr << "rejected banned user '" << ip << "'\n";
					network::send_data(construct_error("You are banned."),sock);
					network::disconnect(sock);
				} else if(ip_exceeds_connection_limit(ip)) {
					std::cerr << "rejected ip '" << ip << "' due to excessive connections\n";
					network::send_data(construct_error("Too many connections from your host."),sock);
					network::disconnect(sock);
				} else {
					network::send_data(version_query_response_,sock);
					not_logged_in_.add_player(sock);
				}
			}

			config data;
			while((sock = network::receive_data(data)) != network::null_connection) {
				metrics_.service_request();
				process_data(sock,data,gamelist);
			}

			metrics_.no_requests();

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
						bool observer = i->is_observer(e.socket);
						const player_map::iterator pl = players_.find(e.socket);
						if(! observer && pl != players_.end()) {
							i->send_data(construct_server_message(pl->second.name() + " has disconnected",*i));
						}
						i->remove_player(e.socket);
					}
				}

				if(e.socket) {
					if(proxy::is_proxy(e.socket)) {
						proxy::disconnect(e.socket);
					}
					e.disconnect();
				}

				sync_scheduled = true;

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

void server::process_data(const network::connection sock, config& data, config& gamelist)
{
	if(proxy::is_proxy(sock)) {
		proxy::received_data(sock,data);
	}
	//if someone who is not yet logged in is sending
	//login details
	else if(not_logged_in_.is_member(sock)) {
		process_login(sock,data,gamelist);
	} else if(const config* query = data.child("query")) {
		process_query(sock,*query,gamelist);
	} else if(lobby_players_.is_member(sock)) {
		process_data_from_player_in_lobby(sock,data,gamelist);
	} else {
		process_data_from_player_in_game(sock,data,gamelist);
	}
}

void server::process_login(const network::connection sock, const config& data, config& gamelist)
{
	//see if client is sending their version number
	const config* const version = data.child("version");
	if(version != NULL) {
		const std::string& version_str = (*version)["version"];

		if(accepted_versions_.count(version_str)) {
			std::cerr << "player joined using accepted version " << version_str << ": telling them to log in\n";
			network::send_data(login_response_,sock);
		} else {
			const std::map<std::string,config>::const_iterator i = redirected_versions_.find(version_str);
			if(i != redirected_versions_.end()) {
				std::cerr << "player joined using version " << version_str << ": redirecting them to "
				          << i->second["host"] << ":" << i->second["port"] << "\n";
				config response;
				response.add_child("redirect",i->second);
				network::send_data(response,sock);
			} else {
				const std::map<std::string,config>::const_iterator i = proxy_versions_.find(version_str);

				if(i != proxy_versions_.end()) {
					std::cerr << "player joined using version " << version_str << ": connecting them by proxy to "
					          << i->second["host"] << ":" << i->second["port"] << "\n";

					proxy::create_proxy(sock,i->second["host"],lexical_cast_default<int>(i->second["port"],15000));
				} else {

					std::cerr << "player joined using unknown version " << version_str << ": rejecting them\n";
					config response;
					if(accepted_versions_.empty() == false) {
						response["version"] = *accepted_versions_.begin();
					} else if(redirected_versions_.empty() == false) {
						response["version"] = redirected_versions_.begin()->first;
					} else {
						std::cerr << "this server doesn't accept any versions at all\n";
						response["version"] = "null";
					}

					network::send_data(response,sock);
				}
			}
		}

		return;
	}

	const config* const login = data.child("login");

	//client must send a login first.
	if(login == NULL) {
		network::send_data(construct_error(
		                   "You must login first"),sock);
		return;
	}

	//check the username is valid (all alpha-numeric or space (but no space at ends))
	std::string username = (*login)["username"];
	const size_t alnum = std::count_if(username.begin(),username.end(),isalnum);
	const size_t spaces = std::count(username.begin(),username.end(),' ');
	if((alnum + spaces != username.size()) || spaces == username.size() || username.empty() || username[0] == ' ' || username[username.size()-1] == ' ') {
		network::send_data(construct_error(
		                   "This username is not valid"),sock);
		return;
	}

	if(username.size() > 32) {
		network::send_data(construct_error(
		                   "This username is too long"),sock);
		return;
	}

	if(username == "server") {
		network::send_data(construct_error(
		                   "The nick 'server' is reserved and can not be used by players"),sock);
		return;
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
		return;
	}

	network::send_data(join_lobby_response_, sock);

	config* const player_cfg = &initial_response_.add_child("user");

	const player new_player(username,*player_cfg);

	players_.insert(std::pair<network::connection,player>(sock,new_player));

	//remove player from the not-logged-in list and place
	//the player in the lobby
	not_logged_in_.remove_player(sock);
	lobby_players_.add_player(sock);

	//send the new player the entire list of games and players
	network::send_data(initial_response_,sock);

	if(motd_ != "") {
		network::send_data(construct_server_message(motd_,lobby_players_),sock);
	}

	//send other players in the lobby the update that the player has joined
	lobby_players_.send_data(sync_initial_response(),sock);

	std::cerr << "'" << username << "' (" << network::ip_address(sock) << ") has logged on\n";

	for(std::vector<game>::iterator g = games_.begin(); g != games_.end(); ++g) {
		g->send_data_observers(construct_server_message(username + " has logged into the lobby",*g));
	}
}

void server::process_query(const network::connection sock, const config& query, config& gamelist)
{
	//process queries from clients in here
	std::ostringstream response;
	if(query["type"] == "metrics") {
		//a query for server data from a player
		response << metrics_;
	} else if(admin_passwd_.empty() == false && query["type"] == admin_passwd_) {
		admins_.insert(sock);
		response << "You are now recognized as an administrator";
		std::cerr << "New Admin recognized:\n";
		std::cerr << "\tIP: "<< network::ip_address(sock)<<"\n";
		std::map<network::connection, player>::iterator temp = players_.find(sock);
		std::cerr << "\tnick: "<< temp->second.name()<<"\n";
		std::cerr << std::endl;
	} else if(admins_.count(sock) != 0) {
		response << process_command(query["type"]);
		std::cerr << "Admin Command:\n";
		std::cerr << "\ttype:" << query["type"]<<"\n";
		std::cerr << "\tIP: "<< network::ip_address(sock)<<"\n";
		std::map<network::connection, player>::iterator temp = players_.find(sock);
		std::cerr << "\tnick: "<< temp->second.name()<<"\n";
		std::cerr << std::endl;
	} else if(admin_passwd_.empty() == false) {
	  std::cerr << "FAILED Admin attempt:\n";
	  std::cerr << "\tIP: "<< network::ip_address(sock)<<"\n";
	  std::map<network::connection, player>::iterator temp = players_.find(sock);
	  std::cerr << "\tnick: "<< temp->second.name()<<"\n";
	  std::cerr << std::endl;
	  response << "Error: unrecognized query";
	} else {
		response << "Error: unrecognized query";
	}

	network::send_data(construct_server_message(response.str(),lobby_players_),sock);
}

void server::process_data_from_player_in_lobby(const network::connection sock, config& data, config& gamelist)
{
	const config* const create_game = data.child("create_game");
	if(create_game != NULL) {

		//std::cerr << "creating game...\n";

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
			pl->second.mark_available(false,games_.back().level()["name"]);

			lobby_players_.send_data(sync_initial_response());
		} else {
			std::cerr << "ERROR: Could not find player in map\n";
		}

		return;
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
			return;
		}

		if(it->player_is_banned(sock)) {
			network::send_data(construct_error("You are banned from this game"),sock);
			return;
		}

		lobby_players_.remove_player(sock);

		//send them the game data
		network::send_data(it->level(),sock);

		it->add_player(sock);

		//mark the player as unavailable in the lobby
		const player_map::iterator pl = players_.find(sock);
		if(pl != players_.end()) {
			pl->second.mark_available(false,(*it->description())["name"]);

			lobby_players_.send_data(sync_initial_response());
		} else {
			std::cerr << "ERROR: Could not find player in map\n";
		}
	}

	//see if it's a message, in which case we add the name
	//of the sender, and forward it to all players in the lobby
	config* const message = data.child("message");
	if(message != NULL) {
		const player_map::iterator p = players_.find(sock);
		wassert(p != players_.end());

		if(p->second.silenced()) {
			return;
		} else if(p->second.is_message_flooding()) {
			network::send_data(construct_server_message("Warning: you are sending too many messages too fast. Your message has not been relayed.",
			                                            lobby_players_),p->first);
			return;
		}

		(*message)["sender"] = p->second.name();

		truncate_message((*message)["message"]);

		lobby_players_.send_data(data,sock);
	}
}

void server::process_data_from_player_in_game(const network::connection sock, config& data, config& gamelist)
{
	std::vector<game>::iterator g;
	for(g = games_.begin(); g != games_.end(); ++g) {
		if(g->is_member(sock))
			break;
	}

	if(g == games_.end()) {
		std::cerr << "ERROR: unknown socket " << games_.size() << "\n";
		return;
	}

	//if info is being provided about the game state
	if(data.child("info") != NULL) {
		const config& info = *data.child("info");
		if(info["type"] == "termination") {
			g->set_termination_reason(info["condition"]);
		}
	}

	//if the owner is changing the controller for a side
	if(g->is_owner(sock) && data.child("change_controller") != NULL) {
		const config& change = *data.child("change_controller");
		const std::string& result = g->transfer_side_control(change);
		if(result == "") {
			const config& msg = construct_server_message(change["player"] + " takes control of side " + change["side"],*g);
			g->send_data(msg);
		} else {
			const config& msg = construct_server_message(result,*g);
			network::send_data(msg,sock);
		}

		return;
	}

	//if the owner is banning someone from the game
	if(g->is_owner(sock) && (data.child("ban") != NULL || data.child("kick") != NULL)) {
		std::string name;
		bool ban;
		if (data.child("ban") != NULL) {
			const config& u = *data.child("ban");
			name = u["username"];
			ban = true;
		} else if (data.child("kick") != NULL) {
			const config& u = *data.child("kick");
			name = u["username"];
			ban = false;
		}

		player_map::iterator pl;
		for(pl = players_.begin(); pl != players_.end(); ++pl) {
			if(pl->second.name() == name) {
				break;
			}
		}

		if(pl->first != sock && pl != players_.end()) {
			if (ban) {
				g->ban_player(pl->first);
				const config& msg = construct_server_message("You have been banned",*g);
				network::send_data(msg, pl->first);
				
				const config& p_msg = construct_server_message(pl->second.name() + " has been banned",*g);
				g->send_data(p_msg);
			} else {
				const config& p_msg = construct_server_message(pl->second.name() + " has been kicked",*g);
				g->send_data(p_msg);
				g->remove_player(pl->first);
			}

			config leave_game;
			leave_game.add_child("leave_game");
			network::send_data(leave_game,pl->first);

			g->describe_slots();
			lobby_players_.add_player(pl->first);

			//mark the player as available in the lobby
			pl->second.mark_available(true,"");

			//send the player who was banned the lobby game list
			network::send_data(initial_response_,pl->first);

			//send all other players in the lobby the update to the lobby
			lobby_players_.send_data(sync_initial_response(),sock);
		} else if(pl == players_.end()) {
			const config& response = construct_server_message(
			             "Ban failed: user '" + name + "' not found",*g);
			network::send_data(response,sock);
		}
	} else if(data.child("ban")) {
		const config& response = construct_server_message(
		             "You cannot ban: not the game creator",*g);
		network::send_data(response,sock);
	}

	//if this is data describing changes to a game.
	else if(g->is_owner(sock) && data.child("scenario_diff")) {
		g->level().apply_diff(*data.child("scenario_diff"));
		g->update_side_data();

		const bool lobby_changes = g->describe_slots();
		if(lobby_changes) {
			lobby_players_.send_data(sync_initial_response());
		}
	}

	//if this is data describing the level for a game.
	else if(g->is_owner(sock) && data.child("side") != NULL) {

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

			g->level().values["mp_era"] = data.child("era") != NULL ? data.child("era")->get_attribute("id") : "";
			g->level().values["mp_scenario"] = data["id"];
			g->level().values["mp_use_map_settings"] = data["mp_use_map_settings"];
			g->level().values["mp_village_gold"] = data["mp_village_gold"];
			g->level().values["mp_fog"] = data["mp_fog"];
			g->level().values["mp_shroud"] = data["mp_shroud"];
			g->level().values["experience_modifier"] = data["experience_modifier"];
			g->level().values["mp_countdown"] = data["mp_countdown"];
			g->level().values["mp_countdown_init_time"] = data["mp_countdown_init_time"];
			g->level().values["mp_countdown_turn_bonus"] = data["mp_countdown_turn_bonus"];

			//update our config object which describes the
			//open games, and notifies the game of where its description
			//is located at
			config& desc = gamelist.add_child("game",g->level());
			g->set_description(&desc);

			//record the full description of the scenario in g->level()
			g->level() = data;
			g->update_side_data();
			g->describe_slots();

			//send all players in the lobby the update to the list of games
			lobby_players_.send_data(sync_initial_response());
		} else {

			//we've already initialized this scenario, but clobber its old
			//contents with the new ones given here
			g->level() = data;
			g->update_side_data();
		}

		//send all players in the level, except the sender, the new data
		g->send_data(data,sock);
		return;
	}

	//if this is data telling us that the scenario did change.
	else if(g->is_owner(sock) && data.child("next_scenario") != NULL) {
		config* scenario = data.child("next_scenario");

		if(g->level_init()) {
			g->level() = *scenario;
			g->reset_history();
			g->update_side_data();
		} else {
			// next_scenario sent while the scenario was not
			// initialized. Something's broken here.
			std::cerr << "Error: next_scenario sent while the scenario is not yet initialized";
			return;
		}
	}

	const string_map::const_iterator side = data.values.find("side");
	if(side != data.values.end()) {
		const bool res = g->take_side(sock,data);
		config response;
		if(res) {
			std::cerr << "player joined side\n";
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
		return;
	}

	if(data.child("start_game")) {
		//send notification of the game starting immediately.
		//g->start_game() will send data that assumes the [start_game]
		//message has been sent
		g->send_data(data,sock);

		g->start_game();
		lobby_players_.send_data(sync_initial_response());
		return;
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
			wassert(gamelist != NULL);
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
					pl->second.mark_available(true,"");
				}
			}

			//put the players back in the lobby and send
			//them the game list and user list again
			g->send_data(initial_response_);
			metrics_.game_terminated(g->termination_reason());
			lobby_players_.add_players(*g);
			games_.erase(g);

			//now sync players in the lobby again, to remove the game
			lobby_players_.send_data(sync_initial_response());
		} else {

			bool obs = g->is_observer(sock);
			g->remove_player(sock);
			g->describe_slots();
			lobby_players_.add_player(sock);

			//mark the player as available in the lobby
			const player_map::iterator pl = players_.find(sock);
			if(pl != players_.end()) {
				if(!obs) {
					const config& msg = construct_server_message(pl->second.name() + " has left the game",*g);
					g->send_data(msg);
				}
				pl->second.mark_available(true,"");
			} else {
				std::cerr << "ERROR: Could not find player in map\n";
			}
			
			//send the player who has quit the game list
			network::send_data(initial_response_,sock);

			//send all other players in the lobby the update to the lobby
			lobby_players_.send_data(sync_initial_response(),sock);
		}

		return;
	} else if(data["side_secured"].empty() == false) {
		return;
	} else if(data["failed"].empty() == false) {
		std::cerr << "ERROR: failure to get side\n";
		return;
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

		//any private 'speak' commands must be repackaged separate
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

			truncate_message((*speak)["message"]);

			//force the description to be correct to prevent
			//spoofing of messages
			const player_map::const_iterator pl = players_.find(sock);
			wassert(pl != players_.end());
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
			return;
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

void server::delete_game(std::vector<game>::iterator i)
{
	metrics_.game_terminated(i->termination_reason());

	//delete the game's configuration
	config* const gamelist = initial_response_.child("gamelist");
	wassert(gamelist != NULL);
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
	size_t nthreads = 5;

	network::set_default_send_size(4096);

	config configuration;

#ifndef FIFODIR
# define FIFODIR "/var/run/wesnothd"
#endif
	std::string fifo_path = std::string(FIFODIR) + "/socket";

	for(int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if((val == "--config" || val == "-c") && arg+1 != argc) {
			scoped_istream stream = istream_file(argv[++arg]);

			std::string errors;
			try {
				read(configuration,*stream,&errors);
				if(errors.empty() == false) {
					std::cerr << "WARNING: errors reading configuration file: " << errors << "\n";
				}
			} catch(config::error& e) {
				std::cerr << "ERROR: could not read configuration file: '" << e.message << "'\n";
				return -1;
			}
		} else if((val == "--max_packet_size" || val == "-m") && arg+1 != argc) {
			network::set_default_send_size(size_t(atoi(argv[++arg])));
		} else if(val == "--verbose" || val == "-v") {
			lg::set_log_domain_severity("all",2);
		} else if((val == "--port" || val == "-p") && arg+1 != argc) {
			port = atoi(argv[++arg]);
		} else if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
				<< " [-dvV] [-c path] [-m n] [-p port] [-t n]\n"
				<< "  -c  --config path          Tells wesnothd where to find the config file to use.\n"
				<< "  -d  --daemon               Runs wesnothd as a daemon.\n"
				<< "  -h  --help                 Shows this usage message.\n"
				<< "  -m, --max_packet_size n    Sets the maximal packet size to n.\n"
				<< "  -p, --port port            Binds the server to the specified port.\n"
				<< "  -t, --threads n            Uses n worker threads for network I/O (default: 5).\n"
				<< "  -v  --verbose              Turns on more verbose logging.\n"
				<< "  -V, --version              Returns the server version.\n";
			return 0;
		} else if(val == "--version" || val == "-V") {
			std::cout << "Battle for Wesnoth server " << game_config::version
				<< "\n";
			return 0;
		} else if(val == "--daemon" || val == "-d") {
#ifdef WIN32
			std::cerr << "Running as a daemon is not supported on this platform\n";
			return -1;
#else
			const pid_t pid = fork();
			if(pid < 0) {
				std::cerr << "Could not fork and run as a daemon\n";
				return -1;
			} else if(pid > 0) {
				std::cout << "Started wesnothd as a daemon with process id " << pid << "\n";
				return 0;
			}

			setsid();
#endif
		} else if((val == "--threads" || val == "-t") && arg+1 != argc) {
			nthreads = atoi(argv[++arg]);
			if(nthreads > 30) {
				nthreads = 30;
			}
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << "\n";
			return 0;
		} else {
			port = atoi(argv[arg]);
		}
	}

	input_stream input(fifo_path);

	try {
		server(port,input,configuration,nthreads).run();
	} catch(network::error& e) {
		std::cerr << "caught network error while server was running. aborting.: " << e.message << "\n";
		return -1;
	}

	return 0;
}
