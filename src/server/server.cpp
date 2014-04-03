/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Wesnoth-Server, for multiplayer-games.
 */

#include "server.hpp"

#include "../global.hpp"

#include "../config.hpp"
#include "../game_config.hpp"
#include "../log.hpp"
#include "../map.hpp" // gamemap::MAX_PLAYERS
#include "../network.hpp"
#include "../filesystem.hpp"
#include "../multiplayer_error_codes.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../serialization/string_utils.hpp"
#include "../serialization/unicode.hpp"
#include "../util.hpp"

#include "game.hpp"
#include "input_stream.hpp"
#include "metrics.hpp"
#include "player.hpp"
#include "proxy.hpp"
#include "simple_wml.hpp"
#include "ban.hpp"


#include "user_handler.hpp"
#include "sample_user_handler.hpp"

#ifdef HAVE_MYSQLPP
#include "forum_user_handler.hpp"
#endif

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <queue>

#include <csignal>

#ifndef _WIN32
#include <sys/times.h>


namespace {

clock_t get_cpu_time(bool active) {
	if(!active) {
		return 0;
	}
	struct tms buf;
	times(&buf);
	return buf.tms_utime + buf.tms_stime;
}

}

#else

// on Windows we don't calculate CPU time
clock_t get_cpu_time(bool /*active*/) {
	return 0;
}

#endif

namespace {

bool match_user(std::pair<network::connection, wesnothd::player> pl, const std::string& username, const std::string& ip) {
	return pl.second.name() == username && network::ip_address(pl.first) == ip;
}
}

static lg::log_domain log_server("server");
/**
 * fatal and directly server related errors/warnings,
 * ie not caused by erroneous client data
 */
#define ERR_SERVER LOG_STREAM(err, log_server)

/** clients send wrong/unexpected data */
#define WRN_SERVER LOG_STREAM(warn, log_server)

/** normal events */
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)

//compatibility code for MS compilers
#ifndef SIGHUP
#define SIGHUP 20
#endif
/** @todo FIXME: should define SIGINT here too, but to what? */

sig_atomic_t config_reload = 0;

#ifndef _MSC_VER
static void reload_config(int signal) {
	assert(signal == SIGHUP);
	config_reload = 1;
}
#endif

static void exit_sigint(int signal) {
	assert(signal == SIGINT);
	LOG_SERVER << "SIGINT caught, exiting without cleanup immediately.\n";
	exit(128 + SIGINT);
}

static void exit_sigterm(int signal) {
	assert(signal == SIGTERM);
	LOG_SERVER << "SIGTERM caught, exiting without cleanup immediately.\n";
	exit(128 + SIGTERM);
}

