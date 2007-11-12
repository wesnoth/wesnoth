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

//! @file server/server.cpp
//! Wesnoth-Server, for multiplayer-games.

#include "../global.hpp"

#include "../config.hpp"
#include "../game_config.hpp"
#include "../log.hpp"
#include "../network.hpp"
#include "../filesystem.hpp"
#include "../wassert.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/string_utils.hpp"

#include "game.hpp"
#include "input_stream.hpp"
#include "metrics.hpp"
#include "player.hpp"
#include "proxy.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <csignal>

//! fatal and directly server related errors/warnings,
//! ie not caused by erroneous client data
#define ERR_SERVER LOG_STREAM(err, mp_server)
//! clients send wrong/unexpected data
#define WRN_SERVER LOG_STREAM(warn, mp_server)
//! normal events
#define LOG_SERVER LOG_STREAM(info, mp_server)
#define DBG_SERVER LOG_STREAM(debug, mp_server)
#define ERR_CONFIG LOG_STREAM(err, config)
#define WRN_CONFIG LOG_STREAM(warn, config)

//compatibility code for MS compilers
#ifndef SIGHUP
#define SIGHUP 20
#endif

sig_atomic_t config_reload = 0;

void reload_config(int signal) {
	wassert(signal == SIGHUP);
	config_reload = 1;
}

