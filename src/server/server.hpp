/*
   Copyright (C) 2009 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "server/user_handler.hpp"
#include "server/metrics.hpp"
#include "server/ban.hpp"
#include "server/player.hpp"
#include "server/simple_wml.hpp"
#include "server/server_base.hpp"
#include "server/player_connection.hpp"

#include <boost/shared_array.hpp>

#include <boost/asio/signal_set.hpp>

namespace wesnothd
{

class server : public server_base
{
public:
	server(int port, bool keep_alive, const std::string& config_file, size_t, size_t);

private:
	void handle_new_client(socket_ptr socket);

	void handle_version(socket_ptr socket);
	void read_version(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);

	void login(socket_ptr socket);
	void handle_login(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);
	void send_password_request(socket_ptr socket, const std::string& msg,
		const std::string& user, const char* error_code = "", bool force_confirmation = false);
	bool accepting_connections() const { return !graceful_restart; }

	void add_player(socket_ptr socket, const wesnothd::player&);
	void read_from_player(socket_ptr socket);
	void handle_read_from_player(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);
	void handle_player_in_lobby(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);
	void handle_player_in_game(socket_ptr socket, std::shared_ptr<simple_wml::document> doc);
	void handle_whisper(socket_ptr socket, simple_wml::node& whisper);
	void handle_query(socket_ptr socket, simple_wml::node& query);
	void handle_nickserv(socket_ptr socket, simple_wml::node& nickserv);
	void handle_message(socket_ptr socket, simple_wml::node& message);
	void handle_create_game(socket_ptr socket, simple_wml::node& create_game);
	void create_game(player_record& host, simple_wml::node& create_game);
	void cleanup_game(game*); // deleter for shared_ptr
	void handle_join_game(socket_ptr socket, simple_wml::node& join);
	void remove_player(socket_ptr socket);

	void send_to_lobby(simple_wml::document& data, socket_ptr exclude = socket_ptr()) const;
	void send_server_message_to_lobby(const std::string& message, socket_ptr exclude = socket_ptr()) const;
	void send_server_message_to_all(const std::string& message, socket_ptr exclude = socket_ptr()) const;
	bool player_is_in_game(socket_ptr socket) const {
		return bool(player_connections_.find(socket)->get_game());
	}

	wesnothd::ban_manager ban_manager_;

	struct connection_log {
		connection_log(std::string _nick, std::string _ip, time_t _log_off) :
			nick(_nick), ip(_ip), log_off(_log_off) {}
		std::string nick, ip;
		time_t log_off;

		bool operator==(const connection_log& c) const
		{
			// log off time does not matter to find ip-nick pairs
			return c.nick == nick && c.ip == ip;
		}
	};

	std::deque<connection_log> ip_log_;

	struct login_log {
		login_log(std::string _ip, int _attempts, time_t _first_attempt) :
			ip(_ip), attempts(_attempts), first_attempt(_first_attempt) {}
		std::string ip;
		int attempts;
		time_t first_attempt;

		bool operator==(const login_log& l) const
		{
			// only the IP matters
			return l.ip == ip;
		}
	};

	std::deque<login_log> failed_logins_;

	std::unique_ptr<user_handler> user_handler_;
	std::map<long int,std::string> seeds_;

	player_connections player_connections_;
	std::deque<std::shared_ptr<game>> games() {
		std::deque<std::shared_ptr<game>> result;
		for(const auto& iter : player_connections_.get<game_t>())
			if(result.empty() || iter.get_game() != result.back())
				result.push_back(iter.get_game());
		if(!result.empty() && result.front() == 0)
			result.pop_front();
		return result;
	}

#ifndef _WIN32
	/** server socket/fifo. */
	std::string input_path_;
#endif

	const std::string config_file_;
	config cfg_;

	/** Read the server config from file 'config_file_'. */
	config read_config() const;

	// settings from the server config
	std::vector<std::string> accepted_versions_;
	std::map<std::string,config> redirected_versions_;
	std::map<std::string,config> proxy_versions_;
	std::vector<std::string> disallowed_names_;
	std::string admin_passwd_;
	std::string motd_;
	size_t default_max_messages_;
	size_t default_time_period_;
	size_t concurrent_connections_;
	bool graceful_restart;
	time_t lan_server_;
	time_t last_user_seen_time_;
	std::string restart_command;
	size_t max_ip_log_size_;
	std::string uh_name_;
	bool deny_unregistered_login_;
	bool save_replays_;
	std::string replay_save_path_;
	bool allow_remote_shutdown_;
	std::vector<std::string> tor_ip_list_;
	int failed_login_limit_;
	time_t failed_login_ban_;
	std::deque<login_log>::size_type failed_login_buffer_size_;

	/** Parse the server config into local variables. */
	void load_config();

	bool ip_exceeds_connection_limit(const std::string& ip) const;
	std::string is_ip_banned(const std::string& ip) const;

	simple_wml::document version_query_response_;
	simple_wml::document login_response_;
	simple_wml::document join_lobby_response_;
	simple_wml::document games_and_users_list_;

	metrics metrics_;

	time_t last_ping_;
	time_t last_stats_;
	void dump_stats(const time_t& now);

	time_t last_uh_clean_;
	void clean_user_handler(const time_t& now);

	/** Process commands from admins and users. */
	std::string process_command(std::string cmd, std::string issuer_name);

	void delete_game(int);

	void update_game_in_lobby(const wesnothd::game& g, const socket_ptr& exclude=socket_ptr());

	void start_new_server();

	void setup_fifo();
#ifndef _WIN32
	void handle_read_from_fifo(const boost::system::error_code& error, std::size_t bytes_transferred);
#endif
	void setup_handlers();

	typedef std::function<void(const std::string&, const std::string&, std::string&, std::ostringstream *)> cmd_handler;
	std::map<std::string, cmd_handler> cmd_handlers_;

	void shut_down_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void restart_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void sample_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void help_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void stats_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void metrics_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void requests_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void games_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void wml_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void netstats_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void adminmsg_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void pm_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void msg_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void lobbymsg_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void status_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void clones_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void bans_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void ban_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void unban_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void ungban_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void kick_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void kickban_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void gban_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void motd_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void searchlog_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void dul_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);

#ifndef _WIN32
	void handle_sighup(const boost::system::error_code& error, int signal_number);
#endif

	boost::asio::deadline_timer timer_;
	void handle_graceful_timeout(const boost::system::error_code& error);
};

}