namespace {

// we take profiling info on every n requests
int request_sample_frequency = 1;

void send_doc(simple_wml::document& doc, network::connection connection, std::string type = "")
{
	if (type.empty())
		type = doc.root().first_child().to_string();
	try {
		simple_wml::string_span s = doc.output_compressed();
		network::send_raw_data(s.begin(), s.size(), connection, type);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

void make_add_diff(const simple_wml::node& src, const char* gamelist,
                   const char* type,
                   simple_wml::document& out, int index=-1)
{
	if (!out.child("gamelist_diff")) {
		out.root().add_child("gamelist_diff");
	}

	simple_wml::node* top = out.child("gamelist_diff");
	if(gamelist) {
		top = &top->add_child("change_child");
		top->set_attr_int("index", 0);
		top = &top->add_child("gamelist");
	}

	simple_wml::node& insert = top->add_child("insert_child");
	const simple_wml::node::child_list& children = src.children(type);
	assert(!children.empty());
	if(index < 0) {
		index = children.size() - 1;
	}

	assert(index < static_cast<int>(children.size()));
	insert.set_attr_int("index", index);
	children[index]->copy_into(insert.add_child(type));
}

bool make_delete_diff(const simple_wml::node& src,
                      const char* gamelist,
                      const char* type,
                      const simple_wml::node* remove,
					  simple_wml::document& out)
{
	if (!out.child("gamelist_diff")) {
		out.root().add_child("gamelist_diff");
	}

	simple_wml::node* top = out.child("gamelist_diff");
	if(gamelist) {
		top = &top->add_child("change_child");
		top->set_attr_int("index", 0);
		top = &top->add_child("gamelist");
	}

	const simple_wml::node::child_list& children = src.children(type);
	const simple_wml::node::child_list::const_iterator itor =
	    std::find(children.begin(), children.end(), remove);
	if(itor == children.end()) {
		return false;
	}
	const int index = itor - children.begin();
	simple_wml::node& del = top->add_child("delete_child");
	del.set_attr_int("index", index);
	del.add_child(type);
	return true;
}

bool make_change_diff(const simple_wml::node& src,
                      const char* gamelist,
                      const char* type,
					  const simple_wml::node* item,
					  simple_wml::document& out)
{
	if (!out.child("gamelist_diff")) {
		out.root().add_child("gamelist_diff");
	}

	simple_wml::node* top = out.child("gamelist_diff");
	if(gamelist) {
		top = &top->add_child("change_child");
		top->set_attr_int("index", 0);
		top = &top->add_child("gamelist");
	}
	const simple_wml::node::child_list& children = src.children(type);
	const simple_wml::node::child_list::const_iterator itor =
	    std::find(children.begin(), children.end(), item);
	if(itor == children.end()) {
		return false;
	}

	simple_wml::node& diff = *top;
	simple_wml::node& del = diff.add_child("delete_child");
	const int index = itor - children.begin();
	del.set_attr_int("index", index);
	del.add_child(type);

	//inserts will be processed first by the client, so insert at index+1,
	//and then when the delete is processed we'll slide into the right position
	simple_wml::node& insert = diff.add_child("insert_child");
	insert.set_attr_int("index", index + 1);
	children[index]->copy_into(insert.add_child(type));
	return true;
}

std::string player_status(wesnothd::player_map::const_iterator pl) {
	std::ostringstream out;
	const network::connection_stats& stats = network::get_connection_stats(pl->first);
	const int time_connected = stats.time_connected / 1000;
	const int seconds = time_connected % 60;
	const int minutes = (time_connected / 60) % 60;
	const int hours = time_connected / (60 * 60);
	out << "'" << pl->second.name() << "' @ " << network::ip_address(pl->first)
		<< " connected for " << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds
		<< " sent " << stats.bytes_sent << " bytes, received "
		<< stats.bytes_received << " bytes";
	return out.str();
}

} // namespace

class fps_limiter {
	size_t start_ticks_;
	size_t ms_per_frame_;
public:
	fps_limiter(size_t ms_per_frame = 20) : start_ticks_(0), ms_per_frame_(ms_per_frame)
	{}

	void limit() {
		size_t current_ticks = SDL_GetTicks();
		if (current_ticks - start_ticks_ < ms_per_frame_) {
			SDL_Delay(ms_per_frame_ - (current_ticks - start_ticks_));
			start_ticks_ += ms_per_frame_;
		} else {
			start_ticks_ = current_ticks;
		}
	}

	void set_ms_per_frame(size_t ms_per_frame)
	{
		ms_per_frame_ = ms_per_frame;
	}

	void set_fps(size_t fps)
	{
		ms_per_frame_ = 1000 / fps;
	}
};

static fps_limiter fps_limit_;

namespace {
	const std::string denied_msg = "You're not allowed to execute this command.";
	const std::string help_msg = "Available commands are: adminmsg <msg>,"
		" ban <mask> <time> <reason>, bans [deleted] [<ipmask>], clones,"
		" dul|deny_unregistered_login [yes|no], kick <mask> [<reason>],"
		" k[ick]ban <mask> <time> <reason>, help, games, metrics,"
		" netstats [all], [lobby]msg <message>, motd [<message>],"
		" pm|privatemsg <nickname> <message>, requests, sample, searchlog <mask>,"
		" signout, stats, status [<mask>], unban <ipmask>\n"
		"Specific strings (those not inbetween <> like the command names)"
		" are case insensitive.";
}


server::server(int port, const std::string& config_file, size_t min_threads,
		size_t max_threads) :
	net_manager_(min_threads, max_threads),
	server_(port),
	ban_manager_(),
	ip_log_(),
	failed_logins_(),
	user_handler_(NULL),
	seeds_(),
	players_(),
	ghost_players_(),
	games_(),
	not_logged_in_(),
	rooms_(players_),
	input_(),
	config_file_(config_file),
	cfg_(read_config()),
	accepted_versions_(),
	redirected_versions_(),
	proxy_versions_(),
	disallowed_names_(),
	admin_passwd_(),
	admins_(),
	motd_(),
	default_max_messages_(0),
	default_time_period_(0),
	concurrent_connections_(0),
	graceful_restart(false),
	lan_server_(time(NULL)),
	last_user_seen_time_(time(NULL)),
	restart_command(),
	max_ip_log_size_(0),
	uh_name_(),
	deny_unregistered_login_(false),
	save_replays_(false),
	replay_save_path_(),
	allow_remote_shutdown_(false),
	tor_ip_list_(),
	failed_login_limit_(),
	failed_login_ban_(),
	failed_login_buffer_size_(),
	version_query_response_("[version]\n[/version]\n", simple_wml::INIT_COMPRESSED),
	login_response_("[mustlogin]\n[/mustlogin]\n", simple_wml::INIT_COMPRESSED),
	join_lobby_response_("[join_lobby]\n[/join_lobby]\n", simple_wml::INIT_COMPRESSED),
	games_and_users_list_("[gamelist]\n[/gamelist]\n", simple_wml::INIT_STATIC),
	metrics_(),
	last_ping_(time(NULL)),
	last_stats_(last_ping_),
	last_uh_clean_(last_ping_),
	cmd_handlers_()
{
	setup_handlers();
	load_config();
	ban_manager_.read();
	rooms_.read_rooms();

#ifndef _MSC_VER
	signal(SIGHUP, reload_config);
#endif

	signal(SIGINT, exit_sigint);
	signal(SIGTERM, exit_sigterm);
}

void server::setup_handlers()
{
	cmd_handlers_["shut_down"] = &server::shut_down_handler;
	cmd_handlers_["restart"] = &server::restart_handler;
	cmd_handlers_["sample"] = &server::sample_handler;
	cmd_handlers_["help"] = &server::help_handler;
	cmd_handlers_["stats"] = &server::stats_handler;
	cmd_handlers_["metrics"] = &server::metrics_handler;
	cmd_handlers_["requests"] = &server::requests_handler;
	cmd_handlers_["games"] = &server::games_handler;
	cmd_handlers_["wml"] = &server::wml_handler;
	cmd_handlers_["netstats"] = &server::netstats_handler;
	cmd_handlers_["report"]   = &server::adminmsg_handler;
	cmd_handlers_["adminmsg"] = &server::adminmsg_handler;
	cmd_handlers_["pm"] = &server::pm_handler;
	cmd_handlers_["privatemsg"] = &server::pm_handler;
	cmd_handlers_["msg"] = &server::msg_handler;
	cmd_handlers_["lobbymsg"] = &server::msg_handler;
	cmd_handlers_["status"] = &server::status_handler;
	cmd_handlers_["clones"] = &server::clones_handler;
	cmd_handlers_["bans"] = &server::bans_handler;
	cmd_handlers_["ban"] = &server::ban_handler;
	cmd_handlers_["unban"] = &server::unban_handler;
	cmd_handlers_["ungban"] = &server::ungban_handler;
	cmd_handlers_["kick"] = &server::kick_handler;
	cmd_handlers_["kickban"] = &server::kickban_handler;
	cmd_handlers_["kban"] = &server::kickban_handler;
	cmd_handlers_["gban"] = &server::gban_handler;
	cmd_handlers_["motd"] = &server::motd_handler;
	cmd_handlers_["searchlog"] = &server::searchlog_handler;
	cmd_handlers_["sl"] = &server::searchlog_handler;
	cmd_handlers_["dul"] = &server::dul_handler;
	cmd_handlers_["deny_unregistered_login"] = &server::dul_handler;
}

void server::send_error(network::connection sock, const char* msg, const char* error_code) const
{
	simple_wml::document doc;
	doc.root().add_child("error").set_attr("message", msg);
	if(*error_code != '\0') {
		doc.child("error")->set_attr("error_code", error_code);
	}

	send_doc(doc, sock, "error");
}

void server::send_warning(network::connection sock, const char* msg, const char* warning_code) const
{
	simple_wml::document doc;
	doc.root().add_child("warning").set_attr("message", msg);
	if(*warning_code != '\0') {
		doc.child("warning")->set_attr("warning_code", warning_code);
	}

	send_doc(doc, sock, "warning");
}

void server::send_password_request(network::connection sock, const std::string& msg,
	const std::string& user, const char* error_code, bool force_confirmation)
{
	std::string salt = user_handler_->create_salt();
	std::string pepper = user_handler_->create_pepper(user);
	std::string spices = pepper + salt;
	if(user_handler_->use_phpbb_encryption() && pepper.empty()) {
		send_error(sock, "Even though your nickname is registered on this server you "
					"cannot log in due to an error in the hashing algorithm. "
					"Logging into your forum account on http://forum.wesnoth.org "
					"may fix this problem.");
		return;
	}

	seeds_.insert(std::pair<network::connection, std::string>(sock, salt));

	simple_wml::document doc;
	simple_wml::node& e = doc.root().add_child("error");
	e.set_attr("message", msg.c_str());
	e.set_attr("password_request", "yes");
	e.set_attr("phpbb_encryption", user_handler_->use_phpbb_encryption() ? "yes" : "no");
	e.set_attr("salt", spices.c_str());
	e.set_attr("force_confirmation", force_confirmation ? "yes" : "no");
	if(*error_code != '\0') {
		e.set_attr("error_code", error_code);
	}

	send_doc(doc, sock, "error");
}

config server::read_config() const {
	config configuration;
	if (config_file_ == "") return configuration;
	try {
		scoped_istream stream = preprocess_file(config_file_);
		read(configuration, *stream);
		LOG_SERVER << "Server configuration from file: '" << config_file_
			<< "' read.\n";
	} catch(config::error& e) {
		ERR_CONFIG << "ERROR: Could not read configuration file: '"
			<< config_file_ << "': '" << e.message << "'.\n";
	}
	return configuration;
}

void server::load_config() {
#ifndef FIFODIR
# ifdef _MSC_VER
#  pragma message ("No FIFODIR set")
#  define FIFODIR "d:/"
# else
#  warning "No FIFODIR set"
#  ifdef _WIN32
#    define FIFODIR "d:/"
#  else
#    define FIFODIR "/var/run/wesnothd"
#   endif
# endif
#endif
	const std::string fifo_path = (cfg_["fifo_path"].empty() ? std::string(FIFODIR) + "/socket" : std::string(cfg_["fifo_path"]));
	input_.reset();
	input_.reset(new input_stream(fifo_path));

	save_replays_ = cfg_["save_replays"].to_bool();
	replay_save_path_ = cfg_["replay_save_path"].str();

	tor_ip_list_ = utils::split(cfg_["tor_ip_list_path"].empty() ? "" : read_file(cfg_["tor_ip_list_path"]), '\n');

	admin_passwd_ = cfg_["passwd"].str();
	motd_ = cfg_["motd"].str();
	lan_server_ = lexical_cast_default<time_t>(cfg_["lan_server"], 0);
	uh_name_ = cfg_["user_handler"].str();

	deny_unregistered_login_ = cfg_["deny_unregistered_login"].to_bool();

	allow_remote_shutdown_ = cfg_["allow_remote_shutdown"].to_bool();

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
	default_max_messages_ = cfg_["max_messages"].to_int(4);
	default_time_period_ = cfg_["messages_time_period"].to_int(10);
	concurrent_connections_ = cfg_["connections_allowed"].to_int(5);
	max_ip_log_size_ = cfg_["max_ip_log_size"].to_int(500);

	failed_login_limit_ = cfg_["failed_logins_limit"].to_int(10);
	failed_login_ban_ = cfg_["failed_logins_ban"].to_int(3600);
	failed_login_buffer_size_ = cfg_["failed_logins_buffer_size"].to_int(500);

	// Example config line:
	// restart_command="./wesnothd-debug -d -c ~/.wesnoth1.5/server.cfg"
	// remember to make new one as a daemon or it will block old one
	restart_command = cfg_["restart_command"].str();

	fps_limit_.set_ms_per_frame(cfg_["ms_per_frame"].to_int(20));

	accepted_versions_.clear();
	const std::string& versions = cfg_["versions_accepted"];
	if (versions.empty() == false) {
		accepted_versions_ = utils::split(versions);
	} else {
		accepted_versions_.push_back(game_config::version);
		accepted_versions_.push_back("test");
	}

	redirected_versions_.clear();
	BOOST_FOREACH(const config &redirect, cfg_.child_range("redirect")) {
		BOOST_FOREACH(const std::string &version, utils::split(redirect["version"])) {
			redirected_versions_[version] = redirect;
		}
	}

	proxy_versions_.clear();
	BOOST_FOREACH(const config &proxy, cfg_.child_range("proxy")) {
		BOOST_FOREACH(const std::string &version, utils::split(proxy["version"])) {
			proxy_versions_[version] = proxy;
		}
	}
	ban_manager_.load_config(cfg_);
	rooms_.load_config(cfg_);

	// If there is a [user_handler] tag in the config file
	// allow nick registration, otherwise we set user_handler_
	// to NULL. Thus we must check user_handler_ for not being
	// NULL everytime we want to use it.
	user_handler_.reset();

	if (const config &user_handler = cfg_.child("user_handler")) {
		if(uh_name_ == "sample") {
			user_handler_.reset(new suh(user_handler));
		}
#ifdef HAVE_MYSQLPP
		else if(uh_name_ == "forum" || uh_name_.empty()) {
			user_handler_.reset(new fuh(user_handler));
		}
#endif
		// Initiate the mailer class with the [mail] tag
		// from the config file
		if (user_handler_) user_handler_->init_mailer(cfg_.child("mail"));
	}
}

bool server::ip_exceeds_connection_limit(const std::string& ip) const {
	if (concurrent_connections_ == 0) return false;
	size_t connections = 0;
	for (wesnothd::player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if (network::ip_address(i->first) == ip) {
			++connections;
		}
	}

	return connections >= concurrent_connections_;
}

std::string server::is_ip_banned(const std::string& ip) const {
	if (!tor_ip_list_.empty()) {
		if (find(tor_ip_list_.begin(), tor_ip_list_.end(), ip) != tor_ip_list_.end()) return "TOR IP";
	}
	return ban_manager_.is_ip_banned(ip);
}

void server::dump_stats(const time_t& now) {
	last_stats_ = now;
	LOG_SERVER << "Statistics:"
		<< "\tnumber_of_games = " << games_.size()
		<< "\tnumber_of_users = " << players_.size()
		<< "\tlobby_users = " << rooms_.lobby().size() << "\n";
}

void server::clean_user_handler(const time_t& now) {
	if(!user_handler_) {
		return;
	}
	last_uh_clean_ = now;
	user_handler_->clean_up();
}

void server::run() {
	int graceful_counter = 0;

	for (int loop = 0;; ++loop) {
		// Try to run with 50 FPS all the time
		// Server will respond a bit faster under heavy load
		fps_limit_.limit();
		try {
			// We are going to waith 10 seconds before shutting down so users can get out of game.
			if (graceful_restart && games_.empty() && ++graceful_counter > 500 )
			{
				// TODO: We should implement client side autoreconnect.
				// Idea:
				// server should send [reconnect]host=host,port=number[/reconnect]
				// Then client would reconnect to new server automatically.
				// This would also allow server to move to new port or address if there is need

				process_command("msg All games ended. Shutting down now. Reconnect to the new server instance.", "system");
				throw network::error("shut down");
			}

			if (config_reload == 1) {
				cfg_ = read_config();
				load_config();
				config_reload = 0;
			}

			// Process commands from the server socket/fifo
			std::string admin_cmd;
			if (input_ && input_->read_line(admin_cmd)) {
				LOG_SERVER << "Admin Command: type: " << admin_cmd << "\n";
				const std::string res = process_command(admin_cmd, "*socket*");
				// Only mark the response if we fake the issuer (i.e. command comes from IRC or so)
				if (admin_cmd.at(0) == '+') {
					LOG_SERVER << "[admin_command_response]\n" << res << "\n" << "[/admin_command_response]\n";
				} else {
					LOG_SERVER << res << "\n";
				}
			}

			time_t now = time(NULL);
			if (last_ping_ + network::ping_interval <= now) {
				if (lan_server_ && players_.empty() && last_user_seen_time_ + lan_server_ < now)
				{
					LOG_SERVER << "Lan server has been empty for  " << (now - last_user_seen_time_) << " seconds. Shutting down!\n";
					// We have to shutdown
					graceful_restart = true;
				}
				// and check if bans have expired
				ban_manager_.check_ban_times(now);
				// Make sure we log stats every 5 minutes
				if (last_stats_ + 5 * 60 <= now) {
					dump_stats(now);
					if (rooms_.dirty()) rooms_.write_rooms();
				}

				// Cleaning the user_handler once a day should be more than enough
				if (last_uh_clean_ + 60 * 60 * 24 <= now) {
					clean_user_handler(now);
				}

				// Send network stats every hour
				static int prev_hour = localtime(&now)->tm_hour;
				if (prev_hour != localtime(&now)->tm_hour)
				{
					prev_hour = localtime(&now)->tm_hour;
					LOG_SERVER << network::get_bandwidth_stats();

				}

				// send a 'ping' to all players to detect ghosts
				DBG_SERVER << "Pinging inactive players.\n" ;
				std::ostringstream strstr ;
				strstr << "ping=\"" << now << "\"" ;
				simple_wml::document ping( strstr.str().c_str(),
							   simple_wml::INIT_COMPRESSED );
				simple_wml::string_span s = ping.output_compressed();
				BOOST_FOREACH(network::connection sock, ghost_players_) {
					if (!lg::debug.dont_log(log_server)) {
						wesnothd::player_map::const_iterator i = players_.find(sock);
						if (i != players_.end()) {
							DBG_SERVER << "Pinging " << i->second.name() << "(" << i->first << ").\n";
						} else {
							ERR_SERVER << "Player " << sock << " is in ghost_players_ but not in players_.\n";
						}
					}
					network::send_raw_data(s.begin(), s.size(), sock, "ping") ;
				}

 				// Copy new player list on top of ghost_players_ list.
 				// Only a single thread should be accessing this
				// Erase before we copy - speeds inserts
				ghost_players_.clear();
				BOOST_FOREACH(const wesnothd::player_map::value_type v, players_) {
					ghost_players_.insert(v.first);
				}
				last_ping_ = now;
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if (sock) {
				const std::string ip = network::ip_address(sock);
				const std::string reason = is_ip_banned(ip);
				if (!reason.empty()) {
					LOG_SERVER << ip << "\trejected banned user. Reason: " << reason << "\n";
					send_error(sock, "You are banned. Reason: " + reason);
					network::queue_disconnect(sock);
				} else if (ip_exceeds_connection_limit(ip)) {
					LOG_SERVER << ip << "\trejected ip due to excessive connections\n";
					send_error(sock, "Too many connections from your IP.");
					network::queue_disconnect(sock);
				} else {
					DBG_SERVER << ip << "\tnew connection accepted. (socket: "
						<< sock << ")\n";
					send_doc(version_query_response_, sock);
				}
				not_logged_in_.insert(sock);
			}

			static int sample_counter = 0;

			std::vector<char> buf;
			network::bandwidth_in_ptr bandwidth_type;
			while ((sock = network::receive_data(buf, &bandwidth_type)) != network::null_connection) {
				metrics_.service_request();

				if(buf.empty()) {
					WRN_SERVER << "received empty packet\n";
					continue;
				}

				const bool sample = request_sample_frequency >= 1 && (sample_counter++ % request_sample_frequency) == 0;

				const clock_t before_parsing = get_cpu_time(sample);

				char* buf_ptr = new char [buf.size()];
				memcpy(buf_ptr, &buf[0], buf.size());
				simple_wml::string_span compressed_buf(buf_ptr, buf.size());
				boost::scoped_ptr<simple_wml::document> data_ptr;
				try {
					data_ptr.reset(new simple_wml::document(compressed_buf)); // might throw a simple_wml::error
					data_ptr->take_ownership_of_buffer(buf_ptr);

				} catch (simple_wml::error& e) {
					WRN_CONFIG << "simple_wml error in received data: " << e.message << std::endl;
					send_error(sock, "Invalid WML received: " + e.message);
					delete [] buf_ptr;
					continue;
				} catch(...) {
					delete [] buf_ptr;
					throw;
				}

				simple_wml::document& data = *data_ptr;
				std::vector<char>().swap(buf);

				const clock_t after_parsing = get_cpu_time(sample);

				process_data(sock, data);

				bandwidth_type->set_type(data.root().first_child().to_string());
				if(sample) {
					const clock_t after_processing = get_cpu_time(sample);
					metrics_.record_sample(data.root().first_child(),
					          after_parsing - before_parsing,
							  after_processing - after_parsing);
				}

			}

			metrics_.no_requests();

		} catch(simple_wml::error& e) {
			WRN_CONFIG << "Warning: error in received data: " << e.message << "\n";
		} catch(network::error& e) {
			if (e.message == "shut down") {
				LOG_SERVER << "Try to disconnect all users...\n";
				for (wesnothd::player_map::const_iterator pl = players_.begin();
					pl != players_.end(); ++pl)
				{
					network::disconnect(pl->first);
				}
				LOG_SERVER << "Shutting server down.\n";
				break;
			}
			if (!e.socket) {
				ERR_SERVER << "network error: " << e.message << "\n";
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
			const wesnothd::player_map::iterator pl_it = players_.find(e.socket);
			if (pl_it == players_.end()) {
				std::set<network::connection>::iterator i = not_logged_in_.find(e.socket);
				if (i != not_logged_in_.end()) {
					DBG_SERVER << ip << "\tNot logged in user disconnected.\n";
					not_logged_in_.erase(i);
				} else {
					WRN_SERVER << ip << "\tWarning: User disconnected right after the connection was accepted.\n";
				}
				e.disconnect();
				DBG_SERVER << "done closing socket...\n";
				continue;
			}
			const simple_wml::node::child_list& users = games_and_users_list_.root().children("user");
			const size_t index = std::find(users.begin(), users.end(), pl_it->second.config_address()) - users.begin();
			if (index < users.size()) {
				simple_wml::document diff;
				if(make_delete_diff(games_and_users_list_.root(), NULL, "user",
				                    pl_it->second.config_address(), diff)) {
					rooms_.lobby().send_data(diff, e.socket);
				}

				games_and_users_list_.root().remove_child("user", index);
			} else {
				ERR_SERVER << ip << "ERROR: Could not find user to remove: "
					<< pl_it->second.name() << " in games_and_users_list_.\n";
			}
			// Was the player in the lobby or a game?
			if (rooms_.in_lobby(e.socket)) {
				rooms_.remove_player(e.socket);
				LOG_SERVER << ip << "\t" << pl_it->second.name()
					<< "\thas logged off. (socket: " << e.socket << ")\n";

			} else {
				for (std::vector<wesnothd::game*>::iterator g = games_.begin();
					g != games_.end(); ++g)
				{
					if (!(*g)->is_member(e.socket)) {
						continue;
					}
					// Did the last player leave?
					if ((*g)->remove_player(e.socket, true)) {
						delete_game(g);
						break;
					} else {
						(*g)->describe_slots();

						update_game_in_lobby(*g, e.socket);
					}
					break;
				}
			}

			// Find the matching nick-ip pair in the log and update the sign off time
			connection_log ip_name = connection_log(pl_it->second.name(), ip, 0);
			std::deque<connection_log>::iterator i = std::find(ip_log_.begin(), ip_log_.end(), ip_name);
			if(i != ip_log_.end()) {
				i->log_off = time(NULL);
			}

			players_.erase(pl_it);
			ghost_players_.erase(e.socket);
			if (lan_server_)
			{
				last_user_seen_time_ = time(0);
			}
			e.disconnect();
			DBG_SERVER << "done closing socket...\n";

		// Catch user_handler exceptions here, to prevent the
		// server from going down completely. Once we are sure
		// all user_handler exceptions are caught correctly
		// this can removed.
		} catch (user_handler::error& e) {
			ERR_SERVER << "Uncaught user_handler exception: " << e.message << "\n";
		}
	}
}

void server::process_data(const network::connection sock,
                          simple_wml::document& data) {
	if (proxy::is_proxy(sock)) {
		proxy::received_data(sock, data);
		return;
	}

	// We know the client is alive for this interval
	// Remove player from ghost_players map if selective_ping
	// is enabled for the player.

	if (ghost_players_.find(sock) != ghost_players_.end()) {
		const wesnothd::player_map::const_iterator pl = players_.find(sock);
		if (pl != players_.end()) {
			if (pl->second.selective_ping() ) {
				ghost_players_.erase(sock);
			}
		}
	}

	// Process the message
	simple_wml::node& root = data.root();
	if(root.has_attr("ping")) {
		// Ignore client side pings for now.
		return;
	} else if(not_logged_in_.find(sock) != not_logged_in_.end()) {
		// Someone who is not yet logged in is sending login details.
		process_login(sock, data);
	} else if (simple_wml::node* query = root.child("query")) {
		process_query(sock, *query);
	} else if (simple_wml::node* nickserv = root.child("nickserv")) {
		process_nickserv(sock, *nickserv);
    } else if (simple_wml::node* whisper = root.child("whisper")) {
		process_whisper(sock, *whisper);
	} else if (rooms_.in_lobby(sock)) {
		process_data_lobby(sock, data);
	} else {
		process_data_game(sock, data);
	}
}



void server::process_login(const network::connection sock,
                           simple_wml::document& data) {

	// See if the client is sending their version number.
	if (const simple_wml::node* const version = data.child("version")) {
		const simple_wml::string_span& version_str_span = (*version)["version"];
		const std::string version_str(version_str_span.begin(),
		                              version_str_span.end());
		std::vector<std::string>::const_iterator accepted_it;
		// Check if it is an accepted version.
		for (accepted_it = accepted_versions_.begin();
			accepted_it != accepted_versions_.end(); ++accepted_it) {
			if (utils::wildcard_string_match(version_str, *accepted_it)) break;
		}
		if (accepted_it != accepted_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using accepted version " << version_str
				<< ":\ttelling them to log in.\n";
			send_doc(login_response_, sock);
			return;
		}
		std::map<std::string, config>::const_iterator config_it;
		// Check if it is a redirected version
		for (config_it = redirected_versions_.begin();
			config_it != redirected_versions_.end(); ++config_it)
		{
			if (utils::wildcard_string_match(version_str, config_it->first))
				break;
		}
		if (config_it != redirected_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using version " << version_str
				<< ":\tredirecting them to " << config_it->second["host"]
				<< ":" << config_it->second["port"] << "\n";
			config response;
			response.add_child("redirect", config_it->second);
			network::send_data(response, sock, "redirect");
			return;
		}
		// Check if it's a version we should start a proxy for.
		for (config_it = proxy_versions_.begin();
			config_it != proxy_versions_.end(); ++config_it)
		{
			if (utils::wildcard_string_match(version_str, config_it->first))
				break;
		}
		if (config_it != proxy_versions_.end()) {
			LOG_SERVER << network::ip_address(sock)
				<< "\tplayer joined using version " << version_str
				<< ":\tconnecting them by proxy to " << config_it->second["host"]
				<< ":" << config_it->second["port"] << "\n";
			proxy::create_proxy(sock, config_it->second["host"],
				config_it->second["port"].to_int(15000));
			return;
		}
		// No match, send a response and reject them.
		LOG_SERVER << network::ip_address(sock)
			<< "\tplayer joined using unknown version " << version_str
			<< ":\trejecting them\n";
		config response;

		// For compatibility with older clients
		response["version"] = *accepted_versions_.begin();

		config &reject = response.add_child("reject");
		reject["accepted_versions"] = utils::join(accepted_versions_);

		if (accepted_versions_.empty()) {
			// This cannot happen with the current way accepted_versions_ is populated
			ERR_SERVER << "ERROR: This server doesn't accept any versions at all.\n";
		}
		network::send_data(response, sock, "error");
		return;
	}

	const simple_wml::node* const login = data.child("login");
	// Client must send a login first.
	if (login == NULL) {
		send_error(sock, "You must login first.", MP_MUST_LOGIN);
		return;
	}

	// Check if the username is valid (all alpha-numeric plus underscore and hyphen)
	std::string username = (*login)["username"].to_string();
	if (!utils::isvalid_username(username)) {
		send_error(sock, "The nickname '" + username + "' contains invalid "
			"characters. Only alpha-numeric characters, underscores and hyphens"
			"are allowed.", MP_INVALID_CHARS_IN_NAME_ERROR);
		return;
	}
	if (username.size() > 20) {
		send_error(sock, "The nickname '" + username + "' is too long. Nicks must be 20 characters or less.",
			MP_NAME_TOO_LONG_ERROR);
		return;
	}
	// Check if the username is allowed.
	for (std::vector<std::string>::const_iterator d_it = disallowed_names_.begin();
		d_it != disallowed_names_.end(); ++d_it)
	{
		if (utils::wildcard_string_match(utf8::lowercase(username),
			utf8::lowercase(*d_it)))
		{
			send_error(sock, "The nickname '" + username + "' is reserved and cannot be used by players",
				MP_NAME_RESERVED_ERROR);
			return;
		}
	}

	// If this is a request for password reminder
	if(user_handler_) {
		std::string password_reminder = (*login)["password_reminder"].to_string();
		if(password_reminder == "yes") {
			try {
				user_handler_->password_reminder(username);
				send_error(sock, "Your password reminder email has been sent.");
			} catch (user_handler::error& e) {
				send_error(sock, "There was an error sending your password reminder email. The error message was: " +
				e.message);
			}
			return;
		}
	}

	// Check the username isn't already taken
	bool name_taken = false;
	wesnothd::player_map::const_iterator p;
	for (p = players_.begin(); p != players_.end(); ++p) {
		if (p->second.name() == username) {
			name_taken = true;
			break;
		}
	}

	// unregistered users may not get auto-kicked to prevent abuse
	if (name_taken && !p->second.registered()) {
		send_error(sock, "The nickname '" + username + "' is already taken.", MP_NAME_TAKEN_ERROR);
		return;
	}

	// Check for password

	// Current login procedure  for registered nicks is:
	// - Client asks to log in with a particular nick
	// - Server sends client random salt plus some info
	// 	generated from the original hash that is required to
	// 	regenerate the hash
	// - Client generates hash for the user provided password
	// 	and mixes it with the received random salt
	// - Server received salted hash, salts the valid hash with
	// 	the same salt it sent to the client and compares the results

	bool registered = false;
	if(user_handler_) {
		std::string password = (*login)["password"].to_string();
		const bool exists = user_handler_->user_exists(username);
		// This name is registered but the account is not active
		if(exists && !user_handler_->user_is_active(username)) {
			send_warning(sock, "The nickname '" + username + "' is inactive. You cannot claim ownership of this "
				"nickname until you activate your account via email or ask an administrator to do it for you.", MP_NAME_INACTIVE_WARNING);
			//registered = false;
		}
		else if(exists) {
			// This name is registered and no password provided
			if(password.empty()) {
				if (!name_taken) {
					send_password_request(sock, "The nickname '" + username +"' is registered on this server.",
							username, MP_PASSWORD_REQUEST);
				} else {
					send_password_request(sock, "The nickname '" + username + "' is registered on this server."
							"\n\nWARNING: There is already a client using this username, "
							"logging in will cause that client to be kicked!",
							username, MP_PASSWORD_REQUEST_FOR_LOGGED_IN_NAME, true);
				}
				return;
			}

			// A password (or hashed password) was provided, however
			// there is no seed
			if(seeds_[sock].empty()) {
				send_password_request(sock, "Please try again.", username, MP_NO_SEED_ERROR);
			}
			// This name is registered and an incorrect password provided
			else if(!(user_handler_->login(username, password, seeds_[sock]))) {
				const time_t now = time(NULL);

				// Reset the random seed
				seeds_.erase(sock);

				login_log login_ip = login_log(network::ip_address(sock), 0, now);
				std::deque<login_log>::iterator i = std::find(failed_logins_.begin(), failed_logins_.end(), login_ip);
				if(i == failed_logins_.end()) {
					failed_logins_.push_back(login_ip);
					i = --failed_logins_.end();

					// Remove oldest entry if maximum size is exceeded
					if(failed_logins_.size() > failed_login_buffer_size_)
						failed_logins_.pop_front();

				}

				if (i->first_attempt + failed_login_ban_ < now) {
					// Clear and move to the beginning
					failed_logins_.erase(i);
					failed_logins_.push_back(login_ip);
					i = --failed_logins_.end();
				}

				i->attempts++;

				if (i->attempts > failed_login_limit_) {
					LOG_SERVER << ban_manager_.ban(login_ip.ip, now + failed_login_ban_, "Maximum login attempts exceeded", "automatic", "", username);
					send_error(sock, "You have made too many failed login attempts.", MP_TOO_MANY_ATTEMPTS_ERROR);
					network::queue_disconnect(sock);
				} else {
					send_password_request(sock, "The password you provided for the nickname '" + username +
						"' was incorrect.", username, MP_INCORRECT_PASSWORD_ERROR);
				}

				// Log the failure
				LOG_SERVER << network::ip_address(sock) << "\t"
						<< "Login attempt with incorrect password for nickname '" << username << "'.\n";
				return;
			}
			// This name exists and the password was neither empty nor incorrect
			registered = true;
			// Reset the random seed
			seeds_.erase(sock);
			user_handler_->user_logged_in(username);

			if (name_taken) {
				// If there is already a client using this username kick it
				process_command("kick " + username + " autokick by registered user", username);
			}
		}
	}

	// If we disallow unregistered users and this user is not registered send an error
	if(user_handler_ && !registered && deny_unregistered_login_) {
		send_error(sock, "The nickname '" + username + "' is not registered. "
				"This server disallows unregistered nicknames.", MP_NAME_UNREGISTERED_ERROR);
		return;
	}

	// if registered, the old client got kicked
	if (name_taken && !registered) {
		send_error(sock, "The nickname '" + username + "' is already taken.", MP_NAME_TAKEN_ERROR);
		return;
	}

	// Check if the version is now available. If it is not, this player must
	// always be pinged.
	bool selective_ping = false ;
	if( (*login)["selective_ping"].to_bool() ) {
	  selective_ping = true ;
	  DBG_SERVER << "selective ping is ENABLED for " << sock << "\n" ;
	} else {
	  DBG_SERVER << "selective ping is DISABLED for  " << sock << "\n" ;
	}

	send_doc(join_lobby_response_, sock);

	simple_wml::node& player_cfg = games_and_users_list_.root().add_child("user");
	const wesnothd::player new_player(username, player_cfg, registered,
		default_max_messages_, default_time_period_, selective_ping,
		user_handler_ && user_handler_->user_is_moderator(username));

	// If the new player does not have selective ping enabled, immediately
	// add the player to the ghost player's list. This ensures a client won't
	// have to wait as long as x2 the current ping delay; which could cause
	// a client-side disconnection.
	players_.insert(std::make_pair(sock, new_player));
	if( !selective_ping )
	  ghost_players_.insert(sock) ;

	not_logged_in_.erase(sock);
	rooms_.enter_lobby(sock);
	// Send the new player the entire list of games and players
	send_doc(games_and_users_list_, sock);

	if (motd_ != "") {
		rooms_.lobby().send_server_message(motd_, sock);
	}

	// Send other players in the lobby the update that the player has joined
	simple_wml::document diff;
	make_add_diff(games_and_users_list_.root(), NULL, "user", diff);
	rooms_.lobby().send_data(diff, sock);

	LOG_SERVER << network::ip_address(sock) << "\t" << username
		<< "\thas logged on" << (registered ? " to a registered account" : "")
		<< ". (socket: " << sock << ")\n";

	for (std::vector<wesnothd::game*>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
		// Note: This string is parsed by the client to identify lobby join messages!
		(*g)->send_server_message_to_all(username + " has logged into the lobby");
	}

	if(user_handler_ && user_handler_->user_is_moderator(username)) {
		LOG_SERVER << "Admin automatically recognized: IP: "
			<< network::ip_address(sock) << "\tnick: "
			<< username << std::endl;
		// This string is parsed by the client!
		rooms_.lobby().send_server_message("You are now recognized as an administrator. "
				"If you no longer want to be automatically authenticated use '/query signout'.", sock);
	}

	// Log the IP
	connection_log ip_name = connection_log(username, network::ip_address(sock), 0);
	if (std::find(ip_log_.begin(), ip_log_.end(), ip_name) == ip_log_.end()) {
		ip_log_.push_back(ip_name);
		// Remove the oldest entry if the size of the IP log exceeds the maximum size
		if(ip_log_.size() > max_ip_log_size_) ip_log_.pop_front();
	}
}

void server::process_query(const network::connection sock,
                           simple_wml::node& query) {
	const wesnothd::player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		DBG_SERVER << "ERROR: process_query(): Could not find player with socket: " << sock << "\n";
		return;
	}
	const std::string command(query["type"].to_string());
	std::ostringstream response;
	const std::string& help_msg = "Available commands are: adminmsg <msg>, help, games, metrics,"
			" motd, netstats [all], requests, sample, stats, status, wml.";
	// Commands a player may issue.
	if (command == "status") {
		response << process_command(command + " " + pl->second.name(), pl->second.name());
	} else if (command.find("adminmsg") == 0
			|| command == "games"
			|| command == "metrics"
			|| command == "motd"
			|| command == "netstats"
			|| command == "netstats all"
			|| command == "requests"
			|| command == "sample"
			|| command == "stats"
			|| command == "status " + pl->second.name()
			|| command == "wml")
	{
		response << process_command(command, pl->second.name());
	} else if (pl->second.is_moderator()) {
		if (command == "signout") {
			LOG_SERVER << "Admin signed out: IP: "
				<< network::ip_address(sock) << "\tnick: "
				<< pl->second.name() << std::endl;
			pl->second.set_moderator(false);
			// This string is parsed by the client!
			response << "You are no longer recognized as an administrator.";
			if(user_handler_) {
				user_handler_->set_is_moderator(pl->second.name(), false);
			}
		} else {
			LOG_SERVER << "Admin Command: type: " << command
				<< "\tIP: "<< network::ip_address(sock)
				<< "\tnick: "<< pl->second.name() << std::endl;
			response << process_command(command, pl->second.name());
			LOG_SERVER << response.str() << std::endl;
		}
	} else if (command == "help" || command.empty()) {
		response << help_msg;
	} else if (command == "admin" || command.find("admin ") == 0) {
		if (admin_passwd_.empty()) {
			rooms_.lobby().send_server_message("No password set.", sock);
			return;
		}
		std::string passwd;
		if (command.size() >= 6) passwd = command.substr(6);
		if (passwd == admin_passwd_) {
			LOG_SERVER << "New Admin recognized: IP: "
				<< network::ip_address(sock) << "\tnick: "
				<< pl->second.name() << std::endl;
			pl->second.set_moderator(true);
			// This string is parsed by the client!
			response << "You are now recognized as an administrator.";
			if (user_handler_) {
				user_handler_->set_is_moderator(pl->second.name(), true);
			}
		} else {
			WRN_SERVER << "FAILED Admin attempt with password: '" << passwd << "'\tIP: "
				<< network::ip_address(sock) << "\tnick: "
				<< pl->second.name() << std::endl;
			response << "Error: wrong password";
		}
	} else {
		response << "Error: unrecognized query: '" << command << "'\n" << help_msg;
	}
	rooms_.lobby().send_server_message(response.str(), sock);
}

void server::start_new_server() {
	if (restart_command.empty())
		return;

	// Example config line:
	// restart_command="./wesnothd-debug -d -c ~/.wesnoth1.5/server.cfg"
	// remember to make new one as a daemon or it will block old one
	if (std::system(restart_command.c_str())) {
		ERR_SERVER << "Failed to start new server with command: " << restart_command << "\n";
	} else {
		LOG_SERVER << "New server started with command: " << restart_command << "\n";
	}
}

std::string server::process_command(std::string query, std::string issuer_name) {
	utils::strip(query);

	if (issuer_name == "*socket*" && query.at(0) == '+') {
		// The first argument might be "+<issuer>: ".
		// In that case we use +<issuer>+ as the issuer_name.
		// (Mostly used for communication with IRC.)
		std::string::iterator issuer_end =
				std::find(query.begin(), query.end(), ':');
		std::string issuer(query.begin() + 1, issuer_end);
		if (!issuer.empty()) {
			issuer_name = "+" + issuer + "+";
			query = std::string(issuer_end + 1, query.end());
			utils::strip(query);
		}
	}

	const std::string::iterator i = std::find(query.begin(), query.end(), ' ');
	const std::string command = utf8::lowercase(std::string(query.begin(), i));
	std::string parameters = (i == query.end() ? "" : std::string(i + 1, query.end()));
	utils::strip(parameters);

	std::ostringstream out;
	std::map<std::string, server::cmd_handler>::iterator handler_itor = cmd_handlers_.find(command);
	if(handler_itor == cmd_handlers_.end()) {
		out << "Command '" << command << "' is not recognized.\n" << help_msg;
	} else {
		const cmd_handler &handler = handler_itor->second;
		handler(this, issuer_name, query, parameters, &out);
	}

	return out.str();
}

// Shutdown, restart and sample commands can only be issued via the socket.
void server::shut_down_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (issuer_name != "*socket*" && !allow_remote_shutdown_) {
		*out << denied_msg;
		return;
	}
	if (parameters == "now") {
		throw network::error("shut down");
	} else {
		// Graceful shut down.
		server_.stop();
		input_.reset();
		graceful_restart = true;
		process_command("msg The server is shutting down. You may finish your games but can't start new ones. Once all games have ended the server will exit.", issuer_name);
		*out << "Server is doing graceful shut down.";
	}
}

void server::restart_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);

