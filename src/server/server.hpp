#ifndef SERVER_HPP_INCLUDED
#define SERVER_HPP_INCLUDED

#include "config.hpp"
#include "user_handler.hpp"
#include "input_stream.hpp"
#include "metrics.hpp"
#include "../network.hpp"
#include "ban.hpp"
#include "player.hpp"
#include "room_manager.hpp"
#include "simple_wml.hpp"

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class server
{
public:
	server(int port, const std::string& config_file, size_t min_threads,size_t max_threads);
	void run();
private:
	void send_error(network::connection sock, const char* msg, const char* error_code ="") const;
	void send_error(network::connection sock, const std::string &msg, const char* error_code = "") const
	{
		send_error(sock, msg.c_str(), error_code);
	}

	void send_warning(network::connection sock, const char* msg, const char* warning_code ="") const;
	void send_warning(network::connection sock, const std::string &msg, const char* warning_code = "") const
	{
		send_warning(sock, msg.c_str(), warning_code);
	}

	// The same as send_error(), we just add an extra child to the response
	// telling the client the chosen username requires a password.
	void send_password_request(network::connection sock, const std::string& msg,
			const std::string& user, const char* error_code ="",
			bool force_confirmation = false);

	const network::manager net_manager_;
	network::server_manager server_;
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

	boost::scoped_ptr<user_handler> user_handler_;
	std::map<network::connection,std::string> seeds_;

	/** std::map<network::connection,player>. */
	wesnothd::player_map players_;
	std::set<network::connection> ghost_players_;

	std::vector<wesnothd::game*> games_;
	std::set<network::connection> not_logged_in_;

	wesnothd::room_manager rooms_;

	/** server socket/fifo. */
	boost::scoped_ptr<input_stream> input_;

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
	std::set<network::connection> admins_;
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

	void process_data(const network::connection sock,
	                  simple_wml::document& data);
	void process_login(const network::connection sock,
	                   simple_wml::document& data);

	/** Handle queries from clients. */
	void process_query(const network::connection sock,
	                   simple_wml::node& query);

	/** Process commands from admins and users. */
	std::string process_command(std::string cmd, std::string issuer_name);

	/** Handle private messages between players. */
	void process_whisper(const network::connection sock,
	                     simple_wml::node& whisper) const;

	/** Handle nickname registration related requests from clients. */
	void process_nickserv(const network::connection sock, simple_wml::node& data);
	void process_data_lobby(const network::connection sock,
	                        simple_wml::document& data);
	void process_data_game(const network::connection sock,
	                       simple_wml::document& data);
	void delete_game(std::vector<wesnothd::game*>::iterator game_it);

	void update_game_in_lobby(const wesnothd::game* g, network::connection exclude=0);

	void start_new_server();

	void setup_handlers();

	typedef boost::function5<void, server*, const std::string&, const std::string&, std::string&, std::ostringstream *> cmd_handler;
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
};

#endif
