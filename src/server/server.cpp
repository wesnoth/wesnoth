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

// fatal and directly server related errors/warnings,
// ie not caused by erroneous client data
#define ERR_SERVER LOG_STREAM(err, general)
// we abuse the warn level for normal logging
#define WRN_SERVER LOG_STREAM(warn, general)
// debugging messages
#define LOG_SERVER LOG_STREAM(info, general)

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
	const size_t max_message_length = 256;
	// The string send can contain utf-8 so truncate as wide_string otherwise
	// an corrupted utf-8 string can be returned.
	wide_string newstr = utils::string_to_wstring(str.str());
	newstr.resize(minimum<size_t>(newstr.size(), max_message_length));
	str = utils::wstring_to_string(newstr);
}

}

class server
{
public:
	server(int port, input_stream& input, const config& cfg, size_t nthreads);
	void run();
private:
	void process_data(network::connection sock, config& data, config& gamelist);

	void process_login(network::connection sock, const config& data);
	void process_query(network::connection sock, const config& query);
	void process_whisper(const network::connection sock, const config& whisper);
	void process_data_from_player_in_lobby(network::connection sock, config& data);
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

	size_t default_max_messages_;
	size_t default_time_period_;
	size_t concurrent_connections_;

	std::set<std::string> accepted_versions_;
	std::map<std::string,config> redirected_versions_;
	std::map<std::string,config> proxy_versions_;
	std::vector<std::string> disallowed_names_;

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

	if(cfg_["disallow_names"] == "") {
		disallowed_names_.push_back("server");
		disallowed_names_.push_back("ai");
		disallowed_names_.push_back("player");
		disallowed_names_.push_back("network");
		disallowed_names_.push_back("human");
		disallowed_names_.push_back("admin");
		disallowed_names_.push_back("computer");
	} else {
		disallowed_names_ = utils::split(cfg_["disallow_names"]);
	}
	default_max_messages_ = lexical_cast_default<int>(cfg_["max_messages"],4);
	default_time_period_ = lexical_cast_default<int>(cfg_["messages_time_period"],10);
	concurrent_connections_ = lexical_cast_default<int>(cfg_["connections_allowed"],5);

	const std::string& versions = cfg_["versions_accepted"];
	if(versions.empty() == false) {
		const std::vector<std::string> accepted(utils::split(versions));
		for(std::vector<std::string>::const_iterator i = accepted.begin(); i != accepted.end(); ++i) {
			accepted_versions_.insert(*i);
		}
	} else {
		accepted_versions_.insert("1.2*");
		accepted_versions_.insert("test");
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
	size_t connections = 0;
	for(player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(network::ip_address(i->first) == ip) {
			++connections;
		}
	}

	return connections > concurrent_connections_;
}

bool server::is_ip_banned(const std::string& ip)
{
	for(std::vector<std::string>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
		LOG_SERVER << "comparing for ban '" << *i << "' vs '" << ip << "'\n";
		if(utils::wildcard_string_match(ip,*i)) {
			LOG_SERVER << ip << " is banned\n";
			return true;
		}
		LOG_SERVER << "not banned\n";
	}

	return false;
}

std::string server::ban_ip(const std::string& mask)
{
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

	WRN_SERVER <<
		"Statistics\n"
		"\tnumber_of_games = " << games_.size() << "\n"
		"\tnumber_of_players = " << players_.size() << "\n"
		"\tlobby_players = " << lobby_players_.nobservers() << "\n"
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
			out << "'" << i->second.name() << "' @ " << network::ip_address(i->first)
				<< " connected for " << hours << ":" << minutes << ":" << seconds
				<< " sent " << stats.bytes_sent << " bytes, received "
				<< stats.bytes_received << " bytes\n";
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
				WRN_SERVER << process_command(admin_cmd) << std::endl;
			}