	if (issuer_name != "*socket*" && !allow_remote_shutdown_) {
		*out << denied_msg;
		return;
	}

	if (restart_command.empty()) {
		*out << "No restart_command configured! Not restarting.";
	} else {
		graceful_restart = true;
		// stop listening socket
		server_.stop();
		input_.reset();
		// start new server
		start_new_server();
		process_command("msg The server has been restarted. You may finish current games but can't start new ones and new players can't join this (old) server instance. (So if a player of your game disconnects you have to save, reconnect and reload the game on the new server instance. It is actually recommended to do that right away.)", issuer_name);
		*out << "New server started.";
	}
}

void server::sample_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters.empty()) {
		*out << "Current sample frequency: " << request_sample_frequency;
		return;
	} else if (issuer_name != "*socket*") {
		*out << denied_msg;
		return;
	}
	request_sample_frequency = atoi(parameters.c_str());
	if (request_sample_frequency <= 0) {
		*out << "Sampling turned off.";
	} else {
		*out << "Sampling every " << request_sample_frequency << " requests.";
	}
}

void server::help_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);
	*out << help_msg;
}

void server::stats_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);

	*out << "Number of games = " << games_.size()
		<< "\nTotal number of users = " << players_.size()
		<< "\nNumber of users in the lobby = " << rooms_.lobby().size();
}

