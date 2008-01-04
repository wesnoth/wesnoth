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
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../serialization/string_utils.hpp"

#include "game.hpp"
#include "input_stream.hpp"
#include "metrics.hpp"
#include "player.hpp"
#include "proxy.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
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
	assert(signal == SIGHUP);
	config_reload = 1;
}

namespace {

//! Function to construct an error message.
//! An error will make the client disconnect, so use it only for unrecoverable
//! errors (use a server message otherwise).
config construct_error(const std::string& msg) {
	config cfg;
	cfg.add_child("error")["message"] = msg;
	return cfg;
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
	game lobby_;

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

	time_t last_ping_;
	time_t last_stats_;
	void dump_stats(const time_t& now);

	//! This is a tempory variable to have a flag to switch between 
	//! gzipped data and not.
	//! @todo remove after 1.3.12 is no longer allowed on the server.
	bool send_gzipped_;

	void process_data(const network::connection sock, const config& data);
	void process_login(const network::connection sock, const config& data);
	//! Handle queries from clients.
	void process_query(const network::connection sock, const config& query);
	//! Process commands from admins and users.
	std::string process_command(const std::string& cmd);
	//! Handle private messages between players.
	void process_whisper(const network::connection sock, const config& whisper) const;
	void process_data_lobby(const network::connection sock, const config& data);
	void process_data_game(const network::connection sock, const config& data);
	void delete_game(std::vector<game>::iterator game_it);
};

server::server(int port, input_stream& input, const std::string& config_file, size_t min_threads,size_t max_threads)
	: net_manager_(min_threads,max_threads), 
	server_(port), 
	not_logged_in_(players_),
	lobby_(players_), 
	input_(input), 
	config_file_(config_file),
	cfg_(read_config()), 
	version_query_response_("version"),
	login_response_("mustlogin"), 
	join_lobby_response_("join_lobby"),
	games_and_users_list_("gamelist"), 
	old_games_and_users_list_(games_and_users_list_),
	last_ping_(time(NULL)), 
	last_stats_(last_ping_),
	send_gzipped_(false)
{
	load_config();
	signal(SIGHUP, reload_config);
}

config server::read_config() const {
	config configuration;
	if (config_file_ == "") return configuration;
	scoped_istream stream = preprocess_file(config_file_);
	std::string errors;
	try {
		read(configuration, *stream, &errors);
		if (errors.empty()) {
			LOG_SERVER << "Server configuration from file: '" << config_file_
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
	// Note this option will not be documented since it's only needed 
	// for a short transition phase.
	send_gzipped_ = utils::string_bool(cfg_["gzipped"], false); 
	game::send_gzipped = send_gzipped_;

	disallowed_names_.clear();
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

	accepted_versions_.clear();
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

	redirected_versions_.clear();
	const config::child_list& redirects = cfg_.get_children("redirect");
	for (config::child_list::const_iterator i = redirects.begin(); i != redirects.end(); ++i) {
		const std::vector<std::string> versions(utils::split((**i)["version"]));
		for (std::vector<std::string>::const_iterator j = versions.begin(); j != versions.end(); ++j) {
			redirected_versions_[*j] = **i;
		}
	}

	proxy_versions_.clear();
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
	last_stats_ = now;
	LOG_SERVER << "Statistics:"
		<< "\tnumber_of_games = " << games_.size()
		<< "\tnumber_of_users = " << players_.size()
		<< "\tlobby_users = " << lobby_.nobservers() << "\n";
}

void server::run() {
	for (int loop = 0;; ++loop) {
		SDL_Delay(20);
		try {
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

			time_t now = time(NULL);
			if ((loop%100) == 0 && last_ping_ + 10 <= now) {
				// Make sure we log stats every 5 minutes
				if (last_stats_ + 5*60 <= now) dump_stats(now);
				// send a 'ping' to all players to detect ghosts
				config ping;
				ping["ping"] = lexical_cast<std::string>(now);
				for (player_map::const_iterator i = players_.begin();
					i != players_.end(); ++i)
				{
					network::send_data(ping, i->first, send_gzipped_);
				}
				last_ping_ = now;
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if (sock) {
				const std::string& ip = network::ip_address(sock);
				if (is_ip_banned(ip)) {
					LOG_SERVER << ip << "\trejected banned user.\n";
					network::send_data(construct_error("You are banned."), sock, send_gzipped_);
					network::disconnect(sock);
				} else if (ip_exceeds_connection_limit(ip)) {
					LOG_SERVER << ip << "\trejected ip due to excessive connections\n";
					network::send_data(construct_error("Too many connections from your IP."), sock, send_gzipped_);
					network::disconnect(sock);
				} else {
					DBG_SERVER << ip << "\tnew connection accepted. (socket: "
						<< sock << ")\n";
					network::send_data(version_query_response_, sock, send_gzipped_);
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
			if (e.message == "shut down") {
				LOG_SERVER << "Kicking everyone...\n";
				for (player_map::const_iterator pl = players_.begin();
					pl != players_.end(); ++pl)
				{
					network::queue_disconnect(pl->first);
				}
				std::cout << "Shutting server down.\n";
				break;
			}
			if (!e.socket) {
				ERR_SERVER << "network error: " << e.message << "\n";
				e.disconnect();
				continue;
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
			// Was the player in the lobby or a game?
			if (lobby_.is_member(e.socket)) {
				lobby_.remove_player(e.socket);
				LOG_SERVER << ip << "\t" << pl_it->second.name()
					<< "\thas logged off. (socket: " << e.socket << ")\n";
				lobby_.send_data(games_and_users_list_diff(), e.socket);
			} else {
				for (std::vector<game>::iterator g = games_.begin();
					g != games_.end(); ++g)
				{
					if (!g->is_member(e.socket)) {
						continue;
					}
					// Did the last player leave?
					if (g->remove_player(e.socket, true)) {
						delete_game(g);
						break;
					} else {
						g->describe_slots();
						lobby_.send_data(games_and_users_list_diff(), e.socket);
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

void server::process_data(const network::connection sock, const config& data) {
	if (proxy::is_proxy(sock)) {
		proxy::received_data(sock, data, send_gzipped_);
	}
	// Ignore client side pings for now.
	if (data.values.find("ping") != data.values.end()) return;
	// If someone who is not yet logged in is sending login details.
	else if (not_logged_in_.is_observer(sock)) {
		process_login(sock, data);
	} else if (const config* query = data.child("query")) {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_query(sock, *query);
	} else if (const config* whisper = data.child("whisper")) {
		process_whisper(sock, *whisper);
	} else if (lobby_.is_observer(sock)) {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_data_lobby(sock, data);
	} else {
		DBG_SERVER << "RECEIVED from: " << sock << ": " << data.debug();
		process_data_game(sock, data);
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
			network::send_data(login_response_, sock, send_gzipped_);
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
			network::send_data(response, sock, send_gzipped_);
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
		network::send_data(response, sock, send_gzipped_);
		return;
	}
	const config* const login = data.child("login");
	// Client must send a login first.
	if (login == NULL) {
		network::send_data(construct_error("You must login first"), sock, send_gzipped_);
		return;
	}
	// Check if the username is valid (all alpha-numeric plus underscore and hyphen)
	std::string username = (*login)["username"];
	if (!utils::isvalid_username(username)) {
		network::send_data(construct_error("This username contains invalid "
			"characters. Only alpha-numeric characters, underscores and hyphens"
			"are allowed."), sock, send_gzipped_);
		return;
	}
	if (username.size() > 18) {
		network::send_data(construct_error("This username is too long"), sock, send_gzipped_);
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
				+ "' is reserved and can not be used by players"), sock, send_gzipped_);
			return;
		}
	}
	// Check the username isn't already taken
	player_map::const_iterator p;
	for (p = players_.begin(); p != players_.end(); ++p) {
		if (p->second.name() == username) {
			network::send_data(construct_error("This username is already taken"), sock, send_gzipped_);
			return;
		}
	}
	network::send_data(join_lobby_response_, sock, send_gzipped_);

	config* const player_cfg = &games_and_users_list_.add_child("user");
	const player new_player(username, *player_cfg, default_max_messages_,
		default_time_period_);
	players_.insert(std::pair<network::connection,player>(sock, new_player));

	not_logged_in_.remove_player(sock);
	lobby_.add_player(sock, true);

	// Send the new player the entire list of games and players
	network::send_data(games_and_users_list_, sock, send_gzipped_);

	if (motd_ != "") {
		network::send_data(lobby_.construct_server_message(motd_), sock, send_gzipped_);
	}

	// Send other players in the lobby the update that the player has joined
	lobby_.send_data(games_and_users_list_diff(), sock);

	LOG_SERVER << network::ip_address(sock) << "\t" << username
		<< "\thas logged on. (socket: " << sock << ")\n";

	for (std::vector<game>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
		// Note: This string is parsed by the client to identify lobby join messages!
		g->send_data(g->construct_server_message(username
			+ " has logged into the lobby"));
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
	network::send_data(lobby_.construct_server_message(response.str()), sock, send_gzipped_);
}

std::string server::process_command(const std::string& query) {
	std::ostringstream out;
	const std::string::const_iterator i = std::find(query.begin(),query.end(),' ');
	const std::string command(query.begin(),i);
	std::string parameters = (i == query.end() ? "" : std::string(i+1,query.end()));
	utils::strip(parameters);
	if (command == "shut_down") {
		throw network::error("shut down");
	}
	if (command == "msg" || command == "lobbymsg") {
		if (parameters == "") {
			return "You must type a message.";
		}
		lobby_.send_data(lobby_.construct_server_message(parameters));
		if (command == "msg") {
			for (std::vector<game>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
				g->send_data(g->construct_server_message(parameters));
			}
		}
		out << "message '" << parameters << "' relayed to players\n";
	} else if (command == "status") {
		out << "STATUS REPORT\n";
		for (player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
			if (parameters == ""
				|| utils::wildcard_string_match(pl->second.name(), parameters)
				|| utils::wildcard_string_match(network::ip_address(pl->first), parameters)) {
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
		out << metrics_ << "Current number of games = " << games_.size() << "\n"
		"Total number of users = " << players_.size() << "\n"
		"Number of users in the lobby = " << lobby_.nobservers() << "\n";
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
							out << "Kicked " << pl->second.name() << ".\n";
							network::queue_disconnect(pl->first);
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
							out << "Kicked " << pl->second.name() << ".\n";
							network::queue_disconnect(pl->first);
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
			return "You must enter a mask to kick.";
		}
		bool kicked = false;
		// if we find 3 '.' consider it an ip mask
		if (std::count(parameters.begin(), parameters.end(), '.') == 3) {
			for (player_map::const_iterator pl = players_.begin();
				pl != players_.end(); ++pl)
			{
				if (utils::wildcard_string_match(network::ip_address(pl->first), parameters)) {
					kicked = true;
					out << "Kicked " << pl->second.name() << ".\n";
					network::queue_disconnect(pl->first);
				}
			}
		} else {
			for (player_map::const_iterator pl = players_.begin();
				pl != players_.end(); ++pl)
			{
				if (utils::wildcard_string_match(pl->second.name(), parameters)) {
					kicked = true;
					out << "Kicked " << pl->second.name() << " ("
						<< network::ip_address(pl->first) << ").\n";
					network::queue_disconnect(pl->first);
				}
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
			", status [<mask>], metrics, (k)ban(s) [<mask>], unban <ipmask>"
			", kick <mask>";
	}

	return out.str();
}

void server::process_whisper(const network::connection sock,
	const config& whisper) const
{
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
					// Don't send to players in a running game.
					if (g->started() && g->is_player(i->first)) {
						do_send = false;
						break;
					}
				}
				if (do_send == false) {
					break;
				}
				config cwhisper;
				cwhisper.add_child("whisper",whisper);
				network::send_data(cwhisper,i->first, send_gzipped_);
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
		network::send_data(data, sock, send_gzipped_);
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
		network::send_data(data, sock, send_gzipped_);
	}
}

void server::process_data_lobby(const network::connection sock, const config& data) {
	DBG_SERVER << "in process_data_lobby...\n";

	const player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find player in players_. (socket: "
			<< sock << ")\n";
		return;
	}

	if (data.child("create_game")) {
		const std::string game_name = (*data.child("create_game"))["name"];
		const std::string game_password = (*data.child("create_game"))["password"];
		DBG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tcreates a new game: " << game_name << ".\n";
		// Create the new game, remove the player from the lobby
		// and set the player as the host/owner.
		games_.push_back(game(players_, sock, game_name));
		game& g = games_.back();
		if(game_password.empty() == false) {
			g.set_password(game_password);
		}
		
		g.level() = (*data.child("create_game"));
		lobby_.remove_player(sock);
		lobby_.send_data(games_and_users_list_diff());
		return;
	}

	// See if the player is joining a game
	if (data.child("join")) {
		const std::string& id = (*data.child("join"))["id"];
		const bool observer = ((*data.child("join"))["observe"] == "yes");
		const std::string& password = (*data.child("join"))["password"];
		int game_id;
		try {
			game_id = lexical_cast<int>(id);
		} catch(bad_lexical_cast&) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join invalid game:\t" << id << "\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"Attempt to join invalid game."), sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		}			
		const std::vector<game>::iterator g =
			std::find_if(games_.begin(),games_.end(), game_id_matches(game_id));
		if (g == games_.end()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join unknown game:\t" << id << ".\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"Attempt to join unknown game."), sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		} else if (g->player_is_banned(sock)) {
			DBG_SERVER << network::ip_address(sock) << "\tReject banned player: "
				<< pl->second.name() << "\tfrom game:\t\"" << g->name()
				<< "\" (" << id << ").\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"You are banned from this game."), sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		} else if(!observer && !g->password_matches(password)) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join game:\t\"" << g->name() << "\" ("
				<< id << ") with bad password\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"Incorrect password"), sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		} else if (observer && !g->allow_observers()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to observe game:\t\"" << g->name() << "\" ("
				<< id << ") which doesn't allow observers.\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"Attempt to observe a game that doesn't allow observers."),
					sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		} else if (!g->level_init()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join uninitialized game:\t\"" << g->name()
				<< "\" (" << id << ").\n";
			network::send_data(config("leave_game"), sock, send_gzipped_);
			network::send_data(lobby_.construct_server_message(
					"Attempt to observe a game that doesn't allow observers."),
					sock, send_gzipped_);
			network::send_data(games_and_users_list_, sock, send_gzipped_);
			return;
		}
		LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tjoined game:\t\"" << g->name()
			<< "\" (" << id << (observer ? ") as an observer.\n" : ").\n");
		lobby_.remove_player(sock);
		g->add_player(sock, observer);
		if (g->describe_slots()) lobby_.send_data(games_and_users_list_diff());
	}

	// See if it's a message, in which case we add the name of the sender,
	// and forward it to all players in the lobby
	if (data.child("message")) {
		// Make a modifiable copy.
		config mdata = data;
		config* const message = mdata.child("message");
		if (pl->second.silenced()) {
			return;
		} else if (pl->second.is_message_flooding()) {
			network::send_data(lobby_.construct_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed."), pl->first, send_gzipped_);
			return;
		}

		(*message)["sender"] = pl->second.name();
		chat_message::truncate_message((*message)["message"]);

		std::string msg = (*message)["message"].base_str();
		if (msg.substr(0,3) == "/me") {
			LOG_SERVER << network::ip_address(sock) << "\t<"
				<< pl->second.name() << msg.substr(3) << ">\n";
		} else {
			LOG_SERVER << network::ip_address(sock) << "\t<"
				<< pl->second.name() << "> " << msg << "\n";
		}

		lobby_.send_data(mdata, sock);
	}

	// Player requests update of lobby content,
	// for example when cancelling the create game dialog
	const config* const refresh = data.child("refresh_lobby");
	if (refresh != NULL) {
		network::send_data(games_and_users_list_, sock, send_gzipped_);
	}
}

//! Process data sent by a player in a game. Note that 'data' by default gets
//! broadcasted and saved in the replay.
void server::process_data_game(const network::connection sock, const config& data) {
	DBG_SERVER << "in process_data_game...\n";
	
	//bool push_immediately = true;
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
	if (data.child("side")) {
		if (!g->is_owner(sock)) {
			return;
		}
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
			// and save a pointer to the description in the new game.
			config* const gamelist = games_and_users_list_.child("gamelist");
			assert(gamelist != NULL);
			config& desc = gamelist->add_child("game",g->level());
			g->set_description(&desc);
			desc["id"] = lexical_cast<std::string>(g->id());
		} else {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tsent scenario data in game:\t\"" << g->name() << "\" ("
				<< g->id() << ") although it's already initialized.\n";
			return;
		}
		config& desc = *g->description();
		// Update the game's description.
		// If there is no shroud, then tell players in the lobby
		// what the map looks like
		if (data["mp_shroud"] != "yes") {
			desc["map_data"] = data["map_data"];
		}
		desc["mp_era"] = data.child("era") != NULL
			? data.child("era")->get_attribute("id") : "";
		// map id
		desc["mp_scenario"] = data["id"];
		desc["observer"] = data["observer"];
		desc["mp_village_gold"] = data["mp_village_gold"];
		desc["experience_modifier"] = data["experience_modifier"];
		desc["mp_fog"] = data["mp_fog"];
		desc["mp_shroud"] = data["mp_shroud"];
		desc["mp_use_map_settings"] = data["mp_use_map_settings"];
		desc["mp_countdown"] = data["mp_countdown"];
		desc["mp_countdown_init_time"] = data["mp_countdown_init_time"];
		desc["mp_countdown_turn_bonus"] = data["mp_countdown_turn_bonus"];
		desc["mp_countdown_reservoir_time"] = data["mp_countdown_reservoir_time"];
		desc["mp_countdown_action_bonus"] = data["mp_countdown_action_bonus"];
		desc["hash"] = data["hash"];
		//desc["map_name"] = data["name"];
		//desc["map_description"] = data["description"];
		//desc[""] = data["objectives"];
		//desc[""] = data["random_start_time"];
		//desc[""] = data["turns"];
		//desc["client_version"] = data["version"];

		// Record the full scenario in g->level()
		g->level() = data;
		// The host already put himself in the scenario so we just need
		// to update_side_data().
		//g->take_side(sock);
		g->update_side_data();
		g->describe_slots();
		// Send the update of the game description to the lobby.
		lobby_.send_data(games_and_users_list_diff());

		//! @todo FIXME: Why not save the level data in the history_?
		return;
// Everything below should only be processed if the game is already intialized.
	} else if (!g->level_init()) {
		return;
	// If the host is sending the next scenario data.
	} else if (data.child("store_next_scenario")) {
		if (!g->is_owner(sock)) return;
		if (!g->level_init()) {
			WRN_SERVER << network::ip_address(sock) << "\tWarning: "
				<< pl->second.name() << "\tsent [store_next_scenario] in game:\t\""
				<< g->name() << "\" (" << g->id()
				<< ") while the scenario is not yet initialized.";
			return;
		}
		LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tadvanced game:\t\"" << g->name() << "\" ("
			<< g->id() << ") to the next scenario.\n";
		if (g->description() == NULL) {
			ERR_SERVER << network::ip_address(sock) << "\tERROR: \""
				<< g->name() << "\" (" << g->id()
				<< ") is initialized but has no description_.\n";
			return;
		}
		const config& s = *data.child("store_next_scenario");
		config& desc = *g->description();
		// Update the game's description.
		// If there is no shroud, then tell players in the lobby
		// what the map looks like.
		if (s["mp_shroud"] != "yes") {
			desc["map_data"] = s["map_data"];
		}
		desc["mp_era"] = s.child("era") != NULL
			? s.child("era")->get_attribute("id") : "";
		// map id
		desc["mp_scenario"] = s["id"];
		desc["observer"] = s["observer"];
		desc["mp_village_gold"] = s["mp_village_gold"];
		desc["experience_modifier"] = s["experience_modifier"];
		desc["mp_fog"] = s["mp_fog"];
		desc["mp_shroud"] = s["mp_shroud"];
		desc["mp_use_map_settings"] = s["mp_use_map_settings"];
		desc["mp_countdown"] = s["mp_countdown"];
		desc["mp_countdown_init_time"] = s["mp_countdown_init_time"];
		desc["mp_countdown_turn_bonus"] = s["mp_countdown_turn_bonus"];
		desc["mp_countdown_reservoir_time"] = s["mp_countdown_reservoir_time"];
		desc["mp_countdown_action_bonus"] = s["mp_countdown_action_bonus"];
		desc["hash"] = s["hash"];
		//desc["map_name"] = s["name"];
		//desc["map_description"] = s["description"];
		//desc[""] = s["objectives"];
		//desc[""] = s["random_start_time"];
		//desc[""] = s["turns"];
		//desc["client_version"] = s["version"];
		// Send the update of the game description to the lobby.
		lobby_.send_data(games_and_users_list_diff());

		// Record the full scenario in g->level()
		g->level() = s;
		g->reset_history();
		// Re-assign sides.
		g->update_side_data();
		// When the host advances tell everyone that the next scenario data is
		// available.
		g->send_data(config("notify_next_scenario"), sock);
		return;
	// If a player advances to the next scenario of a mp campaign. (deprecated)
	} else if(data.child("notify_next_scenario")) {
		//g->send_data(g->construct_server_message(pl->second.name()
		//		+ " advanced to the next scenario."), sock);
		return;
	// A mp client sends a request for the next scenario of a mp campaign.
	} else if (data.child("load_next_scenario")) {
		g->send_data(g->construct_server_message(pl->second.name()
				+ " advances to the next scenario."), sock);
		config cfg_scenario;
		cfg_scenario.add_child("next_scenario", g->level());
		network::send_data(cfg_scenario, sock, send_gzipped_);
		return;
	} else if (data.values.find("side") != data.values.end()) return;
	else if (data.child("side_secured")) return;
	else if (data.values.find("failed") != data.values.end()) return;
	else if (data.values.find("side_drop") != data.values.end()) return;
	else if (data.child("error")) return;
	else if (data.child("start_game")) {
		if (!g->is_owner(sock)) return;
		// Send notification of the game starting immediately.
		// g->start_game() will send data that assumes
		// the [start_game] message has been sent
		g->send_data(data, sock);

		LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tstarted game:\t\"" << g->name() << "\" (" << g->id() << ").\n";

		g->start_game();
		lobby_.send_data(games_and_users_list_diff());
		return;
	} else if (data.child("leave_game")) {
		if ((g->is_player(sock) && g->nplayers() == 1)
			|| (g->is_owner(sock) && !g->started())) {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< (g->started() ? "\tended game:\t\"" : "\taborted game:\t\"")
				<< g->name() << "\" (" << g->id() << ").\n";
			// Remove the player in delete_game() with all other remaining
			// ones so he gets the updated gamelist.
			delete_game(g);
		} else {
			g->remove_player(sock);
			lobby_.add_player(sock, true);
			if (g->describe_slots()) {
				// Send all other players in the lobby the update to the gamelist.
				lobby_.send_data(games_and_users_list_diff(), sock);
			}
			// Send the player who has quit the gamelist.
			network::send_data(games_and_users_list_, sock, send_gzipped_);
		}
		return;
	// If this is data describing side changes by the host.
	} else if (data.child("scenario_diff")) {
		if (!g->is_owner(sock)) return;
		g->level().apply_diff(*data.child("scenario_diff"));
		const config* cfg_change = data.child("scenario_diff")->child("change_child");
		if ((cfg_change != NULL) && (cfg_change->child("side") != NULL)) {
			g->update_side_data();
		}
		if (g->describe_slots()) {
			lobby_.send_data(games_and_users_list_diff());
		}
		g->send_data(data, sock);
		return;
	// If a player changes his faction.
	} else if (data.child("change_faction")) {
		g->send_data(data, sock);
		return;
	// If the owner of a side is changing the controller.
	} else if (data.child("change_controller")) {
		const config& change = *data.child("change_controller");
		g->transfer_side_control(sock, change);
		if (g->describe_slots()) {
			lobby_.send_data(games_and_users_list_diff());
		}
		// FIXME: Why not save it in the history_? (if successful)
		return;
	// If all observers should be muted. (toggles)
	} else if (data.child("muteall")) {
		if (!g->is_owner(sock)) {
			network::send_data(g->construct_server_message(
					"You cannot mute: not the game host."), sock, send_gzipped_);
			return;
		}
		if (g->mute_all_observers()) {
			g->send_data(g->construct_server_message(
				"All observers have been muted."));
		} else {
			g->send_data(g->construct_server_message(
				"Mute of all observers has been removed."));
		}
		return;
	// If an observer should be muted.
	} else if (data.child("mute")) {
		g->mute_observer(*data.child("mute"), pl);
		return;
	// The owner is kicking/banning someone from the game.
	} else if (data.child("kick") || data.child("ban")) {
		bool ban = (data.child("ban") != NULL);
		const network::connection user = 
				(ban ? g->ban_user(*data.child("ban"), pl)
				: g->kick_member(*data.child("kick"), pl));
		if (user) {
			lobby_.add_player(user, true);
			if (g->describe_slots()) {
				lobby_.send_data(games_and_users_list_diff(), sock);
			}
			// Send the removed user the lobby game list.
			network::send_data(games_and_users_list_, user, send_gzipped_);
		}
		return;
	// If info is being provided about the game state.
	} else if (data.child("info")) {
		if (!g->is_player(sock)) return;
		const config& info = *data.child("info");
		if (info["type"] == "termination") {
			g->set_termination_reason(info["condition"]);
		}
		return;
	} else if (data.child("turn")) {
		// Notify the game of the commands, and if it changes
		// the description, then sync the new description
		// to players in the lobby.
		if (g->process_turn(data, pl)) {
			lobby_.send_data(games_and_users_list_diff());
		}
		return;
	}

	// Forward data to all players who are in the game,
	// except for the original data sender
	// FIXME: Relaying arbitrary data that possibly didn't get handled at all
	// seems like a bad idea.
	g->send_data(data, sock);
	DBG_SERVER << "Relaying data RECEIVED from: " << sock << ": " << data.debug();
	if (g->started()) {
		g->record_data(data);
	}
}

void server::delete_game(std::vector<game>::iterator game_it) {
	metrics_.game_terminated(game_it->termination_reason());
	// Delete the game from the games_and_users_list_.
	config* const gamelist = games_and_users_list_.child("gamelist");
	assert(gamelist != NULL);
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
	// Send all other players in the lobby the update to the gamelist.
	lobby_.send_data(games_and_users_list_diff());
	// Put the players back in the lobby, and send
	// them the games_and_users_list_ again.
	lobby_.add_players(*game_it, true);
	game_it->end_game(games_and_users_list_);
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
				<< "  --log-<level>=<domain1>,<domain2>,...\n"
				<< "                             sets the severity level of the debug domains.\n"
				<< "                             'all' can be used to match any debug domain.\n"
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
		//! @todo errno should be passed here with the error or it might not be
		//! the true errno anymore. Seems to work good enough for now though.
		return errno;
	} catch(std::bad_alloc&) {
                ERR_SERVER << "Ran out of memory. Aborting.\n";
		return ENOMEM;
	} catch(...) {
		ERR_SERVER << "Caught unknown error while server was running. Aborting.\n";
		return -1;
	}

	return 0;
}