			//make sure we log stats every 5 minutes
			time_t now = time(NULL);
			if ((loop%100) == 0 && last_stats_+5*60 < now) {
				dump_stats();
				// send a 'ping' to all players to detect ghosts
				config ping;
				ping["ping"] = lexical_cast<std::string>(now);
				for (player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
					network::queue_data(ping, i->first);
				}
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if(sock) {
				const std::string& ip = network::ip_address(sock);
				if(is_ip_banned(ip)) {
					WRN_SERVER << "rejected banned user '" << ip << "'\n";
					network::send_data(construct_error("You are banned."),sock);
					network::disconnect(sock);
				} else if(ip_exceeds_connection_limit(ip)) {
					WRN_SERVER << "rejected ip '" << ip << "' due to excessive connections\n";
					network::send_data(construct_error("Too many connections from your IP."),sock);
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
				ERR_SERVER << "fatal network error: " << e.message << "\n";
				break;
			} else {
				LOG_SERVER << "socket closed: " << e.message << "\n";
				const std::string ip = network::ip_address(e.socket);
				const std::map<network::connection,player>::iterator pl_it = players_.find(e.socket);
				const std::string pl_name = pl_it != players_.end() ? pl_it->second.name() : "";
				if(pl_it != players_.end()) {
					const config::child_list& users = initial_response_.get_children("user");
					const size_t index = std::find(users.begin(),users.end(),pl_it->second.config_address()) - users.begin();
					if(index < users.size())
						initial_response_.remove_child("user",index);

					players_.erase(pl_it);
				}
				not_logged_in_.remove_player(e.socket);
				lobby_players_.remove_player(e.socket);
				for(std::vector<game>::iterator g = games_.begin(); g != games_.end(); ++g) {
					if(g->is_member(e.socket)) {
						const std::string game_name = g->description() ? (*g->description())["name"] : "Warning: Game has no description.";
						const bool needed = g->is_needed(e.socket);
						const bool obs = g->is_observer(e.socket);
						if(obs) {
							WRN_SERVER << pl_name << " (" << ip
								<< ") has left game: \"" << game_name
								<< "\" (" << g->id() << ") as an observer and disconnected.\n";
						} else {
							g->send_data(construct_server_message(pl_name + " has disconnected",*g));
							WRN_SERVER << pl_name << " (" << ip
								<< ") has left game: \"" << game_name
								<< "\" (" << g->id() << ") and disconnected.\n";
						}
						g->remove_player(e.socket);
						if( (g->nplayers() == 0) || (needed && !g->started()) ) {
							// Tell observers the game is over,
							// because the last player has left
							config cfg;
							cfg.add_child("leave_game");
							g->send_data(cfg);
							WRN_SERVER << pl_name << " (" << ip
								<< ") ended game: \"" << game_name
								<< "\" (" << g->id() << ") and disconnected.\n";

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
							break;
						} else if(needed) {
							// Transfer game control to another player
							const player* player = g->transfer_game_control();
							if (player != NULL) {
								g->send_data(construct_server_message(player->name() + " has been chosen as new host", *g));
							}
							// else?

							e.socket = 0; // is this needed?
							break;
						}
					}
				}
				if(pl_it != players_.end()) {
					WRN_SERVER << "'" << pl_name << "' (" << ip << ") has logged off\n";
				}
				if(e.socket) {
					if(proxy::is_proxy(e.socket)) {
						proxy::disconnect(e.socket);
					}
					e.disconnect();
				}

				sync_scheduled = true;

				LOG_SERVER << "done closing socket...\n";
			}

			continue;
		} catch(config::error& e) {
			ERR_SERVER << "error in received data: " << e.message << "\n";
			continue;
		}

		SDL_Delay(20);
	}
}

void server::process_data(const network::connection sock, config& data, config& gamelist)
{
	//std::cerr << "in server::process_data...\n";
	if(proxy::is_proxy(sock)) {
		proxy::received_data(sock,data);
	}
	//if someone who is not yet logged in is sending
	//login details
	else if(not_logged_in_.is_observer(sock)) {
		process_login(sock,data);
	} else if(const config* query = data.child("query")) {
		process_query(sock,*query);
	} else if(const config* whisper = data.child("whisper")) {
		process_whisper(sock,*whisper);
	} else if(lobby_players_.is_observer(sock)) {
		process_data_from_player_in_lobby(sock,data);
	} else {
		process_data_from_player_in_game(sock,data,gamelist);
	}
}