void server::metrics_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);
	*out << metrics_;
}

void server::requests_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);
	metrics_.requests(*out);
}

void server::games_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);
	metrics_.games(*out);
}

void server::wml_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);
	*out << simple_wml::document::stats();
}

void server::netstats_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	network::pending_statistics stats = network::get_pending_stats();
	*out << "Network stats:\nPending send buffers: "
		<< stats.npending_sends << "\nBytes in buffers: "
		<< stats.nbytes_pending_sends << "\n";

	if (utf8::lowercase(parameters) == "all") {
		*out << network::get_bandwidth_stats_all();
	} else {
		*out << network::get_bandwidth_stats(); // stats from previuos hour
	}
}

void server::adminmsg_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "You must type a message.";
		return;
	}

	const std::string& sender = issuer_name;
	const std::string& message = parameters;
	LOG_SERVER << "Admin message: <" << sender << (message.find("/me ") == 0
			? std::string(message.begin() + 3, message.end()) + ">"
			: "> " + message) << "\n";

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("whisper");
	msg.set_attr_dup("sender", ("admin message from " + sender).c_str());
	msg.set_attr_dup("message", message.c_str());
	int n = 0;
	for (wesnothd::player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
		if (pl->second.is_moderator()) {
			++n;
			send_doc(data, pl->first);
		}
	}

	if (n == 0) {
		*out << "Sorry, no admin available right now. But your message got logged.";
		return;
	}

	*out << "Message sent to " << n << " admins.";
}

