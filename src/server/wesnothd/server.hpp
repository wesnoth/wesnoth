/*
	Copyright (C) 2009 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "server/common/user_handler.hpp"
#include "server/wesnothd/metrics.hpp"
#include "server/wesnothd/ban.hpp"
#include "server/common/simple_wml.hpp"
#include "server/common/server_base.hpp"
#include "server/wesnothd/player_connection.hpp"

#include "utils/optional_fwd.hpp"

#include <boost/asio/steady_timer.hpp>

#include <random>

namespace wesnothd
{

class server : public server_base
{
public:
	server(int port, bool keep_alive, const std::string& config_file);

	// We keep this flag for coroutines. Since they get their stack unwinding done after player_connections_
	// is already destroyed they need to know to avoid calling remove_player() on invalid iterators.
	bool destructed = false;
	~server() {
		destructed = true;
	}

private:
	void handle_new_client(socket_ptr socket);
	void handle_new_client(tls_socket_ptr socket);

	template<class SocketPtr> void login_client(boost::asio::yield_context yield, SocketPtr socket);
	template<class SocketPtr> bool is_login_allowed(boost::asio::yield_context yield, SocketPtr socket, const simple_wml::node* const login, const std::string& username, bool& registered, bool& is_moderator);
	template<class SocketPtr> bool authenticate(SocketPtr socket, const std::string& username, const std::string& password, bool name_taken, bool& registered);
	template<class SocketPtr> void send_password_request(SocketPtr socket, const std::string& msg, const char* error_code = "", bool force_confirmation = false);
	bool accepting_connections() const { return !graceful_restart; }

	template<class SocketPtr> void handle_player(boost::asio::yield_context yield, SocketPtr socket, player_iterator player);
	void handle_player_in_lobby(player_iterator player, simple_wml::document& doc);
	void handle_player_in_game(player_iterator player, simple_wml::document& doc);
	void handle_whisper(player_iterator player, simple_wml::node& whisper);
	void handle_query(player_iterator player, simple_wml::node& query);
	void handle_nickserv(player_iterator player, simple_wml::node& nickserv);
	void handle_message(player_iterator player, simple_wml::node& message);
	void handle_create_game(player_iterator player, simple_wml::node& create_game);
	void cleanup_game(game*); // deleter for shared_ptr
	void handle_join_game(player_iterator player, simple_wml::node& join);
	void disconnect_player(player_iterator player);
	void remove_player(player_iterator player);

public:
	template<class SocketPtr> void send_server_message(SocketPtr socket, const std::string& message, const std::string& type);
	void send_server_message(player_iterator player, const std::string& message, const std::string& type) {
		utils::visit(
			[this, &message, &type](auto&& socket) { send_server_message(socket, message, type); },
			player->socket()
		);
	}
	void send_to_lobby(simple_wml::document& data, utils::optional<player_iterator> exclude = {});
	void send_to_player(player_iterator player, simple_wml::document& data) {
		utils::visit(
			[this, &data](auto&& socket) { async_send_doc_queued(socket, data); },
			player->socket()
		);
	}
	void send_server_message_to_lobby(const std::string& message, utils::optional<player_iterator> exclude = {});
	void send_server_message_to_all(const std::string& message, utils::optional<player_iterator> exclude = {});

	bool player_is_in_game(player_iterator player) const {
		return player->get_game() != nullptr;
	}

private:
	wesnothd::ban_manager ban_manager_;

	struct connection_log
	{
		std::string nick, ip;
		std::time_t log_off;

		bool operator==(const connection_log& c) const
		{
			// log off time does not matter to find ip-nick pairs
			return c.nick == nick && c.ip == ip;
		}
	};

	std::deque<connection_log> ip_log_;

	struct login_log
	{
		std::string ip;
		int attempts;
		std::time_t first_attempt;

		bool operator==(const login_log& l) const
		{
			// only the IP matters
			return l.ip == ip;
		}
	};

	std::deque<login_log> failed_logins_;

	std::unique_ptr<user_handler> user_handler_;

	std::mt19937 die_;

#ifndef _WIN32
	/** server socket/fifo. */
	std::string input_path_;
#endif

	std::string uuid_;

	const std::string config_file_;
	config cfg_;

	/** Read the server config from file 'config_file_'. */
	config read_config() const;

	// settings from the server config
	std::vector<std::string> accepted_versions_;
	std::string recommended_version_;
	std::map<std::string,config> redirected_versions_;
	std::map<std::string,config> proxy_versions_;
	std::vector<std::string> disallowed_names_;
	std::string admin_passwd_;
	std::string motd_;
	std::string announcements_;
	std::string server_id_;
	std::string tournaments_;
	std::string information_;
	std::size_t default_max_messages_;
	std::size_t default_time_period_;
	std::size_t concurrent_connections_;
	bool graceful_restart;
	std::time_t lan_server_;
	std::time_t last_user_seen_time_;
	std::string restart_command;
	std::size_t max_ip_log_size_;
	bool deny_unregistered_login_;
	bool save_replays_;
	std::string replay_save_path_;
	bool allow_remote_shutdown_;
	std::set<std::string> client_sources_;
	std::vector<std::string> tor_ip_list_;
	int failed_login_limit_;
	std::time_t failed_login_ban_;
	std::deque<login_log>::size_type failed_login_buffer_size_;

	/** Parse the server config into local variables. */
	void load_config();

	bool ip_exceeds_connection_limit(const std::string& ip) const;
	std::string is_ip_banned(const std::string& ip);

	simple_wml::document version_query_response_;
	simple_wml::document login_response_;
	simple_wml::document games_and_users_list_;

	metrics metrics_;

	player_connections player_connections_;

	std::deque<std::shared_ptr<game>> games() const
	{
		std::deque<std::shared_ptr<game>> result;

		for(const auto& iter : player_connections_.get<game_t>()) {
			if(result.empty() || iter.get_game() != result.back()) {
				result.push_back(iter.get_game());
			}
		}

		if(!result.empty() && result.front() == nullptr) {
			result.pop_front();
		}

		return result;
	}

	boost::asio::steady_timer dump_stats_timer_;
	void start_dump_stats();
	void dump_stats(const boost::system::error_code& ec);

	boost::asio::steady_timer tournaments_timer_;
	void start_tournaments_timer();
	void refresh_tournaments(const boost::system::error_code& ec);

	/** Process commands from admins and users. */
	std::string process_command(std::string cmd, std::string issuer_name);

	void delete_game(int, const std::string& reason="");

	void update_game_in_lobby(const game& g, utils::optional<player_iterator> exclude = {});

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
	void roll_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void games_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void wml_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void adminmsg_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void pm_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
	void version_handler(const std::string &, const std::string &, std::string &, std::ostringstream *);
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
	void stopgame(const std::string &, const std::string &, std::string &, std::ostringstream *);

#ifndef _WIN32
	void handle_sighup(const boost::system::error_code& error, int signal_number);
#endif

	boost::asio::steady_timer timer_;
	void handle_graceful_timeout(const boost::system::error_code& error);

	boost::asio::steady_timer lan_server_timer_;
	void start_lan_server_timer();
	void abort_lan_server_timer();
	void handle_lan_server_shutdown(const boost::system::error_code& error);

	boost::asio::steady_timer dummy_player_timer_;
	int dummy_player_timer_interval_;
	void start_dummy_player_updates();
	void dummy_player_updates(const boost::system::error_code& ec);
};

}