void server::process_login(const network::connection sock, const config& data)
{
	//see if client is sending their version number
	const config* const version = data.child("version");
	if(version != NULL) {
		const std::string& version_str = (*version)["version"];

		bool accepted = false;
		for(std::set<std::string>::const_iterator ver_it = accepted_versions_.begin(); ver_it != accepted_versions_.end(); ++ver_it) {
			if(utils::wildcard_string_match(version_str,*ver_it)) {
				accepted = true;
				break;
			}
		}
		if(accepted) {
			WRN_SERVER << "player (" << network::ip_address(sock) << ") joined using accepted version " << version_str << ": telling them to log in\n";
			network::send_data(login_response_,sock);
		} else {
			std::map<std::string,config>::const_iterator redirect = redirected_versions_.end();
			for(std::map<std::string,config>::const_iterator red_it = redirected_versions_.begin(); red_it != redirected_versions_.end(); ++red_it) {
				if(utils::wildcard_string_match(version_str,red_it->first)) {
					redirect = red_it;
					break;
				}
			}
			if(redirect != redirected_versions_.end()) {
				WRN_SERVER << "player (" << network::ip_address(sock)
					<< ") joined using version " << version_str
					<< ": redirecting them to " << redirect->second["host"]
					<< ":" << redirect->second["port"] << "\n";
				config response;
				response.add_child("redirect",redirect->second);
				network::send_data(response,sock);
			} else {
				std::map<std::string,config>::const_iterator proxy = proxy_versions_.end();
				for(std::map<std::string,config>::const_iterator prox_it = proxy_versions_.begin(); prox_it != proxy_versions_.end(); ++prox_it) {
					if(utils::wildcard_string_match(version_str,prox_it->first)) {
						proxy = prox_it;
						break;
					}
				}

				if(proxy != proxy_versions_.end()) {
					WRN_SERVER << "player (" << network::ip_address(sock)
						<< ") joined using version " << version_str
						<< ": connecting them by proxy to " << proxy->second["host"]
						<< ":" << proxy->second["port"] << "\n";

					proxy::create_proxy(sock,proxy->second["host"],lexical_cast_default<int>(proxy->second["port"],15000));
				} else {

					WRN_SERVER << "player (" << network::ip_address(sock)
						<< ") joined using unknown version " << version_str
						<< ": rejecting them\n";
					config response;
					if(accepted_versions_.empty() == false) {
						response["version"] = *accepted_versions_.begin();
					} else if(redirected_versions_.empty() == false) {
						response["version"] = redirected_versions_.begin()->first;
					} else {
						ERR_SERVER << "this server doesn't accept any versions at all\n";
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

	//check if the username is valid (all alpha-numeric plus underscore)
	std::string username = (*login)["username"];
	if(!utils::isvalid_username(username)) {
		network::send_data(construct_error(
		                   "This username contains invalid characters. Only alpha-numeric characters and underscores are allowed."),sock);
		return;
	}

	if(username.size() > 18) {
		network::send_data(construct_error(
		                   "This username is too long"),sock);
		return;
	}

	for(std::vector<std::string>::const_iterator d_it = disallowed_names_.begin(); d_it != disallowed_names_.end(); ++d_it) {
		if(utils::wildcard_string_match(utils::lowercase(username),utils::lowercase(*d_it))) {
			network::send_data(construct_error(
							"The nick '" + username + "' is reserved and can not be used by players"),sock);
			return;
		}
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

	const player new_player(username,*player_cfg,default_max_messages_,default_time_period_);

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

	WRN_SERVER << "'" << username << "' (" << network::ip_address(sock) << ") has logged on\n";

	for(std::vector<game>::iterator g = games_.begin(); g != games_.end(); ++g) {
		g->send_data_observers(construct_server_message(username + " has logged into the lobby",*g));
	}
}

void server::process_query(const network::connection sock, const config& query)
{
	const player_map::iterator pl = players_.find(sock);
	if(pl == players_.end()) {
		LOG_SERVER << "ERROR: Could not find player socket.\n";
		return;
	}
	// Process queries from clients in here
	std::ostringstream response;

	if(query["type"] == "metrics") {
		//a query for server data from a player
		response << metrics_;
	} else if(admin_passwd_.empty() == false && query["type"] == admin_passwd_) {
		admins_.insert(sock);
		response << "You are now recognized as an administrator";
		WRN_SERVER << "New Admin recognized:" << "\tIP: "
			<< network::ip_address(sock) << "\tnick: "
			<< pl->second.name() << std::endl;
	} else if(admins_.count(sock) != 0) {
		response << process_command(query["type"]);
		WRN_SERVER << "Admin Command:" << "\ttype: " << query["type"]
			<< "\tIP: "<< network::ip_address(sock) 
			<< "\tnick: "<< pl->second.name() << std::endl;
	} else if(admin_passwd_.empty() == false) {
		WRN_SERVER << "FAILED Admin attempt:" << "\tIP: "
			<< network::ip_address(sock) << "\tnick: "
			<< pl->second.name() << std::endl;
		response << "Error: unrecognized query";
	} else {
		response << "Error: unrecognized query";
	}

	network::send_data(construct_server_message(response.str(),lobby_players_),sock);
}

void server::process_whisper(const network::connection sock, const config& whisper)
{
	const player_map::iterator pl = players_.find(sock);
	if(pl == players_.end()) {
		LOG_SERVER << "ERROR: Could not find player socket.\n";
		return;
	}
	bool sent = false;
	bool do_send = true;
	std::vector<game>::iterator g;
	if ((whisper["receiver"]!="") && (whisper["message"]!="") && (whisper["sender"]!="")) {
		for(player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
			if(i->second.name() == whisper["receiver"]) {
				for(g = games_.begin(); g != games_.end(); ++g) {
					if(g->is_player(i->first)) {
						do_send = false;
						break;
					}
				}
				if(do_send == false) {
					break;
				}
				config cwhisper;
				cwhisper.add_child("whisper",whisper);
				network::send_data(cwhisper,i->first);
				sent = true;
				break;
			}
		}
	} else {
		config msg;
		config data;
		msg["message"] = "Invalid number of arguments";
		msg["sender"] = "server";
		data.add_child("message", msg);
		network::send_data(data,sock);
		sent = true;
	}

	if (sent == false) {
		config msg;
		config data;
		if(do_send == false) {
			msg["message"] = "You cannot send private messages to players in a game.";
		} else {
			msg["message"] = "Can't find player "+whisper["receiver"];
		}
		msg["sender"] = "server";
		data.add_child("message", msg);
		network::send_data(data,sock);
	}
}

void server::process_data_from_player_in_lobby(const network::connection sock, config& data)
{
	LOG_SERVER << "in process_data_from_player_in_lobby...\n";

	const player_map::iterator pl = players_.find(sock);
	if(pl == players_.end()) {
		LOG_SERVER << "ERROR: Could not find player socket.\n";
		return;
	}

	const config* const create_game = data.child("create_game");
	if(create_game != NULL) {

		//create the new game, remove the player from the
		//lobby and put him/her in the game they have created
		games_.push_back(game(players_));
		games_.back().level() = *create_game;
		lobby_players_.remove_player(sock);
		games_.back().set_owner(sock);
		games_.back().add_player(sock);

		//store the game data here at the moment
		std::stringstream converter;
		converter << games_.back().id();
		games_.back().level()["id"] = converter.str();

		//mark the player as unavailable in the lobby
		pl->second.mark_available(false,games_.back().level()["name"]);

		lobby_players_.send_data(sync_initial_response());

		return;
	}

	//see if the player is joining a game
	const config* const join = data.child("join");
	if(join != NULL) {
		const std::string& id = (*join)["id"];
		const std::string& str_observer = (*join)["observe"];
		const int nid = atoi(id.c_str());
		const std::vector<game>::iterator it =
		             std::find_if(games_.begin(),games_.end(),
		                          game_id_matches(nid));
		if(it == games_.end()) {
			//send a response saying the game has been cancelled
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg,sock);

			LOG_SERVER << pl->second.name() << " (" << network::ip_address(sock)
				<< ") attempted to join unknown game: " << id << "\n";
			return;
		}

		if(it->player_is_banned(sock)) {
			LOG_SERVER << "Reject banned player: " << pl->second.name()
				<< " (" << network::ip_address(sock)
				<< ") from game: \"" << (*it->description())["name"]
				<< "\" (" << id << ").\n";
			network::send_data(construct_error("You are banned from this game"),sock);
			return;
		}

		if(str_observer == "yes") {
			WRN_SERVER << pl->second.name() << " (" << network::ip_address(sock)
				<< ") joined game: \"" << (*it->description())["name"]
				<< "\" (" << id << ") as an observer.\n";
		} else {
			WRN_SERVER << pl->second.name() << " joined game: \""
				<< (*it->description())["name"] << "\" (" << id
				<< ").\n";
		}
		lobby_players_.remove_player(sock);

		//send them the game data
		network::send_data(it->level(),sock);

		it->add_player(sock, str_observer == "yes");

		//mark the player as unavailable in the lobby
		pl->second.mark_available(false,(*it->description())["name"]);

		lobby_players_.send_data(sync_initial_response());
	}

	//see if it's a message, in which case we add the name
	//of the sender, and forward it to all players in the lobby
	config* const message = data.child("message");
	if(message != NULL) {
		if(pl->second.silenced()) {
			return;
		} else if(pl->second.is_message_flooding()) {
			network::send_data(construct_server_message("Warning: you are sending too many messages too fast. Your message has not been relayed.",
			                                            lobby_players_),pl->first);
			return;
		}

		(*message)["sender"] = pl->second.name();
		truncate_message((*message)["message"]);

		std::string msg = (*message)["message"].base_str();
		if(msg.substr(0,3) == "/me") {
			WRN_SERVER << network::ip_address(sock) << ": <"
				<< pl->second.name() << msg.substr(3) << ">\n";
		} else {
			WRN_SERVER << network::ip_address(sock) << ": <"
				<< pl->second.name() << "> " << msg << "\n";
		}

		lobby_players_.send_data(data,sock);
	}

	//player requests update of lobby content, for example when cancelling
	//the create game dialog
	config* const refresh = data.child("refresh_lobby");
	if (refresh != NULL) {
		network::send_data(initial_response_, sock);
	}
}

void server::process_data_from_player_in_game(const network::connection sock, config& data, config& gamelist)
{
	LOG_SERVER << "in process_data_from_player_in_game...\n";

	const player_map::iterator pl = players_.find(sock);
	if(pl == players_.end()) {
		LOG_SERVER << "ERROR: Could not find player socket.\n";
		return;
	}

	std::vector<game>::iterator g;
	for(g = games_.begin(); g != games_.end(); ++g) {
		if(g->is_member(sock))
			break;
	}

	if(g == games_.end()) {
		LOG_SERVER << "ERROR: Could not find game for player: " << pl->second.name() << "\n";
		return;
	}
	// If this is data describing the level for a game.
	if(g->is_owner(sock) && data.child("side") != NULL) {

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
			g->level().values["mp_countdown_reservoir_time"] = data["mp_countdown_reservoir_time"];
			g->level().values["mp_countdown_action_bonus"] = data["mp_countdown_action_bonus"];

			//update our config object which describes the
			//open games, and notifies the game of where its description
			//is located at
			config& desc = gamelist.add_child("game",g->level());
			g->set_description(&desc);
			desc["hash"] = data["hash"];

			//record the full description of the scenario in g->level()
			g->level() = data;
			g->update_side_data();
			g->describe_slots();

			WRN_SERVER << pl->second.name() << " (" << network::ip_address(sock) << ") created game: \""
				<< (g->description() ? (*g->description())["name"] : "Warning: Game has no description.")
				<< "\" (" << g->id() << ").\n";

			// Send all players in the lobby the update to the list of games
			lobby_players_.send_data(sync_initial_response());
		} else {

			//we've already initialized this scenario, but clobber its old
			//contents with the new ones given here
			g->level() = data;
			g->update_side_data();

			WRN_SERVER << pl->second.name() << " (" << network::ip_address(sock) << ") advanced game: \""
				<< (g->description() ? (*g->description())["name"] : "Warning: Game has no description.")
				<< "\" (" << g->id() << ") to the next scenario.\n";

			// Send all players in the lobby the update to the list of games
			lobby_players_.send_data(sync_initial_response());
		}

		//send all players in the level, except the sender, the new data
		g->send_data(data,sock);
		return;
	}

	const std::string game_name = g->description() ? (*g->description())["name"] : "Warning: Game has no description.";

	// If this is data telling us that the scenario did change.
	if(g->is_owner(sock) && data.child("next_scenario") != NULL) {
		config* scenario = data.child("next_scenario");

		if(g->level_init()) {
			g->level() = *scenario;
			g->reset_history();
			g->update_side_data();
		} else {
			// next_scenario sent while the scenario was not
			// initialized. Something's broken here.
			WRN_SERVER << "Warning: " << pl->second.name() << " ("
				<< network::ip_address(sock)
				<< ") sent [next_scenario] in game: \""
				<< game_name << "\" (" << g->id()
				<< ") while the scenario is not yet initialized";
			return;
		}
	}

	const string_map::const_iterator side = data.values.find("side");
	if(side != data.values.end()) {
		const bool res = g->take_side(sock,data);
		config response;
		if(res) {
			LOG_SERVER << pl->second.name() << " ("
				<< network::ip_address(sock) << ") joined a side in game: "
				<< game_name << "\" (" << g->id() << ").\n";
			response["side_secured"] = side->second;

			//update the number of available slots
			const bool res = g->describe_slots();
			if(res) {
				lobby_players_.send_data(sync_initial_response());
			}
		} else if (g->is_observer(sock)) {
			network::send_data(construct_server_message("Sorry " + pl->second.name() + ", someone else entered before you.",*g), sock);
			return;
		} else {
			response["failed"] = "yes";
			LOG_SERVER << "Warning: " << pl->second.name()
				<< " (" << network::ip_address(sock)
				<< ") failed to get a side in game: \""
				<< game_name << "\" (" << g->id() << ").\n";
		}

		network::send_data(response,sock);
		return;
	}

	if(data.child("start_game")) {
		//send notification of the game starting immediately.
		//g->start_game() will send data that assumes the [start_game]
		//message has been sent
		g->send_data(data,sock);

		WRN_SERVER << pl->second.name() << " ("
			<< network::ip_address(sock) << ") started game: \""
			<< game_name << "\" (" << g->id() << ").\n";

		g->start_game();
		lobby_players_.send_data(sync_initial_response());
		return;
	} else if(data.child("leave_game")) {
		const bool needed = g->is_needed(sock);
		const bool obs = g->is_observer(sock);
		g->remove_player(sock);
		g->describe_slots();

		if( (g->nplayers() == 0) || (needed && !g->started()) ) {

			// Tell observers the game is over,
			// because the last player has left
			config cfg;
			cfg.add_child("leave_game");
			g->send_data(cfg);
			WRN_SERVER << pl->second.name() << " ("
				<< network::ip_address(sock) << ") ended game: \""
				<< game_name << "\" (" << g->id() << ").\n";

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
			if(!obs) {
				g->send_data(construct_server_message(pl->second.name() + " has left the game",*g));
				WRN_SERVER << pl->second.name() << " ("
					<< network::ip_address(sock)
					<< ") has left game: \"" << game_name
					<< "\" (" << g->id() << ").\n";
			} else {
				WRN_SERVER << pl->second.name() << " ("
					<< network::ip_address(sock)
					<< ") has left game: \"" << game_name
					<< "\" (" << g->id() << ") as an observer.\n";
			}
			if (needed) {
				//transfer game control to another player
				const player* player = g->transfer_game_control();
				if (player != NULL) {
					const config& msg = construct_server_message(player->name() + " has been chosen as new host", *g);
					g->send_data(msg);
				}
			}
		}
		//mark the player as available in the lobby
		pl->second.mark_available(true,"");
		lobby_players_.add_player(sock);

		//send the player who has quit the game list
		network::send_data(initial_response_,sock);

		//send all other players in the lobby the update to the lobby
		lobby_players_.send_data(sync_initial_response(),sock);

		return;
	} else if(data.child("side_secured")) {
		return;
	} else if(data.child("failed")) {
		return;
	}

	// If this is data describing changes to a game.
	else if(g->is_owner(sock) && data.child("scenario_diff")) {
		g->level().apply_diff(*data.child("scenario_diff"));
		config* cfg_change = data.child("scenario_diff")->child("change_child");
		if ((cfg_change != NULL) && (cfg_change->child("side") != NULL)) {
			g->update_side_data();
		}

		const bool lobby_changes = g->describe_slots();
		if (lobby_changes) {
			lobby_players_.send_data(sync_initial_response());
		}
	}

	// If info is being provided about the game state
	if(data.child("info") != NULL) {
		const config& info = *data.child("info");
		if(info["type"] == "termination") {
			g->set_termination_reason(info["condition"]);
		}
	}

	// If the owner is changing the controller for a side
	if (data.child("change_controller") != NULL) {
		const config& change = *data.child("change_controller");
		//the player is either host of the game or gives away his own side
		if(g->is_owner(sock) || change["own_side"] == "yes") {
			const std::string& result = g->transfer_side_control(change);
			if(result == "") {
				g->send_data(construct_server_message(change["player"] + " takes control of side " + change["side"], *g));
			} else {
				network::send_data(construct_server_message(result,*g), sock);
			}
			const bool lobby_changes = g->describe_slots();
			if (lobby_changes) {
				lobby_players_.send_data(sync_initial_response());
			}
			return;
		}
	}

	//if all observers are muted
	if (g->is_owner(sock) && data.child("muteall") != NULL) {
		if (!g->all_observers_muted()) {
			g->mute_all_observers(true);
			g->send_data(construct_server_message("all observers have been muted", *g));
		}
		else{
			g->mute_all_observers(false);
			g->send_data(construct_server_message("mute of all observers is removed", *g));
		}
	}

	//if an observer is muted
	if(g->is_owner(sock) && data.child("mute") != NULL) {
		const config& u = *data.child("mute");
		std::string name = u["username"];
		std::string lower_name;
		lower_name.resize(name.size());
		std::transform(name.begin(), name.end(), lower_name.begin(), tolower);

		if (!name.empty()) {
			player_map::iterator pl;
			for(pl = players_.begin(); pl != players_.end(); ++pl) {
				if(pl->second.name() == name) {
					break;
				}
			}
			if(pl->first != sock && pl != players_.end() && g->is_observer(pl->first)) {
				const player* player = g->mute_observer(pl->first);
				if (player != NULL) {
					network::send_data(construct_server_message("You have been muted", *g), pl->first);
					g->send_data(construct_server_message(pl->second.name() + " has been muted", *g), pl->first);
				}
			}
		}
		else {
			std::string muted_nicks = "";
			user_vector users = g->all_game_users();
			const player* player;

			for (user_vector::const_iterator user = users.begin(); user != users.end(); user++) {
				if ((g->all_observers_muted() && g->is_observer(*user))
					|| (!g->all_observers_muted() && g->is_muted_observer(*user))) {
					player = g->find_player(*user);
					if (player != NULL) {
						if (muted_nicks != "") { muted_nicks += ", "; }
						muted_nicks += player->name();
					}
				}
			}
			g->send_data(construct_server_message("muted observers: " + muted_nicks, *g));
		}
	}

	// The owner is kicking/banning someone from the game.
	if(g->is_owner(sock) && (data.child("ban") != NULL || data.child("kick") != NULL)) {
		std::string name;
		bool ban = false;
		if (data.child("ban") != NULL) {
			const config& u = *data.child("ban");
			name = u["username"];
			ban = true;
		} else if (data.child("kick") != NULL) {
			const config& u = *data.child("kick");
			name = u["username"];
			ban = false;
		}

		std::string owner = pl->second.name();

		// new player_map iterator that masks the one of the sender
		player_map::iterator pl;
		for(pl = players_.begin(); pl != players_.end(); ++pl) {
			if(pl->second.name() == name) {
				break;
			}
		}

		if(pl->first != sock && pl != players_.end()) {
			if (ban) {
				network::send_data(construct_server_message("You have been banned", *g), pl->first);
				g->send_data(construct_server_message(name + " has been banned", *g));
				g->ban_player(pl->first);
				WRN_SERVER << owner << " ("
					<< network::ip_address(sock)
					<< ") banned: " << name << " from game: "
					<< game_name << "\" (" << g->id() << ")\n";
			} else {
				network::send_data(construct_server_message("You have been kicked", *g), pl->first);
				g->send_data(construct_server_message(name + " has been kicked", *g));
				g->remove_player(pl->first);
				WRN_SERVER << owner << " ("
					<< network::ip_address(sock)
					<< ") kicked: " << name << " from game: \""
					<< game_name << "\" (" << g->id() << ")\n";
			}

			config leave_game;
			leave_game.add_child("leave_game");
			network::send_data(leave_game, pl->first);

			g->describe_slots();
			lobby_players_.add_player(pl->first);

			//mark the player as available in the lobby
			pl->second.mark_available(true,"");

			//send the player who was banned the lobby game list
			network::send_data(initial_response_, pl->first);

			//send all other players in the lobby the update to the lobby
			lobby_players_.send_data(sync_initial_response(), sock);
		} else if(pl == players_.end()) {
			network::send_data(construct_server_message("Kick/ban failed: user '" + name + "' not found", *g), sock);
		}
	} else if(data.child("ban")) {
		const config& response = construct_server_message(
		             "You cannot ban: not the game host", *g);
		network::send_data(response,sock);
	} else if(data.child("kick")) {
		const config& response = construct_server_message(
		             "You cannot kick: not the game host", *g);
		network::send_data(response,sock);
	}

	config* const turn = data.child("turn");
	if(turn != NULL) {
		g->filter_commands(sock, *turn);

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
			if ((g->all_observers_muted() && g->is_observer(sock)) || g->is_muted_observer(sock)) {
				network::send_data(construct_server_message("You have been muted, others can't see your message!", *g), pl->first);
				return;
			}
			truncate_message((*speak)["message"]);

			//force the description to be correct to prevent
			//spoofing of messages
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
			g->send_data_team(data, team_name, sock);
			return;
		}

		//at the moment, if private messages are mixed in with other
		//data, then let them go through. It's exceedingly unlikely that
		//this will happen anyway, and if it does, the client should
		//respect not displaying the message.
	}

	//forward data to all players who are in the game,
	//except for the original data sender
	g->send_data(data, sock);

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
	const config::child_list::iterator g = std::find(vg.first, vg.second, i->description());
	if(g != vg.second) {
		const size_t index = g - vg.first;
		gamelist->remove_child("game", index);
	}

	i->disconnect();
	games_.erase(i);
}

int main(int argc, char** argv)
{
	int port = 15000;
	size_t nthreads = 5;

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
					ERR_SERVER << "WARNING: errors reading configuration file: " << errors << "\n";
				}
			} catch(config::error& e) {
				ERR_SERVER << "ERROR: could not read configuration file: '" << e.message << "'\n";
				return -1;
			}
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
			ERR_SERVER << "Running as a daemon is not supported on this platform\n";
			return -1;
#else
			const pid_t pid = fork();
			if(pid < 0) {
				ERR_SERVER << "Could not fork and run as a daemon\n";
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
			ERR_SERVER << "unknown option: " << val << "\n";
			return 0;
		} else {
			port = atoi(argv[arg]);
		}
	}
	// show 'warnings' by default
	lg::set_log_domain_severity("general", 1);
	lg::timestamps(true);

	input_stream input(fifo_path);

	try {
		server(port, input, configuration, nthreads).run();
	} catch(network::error& e) {
		ERR_SERVER << "caught network error while server was running. aborting.: " << e.message << "\n";
		return -1;
	}

	return 0;
}