void server::pm_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	std::string::iterator first_space = std::find(parameters.begin(), parameters.end(), ' ');
	if (first_space == parameters.end()) {
		*out << "You must name a receiver.";
		return;
	}

	const std::string& sender = issuer_name;
	const std::string receiver(parameters.begin(), first_space);
	std::string message(first_space + 1, parameters.end());
	utils::strip(message);
	if (message.empty()) {
		*out << "You must type a message.";
		return;
	}

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("whisper");
	// This string is parsed by the client!
	msg.set_attr_dup("sender", ("server message from " + sender).c_str());
	msg.set_attr_dup("message", message.c_str());
	for (wesnothd::player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
		if (receiver != pl->second.name().c_str()) {
			continue;
		}
		send_doc(data, pl->first);
		*out << "Message to " << receiver << " successfully sent.";
		return;
	}

	*out << "No such nick: " << receiver;
}

void server::msg_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "You must type a message.";
		return;
	}

	rooms_.lobby().send_server_message_to_all(parameters);
	for (std::vector<wesnothd::game*>::const_iterator g = games_.begin(); g != games_.end(); ++g) {
		(*g)->send_server_message_to_all(parameters);
	}

	LOG_SERVER << "<server" << (parameters.find("/me ") == 0
			? std::string(parameters.begin() + 3, parameters.end()) + ">"
			: "> " + parameters) << "\n";

	*out << "message '" << parameters << "' relayed to players";
}

void server::lobbymsg_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "You must type a message.";
		return;
	}

	rooms_.lobby().send_server_message_to_all(parameters);
	LOG_SERVER << "<server" << (parameters.find("/me ") == 0
			? std::string(parameters.begin() + 3, parameters.end()) + ">"
			: "> " + parameters) << "\n";

	*out << "message '" << parameters << "' relayed to players";
}

void server::status_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	*out << "STATUS REPORT for '" << parameters << "'";
	bool found_something = false;
	// If a simple username is given we'll check for its IP instead.
	if (utils::isvalid_username(parameters)) {
		for (wesnothd::player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
			if (parameters == pl->second.name()) {
				parameters = network::ip_address(pl->first);
				found_something = true;
				break;
			}
		}
		if (!found_something) {
			//out << "\nNo match found. You may want to check with 'searchlog'.";
			//return out.str();
			*out << process_command("searchlog " + parameters, issuer_name);
			return;
		}
	}
	const bool match_ip = (std::count(parameters.begin(), parameters.end(), '.') >= 1);
	for (wesnothd::player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
		if (parameters == "" || parameters == "*"
		|| (match_ip && utils::wildcard_string_match(network::ip_address(pl->first), parameters))
		|| (!match_ip && utils::wildcard_string_match(pl->second.name(), parameters))) {
			found_something = true;
			*out << std::endl << player_status(pl);
		}
	}
	if (!found_something) *out << "\nNo match found. You may want to check with 'searchlog'.";
}

void server::clones_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);

	*out << "CLONES STATUS REPORT";
	std::set<std::string> clones;
	for (wesnothd::player_map::const_iterator pl = players_.begin(); pl != players_.end(); ++pl) {
		if (clones.find(network::ip_address(pl->first)) != clones.end()) continue;
		bool found = false;
		for (wesnothd::player_map::const_iterator clone = boost::next(pl); clone != players_.end(); ++clone) {
			if (network::ip_address(pl->first) == network::ip_address(clone->first)) {
				if (!found) {
					found = true;
					clones.insert(network::ip_address(pl->first));
					*out << std::endl << player_status(pl);
				}
				*out << std::endl << player_status(clone);
			}
		}
	}
	if (clones.empty()) {
		*out << "No clones found.";
	}
}

void server::bans_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters.empty()) {
		ban_manager_.list_bans(*out);
	} else if (utf8::lowercase(parameters) == "deleted") {
		ban_manager_.list_deleted_bans(*out);
	} else if (utf8::lowercase(parameters).find("deleted") == 0) {
		std::string mask = parameters.substr(7);
		ban_manager_.list_deleted_bans(*out, utils::strip(mask));
	} else {
		ban_manager_.list_bans(*out, utils::strip(parameters));
	}
}

void server::ban_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	bool banned = false;
	std::string::iterator first_space = std::find(parameters.begin(), parameters.end(), ' ');

	if (first_space == parameters.end()) {
		*out << ban_manager_.get_ban_help();
		return;
	}

	std::string::iterator second_space = std::find(first_space + 1, parameters.end(), ' ');
	const std::string target(parameters.begin(), first_space);

	const std::string duration(first_space + 1, second_space);
	time_t parsed_time = time(NULL);
	if (ban_manager_.parse_time(duration, &parsed_time) == false) {
		*out << "Failed to parse the ban duration: '" << duration << "'\n"
			<< ban_manager_.get_ban_help();
		return;
	}

	if (second_space == parameters.end()) {
		--second_space;
	}
	std::string reason(second_space + 1, parameters.end());
	utils::strip(reason);
	if (reason.empty()) {
		*out << "You need to give a reason for the ban.";
		return;
	}

	std::string dummy_group;

	// if we find a '.' consider it an ip mask
	/** @todo  FIXME: make a proper check for valid IPs. */
	if (std::count(target.begin(), target.end(), '.') >= 1) {
		banned = true;

		*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, dummy_group);
	} else {
		for (wesnothd::player_map::const_iterator pl = players_.begin();
				pl != players_.end(); ++pl)
		{
			if (utils::wildcard_string_match(pl->second.name(), target)) {
				if (banned) *out << "\n";
				else banned = true;
				const std::string ip = network::ip_address(pl->first);
				*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, dummy_group, target);
			}
		}
		if (!banned) {
			// If nobody was banned yet check the ip_log but only if a
			// simple username was used to prevent accidental bans.
			// @todo FIXME: since we can have several entries now we should only ban the latest or so
			/*if (utils::isvalid_username(target)) {
				for (std::deque<connection_log>::const_iterator i = ip_log_.begin();
						i != ip_log_.end(); ++i) {
					if (i->nick == target) {
						if (banned) out << "\n";
						else banned = true;
						out << ban_manager_.ban(i->ip, parsed_time, reason, issuer_name, group, target);
					}
				}
			}*/
			if(!banned) {
				*out << "Nickname mask '" << target << "' did not match, no bans set.";
			}
		}
	}
}

void server::kickban_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
		assert(out != NULL);

		bool banned = false;
		std::string::iterator first_space = std::find(parameters.begin(), parameters.end(), ' ');
		if (first_space == parameters.end()) {
			*out << ban_manager_.get_ban_help();
			return;
		}
		std::string::iterator second_space = std::find(first_space + 1, parameters.end(), ' ');
		const std::string target(parameters.begin(), first_space);
		const std::string duration(first_space + 1, second_space);
		time_t parsed_time = time(NULL);
		if (ban_manager_.parse_time(duration, &parsed_time) == false) {
			*out << "Failed to parse the ban duration: '" << duration << "'\n"
				<< ban_manager_.get_ban_help();
			return;
		}

		if (second_space == parameters.end()) {
			--second_space;
		}
		std::string reason(second_space + 1, parameters.end());
		utils::strip(reason);
		if (reason.empty()) {
			*out << "You need to give a reason for the ban.";
			return;
		}

		std::string dummy_group;

		// if we find a '.' consider it an ip mask
		/** @todo  FIXME: make a proper check for valid IPs. */
		if (std::count(target.begin(), target.end(), '.') >= 1) {
			banned = true;

			*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, dummy_group);

			for (wesnothd::player_map::const_iterator pl = players_.begin();
					pl != players_.end(); ++pl)
			{
				if (utils::wildcard_string_match(network::ip_address(pl->first), target)) {
					*out << "\nKicked " << pl->second.name() << " ("
						<< network::ip_address(pl->first) << ").";
					send_error(pl->first, "You have been banned. Reason: " + reason);
					network::queue_disconnect(pl->first);
				}
			}
		} else {
			for (wesnothd::player_map::const_iterator pl = players_.begin();
					pl != players_.end(); ++pl)
			{
				if (utils::wildcard_string_match(pl->second.name(), target)) {
					if (banned) *out << "\n";
					else banned = true;
					const std::string ip = network::ip_address(pl->first);
					*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, dummy_group, target);
					*out << "\nKicked " << pl->second.name() << " (" << ip << ").";
					send_error(pl->first, "You have been banned. Reason: " + reason);
					network::queue_disconnect(pl->first);
				}
			}
			if (!banned) {
				// If nobody was banned yet check the ip_log but only if a
				// simple username was used to prevent accidental bans.
				// @todo FIXME: since we can have several entries now we should only ban the latest or so
				/*if (utils::isvalid_username(target)) {
					for (std::deque<connection_log>::const_iterator i = ip_log_.begin();
							i != ip_log_.end(); ++i) {
						if (i->nick == target) {
							if (banned) out << "\n";
							else banned = true;
							out << ban_manager_.ban(i->ip, parsed_time, reason, issuer_name, group, target);
						}
					}
				}*/
				if(!banned) {
					*out << "Nickname mask '" << target << "' did not match, no bans set.";
				}
			}
		}
	}