namespace {

config construct_error(const std::string& msg) {
	config cfg;
	cfg.add_child("error")["message"] = msg;
	return cfg;
}

config construct_server_message(const std::string& message, const game& g) {
	config turn;
	if (g.started()) {
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

void truncate_message(t_string& str) {
	const size_t max_message_length = 256;
	// The string send can contain utf-8 so truncate as wide_string otherwise
	// an corrupted utf-8 string can be returned.
	std::string tmp = str.str();
	utils::truncate_as_wstring(tmp, max_message_length);
	str = tmp;
}

} // end anon namespace

class server
{
public:
	server(int port, input_stream& input, const std::string& config_file, size_t min_threads,size_t max_threads);
	void run();
private:
	const network::manager net_manager_;
	const network::server_manager server_;

	//! std::map<network::connection,player>
	player_map players_;
	std::vector<game> games_;
	game not_logged_in_;
	//! The lobby is implemented as a game.
	game lobby_players_;

	//! server socket/fifo
	input_stream& input_;

	const std::string config_file_;
	config cfg_;
	//! Read the server config from file 'config_file_'.
	config read_config() const;
	
	// settings from the server config
	std::set<std::string> accepted_versions_;
	std::map<std::string,config> redirected_versions_;
	std::map<std::string,config> proxy_versions_;
	std::vector<std::string> disallowed_names_;
	std::string admin_passwd_;
	std::set<network::connection> admins_;
	std::string motd_;
	size_t default_max_messages_;
	size_t default_time_period_;
	size_t concurrent_connections_;
	//! Parse the server config into local variables.
	void load_config();
	
	bool ip_exceeds_connection_limit(const std::string& ip) const;
	bool is_ip_banned(const std::string& ip) const;
	std::vector<std::string> bans_;

	const config version_query_response_;
	const config login_response_;
	const config join_lobby_response_;
	config games_and_users_list_;
	config old_games_and_users_list_;
	config games_and_users_list_diff();

	metrics metrics_;

	time_t last_stats_;
	void dump_stats(const time_t& now);

	void process_data(const network::connection sock, config& data);
	void process_login(const network::connection sock, const config& data);
	//! Handle queries from clients.
	void process_query(const network::connection sock, const config& query);
	//! Process commands from admins and users.
	std::string process_command(const std::string& cmd);
	//! Handle private messages between players.
	void process_whisper(const network::connection sock, const config& whisper) const;
	void process_data_from_player_in_lobby(const network::connection sock, config& data);
	void process_data_from_player_in_game(const network::connection sock, config& data);
	void delete_game(std::vector<game>::iterator game_it);
};

server::server(int port, input_stream& input, const std::string& config_file, size_t min_threads,size_t max_threads)
	: net_manager_(min_threads,max_threads), server_(port), not_logged_in_(players_),
	lobby_players_(players_), input_(input), config_file_(config_file),
	cfg_(read_config()), version_query_response_("version"),
	login_response_("mustlogin"), join_lobby_response_("join_lobby"),
	games_and_users_list_("gamelist"), old_games_and_users_list_(games_and_users_list_),
	last_stats_(time(NULL))
{
	load_config();
	signal(SIGHUP, reload_config);
}

config server::read_config() const {
	config configuration;
	if (config_file_ == "") return configuration;
	scoped_istream stream = istream_file(config_file_);
	std::string errors;
	try {
		read(configuration, *stream, &errors);
		if (errors.empty()) {
			WRN_CONFIG << "Server configuration from file: '" << config_file_
				<< "' read.\n";
		} else {
			ERR_CONFIG << "ERROR: Errors reading configuration file: '"
				<< errors << "'.\n";
		}
	} catch(config::error& e) {
		ERR_CONFIG << "ERROR: Could not read configuration file: '"
			<< config_file_ << "': '" << e.message << "'.\n";
	}
	return configuration;
}

void server::load_config() {
	admin_passwd_ = cfg_["passwd"];
	motd_ = cfg_["motd"];
	if (cfg_["disallow_names"] == "") {
		disallowed_names_.push_back("*admin*");
		disallowed_names_.push_back("*admln*");
		disallowed_names_.push_back("*server*");
		disallowed_names_.push_back("player");
		disallowed_names_.push_back("network");
		disallowed_names_.push_back("human");
		disallowed_names_.push_back("computer");
		disallowed_names_.push_back("ai");
		disallowed_names_.push_back("ai?");
	} else {
		disallowed_names_ = utils::split(cfg_["disallow_names"]);
	}
	default_max_messages_ = lexical_cast_default<int>(cfg_["max_messages"],4);
	default_time_period_ = lexical_cast_default<int>(cfg_["messages_time_period"],10);
	concurrent_connections_ = lexical_cast_default<int>(cfg_["connections_allowed"],5);

	const std::string& versions = cfg_["versions_accepted"];
	if (versions.empty() == false) {
		const std::vector<std::string> accepted(utils::split(versions));
		for (std::vector<std::string>::const_iterator i = accepted.begin(); i != accepted.end(); ++i) {
			accepted_versions_.insert(*i);
		}
	} else {
		accepted_versions_.insert(game_config::version);
		accepted_versions_.insert("test");
	}

	const config::child_list& redirects = cfg_.get_children("redirect");
	for (config::child_list::const_iterator i = redirects.begin(); i != redirects.end(); ++i) {
		const std::vector<std::string> versions(utils::split((**i)["version"]));
		for (std::vector<std::string>::const_iterator j = versions.begin(); j != versions.end(); ++j) {
			redirected_versions_[*j] = **i;
		}
	}

	const config::child_list& proxies = cfg_.get_children("proxy");
	for (config::child_list::const_iterator p = proxies.begin(); p != proxies.end(); ++p) {
		const std::vector<std::string> versions(utils::split((**p)["version"]));
		for (std::vector<std::string>::const_iterator j = versions.begin(); j != versions.end(); ++j) {
			proxy_versions_[*j] = **p;
		}
	}
}

bool server::ip_exceeds_connection_limit(const std::string& ip) const {
	size_t connections = 0;
	for (player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if (network::ip_address(i->first) == ip) {
			++connections;
		}
	}

	return connections > concurrent_connections_;
}

bool server::is_ip_banned(const std::string& ip) const {
	for (std::vector<std::string>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
		DBG_SERVER << "comparing ban '" << *i << "' vs '" << ip << "'\t";
		if (utils::wildcard_string_match(ip,*i)) {
			DBG_SERVER << "banned\n";
			return true;
		}
		DBG_SERVER << "not banned\n";
	}
	return false;
}

config server::games_and_users_list_diff() {
	config res;
	res.add_child("gamelist_diff",games_and_users_list_.get_diff(old_games_and_users_list_));
	old_games_and_users_list_ = games_and_users_list_;
	return res;
}

void server::dump_stats(const time_t& now) {
//	time_t old_stats = last_stats_;
	last_stats_ = now;
	LOG_SERVER << "Statistics:\n"
		"\tnumber_of_games   = " << games_.size() << "\n"
		"\tnumber_of_players = " << players_.size() << "\n"
		"\tlobby_players     = " << lobby_players_.nobservers() << "\n";
//		"\tstart_interval    = " << old_stats << "\n"
//		"\tend_interval      = " << last_stats_ << "\n";
}

void server::run() {
	bool sync_scheduled = false;
	for (int loop = 0;; ++loop) {
		SDL_Delay(20);
		try {
			if (sync_scheduled) {
				// Send all players the information
				// that a player has logged out of the system
				lobby_players_.send_data(games_and_users_list_diff());
				sync_scheduled = false;
			}
			
			if (config_reload == 1) {
				cfg_ = read_config();
				load_config();
				config_reload = 0;
			}
			// Process commands from the server socket/fifo
			std::string admin_cmd;
			if (input_.read_line(admin_cmd)) {
				LOG_SERVER << process_command(admin_cmd) << std::endl;
			}

			// Make sure we log stats every 5 minutes
			time_t now = time(NULL);
			if ((loop%100) == 0 && last_stats_+5*60 < now) {
				dump_stats(now);
				// send a 'ping' to all players to detect ghosts
				config ping;
				ping["ping"] = lexical_cast<std::string>(now);
				for (player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
					network::queue_data(ping, i->first);
				}
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if (sock) {
				const std::string& ip = network::ip_address(sock);
				if (is_ip_banned(ip)) {
					LOG_SERVER << ip << "\trejected banned user.\n";
					network::send_data(construct_error("You are banned."),sock);
					network::disconnect(sock);
				} else if (ip_exceeds_connection_limit(ip)) {
					LOG_SERVER << ip << "\trejected ip due to excessive connections\n";
					network::send_data(construct_error("Too many connections from your IP."),sock);
					network::disconnect(sock);
				} else {
					DBG_SERVER << ip << "\tnew connection accepted. (socket: "
						<< sock << ")\n";
					network::send_data(version_query_response_,sock);
					not_logged_in_.add_player(sock, true);
				}
			}

			config data;
			while ((sock = network::receive_data(data)) != network::null_connection) {
				metrics_.service_request();
				process_data(sock, data);
			}

			metrics_.no_requests();

		} catch(config::error& e) {
			WRN_CONFIG << "Warning: error in received data: " << e.message << "\n";
		} catch(network::error& e) {
			if (!e.socket) {
				// "Could not send initial handshake" really fatal?
				ERR_SERVER << "fatal network error: " << e.message << "\n";
				exit(1);
				throw;
				break;
			}
			DBG_SERVER << "socket closed: " << e.message << "\n";
			const std::string ip = network::ip_address(e.socket);
			if (proxy::is_proxy(e.socket)) {
				LOG_SERVER << ip << "\tProxy user disconnected.\n";
				proxy::disconnect(e.socket);
				e.disconnect();
				DBG_SERVER << "done closing socket...\n";
				continue;
			}
			// Was the user already logged in?
			const player_map::iterator pl_it = players_.find(e.socket);
			if (pl_it == players_.end()) {
				if (not_logged_in_.is_observer(e.socket)) {
					DBG_SERVER << ip << "\tNot logged in user disconnected.\n";
					not_logged_in_.remove_player(e.socket);
				} else {
					WRN_SERVER << ip << "\tWarning: User disconnected right after the connection was accepted.\n";
				}
				e.disconnect();
				DBG_SERVER << "done closing socket...\n";
				continue;
			}
			const config::child_list& users = games_and_users_list_.get_children("user");
			const size_t index = std::find(users.begin(), users.end(), pl_it->second.config_address()) - users.begin();
			if (index < users.size()) {
				games_and_users_list_.remove_child("user",index);
			} else {
				ERR_SERVER << ip << "ERROR: Could not find user: "
					<< pl_it->second.name() << " in games_and_users_list_.\n";
			}
			sync_scheduled = true;
			// Was the player in the lobby or a game?
			if (lobby_players_.is_observer(e.socket)) {
				lobby_players_.remove_player(e.socket);
				LOG_SERVER << ip << "\t" << pl_it->second.name() << "\thas logged off.\n";
			} else {
				for (std::vector<game>::iterator g = games_.begin();
					g != games_.end(); ++g)
				{
					if (!g->is_member(e.socket)) {
						continue;
					}
					const bool host = g->is_owner(e.socket);
					const bool obs = g->is_observer(e.socket);
					g->remove_player(e.socket);
					g->describe_slots();
					// Did the last player leave?
					if ( (g->nplayers() == 0) || (host && !g->started()) ) {
						LOG_SERVER << ip << "\t" << pl_it->second.name()
							<< "\tended game:\t\"" << g->name() << "\" ("
							<< g->id() << ") and disconnected.\n";
						delete_game(g);
						break;
					}
					if (obs) {
						LOG_SERVER << ip << "\t" << pl_it->second.name()
							<< "\thas left game:\t\"" << g->name() << "\" ("
							<< g->id() << ") as an observer and disconnected.\n";
					} else {
						g->send_data(construct_server_message(pl_it->second.name() + " has disconnected",*g));
						LOG_SERVER << ip << "\t" << pl_it->second.name()
							<< "\thas left game:\t\"" << g->name() << "\" ("
							<< g->id() << ") and disconnected.\n";
					}
					break;
				}
			}
			players_.erase(pl_it);
			e.disconnect();
			DBG_SERVER << "done closing socket...\n";
		}
	}
}

void server::process_data(const network::connection sock, config& data) {
	if (proxy::is_proxy(sock)) {
		proxy::received_data(sock, data);
	}
	// If someone who is not yet logged in
	// is sending login details
	else if (not_logged_in_.is_observer(sock)) {
		process_login(sock, data);
	} else if (const config* query = data.child("query")) {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_query(sock, *query);
	} else if (const config* whisper = data.child("whisper")) {
		process_whisper(sock, *whisper);
	} else if (lobby_players_.is_observer(sock)) {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_data_from_player_in_lobby(sock, data);
	} else {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_data_from_player_in_game(sock, data);
	}
}


void server::process_login(const network::connection sock, const config& data) {
	// See if the client is sending their version number.
	if (const config* const version = data.child("version")) {
		const std::string& version_str = (*version)["version"];
		std::set<std::string>::const_iterator accepted_it;
		// Check if it is an accepted version.
		for (accepted_it = accepted_versions_.begin();
			accepted_it != accepted_versions_.end(); ++accepted_it) {
			if (utils::wildcard_string_match(version_str,*accepted_it)) break;
		}
		if (accepted_it != accepted_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using accepted version " << version_str
				<< ":\ttelling them to log in.\n";
			network::send_data(login_response_,sock);
			return;
		}
		std::map<std::string,config>::const_iterator config_it;
		// Check if it is a redirected version
		for (config_it = redirected_versions_.begin();
			config_it != redirected_versions_.end(); ++config_it)
		{
			if (utils::wildcard_string_match(version_str,config_it->first))
				break;
		}
		if (config_it != redirected_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using version " << version_str
				<< ":\tredirecting them to " << config_it->second["host"]
				<< ":" << config_it->second["port"] << "\n";
			config response;
			response.add_child("redirect",config_it->second);
			network::send_data(response,sock);
			return;
		}
		// Check if it's a version we should start a proxy for.
		for (config_it = proxy_versions_.begin();
			config_it != proxy_versions_.end(); ++config_it)
		{
			if (utils::wildcard_string_match(version_str,config_it->first))
				break;
		}
		if (config_it != proxy_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using version " << version_str
				<< ":\tconnecting them by proxy to " << config_it->second["host"]
				<< ":" << config_it->second["port"] << "\n";
			proxy::create_proxy(sock,config_it->second["host"],
				lexical_cast_default<int>(config_it->second["port"],15000));
			return;
		}
		// No match, send a response and reject them.
		LOG_SERVER << network::ip_address(sock)
			<< "\tplayer joined using unknown version " << version_str
			<< ":\trejecting them\n";
		config response;
		if (!accepted_versions_.empty()) {
			response["version"] = *accepted_versions_.begin();
		} else if (redirected_versions_.empty() == false) {
			response["version"] = redirected_versions_.begin()->first;
		} else {
			ERR_SERVER << "ERROR: This server doesn't accept any versions at all.\n";
			response["version"] = "null";
		}
		network::send_data(response,sock);
		return;
	}
	const config* const login = data.child("login");
	// Client must send a login first.
	if (login == NULL) {
		network::send_data(construct_error("You must login first"),sock);
		return;
	}
	// Check if the username is valid (all alpha-numeric plus underscore and hyphen)
	std::string username = (*login)["username"];
	if (!utils::isvalid_username(username)) {
		network::send_data(construct_error("This username contains invalid "
			"characters. Only alpha-numeric characters, underscores and hyphens"
			"are allowed."), sock);
		return;
	}
	if (username.size() > 18) {
		network::send_data(construct_error("This username is too long"),sock);
		return;
	}
	// Check if the uername is allowed.
	for (std::vector<std::string>::const_iterator d_it = disallowed_names_.begin();
		d_it != disallowed_names_.end(); ++d_it)
	{
		if (utils::wildcard_string_match(utils::lowercase(username),
			utils::lowercase(*d_it)))
		{
			network::send_data(construct_error("The nick '" + username
				+ "' is reserved and can not be used by players"), sock);
			return;
		}
	}
	// Check the username isn't already taken
	player_map::const_iterator p;
	for (p = players_.begin(); p != players_.end(); ++p) {
		if (p->second.name() == username) {
			network::send_data(construct_error("This username is already taken"), sock);
			return;
		}
	}
	network::send_data(join_lobby_response_, sock);

	config* const player_cfg = &games_and_users_list_.add_child("user");
	const player new_player(username, *player_cfg, default_max_messages_,
		default_time_period_);
	players_.insert(std::pair<network::connection,player>(sock, new_player));

	not_logged_in_.remove_player(sock);
	lobby_players_.add_player(sock, true);

	// Send the new player the entire list of games and players
	network::send_data(games_and_users_list_,sock);

	if (motd_ != "") {
		network::send_data(construct_server_message(motd_,lobby_players_),sock);
	}

	// Send other players in the lobby the update that the player has joined
	lobby_players_.send_data(games_and_users_list_diff(),sock);

	LOG_SERVER << network::ip_address(sock) << "\t" << username
		<< "\thas logged on. (socket: " << sock << ")\n";

	for (std::vector<game>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
		g->send_data_observers(construct_server_message(username
			+ " has logged into the lobby",*g));
	}
}

void server::process_query(const network::connection sock, const config& query) {
	const player_map::const_iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		DBG_SERVER << "ERROR: Could not find player with socket: " << sock << "\n";
		return;
	}
	std::string command(query["type"]);
	std::ostringstream response;
	if (command == "status" && admins_.count(sock) == 0) {
		command += " " + pl->second.name();
	}
	if (command.empty()) {
	// commands a player may issue
	} else if (command == "metrics" || command == "motd"
		|| command == "status " + pl->second.name()) {
		response << process_command(command);
	} else if (admin_passwd_.empty() == false && command == admin_passwd_) {
		admins_.insert(sock);
		response << "You are now recognized as an administrator";
		LOG_SERVER << "New Admin recognized:" << "\tIP: "
			<< network::ip_address(sock) << "\tnick: "
			<< pl->second.name() << std::endl;
	} else if (admins_.count(sock) != 0) {
		response << process_command(command);
		LOG_SERVER << "Admin Command:" << "\ttype: " << command
			<< "\tIP: "<< network::ip_address(sock) 
			<< "\tnick: "<< pl->second.name() << std::endl;
	} else if (admin_passwd_.empty() == false) {
		WRN_SERVER << "FAILED Admin attempt:" << "\tIP: "
			<< network::ip_address(sock) << "\tnick: "
			<< pl->second.name() << std::endl;
		response << "Error: unrecognized query";
	} else {
		response << "Error: unrecognized query";
	}
	network::send_data(construct_server_message(response.str(),lobby_players_),sock);
}

std::string server::process_command(const std::string& query) {
	std::ostringstream out;
	const std::string::const_iterator i = std::find(query.begin(),query.end(),' ');
	const std::string command(query.begin(),i);
	std::string parameters = (i == query.end() ? "" : std::string(i+1,query.end()));
	utils::strip(parameters);
	if (command == "msg" || command == "lobbymsg") {
		if (parameters == "") {
			return "You must type a message.";
		}
		lobby_players_.send_data(construct_server_message(parameters, lobby_players_));
		if (command == "msg") {
			for (std::vector<game>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
				g->send_data(construct_server_message(parameters, *g));
			}
		}
		out << "message '" << parameters << "' relayed to players\n";
	} else if (command == "status") {
		out << "STATUS REPORT\n";
		for (player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
			if (parameters == "" || utils::wildcard_string_match(pl->second.name(), parameters)) {
				const network::connection_stats& stats = network::get_connection_stats(pl->first);
				const int time_connected = stats.time_connected/1000;
				const int seconds = time_connected%60;
				const int minutes = (time_connected/60)%60;
				const int hours = time_connected/(60*60);
				out << "'" << pl->second.name() << "' @ " << network::ip_address(pl->first)
					<< " connected for " << hours << ":" << minutes << ":" << seconds
					<< " sent " << stats.bytes_sent << " bytes, received "
					<< stats.bytes_received << " bytes\n";
			}
		}
	} else if (command == "metrics") {
		out << metrics_;
	} else if (command == "ban" || command == "bans" || command == "kban") {
		if (parameters == "") {
			if (bans_.empty()) return "No bans set.";
			out << "BAN LIST\n";
			for (std::vector<std::string>::const_iterator i = bans_.begin();
				i != bans_.end(); ++i)
			{
				out << *i << "\n";
			}
		} else {
			bool banned = false;
			// if we find 3 '.' consider it an ip mask
			if (std::count(parameters.begin(), parameters.end(), '.') == 3) {
				banned = true;
				out << "Set ban on '" << parameters << "'\n";
				bans_.push_back(parameters);
				if (command == "kban") {
					for (player_map::const_iterator pl = players_.begin();
						pl != players_.end(); ++pl)
					{
						if (utils::wildcard_string_match(network::ip_address(pl->first), parameters)) {
							network::queue_disconnect(pl->first);
							out << "Kicked " << pl->second.name() << ".\n";
						}
					}
				}
			} else {
				for (player_map::const_iterator pl = players_.begin();
					pl != players_.end(); ++pl)
				{
					if (utils::wildcard_string_match(pl->second.name(), parameters)) {
						banned = true;
						const std::string& ip = network::ip_address(pl->first);
						if (!is_ip_banned(ip)) {
							bans_.push_back(ip);
							out << "Set ban on '" << ip << "'.\n";
						}
						if (command == "kban") {
							network::queue_disconnect(pl->first);
							out << "Kicked " << pl->second.name() << ".\n";
						}
					}
				}
				if (!banned) {
					out << "Nickmask '" << parameters << "'did not match, no bans set.";
				}
			}
		}
	} else if (command == "unban") {
		if (parameters == "") {
			return "You must enter an ipmask to unban.";
		}
		const std::vector<std::string>::iterator itor = 
			std::remove(bans_.begin(), bans_.end(), parameters);
		if (itor == bans_.end()) {
			out << "There is no ban on '" << parameters << "'.";
		} else {
			bans_.erase(itor, bans_.end());
			out << "Ban on '" << parameters << "' removed.";
		}
	} else if (command == "kick") {
		if (parameters == "") {
			return "You must enter a nickmask to kick.";
		}
		bool kicked = false;
		for (player_map::const_iterator pl = players_.begin();
			pl != players_.end(); ++pl)
		{
			if (utils::wildcard_string_match(pl->second.name(), parameters)) {
				kicked = true;
				const std::string name(pl->second.name());
				const std::string ip(network::ip_address(pl->first));
				network::queue_disconnect(pl->first);
				out << "Kicked " << name << " (" << ip << ").\n";
			}
		}
		if (!kicked) out << "No user matched '" << parameters << "'.\n";
	} else if (command == "motd") {
		if (parameters == "") {
			if (motd_ != "") {
				out << "Message of the day: " << motd_;
				return out.str();
			} else {
				return "No message of the day set.";
			}
		}
		motd_ = parameters;
		out << "Message of the day set to: " << motd_;
	} else {
		out << "Command '" << command << "' is not recognized.\n";
		out << "Available commands are: (lobby)msg <message>, motd [<message>]"
			", status [<nickmask>], metrics, (k)ban(s) [<mask>], unban <ipmask>"
			", kick <nickmask>";
	}

	return out.str();
}

void server::process_whisper(const network::connection sock, const config& whisper) const {
	const player_map::const_iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		DBG_SERVER << "ERROR: Could not find player socket.\n";
		return;
	}
	bool sent = false;
	bool do_send = true;
	std::vector<game>::const_iterator g;
	if ((whisper["receiver"]!="") && (whisper["message"]!="") && (whisper["sender"]!="")) {
		for (player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
			if (i->second.name() == whisper["receiver"]) {
				for (g = games_.begin(); g != games_.end(); ++g) {
					if (g->is_player(i->first)) {
						do_send = false;
						break;
					}
				}
				if (do_send == false) {
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
		if (do_send == false) {
			msg["message"] = "You cannot send private messages to players in a game.";
		} else {
			msg["message"] = "Can't find player "+whisper["receiver"];
		}
		msg["sender"] = "server";
		data.add_child("message", msg);
		network::send_data(data, sock);
	}
}

void server::process_data_from_player_in_lobby(const network::connection sock, config& data) {
	DBG_SERVER << "in process_data_from_player_in_lobby...\n";

	const player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find player in players_. (socket: "
			<< sock << ")\n";
		return;
	}

	if (data.child("create_game") != NULL) {
		const std::string game_name = (*data.child("create_game"))["name"];
		DBG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tcreates a new game: " << game_name << ".\n";
		// Create the new game, remove the player from the lobby
		// and set the player as the host/owner.
		games_.push_back(game(players_, sock, game_name));
		game& g = games_.back();
		g.level() = (*data.child("create_game"));

		lobby_players_.remove_player(sock);
		lobby_players_.send_data(games_and_users_list_diff());
		return;
	}

	// See if the player is joining a game
	const config* const join = data.child("join");
	if (join != NULL) {
		const std::string& id = (*join)["id"];
		const bool observer = ((*join)["observe"] == "yes");
		const int game_id = lexical_cast<int>(id);
		const std::vector<game>::iterator g =
			std::find_if(games_.begin(),games_.end(), game_id_matches(game_id));
		if (g == games_.end()) {
			DBG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join unknown game:\t" << id << "\n";
			network::send_data(config("leave_game"),sock);
			return;
		}
		if (g->player_is_banned(sock)) {
			DBG_SERVER << network::ip_address(sock) << "\tReject banned player: "
				<< pl->second.name() << "\tfrom game:\t\"" << g->name()
				<< "\" (" << id << ").\n";
			network::send_data(construct_error("You are banned from this game."), sock);
			return;
		}
		LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tjoined game:\t\"" << g->name()
			<< "\" (" << id << (observer ? ") as an observer.\n" : ").\n");

		lobby_players_.remove_player(sock);
		g->add_player(sock, observer);

		lobby_players_.send_data(games_and_users_list_diff());
	}

	// See if it's a message, in which case we add the name of the sender,
	// and forward it to all players in the lobby
	config* const message = data.child("message");
	if (message != NULL) {
		if (pl->second.silenced()) {
			return;
		} else if (pl->second.is_message_flooding()) {
			network::send_data(construct_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed.",
				lobby_players_),pl->first);
			return;
		}

		(*message)["sender"] = pl->second.name();
		truncate_message((*message)["message"]);

		std::string msg = (*message)["message"].base_str();
		if (msg.substr(0,3) == "/me") {
			LOG_SERVER << network::ip_address(sock) << "\t<"
				<< pl->second.name() << msg.substr(3) << ">\n";
		} else {
			LOG_SERVER << network::ip_address(sock) << "\t<"
				<< pl->second.name() << "> " << msg << "\n";
		}

		lobby_players_.send_data(data,sock);
	}

	// Player requests update of lobby content,
	// for example when cancelling the create game dialog
	config* const refresh = data.child("refresh_lobby");
	if (refresh != NULL) {
		network::send_data(games_and_users_list_, sock);
	}
}

//! Process data sent by a player in a game. Note that 'data' by default gets
//! broadcasted and saved in the replay.
void server::process_data_from_player_in_game(const network::connection sock, config& data) {
	DBG_SERVER << "in process_data_from_player_in_game...\n";
	
	bool push_immediately = true;
	const player_map::const_iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find player in players_. (socket: "
			<< sock << ")\n";
		return;
	}

	std::vector<game>::iterator g;
	for (g = games_.begin(); g != games_.end(); ++g) {
		if (g->is_owner(sock) || g->is_member(sock))
			break;
	}
	if (g == games_.end()) {
		ERR_SERVER << "ERROR: Could not find game for player: "
			<< pl->second.name() << ". (socket:" << sock << ")\n";
		return;
	}

	// If this is data describing the level for a game.
	if (g->is_owner(sock) && data.child("side") != NULL) {

		const bool is_init = g->level_init();

		// If this game is having its level data initialized
		// for the first time, and is ready for players to join.
		// We should currently have a summary of the game in g->level().
		// We want to move this summary to the games_and_users_list_, and
		// place a pointer to that summary in the game's description.
		// g->level() should then receive the full data for the game.
		if (!is_init) {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tcreated game:\t\"" << g->name() << "\" ("
				<< g->id() << ").\n";

			// Update our config object which describes the open games,
			// and notifies the game of where its description is located at
			config* const gamelist = games_and_users_list_.child("gamelist");
			wassert(gamelist != NULL);
			config& desc = gamelist->add_child("game",g->level());
			g->set_description(&desc);
			desc["hash"] = data["hash"];
			// If there is no shroud, then tell players in the lobby
			// what the map looks like
			if (data["mp_shroud"] != "yes") {
				desc["map_data"] = data["map_data"];
				desc["map"] = data["map"];
			}
			desc["observer"] = data["observer"];
			desc["mp_era"] = data.child("era") != NULL ? data.child("era")->get_attribute("id") : "";
			// map id
			desc["mp_scenario"] = data["id"];
			desc["mp_use_map_settings"] = data["mp_use_map_settings"];
			desc["mp_village_gold"] = data["mp_village_gold"];
			desc["mp_fog"] = data["mp_fog"];
			desc["mp_shroud"] = data["mp_shroud"];
			desc["experience_modifier"] = data["experience_modifier"];
			desc["mp_countdown"] = data["mp_countdown"];
			desc["mp_countdown_init_time"] = data["mp_countdown_init_time"];
			desc["mp_countdown_turn_bonus"] = data["mp_countdown_turn_bonus"];
			desc["mp_countdown_reservoir_time"] = data["mp_countdown_reservoir_time"];
			desc["mp_countdown_action_bonus"] = data["mp_countdown_action_bonus"];
			//desc["map_name"] = data["name"];
			//desc["map_description"] = data["description"];
			//desc[""] = data["objectives"];
			//desc[""] = data["random_start_time"];
			//desc[""] = data["turns"];
			//desc["client_version"] = data["version"];

			// Record the full description of the scenario in g->level()
			g->level() = data;
			// Let the owner take a side once we have the level data.
			g->take_side(sock);
			g->update_side_data();
			g->describe_slots();

			// Send all players in the lobby the update to the list of games.
			lobby_players_.send_data(games_and_users_list_diff());
		} else {
			// We've already initialized this scenario, but clobber
			// its old contents with the new ones given here.
			g->level() = data;
			g->update_side_data();
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tadvanced game:\t\"" << g->name() << "\" ("
				<< g->id() << ") to the next scenario.\n";
			// Send the update of the game description to the lobby.
			//lobby_players_.send_data(games_and_users_list_diff());
		}

		// Send the new data to all players in the level (except the sender).
		g->send_data(data,sock);
		return;
	}

	// If this is data telling us that the scenario did change.
	if(g->is_owner(sock) && data.child("store_next_scenario") != NULL) {
		if(g->level_init()) {
			g->level() = (*data.child("store_next_scenario"));
			g->reset_history();
			// Re-assign sides.
			g->update_side_data();
			// Send the update of the game description to the lobby
			lobby_players_.send_data(games_and_users_list_diff());
		} else {
			// next_scenario sent while the scenario was not initialized.
			// Something's broken here.
			WRN_SERVER << network::ip_address(sock) << "\tWarning: "
				<< pl->second.name() << "\tsent [next_scenario] in game:\t\""
				<< g->name() << "\" (" << g->id()
				<< ") while the scenario is not yet initialized.";
			return;
		}

		//Pushing immediately does not comply with linger mode for mp campaigns.
		//Here the clients need to determine by themselves, when to get the data
		//for the next scenario.
		push_immediately = false;
	}

	//! @todo: The player has already joined and got a side or not. This is
	//! pointless.
	const string_map::const_iterator side = data.values.find("side");
	if (side != data.values.end()) {
		const bool res = g->take_side(sock, data);
		config response;
		if (res) {
			DBG_SERVER << network::ip_address(sock) << "\t"
				<< pl->second.name() << "\tjoined a side in game:\t"
				<< g->name() << "\" (" << g->id() << ").\n";
			response["side_secured"] = side->second;
			// Update the number of available slots
			if (g->describe_slots()) {
				lobby_players_.send_data(games_and_users_list_diff());
			}
		} else if (g->is_observer(sock)) {
			response = construct_server_message("Sorry "
				+ pl->second.name() + ", someone else entered before you.",*g);
		} else {
			response["failed"] = "yes";
			DBG_SERVER << "Warning: " << network::ip_address(sock) << "\t"
				<< pl->second.name() << "\tfailed to get a side in game:\t\""
				<< g->name() << "\" (" << g->id() << ").\n";
		}
		network::send_data(response,sock);
		return;
	} else if (data.child("side_secured")) return;
	else if (data.child("failed")) return;
	else if (data.child("error")) return;
	else if (data.child("start_game")) {
		// Send notification of the game starting immediately.
		// g->start_game() will send data that assumes
		// the [start_game] message has been sent
		g->send_data(data,sock);

		LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tstarted game:\t\"" << g->name() << "\" (" << g->id() << ").\n";

		g->start_game();
		lobby_players_.send_data(games_and_users_list_diff());
		return;
	} else if (data.child("leave_game")) {
		const bool host = g->is_owner(sock);
		const bool obs = g->is_observer(sock);
		g->remove_player(sock);
		lobby_players_.add_player(sock, true);
		g->describe_slots();
		if ( (g->nplayers() == 0) || (host && !g->started()) ) {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tended game:\t\"" << g->name() << "\" (" << g->id() << ").\n";
			delete_game(g);
		} else {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\thas left game:\t\"" << g->name() << "\" (" << g->id()
				<< (obs ? ") as an observer.\n" : ").\n");
		}
		// Send the player who has quit the game list
		network::send_data(games_and_users_list_,sock);

		// Send all other players in the lobby the update to the lobby
		lobby_players_.send_data(games_and_users_list_diff(),sock);
		return;
	// If this is data describing side changes (so far only by the host).
	} else if (data.child("scenario_diff")) {
		g->level().apply_diff(*data.child("scenario_diff"));
		config* cfg_change = data.child("scenario_diff")->child("change_child");
		if ((cfg_change != NULL) && (cfg_change->child("side") != NULL)) {
			g->update_side_data();
		}
		if (g->describe_slots()) {
			lobby_players_.send_data(games_and_users_list_diff());
		}
	// If the owner of a side is changing the controller.
	} else if (data.child("change_controller") != NULL) {
		const config& change = *data.child("change_controller");
		g->transfer_side_control(sock, change);
		if (g->describe_slots()) {
			lobby_players_.send_data(games_and_users_list_diff());
		}
		// FIXME: Why not save it in the history_? (if successful)
		return;
	// If all observers should be muted. (toggles)
	} else if (g->is_owner(sock) && data.child("muteall") != NULL) {
		if (g->mute_all_observers()) {
			g->send_data(construct_server_message("All observers have been muted.", *g));
		} else {
			g->send_data(construct_server_message("Mute of all observers has been removed.", *g));
		}
	// If an observer should be muted.
	} else if (g->is_owner(sock) && data.child("mute") != NULL) {
		g->mute_observer(sock, *data.child("mute"));
	// The owner is kicking/banning someone from the game.
	} else if (g->is_owner(sock) && (data.child("ban") || data.child("kick"))) {
		//! @todo All this processing should be done in game.cpp.
		std::string name;
		bool ban = false;
		if (data.child("ban") != NULL) {
			const config& u = *data.child("ban");
			name = u["username"];
			ban = true;
		} else {
			const config& u = *data.child("kick");
			name = u["username"];
			ban = false;
		}

		std::string owner = pl->second.name();
		
		player_map::iterator banned_pl = players_.begin();
		for (; banned_pl != players_.end(); banned_pl++) {
			if (banned_pl->second.name() == name) {
				break;
			}
		}
		if (banned_pl != players_.end() && banned_pl->first != sock) {
			if (ban) {
				LOG_SERVER << network::ip_address(sock) << "\t"
					<< owner << "\tbanned: " << name << "\tfrom game:\t"
					<< g->name() << "\" (" << g->id() << ")\n";
				g->ban_player(banned_pl->first);
			} else {
				LOG_SERVER << network::ip_address(sock) << "\t"
					<< owner << "\tkicked: " << name << " from game:\t\""
					<< g->name() << "\" (" << g->id() << ")\n";
				network::send_data(construct_server_message(
					"You have been kicked.", *g), banned_pl->first);
				g->send_data(construct_server_message(name + " has been kicked.", *g));
				g->remove_player(banned_pl->first);
			}
			lobby_players_.add_player(banned_pl->first, true);
			network::send_data(config("leave_game"), banned_pl->first);

			g->describe_slots();
			// Send the player who was banned the lobby game list.
			network::send_data(games_and_users_list_, banned_pl->first);
			// Send all other players in the lobby the update to the lobby.
			lobby_players_.send_data(games_and_users_list_diff(), sock);
			// FIXME: Why not save it in the history_?
			return;
		} else if (banned_pl == players_.end()) {
			network::send_data(construct_server_message("Kick/ban failed: user '"
				+ name + "' not found.", *g), sock);
			return;
		}
	} else if (data.child("ban")) {
		const config& response = construct_server_message(
		             "You cannot ban: not the game host.", *g);
		network::send_data(response,sock);
		return;
	} else if (data.child("kick")) {
		const config& response = construct_server_message(
		             "You cannot kick: not the game host.", *g);
		network::send_data(response,sock);
		return;
	// If info is being provided about the game state.
	} else if (data.child("info") != NULL) {
		const config& info = *data.child("info");
		if (info["type"] == "termination") {
			g->set_termination_reason(info["condition"]);
		}
	}

	config* const turn = data.child("turn");
	if (turn != NULL) {
		g->filter_commands(sock, *turn);

		// Notify the game of the commands, and if it changes
		// the description, then sync the new description
		// to players in the lobby.
		const bool res = g->process_commands(*turn);
		if (res) {
			lobby_players_.send_data(games_and_users_list_diff());
		}

		// Any private 'speak' commands must be repackaged separate
		// to other commands, and re-sent, since they should only go
		// to some clients.
		const config::child_itors speaks = turn->child_range("command");
		int npublic = 0, nprivate = 0, nother = 0;
		std::string team_name;
		for (config::child_iterator i = speaks.first; i != speaks.second; ++i) {
			config* const speak = (*i)->child("speak");
			if (speak == NULL) {
				++nother;
				continue;
			}
			if ((g->all_observers_muted() && g->is_observer(sock))
				|| g->is_muted_observer(sock))
			{
				network::send_data(construct_server_message(
					"You have been muted, others can't see your message!", *g),
					pl->first);
				return;
			}
			truncate_message((*speak)["message"]);

			// Force the description to be correct,
			// to prevent spoofing of messages
			(*speak)["description"] = pl->second.name();

			if ((*speak)["team_name"] == "") {
				++npublic;
			} else {
				++nprivate;
				team_name = (*speak)["team_name"];
			}
		}

		// If all there are are messages and they're all private, then
		// just forward them on to the client that should receive them.
		if (nprivate > 0 && npublic == 0 && nother == 0) {
			if (team_name == game_config::observer_team_name) {
				g->send_data_observers(data, sock);
			} else {
				g->send_data_team(data, team_name, sock);
			}
			return;
		}

		// At the moment, if private messages are mixed in with other data,
		// then let them go through. It's exceedingly unlikely that
		// this will happen anyway, and if it does, the client should
		// respect not displaying the message.
	}

	// If a player advances to the next scenario of a mp campaign.
	if(data.child("notify_next_scenario") != NULL) {
		g->send_data(construct_server_message(pl->second.name()
			+ " advanced to the next scenario.", *g), sock);
	}

	// A mp client sends a request for the next scenario of a mp campaign.
	if (data.child("load_next_scenario") != NULL) {
		config cfg_scenario;

		cfg_scenario.add_child("next_scenario", g->level());
		network::send_data(cfg_scenario, sock);
		//Since every client decides himself when to get the data of the next
		//scenario, we must not push this to other players.
		push_immediately = false;
	}

	// Forward data to all players who are in the game,
	// except for the original data sender
	// FIXME: Relaying arbitrary data that possibly didn't get handled at all
	// seems like a bad idea.
	if (push_immediately)
		g->send_data(data,sock);

	if (g->started()) {
		g->record_data(data);
	}
}

void server::delete_game(std::vector<game>::iterator game_it) {
	metrics_.game_terminated(game_it->termination_reason());
	// Delete the game from the games_and_users_list_.
	config* const gamelist = games_and_users_list_.child("gamelist");
	wassert(gamelist != NULL);
	const config::child_itors games = gamelist->child_range("game");
	const config::child_list::const_iterator g =
		std::find(games.first, games.second, game_it->description());
	if (g != games.second) {
		const size_t index = g - games.first;
		gamelist->remove_child("game", index);
	} else {
		// Can happen when the game ends before the scenario was transfered.
		DBG_SERVER << "Could not find game (" << game_it->id()
			<< ") to delete in games_and_users_list_.\n";
	}
	// Put the players back in the lobby, and send
	// them the games_and_users_list_ again.
	game_it->send_data(games_and_users_list_);
	lobby_players_.add_players(*game_it);
	game_it->end_game();
	games_.erase(game_it);
}

int main(int argc, char** argv) {
	int port = 15000;
	size_t min_threads = 5;
	size_t max_threads = 0;

	std::string config_file;

#ifndef FIFODIR
# define FIFODIR "/var/run/wesnothd"
#endif
	std::string fifo_path = std::string(FIFODIR) + "/socket";

	// show 'info' by default
	lg::set_log_domain_severity("server", 2);
	lg::timestamps(true);

	for (int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if (val.empty()) {
			continue;
		}

		if ((val == "--config" || val == "-c") && arg+1 != argc) {
			config_file = argv[++arg];
		} else if (val == "--verbose" || val == "-v") {
			lg::set_log_domain_severity("all",3);
		} else if (val.substr(0, 6) == "--log-") {
			size_t p = val.find('=');
			if (p == std::string::npos) {
				std::cerr << "unknown option: " << val << '\n';
				return 0;
			}
			std::string s = val.substr(6, p - 6);
			int severity;
			if (s == "error") severity = 0;
			else if (s == "warning") severity = 1;
			else if (s == "info") severity = 2;
			else if (s == "debug") severity = 3;
			else {
				std::cerr << "unknown debug level: " << s << '\n';
				return 0;
			}
			while (p != std::string::npos) {
				size_t q = val.find(',', p + 1);
				s = val.substr(p + 1, q == std::string::npos ? q : q - (p + 1));
				if (!lg::set_log_domain_severity(s, severity)) {
					std::cerr << "unknown debug domain: " << s << '\n';
					return 0;
				}
				p = q;
			}
		} else if ((val == "--port" || val == "-p") && arg+1 != argc) {
			port = atoi(argv[++arg]);
		} else if (val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
				<< " [-dvV] [-c path] [-m n] [-p port] [-t n]\n"
				<< "  -c, --config <path>        Tells wesnothd where to find the config file to use.\n"
				<< "  -d, --daemon               Runs wesnothd as a daemon.\n"
				<< "  -h, --help                 Shows this usage message.\n"
				<< "  --log-<level>=\"<domain1>,<domain2>,...\"\n"
				<< "                             sets the severity level of the debug domains.\n"
				<< "                             \"all\" can be used to match any debug domain.\n"
				<< "                             Available levels: error, warning, info, debug.\n"
				<< "  -p, --port <port>          Binds the server to the specified port.\n"
				<< "  -t, --threads <n>          Uses n worker threads for network I/O (default: 5).\n"
				<< "  -v  --verbose              Turns on more verbose logging.\n"
				<< "  -V, --version              Returns the server version.\n";
			return 0;
		} else if (val == "--version" || val == "-V") {
			std::cout << "Battle for Wesnoth server " << game_config::version
				<< "\n";
			return 0;
		} else if (val == "--daemon" || val == "-d") {
#ifdef WIN32
			ERR_SERVER << "Running as a daemon is not supported on this platform\n";
			return -1;
#else
			const pid_t pid = fork();
			if (pid < 0) {
				ERR_SERVER << "Could not fork and run as a daemon\n";
				return -1;
			} else if (pid > 0) {
				std::cout << "Started wesnothd as a daemon with process id "
					<< pid << "\n";
				return 0;
			}

			setsid();
#endif
		} else if ((val == "--threads" || val == "-t") && arg+1 != argc) {
			min_threads = atoi(argv[++arg]);
			if (min_threads > 30) {
				min_threads = 30;
			}
		} else if ((val == "--max-threads" || val == "-T") && arg+1 != argc) {
			max_threads = atoi(argv[++arg]);
		} else if (val[0] == '-') {
			ERR_SERVER << "unknown option: " << val << "\n";
			return 0;
		} else {
			port = atoi(argv[arg]);
		}
	}
	input_stream input(fifo_path);

	try {
		server(port, input, config_file, min_threads, max_threads).run();
	} catch(network::error& e) {
		ERR_SERVER << "Caught network error while server was running. Aborting.: "
			<< e.message << "\n";
		return -1;
	} catch(...) {
		ERR_SERVER << "Caught unknown error while server was running. Aborting.\n";
		return -1;
	}

	return 0;
}