void server::gban_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
			assert(out != NULL);

			bool banned = false;
			std::string::iterator first_space = std::find(parameters.begin(), parameters.end(), ' ');
			if (first_space == parameters.end()) {
				*out << ban_manager_.get_ban_help();
				return;
			}
			std::string::iterator second_space = std::find(first_space + 1, parameters.end(), ' ');
			const std::string target(parameters.begin(), first_space);

			std::string group = std::string(first_space + 1, second_space);
			first_space = second_space;
			second_space = std::find(first_space + 1, parameters.end(), ' ');

			const std::string duration(first_space + 1, second_space);
			time_t parsed_time = time(NULL);
			if (ban_manager_.parse_time(duration, &parsed_time) == false) {
				*out << "Failed to parse the ban duration: '" << duration << "'\n"
					<< ban_manager_.get_ban_help();
				return;
			}

			if (second_space == parameters.end()) {
				--second_space;
			}
			std::string reason(second_space + 1, parameters.end());
			utils::strip(reason);
			if (reason.empty()) {
				*out << "You need to give a reason for the ban.";
				return;
			}

			// if we find a '.' consider it an ip mask
			/** @todo  FIXME: make a proper check for valid IPs. */
			if (std::count(target.begin(), target.end(), '.') >= 1) {
				banned = true;

				*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, group);
			} else {
				for (wesnothd::player_map::const_iterator pl = players_.begin();
						pl != players_.end(); ++pl)
				{
					if (utils::wildcard_string_match(pl->second.name(), target)) {
						if (banned) *out << "\n";
						else banned = true;
						const std::string ip = network::ip_address(pl->first);
						*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, group, target);
					}
				}
				if (!banned) {
					// If nobody was banned yet check the ip_log but only if a
					// simple username was used to prevent accidental bans.
					// @todo FIXME: since we can have several entries now we should only ban the latest or so
					/*if (utils::isvalid_username(target)) {
						for (std::deque<connection_log>::const_iterator i = ip_log_.begin();
								i != ip_log_.end(); ++i) {
							if (i->nick == target) {
								if (banned) out << "\n";
								else banned = true;
								out << ban_manager_.ban(i->ip, parsed_time, reason, issuer_name, group, target);
							}
						}
					}*/
					if(!banned) {
						*out << "Nickname mask '" << target << "' did not match, no bans set.";
					}
				}
			}
		}

void server::unban_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "You must enter an ipmask to unban.";
		return;
	}
	ban_manager_.unban(*out, parameters);
}

void server::ungban_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "You must enter an ipmask to ungban.";
		return;
	}
	ban_manager_.unban_group(*out, parameters);
}

void server::kick_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out <<  "You must enter a mask to kick.";
		return;
	}
	std::string::iterator i = std::find(parameters.begin(), parameters.end(), ' ');
	const std::string kick_mask = std::string(parameters.begin(), i);
	const std::string kick_message =
			(i == parameters.end() ? "You have been kicked."
			: "You have been kicked. Reason: " + std::string(i + 1, parameters.end()));
	bool kicked = false;
	// if we find a '.' consider it an ip mask
	const bool match_ip = (std::count(kick_mask.begin(), kick_mask.end(), '.') >= 1);
	for (wesnothd::player_map::const_iterator pl = players_.begin();
		pl != players_.end(); ++pl)
	{
		if ((match_ip && utils::wildcard_string_match(network::ip_address(pl->first), kick_mask))
		|| (!match_ip && utils::wildcard_string_match(pl->second.name(), kick_mask))) {
			if (kicked) *out << "\n";
			else kicked = true;
			*out << "Kicked " << pl->second.name() << " ("
				<< network::ip_address(pl->first) << "). '"
				<< kick_message << "'";
			send_error(pl->first, kick_message);
			network::queue_disconnect(pl->first);
		}
	}
	if (!kicked) *out << "No user matched '" << kick_mask << "'.";
}

void server::motd_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		if (motd_ != "") {
			*out << "Message of the day:\n" << motd_;
			return;
		} else {
			*out << "No message of the day set.";
			return;
		}
	}

	motd_ = parameters;
	*out << "Message of the day set to: " << motd_;
}

void server::searchlog_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters.empty()) {
		*out << "You must enter a mask to search for.";
		return;
	}
	*out << "IP/NICK LOG for '" << parameters << "'";

	bool found_something = false;

	// If this looks like an IP look up which nicks have been connected from it
	// Otherwise look for the last IP the nick used to connect
	const bool match_ip = (std::count(parameters.begin(), parameters.end(), '.') >= 1);
	for (std::deque<connection_log>::const_iterator i = ip_log_.begin();
			i != ip_log_.end(); ++i) {
		const std::string& username = i->nick;
		const std::string& ip = i->ip;
		if ((match_ip && utils::wildcard_string_match(ip, parameters))
		|| (!match_ip && utils::wildcard_string_match(username, parameters))) {
			found_something = true;
			wesnothd::player_map::const_iterator pl = std::find_if(players_.begin(), players_.end(), boost::bind(&::match_user, _1, username, ip));
			if (pl != players_.end()) {
				*out << std::endl << player_status(pl);
			} else {
				*out << "\n'" << username << "' @ " << ip << " last seen: " << lg::get_timestamp(i->log_off, "%H:%M:%S %d.%m.%Y");
			}
		}
	}
	if (!found_something) *out << "\nNo match found.";
}

void server::dul_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (parameters == "") {
		*out << "Unregistered login is " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
	} else {
		deny_unregistered_login_ = (utf8::lowercase(parameters) == "yes");
		*out << "Unregistered login is now " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
	}
}

void server::process_nickserv(const network::connection sock, simple_wml::node& data) {
	const wesnothd::player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		DBG_SERVER << "ERROR: Could not find player with socket: " << sock << "\n";
		return;
	}

	// Check if this server allows nick registration at all
	if(!user_handler_) {
		rooms_.lobby().send_server_message("This server does not allow username registration.", sock);
		return;
	}

	if(data.child("register")) {
		try {
			(user_handler_->add_user(pl->second.name(), (*data.child("register"))["mail"].to_string(),
				(*data.child("register"))["password"].to_string()));

			std::stringstream msg;
			msg << "Your username has been registered." <<
					// Warn that providing an email address might be a good idea
					((*data.child("register"))["mail"].empty() ?
					" It is recommended that you provide an email address for password recovery." : "");
			rooms_.lobby().send_server_message(msg.str(), sock);

			// Mark the player as registered and send the other clients
			// an update to dislpay this change
			pl->second.mark_registered();

			simple_wml::document diff;
			make_change_diff(games_and_users_list_.root(), NULL,
						 "user", pl->second.config_address(), diff);
			rooms_.lobby().send_data(diff);

		} catch (user_handler::error& e) {
			rooms_.lobby().send_server_message("There was an error registering your username. The error message was: "
			+ e.message, sock);
		}
		return;
	}

	// A user requested to update his password or mail
	if(data.child("set")) {
		if(!(user_handler_->user_exists(pl->second.name()))) {
			rooms_.lobby().send_server_message("You are not registered. Please register first.", sock);
			return;
		}

		const simple_wml::node& set = *(data.child("set"));

		try {
			user_handler_->set_user_detail(pl->second.name(), set["detail"].to_string(), set["value"].to_string());

			rooms_.lobby().send_server_message("Your details have been updated.", sock);

		} catch (user_handler::error& e) {
			rooms_.lobby().send_server_message("There was an error updating your details. The error message was: "
			+ e.message, sock);
		}

		return;
	}

	// A user requested information about another user
	if(data.child("details")) {
		rooms_.lobby().send_server_message("Valid details for this server are: " +
				user_handler_->get_valid_details(), sock);
		return;
	}

	// A user requested a list of which details can be set
	if(data.child("info")) {
		try {
			std::string res = user_handler_->user_info((*data.child("info"))["name"].to_string());
			rooms_.lobby().send_server_message(res, sock);
		} catch (user_handler::error& e) {
			rooms_.lobby().send_server_message("There was an error looking up the details of the user '" +
			(*data.child("info"))["name"].to_string() + "'. " +" The error message was: "
			+ e.message, sock);
		}
		return;
	}

	// A user requested to delete his nick
	if(data.child("drop")) {
		if(!(user_handler_->user_exists(pl->second.name()))) {
			rooms_.lobby().send_server_message("You are not registered.", sock);
			return;
		}

		// With the current policy of dissallowing to log in with a
		// registerd username without the password we should never get
		// to call this
		if(!(pl->second.registered())) {
			rooms_.lobby().send_server_message("You are not logged in.", sock);
			return;
		}

		try {
			user_handler_->remove_user(pl->second.name());
			rooms_.lobby().send_server_message("Your username has been dropped.", sock);

			// Mark the player as not registered and send the other clients
			// an update to dislpay this change
			pl->second.mark_registered(false);

			simple_wml::document diff;
			make_change_diff(games_and_users_list_.root(), NULL,
						 "user", pl->second.config_address(), diff);
			rooms_.lobby().send_data(diff);
		} catch (user_handler::error& e) {
			rooms_.lobby().send_server_message("There was an error dropping your username. The error message was: "
			+ e.message, sock);
		}
		return;
	}
}

void server::process_whisper(const network::connection sock,
                             simple_wml::node& whisper) const {
	if ((whisper["receiver"] == "") || (whisper["message"] == "")) {
		static simple_wml::document data(
		  "[message]\n"
		  "message=\"Invalid number of arguments\"\n"
		  "sender=\"server\"\n"
		  "[/message]\n", simple_wml::INIT_COMPRESSED);
		send_doc(data, sock);
		return;
	}
	const wesnothd::player_map::const_iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find whispering player. (socket: "
			<< sock << ")\n";
		return;
	}
	whisper.set_attr_dup("sender", pl->second.name().c_str());
	bool dont_send = false;
	const simple_wml::string_span& whisper_receiver = whisper["receiver"];
	for (wesnothd::player_map::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if (whisper_receiver != i->second.name().c_str()) {
			continue;
		}

		std::vector<wesnothd::game*>::const_iterator g;
		for (g = games_.begin(); g != games_.end(); ++g) {
			if (!(*g)->is_member(i->first)) continue;
			// Don't send to players in a running game the sender is part of.
			dont_send = ((*g)->started() && (*g)->is_player(i->first) && (*g)->is_member(sock));
			break;
		}
		if (dont_send) {
			break;
		}

		simple_wml::document cwhisper;
		whisper.copy_into(cwhisper.root().add_child("whisper"));
		send_doc(cwhisper, i->first);
		return;
	}

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("message");

	if (dont_send) {
		msg.set_attr("message", "You cannot send private messages to players in a running game you observe.");
	} else {
		msg.set_attr_dup("message", ("Can't find '" + whisper["receiver"].to_string() + "'.").c_str());
	}

	msg.set_attr("sender", "server");
	send_doc(data, sock);
}

void server::process_data_lobby(const network::connection sock,
                                simple_wml::document& data) {
	DBG_SERVER << "in process_data_lobby...\n";

	const wesnothd::player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find player in players_. (socket: "
			<< sock << ")\n";
		return;
	}

	if (const simple_wml::node* create_game = data.child("create_game")) {
		if (graceful_restart) {
			static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("This server is shutting down. You aren't allowed to make new games. Please reconnect to the new server.", sock);
			send_doc(games_and_users_list_, sock);
			return;
		}
		const std::string game_name = (*create_game)["name"].to_string();
		const std::string game_password = (*create_game)["password"].to_string();
		DBG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
			<< "\tcreates a new game: \"" << game_name << "\".\n";
		// Create the new game, remove the player from the lobby
		// and set the player as the host/owner.
		games_.push_back(new wesnothd::game(players_, sock, game_name, save_replays_, replay_save_path_));
		wesnothd::game& g = *games_.back();
		if(game_password.empty() == false) {
			g.set_password(game_password);
		}

		create_game->copy_into(g.level().root());
		rooms_.exit_lobby(sock);
		simple_wml::document diff;
		if(make_change_diff(games_and_users_list_.root(), NULL,
		                    "user", pl->second.config_address(), diff)) {
			rooms_.lobby().send_data(diff);
		}
		return;
	}

	// See if the player is joining a game
	if (const simple_wml::node* join = data.child("join")) {
		const bool observer = join->attr("observe").to_bool();
		const std::string& password = (*join)["password"].to_string();
		int game_id = (*join)["id"].to_int();

		const std::vector<wesnothd::game*>::iterator g =
			std::find_if(games_.begin(), games_.end(), wesnothd::game_id_matches(game_id));

		static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
		if (g == games_.end()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join unknown game:\t" << game_id << ".\n";
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("Attempt to join unknown game.", sock);
			send_doc(games_and_users_list_, sock);
			return;
		} else if (!(*g)->level_init()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join uninitialized game:\t\"" << (*g)->name()
				<< "\" (" << game_id << ").\n";
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("Attempt to join an uninitialized game.", sock);
			send_doc(games_and_users_list_, sock);
			return;
		} else if (pl->second.is_moderator()) {
			// Admins are always allowed to join.
		} else if ((*g)->player_is_banned(sock)) {
			DBG_SERVER << network::ip_address(sock) << "\tReject banned player: "
				<< pl->second.name() << "\tfrom game:\t\"" << (*g)->name()
				<< "\" (" << game_id << ").\n";
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("You are banned from this game.", sock);
			send_doc(games_and_users_list_, sock);
			return;
		} else if(!observer && !(*g)->password_matches(password)) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join game:\t\"" << (*g)->name() << "\" ("
				<< game_id << ") with bad password\n";
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("Incorrect password.", sock);
			send_doc(games_and_users_list_, sock);
			return;
		}
		// Hack to work around problems of players not getting properly removed
		// from a game. Still need to figure out actual cause...
		const std::vector<wesnothd::game*>::iterator g2 =
			std::find_if(games_.begin(), games_.end(), wesnothd::game_is_member(sock));
		if (g2 != games_.end()) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to join a second game. He's already in game:\t\""
				<< (*g2)->name() << "\" (" << (*g2)->id()
				<< ") and the lobby. (socket: " << sock << ")\n"
				<< "Removing him from that game to fix the inconsistency...\n";
			(*g2)->remove_player(sock);
		}
		bool joined = (*g)->add_player(sock, observer);
		if (!joined) {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tattempted to observe game:\t\"" << (*g)->name() << "\" ("
				<< game_id << ") which doesn't allow observers.\n";
			send_doc(leave_game_doc, sock);
			rooms_.lobby().send_server_message("Attempt to observe a game that doesn't allow observers. (You probably joined the game shortly after it filled up.)", sock);
			send_doc(games_and_users_list_, sock);
			return;
		}
		rooms_.exit_lobby(sock);
		(*g)->describe_slots();

		//send notification of changes to the game and user
		simple_wml::document diff;
		bool diff1 = make_change_diff(*games_and_users_list_.child("gamelist"),
					      "gamelist", "game", (*g)->description(), diff);
		bool diff2 = make_change_diff(games_and_users_list_.root(), NULL,
					      "user", pl->second.config_address(), diff);
		if (diff1 || diff2) {
			rooms_.lobby().send_data(diff);
		}
	}

	if (data.child("room_join")) {
		rooms_.process_room_join(data, pl);
	}

	if (data.child("room_part")) {
		rooms_.process_room_part(data, pl);
	}

	if (data.child("room_query")) {
		rooms_.process_room_query(data, pl);
	}

	// See if it's a message, in which case we add the name of the sender,
	// and forward it to all players in the lobby.
	if (data.child("message")) {
		rooms_.process_message(data, pl);
	}

	// Player requests update of lobby content,
	// for example when cancelling the create game dialog
	if (data.child("refresh_lobby")) {
		send_doc(games_and_users_list_, sock);
	}
}

/**
 * Process data sent from a member of a game.
 */
void server::process_data_game(const network::connection sock,
                               simple_wml::document& data) {
	DBG_SERVER << "in process_data_game...\n";

	const wesnothd::player_map::iterator pl = players_.find(sock);
	if (pl == players_.end()) {
		ERR_SERVER << "ERROR: Could not find player in players_. (socket: "
			<< sock << ")\n";
		return;
	}

	const std::vector<wesnothd::game*>::iterator itor =
		std::find_if(games_.begin(), games_.end(), wesnothd::game_is_member(sock));
	if (itor == games_.end()) {
		ERR_SERVER << "ERROR: Could not find game for player: "
			<< pl->second.name() << ". (socket: " << sock << ")\n";
		return;
	}

	wesnothd::game* g = *itor;

	// If this is data describing the level for a game.
	if (data.child("side")) {
		if (!g->is_owner(sock)) {
			return;
		}
		size_t nsides = 0;
		const simple_wml::node::child_list& sides = data.root().children("side");
		for (simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
			++nsides;
		}
		if (nsides > gamemap::MAX_PLAYERS) {
			delete_game(itor);
			std::stringstream msg;
			msg << "This server does not support games with more than "
				<< gamemap::MAX_PLAYERS << " sides. Game aborted.";
			rooms_.lobby().send_server_message(msg.str(), sock);
			return;
		}

		// If this game is having its level data initialized
		// for the first time, and is ready for players to join.
		// We should currently have a summary of the game in g->level().
		// We want to move this summary to the games_and_users_list_, and
		// place a pointer to that summary in the game's description.
		// g->level() should then receive the full data for the game.
		if (!g->level_init()) {
			LOG_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tcreated game:\t\"" << g->name() << "\" ("
				<< g->id() << ").\n";
			// Update our config object which describes the open games,
			// and save a pointer to the description in the new game.
			simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
			assert(gamelist != NULL);
			simple_wml::node& desc = gamelist->add_child("game");
			g->level().root().copy_into(desc);
			if (const simple_wml::node* m = data.child("multiplayer")) {
				m->copy_into(desc);
			} else {
				WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
					<< "\tsent scenario data in game:\t\"" << g->name() << "\" ("
					<< g->id() << ") without a 'multiplayer' child.\n";
				// Set the description so it can be removed in delete_game().
				g->set_description(&desc);
				delete_game(itor);
				rooms_.lobby().send_server_message("The scenario data is missing the [multiplayer] tag which contains the game settings. Game aborted.", sock);
				return;
			}

			g->set_description(&desc);
			desc.set_attr_dup("id", lexical_cast<std::string>(g->id()).c_str());
		} else {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tsent scenario data in game:\t\"" << g->name() << "\" ("
				<< g->id() << ") although it's already initialized.\n";
			return;
		}

		assert(games_and_users_list_.child("gamelist")->children("game").empty() == false);

		simple_wml::node& desc = *g->description();
		// Update the game's description.
		// If there is no shroud, then tell players in the lobby
		// what the map looks like
		if (!data["mp_shroud"].to_bool()) {
			desc.set_attr_dup("map_data", data["map_data"]);
		}
		if (const simple_wml::node* e = data.child("era")) {
			if (!e->attr("require_era").to_bool(true)) {
				desc.set_attr("require_era", "no");
			}
		}

		if (data.attr("require_scenario").to_bool(false)) {
			desc.set_attr("require_scenario", "yes");
		}

		const simple_wml::node::child_list& mlist = data.children("modification");
		BOOST_FOREACH (const simple_wml::node* m, mlist) {
			desc.add_child_at("modification", 0);
			desc.child("modification")->set_attr_dup("id", m->attr("id"));
			if (m->attr("require_modification").to_bool(false))
				desc.child("modification")->set_attr("require_modification", "yes");
		}

		// Record the full scenario in g->level()
		g->level().swap(data);
		// The host already put himself in the scenario so we just need
		// to update_side_data().
		//g->take_side(sock);
		g->update_side_data();
		g->describe_slots();

		assert(games_and_users_list_.child("gamelist")->children("game").empty() == false);

		// Send the update of the game description to the lobby.
		simple_wml::document diff;
		make_add_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", diff);
		rooms_.lobby().send_data(diff);

		/** @todo FIXME: Why not save the level data in the history_? */
		return;
// Everything below should only be processed if the game is already intialized.
	} else if (!g->level_init()) {
		WRN_SERVER << network::ip_address(sock) << "\tReceived unknown data from: "
			<< pl->second.name() << " (socket:" << sock
			<< ") while the scenario wasn't yet initialized.\n" << data.output();
		return;
	// If the host is sending the next scenario data.
	} else if (const simple_wml::node* scenario = data.child("store_next_scenario")) {
		if (!g->is_owner(sock)) return;
		if (!g->level_init()) {
			WRN_SERVER << network::ip_address(sock) << "\tWarning: "
				<< pl->second.name() << "\tsent [store_next_scenario] in game:\t\""
				<< g->name() << "\" (" << g->id()
				<< ") while the scenario is not yet initialized.";
			return;
		}
		g->save_replay();
		size_t nsides = 0;
		const simple_wml::node::child_list& sides = scenario->children("side");
		for (simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
	        	++nsides;
		}
		if (nsides > gamemap::MAX_PLAYERS) {
			delete_game(itor);
			std::stringstream msg;
			msg << "This server does not support games with more than "
				<< gamemap::MAX_PLAYERS << " sides.";
			rooms_.lobby().send_server_message(msg.str(), sock);
			return;
		}
		// Record the full scenario in g->level()
		g->level().clear();
		scenario->copy_into(g->level().root());

		if (g->description() == NULL) {
			ERR_SERVER << network::ip_address(sock) << "\tERROR: \""
				<< g->name() << "\" (" << g->id()
				<< ") is initialized but has no description_.\n";
			return;
		}
		simple_wml::node& desc = *g->description();
		// Update the game's description.
		if (const simple_wml::node* m = scenario->child("multiplayer")) {
			m->copy_into(desc);
		} else {
			WRN_SERVER << network::ip_address(sock) << "\t" << pl->second.name()
				<< "\tsent scenario data in game:\t\"" << g->name() << "\" ("
				<< g->id() << ") without a 'multiplayer' child.\n";
			delete_game(itor);
			rooms_.lobby().send_server_message("The scenario data is missing the [multiplayer] tag which contains the game settings. Game aborted.", sock);
			return;
		}

		// If there is no shroud, then tell players in the lobby
		// what the map looks like.
		const simple_wml::node& s = g->level().root();
		desc.set_attr_dup("map_data", s["mp_shroud"].to_bool() ? "" :
			s["map_data"]);
		if (const simple_wml::node* e = data.child("era")) {
			if (!e->attr("require_era").to_bool(true)) {
				desc.set_attr("require_era", "no");
			}
		}

		if (data.attr("require_scenario").to_bool(false)) {
			desc.set_attr("require_scenario", "yes");
		}

		// Tell everyone that the next scenario data is available.
		static simple_wml::document notify_next_scenario(
			"[notify_next_scenario]\n[/notify_next_scenario]\n",
			simple_wml::INIT_COMPRESSED);
		g->send_data(notify_next_scenario, sock);

		// Send the update of the game description to the lobby.
		update_game_in_lobby(g);

		return;
	// A mp client sends a request for the next scenario of a mp campaign.
	} else if (data.child("load_next_scenario")) {
		g->load_next_scenario(pl);
		return;
	} else if (data.child("start_game")) {
		if (!g->is_owner(sock)) return;
		//perform controller tweaks, assigning sides as human for their owners etc.
		g->perform_controller_tweaks();
		// Send notification of the game starting immediately.
		// g->start_game() will send data that assumes
		// the [start_game] message has been sent
		g->send_data(data, sock);
		g->start_game(pl);

		//update the game having changed in the lobby
		update_game_in_lobby(g);
		return;
	} else if (data.child("update_game")) {
		g->update_game();
		update_game_in_lobby(g);
		return;
	} else if (data.child("leave_game")) {
		// May be better to just let remove_player() figure out when a game ends.
		if ((g->is_player(sock) && g->nplayers() == 1)
		|| (g->is_owner(sock) && (!g->started() || g->nplayers() == 0))) {
			// Remove the player in delete_game() with all other remaining
			// ones so he gets the updated gamelist.
			delete_game(itor);
		} else {
			g->remove_player(sock);
			rooms_.enter_lobby(sock);
			g->describe_slots();

			// Send all other players in the lobby the update to the gamelist.
			simple_wml::document diff;
			bool diff1 = make_change_diff(*games_and_users_list_.child("gamelist"),
						      "gamelist", "game", g->description(), diff);
			bool diff2 = make_change_diff(games_and_users_list_.root(), NULL,
						      "user", pl->second.config_address(), diff);
			if (diff1 || diff2) {
				rooms_.lobby().send_data(diff, sock);
			}

			// Send the player who has quit the gamelist.
			send_doc(games_and_users_list_, sock);
		}
		return;
	// If this is data describing side changes by the host.
	} else if (const simple_wml::node* diff = data.child("scenario_diff")) {
		if (!g->is_owner(sock)) return;
		g->level().root().apply_diff(*diff);
		const simple_wml::node* cfg_change = diff->child("change_child");
		if (cfg_change && cfg_change->child("side")) {
			g->update_side_data();
		}
		if (g->describe_slots()) {
			update_game_in_lobby(g);
		}
		g->send_data(data, sock);
		return;
	// If a player changes his faction.
	} else if (data.child("change_faction")) {
		g->send_data(data, sock);
		return;
	// If the owner of a side is changing the controller.
	} else if (const simple_wml::node *change = data.child("change_controller")) {
		g->transfer_side_control(sock, *change);
		if (g->describe_slots()) {
			update_game_in_lobby(g);
		}
		return;
	// If all observers should be muted. (toggles)
	} else if (data.child("muteall")) {
		if (!g->is_owner(sock)) {
			g->send_server_message("You cannot mute: not the game host.", sock);
			return;
		}
		g->mute_all_observers();
		return;
	// If an observer should be muted.
	} else if (const simple_wml::node* mute = data.child("mute")) {
		g->mute_observer(*mute, pl);
		return;
	// If an observer should be unmuted.
	} else if (const simple_wml::node* unmute = data.child("unmute")) {
		g->unmute_observer(*unmute, pl);
		return;
	// The owner is kicking/banning someone from the game.
	} else if (data.child("kick") || data.child("ban")) {
		bool ban = (data.child("ban") != NULL);
		const network::connection user =
				(ban ? g->ban_user(*data.child("ban"), pl)
				: g->kick_member(*data.child("kick"), pl));
		if (user) {
			rooms_.enter_lobby(user);
			if (g->describe_slots()) {
				update_game_in_lobby(g, user);
			}
			// Send all other players in the lobby the update to the gamelist.
			simple_wml::document diff;
			make_change_diff(*games_and_users_list_.child("gamelist"),
						      "gamelist", "game", g->description(), diff);
			const wesnothd::player_map::iterator pl2 = players_.find(user);
			if (pl2 == players_.end()) {
				ERR_SERVER << "ERROR: Could not find kicked player in players_."
				" (socket: " << user << ")\n";
			} else {
				make_change_diff(games_and_users_list_.root(), NULL, "user",
						pl2->second.config_address(), diff);
			}
			rooms_.lobby().send_data(diff, sock);
			// Send the removed user the lobby game list.
			send_doc(games_and_users_list_, user);
		}
		return;
	} else if (const simple_wml::node* unban = data.child("unban")) {
		g->unban_user(*unban, pl);
		return;
	// If info is being provided about the game state.
	} else if (const simple_wml::node* info = data.child("info")) {
		if (!g->is_player(sock)) return;
		if ((*info)["type"] == "termination") {
			g->set_termination_reason((*info)["condition"].to_string());
			if ((*info)["condition"].to_string() == "out of sync") {
				g->send_server_message_to_all(pl->second.name() + " reports out of sync errors.");
			}
		}
		return;
	} else if (data.child("turn")) {
		// Notify the game of the commands, and if it changes
		// the description, then sync the new description
		// to players in the lobby.
		if (g->process_turn(data, pl)) {
			update_game_in_lobby(g);
		}
		return;
	} else if (data.child("whiteboard")) {
		g->process_whiteboard(data,pl);
		return;
	} else if (data.child("require_random")) {
		g->require_random(data,pl);
		return;
	} else if (data.child("message")) {
		g->process_message(data, pl);
		return;
	} else if (data.child("stop_updates")) {
		g->send_data(data, sock);
		return;
	} else if (data.child("wait_global")) {
		g->allow_global(data);
		return;
	// Data to ignore.
	} else if (data.child("error")
	|| data.child("side_secured")
	|| data.root().has_attr("failed")
	|| data.root().has_attr("side_drop")
	|| data.root().has_attr("side")) {
		return;
	}

	WRN_SERVER << network::ip_address(sock) << "\tReceived unknown data from: "
		<< pl->second.name() << " (socket:" << sock << ") in game: \""
		<< g->name() << "\" (" << g->id() << ")\n" << data.output();
}

void server::delete_game(std::vector<wesnothd::game*>::iterator game_it) {
	metrics_.game_terminated((*game_it)->termination_reason());

	simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
	assert(gamelist != NULL);

	// Send a diff of the gamelist with the game deleted to players in the lobby
	simple_wml::document diff;
	if(make_delete_diff(*gamelist, "gamelist", "game",
	                    (*game_it)->description(), diff)) {
		rooms_.lobby().send_data(diff);
	}

	// Delete the game from the games_and_users_list_.
	const simple_wml::node::child_list& games = gamelist->children("game");
	const simple_wml::node::child_list::const_iterator g =
		std::find(games.begin(), games.end(), (*game_it)->description());
	if (g != games.end()) {
		const size_t index = g - games.begin();
		gamelist->remove_child("game", index);
	} else {
		// Can happen when the game ends before the scenario was transferred.
		LOG_SERVER << "Could not find game (" << (*game_it)->id()
			<< ") to delete in games_and_users_list_.\n";
	}

	const wesnothd::user_vector& users = (*game_it)->all_game_users();
	// Set the availability status for all quitting users.
	for (wesnothd::user_vector::const_iterator user = users.begin();
		user != users.end(); ++user)
	{
		const wesnothd::player_map::iterator pl = players_.find(*user);
		if (pl != players_.end()) {
			pl->second.mark_available();
			simple_wml::document udiff;
			if (make_change_diff(games_and_users_list_.root(), NULL,
					     "user", pl->second.config_address(), udiff)) {
				rooms_.lobby().send_data(udiff);
			}
		} else {
			ERR_SERVER << "ERROR: delete_game(): Could not find user in players_. (socket: "
				<< *user << ")\n";
		}
	}

	//send users in the game a notification to leave the game since it has ended
	static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	(*game_it)->send_data(leave_game_doc);
	// Put the remaining users back in the lobby.
	rooms_.enter_lobby(**game_it);

	(*game_it)->send_data(games_and_users_list_);

	delete *game_it;
	games_.erase(game_it);
}

void server::update_game_in_lobby(const wesnothd::game* g, network::connection exclude)
{
	simple_wml::document diff;
	if (make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", g->description(), diff)) {
		rooms_.lobby().send_data(diff, exclude);
	}
}

       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#ifndef _MSC_VER
       #include <unistd.h>
#endif

int main(int argc, char** argv) {
	int port = 15000;
	size_t min_threads = 5;
	size_t max_threads = 0;

	srand(static_cast<unsigned>(time(NULL)));

	std::string config_file;

	// setting path to currentworking directory
	game_config::path = get_cwd();

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
			lg::set_log_domain_severity("all", 3);
		} else if (val.substr(0, 6) == "--log-") {
			size_t p = val.find('=');
			if (p == std::string::npos) {
				std::cerr << "unknown option: " << val << '\n';
				return 2;
			}
			std::string s = val.substr(6, p - 6);
			int severity;
			if (s == "error") severity = 0;
			else if (s == "warning") severity = 1;
			else if (s == "info") severity = 2;
			else if (s == "debug") severity = 3;
			else {
				std::cerr << "unknown debug level: " << s << '\n';
				return 2;
			}
			while (p != std::string::npos) {
				size_t q = val.find(',', p + 1);
				s = val.substr(p + 1, q == std::string::npos ? q : q - (p + 1));
				if (!lg::set_log_domain_severity(s, severity)) {
					std::cerr << "unknown debug domain: " << s << '\n';
					return 2;
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
#ifdef _WIN32
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
		} else if(val == "--request_sample_frequency" && arg+1 != argc) {
			request_sample_frequency = atoi(argv[++arg]);
		} else {
			ERR_SERVER << "unknown option: " << val << "\n";
			return 2;
		}
	}

	network::set_raw_data_only();

	try {
		server(port, config_file, min_threads, max_threads).run();
	} catch(network::error& e) {
		ERR_SERVER << "Caught network error while server was running. Aborting.: "
			<< e.message << "\n";

		/**
		 * @todo errno should be passed here with the error or it might not be
		 * the true errno anymore. Seems to work good enough for now though.
		 */
		return errno;
	}

	return 0;
}
