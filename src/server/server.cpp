/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "server/server.hpp"

#include "config.hpp"
#include "game_config.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "filesystem.hpp"
#include "multiplayer_error_codes.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "utils/iterable_pair.hpp"

#include "server/game.hpp"
#include "server/metrics.hpp"
#include "server/player.hpp"
#include "server/simple_wml.hpp"
#include "server/ban.hpp"
#include "exceptions.hpp"

#include "server/user_handler.hpp"
#include "server/sample_user_handler.hpp"

#ifdef HAVE_MYSQLPP
#include "server/forum_user_handler.hpp"
#endif

#include "utils/functional.hpp"

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

#include <boost/algorithm/string.hpp>

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

#include "server/send_receive_wml_helpers.ipp"

namespace wesnothd
{

// we take profiling info on every n requests
int request_sample_frequency = 1;

static void make_add_diff(const simple_wml::node& src, const char* gamelist,
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

static bool make_delete_diff(const simple_wml::node& src,
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

static bool make_change_diff(const simple_wml::node& src,
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

static std::string player_status(const wesnothd::player_record& player) {
	std::ostringstream out;
	//const network::connection_stats& stats = network::get_connection_stats(pl->first);
	//const int time_connected = stats.time_connected / 1000;
	//const int seconds = time_connected % 60;
	//const int minutes = (time_connected / 60) % 60;
	//const int hours = time_connected / (60 * 60);
	out << "'" << player.name() << "' @ " << client_address(player.socket());
	//		<< " connected for " << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds
	//		<< " sent " << stats.bytes_sent << " bytes, received "
	//		<< stats.bytes_received << " bytes";
	return out.str();
}

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

server::server(int port, bool keep_alive, const std::string& config_file, size_t /*min_threads*/,
			   size_t /*max_threads*/) :
	server_base(port, keep_alive),
	ban_manager_(),
	ip_log_(),
	failed_logins_(),
	user_handler_(nullptr),
#ifndef _WIN32
	input_path_(),
#endif
	config_file_(config_file),
	cfg_(read_config()),
	accepted_versions_(),
	redirected_versions_(),
	proxy_versions_(),
	disallowed_names_(),
	admin_passwd_(),
	motd_(),
	default_max_messages_(0),
	default_time_period_(0),
	concurrent_connections_(0),
	graceful_restart(false),
	lan_server_(time(nullptr)),
	last_user_seen_time_(time(nullptr)),
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
	last_ping_(time(nullptr)),
	last_stats_(last_ping_),
	last_uh_clean_(last_ping_),
	cmd_handlers_(),
	timer_(io_service_)
{
	setup_handlers();
	load_config();
	ban_manager_.read();

	start_server();
}

#ifndef _WIN32
void server::handle_sighup(const boost::system::error_code& error, int) {
	assert(!error);

	WRN_SERVER << "SIGHUP caught, reloading config\n";

	cfg_ = read_config();
	load_config();

	sighup_.async_wait(std::bind(&server::handle_sighup, this, _1, _2));
}
#endif

void server::handle_graceful_timeout(const boost::system::error_code& error)
{
	assert(!error);

	if(games().empty()) {
		process_command("msg All games ended. Shutting down now. Reconnect to the new server instance.", "system");
		throw server_shutdown("graceful shutdown timeout");
	} else {
		timer_.expires_from_now(boost::posix_time::seconds(1));
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, _1));
	}
}

void server::setup_fifo() {
#ifndef _WIN32
	const int res = mkfifo(input_path_.c_str(),0660);
	if(res != 0 && errno != EEXIST) {
		ERR_SERVER << "could not make fifo at '" << input_path_ << "' (" << strerror(errno) << ")\n";
		return;
	}
	int fifo = open(input_path_.c_str(), O_RDWR|O_NONBLOCK);
	input_.assign(fifo);
	LOG_SERVER << "opened fifo at '" << input_path_ << "'. Server commands may be written to this file.\n";
	read_from_fifo();
#endif
}

#ifndef _WIN32

void server::handle_read_from_fifo(const boost::system::error_code& error, std::size_t) {
	if(error) {
		std::cout << error.message() << std::endl;
		return;
	}

	std::istream is(&admin_cmd_);
	std::string cmd;
	std::getline(is, cmd);

	LOG_SERVER << "Admin Command: type: " << cmd << "\n";
	const std::string res = process_command(cmd, "*socket*");
	// Only mark the response if we fake the issuer (i.e. command comes from IRC or so)
	if (cmd.at(0) == '+') {
		LOG_SERVER << "[admin_command_response]\n" << res << "\n" << "[/admin_command_response]\n";
	} else {
		LOG_SERVER << res << "\n";
	}

	read_from_fifo();
}

#endif

void server::setup_handlers()
{
#define SETUP_HANDLER(name, function) \
	cmd_handlers_[name] = std::bind(function, this, \
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	SETUP_HANDLER("shut_down", &server::shut_down_handler);
	SETUP_HANDLER("restart", &server::restart_handler);
	SETUP_HANDLER("sample", &server::sample_handler);
	SETUP_HANDLER("help", &server::help_handler);
	SETUP_HANDLER("stats", &server::stats_handler);
	SETUP_HANDLER("metrics", &server::metrics_handler);
	SETUP_HANDLER("requests", &server::requests_handler);
	SETUP_HANDLER("games", &server::games_handler);
	SETUP_HANDLER("wml", &server::wml_handler);
	SETUP_HANDLER("netstats", &server::netstats_handler);
	SETUP_HANDLER("report", &server::adminmsg_handler);
	SETUP_HANDLER("adminmsg", &server::adminmsg_handler);
	SETUP_HANDLER("pm", &server::pm_handler);
	SETUP_HANDLER("privatemsg", &server::pm_handler);
	SETUP_HANDLER("msg", &server::msg_handler);
	SETUP_HANDLER("lobbymsg", &server::msg_handler);
	SETUP_HANDLER("status", &server::status_handler);
	SETUP_HANDLER("clones", &server::clones_handler);
	SETUP_HANDLER("bans", &server::bans_handler);
	SETUP_HANDLER("ban", &server::ban_handler);
	SETUP_HANDLER("unban", &server::unban_handler);
	SETUP_HANDLER("ungban", &server::ungban_handler);
	SETUP_HANDLER("kick", &server::kick_handler);
	SETUP_HANDLER("kickban", &server::kickban_handler);
	SETUP_HANDLER("kban", &server::kickban_handler);
	SETUP_HANDLER("gban", &server::gban_handler);
	SETUP_HANDLER("motd", &server::motd_handler);
	SETUP_HANDLER("searchlog", &server::searchlog_handler);
	SETUP_HANDLER("sl", &server::searchlog_handler);
	SETUP_HANDLER("dul", &server::dul_handler);
	SETUP_HANDLER("deny_unregistered_login", &server::dul_handler);

#undef SETUP_HANDLER
}

config server::read_config() const {
	config configuration;
	if (config_file_ == "") return configuration;
	try {
		filesystem::scoped_istream stream = preprocess_file(config_file_);
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
#	ifndef _WIN32
#		ifndef FIFODIR
#			warning No FIFODIR set
#		    define FIFODIR "/var/run/wesnothd"
#		endif
		const std::string fifo_path = (cfg_["fifo_path"].empty() ? std::string(FIFODIR) + "/socket" : std::string(cfg_["fifo_path"]));
		// Reset (replace) the input stream only if the FIFO path changed.
		if(fifo_path != input_path_) {
			input_.close();
			input_path_ = fifo_path;
			setup_fifo();
		}
#	endif

	save_replays_ = cfg_["save_replays"].to_bool();
	replay_save_path_ = cfg_["replay_save_path"].str();

	tor_ip_list_ = utils::split(cfg_["tor_ip_list_path"].empty() ? "" : filesystem::read_file(cfg_["tor_ip_list_path"]), '\n');

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

	accepted_versions_.clear();
	const std::string& versions = cfg_["versions_accepted"];
	if (versions.empty() == false) {
		accepted_versions_ = utils::split(versions);
	} else {
		accepted_versions_.push_back(game_config::version);
		accepted_versions_.push_back("test");
	}

	redirected_versions_.clear();
	for(const config &redirect : cfg_.child_range("redirect")) {
		for(const std::string &version : utils::split(redirect["version"])) {
			redirected_versions_[version] = redirect;
		}
	}

	proxy_versions_.clear();
	for(const config &proxy : cfg_.child_range("proxy")) {
		for(const std::string &version : utils::split(proxy["version"])) {
			proxy_versions_[version] = proxy;
		}
	}
	ban_manager_.load_config(cfg_);

	// If there is a [user_handler] tag in the config file
	// allow nick registration, otherwise we set user_handler_
	// to nullptr. Thus we must check user_handler_ for not being
	// nullptr everytime we want to use it.
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

std::string server::is_ip_banned(const std::string& ip) const {
	if (!tor_ip_list_.empty()) {
		if (find(tor_ip_list_.begin(), tor_ip_list_.end(), ip) != tor_ip_list_.end()) return "TOR IP";
	}
	return ban_manager_.is_ip_banned(ip);
}

void server::dump_stats(const time_t& now) {
	last_stats_ = now;
	LOG_SERVER << "Statistics:"
			   << "\tnumber_of_games = " << games().size()
			   << "\tnumber_of_users = " << player_connections_.size() << "\n";
}

void server::clean_user_handler(const time_t& now) {
	if(!user_handler_) {
		return;
	}
	last_uh_clean_ = now;
	user_handler_->clean_up();
}

void server::handle_new_client(socket_ptr socket)
{
	async_send_doc(socket, version_query_response_,
				   std::bind(&server::handle_version, this, _1)
				   );
}

void server::handle_version(socket_ptr socket)
{
	async_receive_doc(socket,
		std::bind(&server::read_version, this, _1, _2)
	);
}

void server::read_version(socket_ptr socket, std::shared_ptr<simple_wml::document> doc)
{
	if (const simple_wml::node* const version = doc->child("version")) {
		const simple_wml::string_span& version_str_span = (*version)["version"];
		const std::string version_str(version_str_span.begin(),
									  version_str_span.end());
		std::vector<std::string>::const_iterator accepted_it;
		// Check if it is an accepted version.
		accepted_it = std::find_if(accepted_versions_.begin(), accepted_versions_.end(),
			std::bind(&utils::wildcard_string_match, version_str, _1));
		if(accepted_it != accepted_versions_.end()) {
			LOG_SERVER << client_address(socket)
				<< "\tplayer joined using accepted version " << version_str
				<< ":\ttelling them to log in.\n";
			async_send_doc(socket, login_response_, std::bind(&server::login, this, _1));
			return;
		}

		simple_wml::document response;

		// Check if it is a redirected version
		for(const auto& redirect_version : redirected_versions_) {
			if(utils::wildcard_string_match(version_str, redirect_version.first)) {
				LOG_SERVER << client_address(socket)
						   << "\tplayer joined using version " << version_str
						   << ":\tredirecting them to " << redirect_version.second["host"]
						   << ":" << redirect_version.second["port"] << "\n";
				simple_wml::node& redirect = response.root().add_child("redirect");
				for(const auto& attr : redirect_version.second.attribute_range()) {
					redirect.set_attr(attr.first.c_str(), attr.second.str().c_str());
				}
				send_to_player(socket, response);
				return;
			}
		}

		LOG_SERVER << client_address(socket)
				   << "\tplayer joined using unknown version " << version_str
				   << ":\trejecting them\n";

		// For compatibility with older clients
		response.set_attr("version", accepted_versions_.begin()->c_str());

		simple_wml::node& reject = response.root().add_child("reject");
		reject.set_attr_dup("accepted_versions", utils::join(accepted_versions_).c_str());
		send_to_player(socket, response);
	} else {
		LOG_SERVER << client_address(socket)
				   << "\tclient didn't send its version: rejecting\n";
	}
}

void server::login(socket_ptr socket)
{
	async_receive_doc(socket,
		std::bind(&server::handle_login, this, _1, _2)
	);
}

void server::handle_login(socket_ptr socket, std::shared_ptr<simple_wml::document> doc)
{
	if(const simple_wml::node* const login = doc->child("login")) {
		// Check if the username is valid (all alpha-numeric plus underscore and hyphen)
		std::string username = (*login)["username"].to_string();
		if (!utils::isvalid_username(username)) {
			async_send_error(socket, "The nickname '" + username + "' contains invalid "
									 "characters. Only alpha-numeric characters, underscores and hyphens"
									 "are allowed.", MP_INVALID_CHARS_IN_NAME_ERROR);
			server::login(socket);
			return;
		}
		if (username.size() > 20) {
			async_send_error(socket, "The nickname '" + username + "' is too long. Nicks must be 20 characters or less.",
							 MP_NAME_TOO_LONG_ERROR);
			server::login(socket);
			return;
		}
		// Check if the username is allowed.
		for (std::vector<std::string>::const_iterator d_it = disallowed_names_.begin();
			 d_it != disallowed_names_.end(); ++d_it)
		{
			if (utils::wildcard_string_match(utf8::lowercase(username),
											 utf8::lowercase(*d_it)))
			{
				async_send_error(socket, "The nickname '" + username + "' is reserved and cannot be used by players",
								 MP_NAME_RESERVED_ERROR);
				server::login(socket);
				return;
			}
		}

		// If this is a request for password reminder
		if(user_handler_) {
			std::string password_reminder = (*login)["password_reminder"].to_string();
			if(password_reminder == "yes") {
				try {
					user_handler_->password_reminder(username);
					async_send_error(socket, "Your password reminder email has been sent.");
				} catch (user_handler::error& e) {
					async_send_error(socket, "There was an error sending your password reminder email. The error message was: " +
									 e.message);
				}
				return;
			}
		}

		// Check the username isn't already taken
		auto p = player_connections_.get<name_t>().find(username);
		bool name_taken = p != player_connections_.get<name_t>().end();

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
				async_send_warning(socket, "The nickname '" + username + "' is inactive. You cannot claim ownership of this "
																		 "nickname until you activate your account via email or ask an administrator to do it for you.", MP_NAME_INACTIVE_WARNING);
				//registered = false;
			}
			else if(exists) {
				// This name is registered and no password provided
				if(password.empty()) {
					if(!name_taken) {
						send_password_request(socket, "The nickname '" + username +"' is registered on this server.",
											  username, MP_PASSWORD_REQUEST);
					} else {
						send_password_request(socket, "The nickname '" + username + "' is registered on this server."
													  "\n\nWARNING: There is already a client using this username, "
													  "logging in will cause that client to be kicked!",
											  username, MP_PASSWORD_REQUEST_FOR_LOGGED_IN_NAME, true);
					}
					return;
				}

				// A password (or hashed password) was provided, however
				// there is no seed
				if(seeds_[reinterpret_cast<long int>(socket.get())].empty()) {
					send_password_request(socket, "Please try again.", username, MP_NO_SEED_ERROR);
					return;
				}
				// This name is registered and an incorrect password provided
				else if(!(user_handler_->login(username, password, seeds_[reinterpret_cast<unsigned long>(socket.get())]))) {
					const time_t now = time(NULL);

					// Reset the random seed
					seeds_.erase(reinterpret_cast<unsigned long>(socket.get()));

					login_log login_ip = login_log(client_address(socket), 0, now);
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
						async_send_error(socket, "You have made too many failed login attempts.", MP_TOO_MANY_ATTEMPTS_ERROR);
					} else {
						send_password_request(socket, "The password you provided for the nickname '" + username +
											  "' was incorrect.", username, MP_INCORRECT_PASSWORD_ERROR);
					}

					// Log the failure
					LOG_SERVER << client_address(socket) << "\t"
							   << "Login attempt with incorrect password for nickname '" << username << "'.\n";
					return;
				}
				// This name exists and the password was neither empty nor incorrect
				registered = true;
				// Reset the random seed
				seeds_.erase(reinterpret_cast<long int>(socket.get()));
				user_handler_->user_logged_in(username);
			}
		}

		// If we disallow unregistered users and this user is not registered send an error
		if(user_handler_ && !registered && deny_unregistered_login_) {
			async_send_error(socket, "The nickname '" + username + "' is not registered. "
									 "This server disallows unregistered nicknames.", MP_NAME_UNREGISTERED_ERROR);
			return;
		}

		if(name_taken) {
			if(registered) {
				// If there is already a client using this username kick it
				process_command("kick " + p->info().name() + " autokick by registered user", username);
			} else {
				async_send_error(socket, "The nickname '" + username + "' is already taken.", MP_NAME_TAKEN_ERROR);
				server::login(socket);
				return;
			}
		}

		simple_wml::node& player_cfg = games_and_users_list_.root().add_child("user");
		async_send_doc(socket, join_lobby_response_,
			std::bind(&server::add_player, this, _1,
				wesnothd::player(username, player_cfg, registered,
				default_max_messages_, default_time_period_, false/*selective_ping*/,
				user_handler_ && user_handler_->user_is_moderator(username))
			)
		);
		LOG_SERVER << client_address(socket) << "\t" << username
				   << "\thas logged on" << (registered ? " to a registered account" : "") << "\n";

		if(user_handler_ && user_handler_->user_is_moderator(username)) {
			LOG_SERVER << "Admin automatically recognized: IP: "
					   << client_address(socket) << "\tnick: "
					   << username << std::endl;
			// This string is parsed by the client!
			send_server_message(socket, "You are now recognized as an administrator. "
										"If you no longer want to be automatically authenticated use '/query signout'.");
		}

		// Log the IP
		connection_log ip_name = connection_log(username, client_address(socket), 0);
		if (std::find(ip_log_.begin(), ip_log_.end(), ip_name) == ip_log_.end()) {
			ip_log_.push_back(ip_name);
			// Remove the oldest entry if the size of the IP log exceeds the maximum size
			if(ip_log_.size() > max_ip_log_size_) ip_log_.pop_front();
		}
	} else {
		async_send_error(socket, "You must login first.", MP_MUST_LOGIN);
	}
}

void server::send_password_request(socket_ptr socket, const std::string& msg,
								   const std::string& user, const char* error_code, bool force_confirmation)
{
	std::string salt = user_handler_->create_salt();
	std::string pepper = user_handler_->create_pepper(user);
	std::string spices = pepper + salt;
	if(user_handler_->use_phpbb_encryption() && pepper.empty()) {
		async_send_error(socket, "Even though your nickname is registered on this server you "
								 "cannot log in due to an error in the hashing algorithm. "
								 "Logging into your forum account on https://forums.wesnoth.org "
								 "may fix this problem.");
		login(socket);
		return;
	}

	seeds_[reinterpret_cast<long int>(socket.get())] = salt;

	simple_wml::document doc;
	simple_wml::node& e = doc.root().add_child("error");
	e.set_attr_dup("message", msg.c_str());
	e.set_attr("password_request", "yes");
	e.set_attr("phpbb_encryption", user_handler_->use_phpbb_encryption() ? "yes" : "no");
	e.set_attr_dup("salt", spices.c_str());
	e.set_attr("force_confirmation", force_confirmation ? "yes" : "no");
	if(*error_code != '\0') {
		e.set_attr("error_code", error_code);
	}

	async_send_doc(socket, doc,
		std::bind(&server::login, this, _1)
	);
}

void server::add_player(socket_ptr socket, const wesnothd::player& player)
{
	bool inserted;
	std::tie(std::ignore, inserted) = player_connections_.insert(player_connections::value_type(socket, player));
	assert(inserted);

	send_to_player(socket, games_and_users_list_);

	if (motd_ != "") {
		send_server_message(socket, motd_);
	}

	read_from_player(socket);

	// Send other players in the lobby the update that the player has joined
	simple_wml::document diff;
	make_add_diff(games_and_users_list_.root(), NULL, "user", diff);
	send_to_lobby(diff, socket);
}

void server::read_from_player(socket_ptr socket)
{
	async_receive_doc(socket,
		std::bind(&server::handle_read_from_player, this, _1, _2),
		std::bind(&server::remove_player, this, _1)
	);
}

void server::handle_read_from_player(socket_ptr socket, std::shared_ptr<simple_wml::document> doc)
{
	read_from_player(socket);
	//DBG_SERVER << client_address(socket) << "\tWML received:\n" << doc->output() << std::endl;
	if(doc->child("refresh_lobby")) {
		send_to_player(socket, games_and_users_list_);
	}

	if(simple_wml::node* whisper = doc->child("whisper")) {
		handle_whisper(socket, *whisper);
	}
	if(simple_wml::node* query = doc->child("query")) {
		handle_query(socket, *query);
	}
	if(simple_wml::node* nickserv = doc->child("nickserv")) {
		handle_nickserv(socket, *nickserv);
	}

	if(!player_is_in_game(socket))
		handle_player_in_lobby(socket, doc);
	else
		handle_player_in_game(socket, doc);

}

void server::handle_player_in_lobby(socket_ptr socket, std::shared_ptr<simple_wml::document> doc) {
	if(simple_wml::node* message = doc->child("message")) {
		handle_message(socket, *message);
	}

	if(simple_wml::node* create_game = doc->child("create_game")) {
		handle_create_game(socket, *create_game);
	}

	if(simple_wml::node* join = doc->child("join")) {
		handle_join_game(socket, *join);
	}
}

void server::handle_whisper(socket_ptr socket, simple_wml::node& whisper)
{
	if((whisper["receiver"] == "") || (whisper["message"] == "")) {
		static simple_wml::document data(
					"[message]\n"
					"message=\"Invalid number of arguments\"\n"
					"sender=\"server\"\n"
					"[/message]\n", simple_wml::INIT_COMPRESSED);
		send_to_player(socket, data);
		return;
	}

	auto receiver_iter = player_connections_.get<name_t>().find(whisper["receiver"].to_string());
	if(receiver_iter == player_connections_.get<name_t>().end()) {
		send_server_message(socket, "Can't find '" + whisper["receiver"].to_string() + "'.");
	} else {
		simple_wml::document cwhisper;
		whisper.copy_into(cwhisper.root().add_child("whisper"));
		send_to_player(receiver_iter->socket(), cwhisper);
		// TODO: Refuse to send from an observer to a game he observes
	}
}

void server::handle_query(socket_ptr socket, simple_wml::node& query)
{
	auto iter = player_connections_.find(socket);
	if(iter == player_connections_.end())
		return;

	wesnothd::player& player = iter->info();

	const std::string command(query["type"].to_string());
	std::ostringstream response;
	const std::string& query_help_msg = "Available commands are: adminmsg <msg>, help, games, metrics,"
								  " motd, netstats [all], requests, sample, stats, status, wml.";
	// Commands a player may issue.
	if (command == "status") {
		response << process_command(command + " " + player.name(), player.name());
	} else if (command.compare(0, 8, "adminmsg") == 0 || command.compare(0, 6, "report") == 0
			   || command == "games"
			   || command == "metrics"
			   || command == "motd"
			   || command == "netstats"
			   || command == "netstats all"
			   || command == "requests"
			   || command == "sample"
			   || command == "stats"
			   || command == "status " + player.name()
			   || command == "wml")
	{
		response << process_command(command, player.name());
	} else if (player.is_moderator()) {
		if (command == "signout") {
			LOG_SERVER << "Admin signed out: IP: "
					   << client_address(socket) << "\tnick: "
					   << player.name() << std::endl;
			player.set_moderator(false);
			// This string is parsed by the client!
			response << "You are no longer recognized as an administrator.";
			if(user_handler_) {
				user_handler_->set_is_moderator(player.name(), false);
			}
		} else {
			LOG_SERVER << "Admin Command: type: " << command
					   << "\tIP: "<< client_address(socket)
					   << "\tnick: "<< player.name() << std::endl;
			response << process_command(command, player.name());
			LOG_SERVER << response.str() << std::endl;
		}
	} else if (command == "help" || command.empty()) {
		response << query_help_msg;
	} else if (command == "admin" || command.compare(0, 6, "admin ") == 0) {
		if (admin_passwd_.empty()) {
			send_server_message(socket, "No password set.");
			return;
		}
		std::string passwd;
		if (command.size() >= 6) passwd = command.substr(6);
		if (passwd == admin_passwd_) {
			LOG_SERVER << "New Admin recognized: IP: "
					   << client_address(socket) << "\tnick: "
					   << player.name() << std::endl;
			player.set_moderator(true);
			// This string is parsed by the client!
			response << "You are now recognized as an administrator.";
			if (user_handler_) {
				user_handler_->set_is_moderator(player.name(), true);
			}
		} else {
			WRN_SERVER << "FAILED Admin attempt with password: '" << passwd << "'\tIP: "
					   << client_address(socket) << "\tnick: "
					   << player.name() << std::endl;
			response << "Error: wrong password";
		}
	} else {
		response << "Error: unrecognized query: '" << command << "'\n" << query_help_msg;
	}
	send_server_message(socket, response.str());
}

void server::handle_nickserv(socket_ptr socket, simple_wml::node& nickserv)
{
	// Check if this server allows nick registration at all
	if(!user_handler_) {
		send_server_message(socket, "This server does not allow username registration.");
		return;
	}

	if(nickserv.child("register")) {
		try {
			(user_handler_->add_user(player_connections_.find(socket)->name(), (*nickserv.child("register"))["mail"].to_string(),
					(*nickserv.child("register"))["password"].to_string()));

			std::stringstream msg;
			msg << "Your username has been registered." <<
				   // Warn that providing an email address might be a good idea
				   ((*nickserv.child("register"))["mail"].empty() ?
					   " It is recommended that you provide an email address for password recovery." : "");
			send_server_message(socket, msg.str());

			// Mark the player as registered and send the other clients
			// an update to dislpay this change
			player_connections_.find(socket)->info().mark_registered();

			simple_wml::document diff;
			make_change_diff(games_and_users_list_.root(), NULL,
							 "user", player_connections_.find(socket)->info().config_address(), diff);
			send_to_lobby(diff);

		} catch (user_handler::error& e) {
			send_server_message(socket, "There was an error registering your username. The error message was: "
								+ e.message);
		}
		return;
	}

	// A user requested to update his password or mail
	if(nickserv.child("set")) {
		if(!(user_handler_->user_exists(player_connections_.find(socket)->name()))) {
			send_server_message(socket, "You are not registered. Please register first.");
			return;
		}

		const simple_wml::node& set = *(nickserv.child("set"));

		try {
			user_handler_->set_user_detail(player_connections_.find(socket)->name(), set["detail"].to_string(), set["value"].to_string());

			send_server_message(socket, "Your details have been updated.");

		} catch (user_handler::error& e) {
			send_server_message(socket, "There was an error updating your details. The error message was: "
								+ e.message);
		}

		return;
	}

	// A user requested information about another user
	if(nickserv.child("details")) {
		send_server_message(socket, "Valid details for this server are: " +
							user_handler_->get_valid_details());
		return;
	}

	// A user requested a list of which details can be set
	if(nickserv.child("info")) {
		try {
			std::string res = user_handler_->user_info((*nickserv.child("info"))["name"].to_string());
			send_server_message(socket, res);
		} catch (user_handler::error& e) {
			send_server_message(socket, "There was an error looking up the details of the user '" +
								(*nickserv.child("info"))["name"].to_string() + "'. " +" The error message was: "
					+ e.message);
		}
		return;
	}

	// A user requested to delete his nick
	if(nickserv.child("drop")) {
		if(!(user_handler_->user_exists(player_connections_.find(socket)->name()))) {
			send_server_message(socket, "You are not registered.");
			return;
		}

		// With the current policy of dissallowing to log in with a
		// registerd username without the password we should never get
		// to call this
		if(!(player_connections_.find(socket)->info().registered())) {
			send_server_message(socket, "You are not logged in.");
			return;
		}

		try {
			user_handler_->remove_user(player_connections_.find(socket)->name());
			send_server_message(socket, "Your username has been dropped.");

			// Mark the player as not registered and send the other clients
			// an update to dislpay this change
			player_connections_.find(socket)->info().mark_registered(false);

			simple_wml::document diff;
			make_change_diff(games_and_users_list_.root(), NULL,
							 "user", player_connections_.find(socket)->info().config_address(), diff);
			send_to_lobby(diff);
		} catch (user_handler::error& e) {
			send_server_message(socket, "There was an error dropping your username. The error message was: "
								+ e.message);
		}
		return;
	}
}

void server::handle_message(socket_ptr socket, simple_wml::node& message)
{
	simple_wml::document relay_message;
	message.copy_into(relay_message.root().add_child("message"));
	send_to_lobby(relay_message, socket);
}

void server::handle_create_game(socket_ptr socket, simple_wml::node& create_game)
{
	if (graceful_restart) {
		static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
		send_to_player(socket, leave_game_doc);
		send_server_message(socket, "This server is shutting down. You aren't allowed to make new games. Please reconnect to the new server.");
		send_to_player(socket, games_and_users_list_);
		return;
	}

	player_connections_.modify(
		player_connections_.find(socket),
		std::bind(&server::create_game, this, _1, std::ref(create_game))
	);

	simple_wml::document diff;
	if(make_change_diff(games_and_users_list_.root(), NULL,
						"user", player_connections_.find(socket)->info().config_address(), diff)) {
		send_to_lobby(diff);
	}
	return;
}

void server::create_game(player_record& host_record, simple_wml::node& create_game)
{
	const std::string game_name = create_game["name"].to_string();
	const std::string game_password = create_game["password"].to_string();

	DBG_SERVER << client_address(host_record.socket()) << "\t" << host_record.info().name()
			   << "\tcreates a new game: \"" << game_name << "\".\n";

	// Create the new game, remove the player from the lobby
	// and set the player as the host/owner.
	host_record.get_game().reset(
		new wesnothd::game(player_connections_, host_record.socket(), game_name, save_replays_, replay_save_path_),
		std::bind(&server::cleanup_game, this, _1));
	wesnothd::game& g = *host_record.get_game();
	if(game_password.empty() == false) {
		g.set_password(game_password);
	}

	create_game.copy_into(g.level().root());
}

void server::cleanup_game(game* game_ptr)
{
	metrics_.game_terminated(game_ptr->termination_reason());

	simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
	assert(gamelist != NULL);

	// Send a diff of the gamelist with the game deleted to players in the lobby
	simple_wml::document diff;
	if(make_delete_diff(*gamelist, "gamelist", "game",
						game_ptr->description(), diff)) {
		send_to_lobby(diff);
	}

	// Delete the game from the games_and_users_list_.
	const simple_wml::node::child_list& games = gamelist->children("game");
	const simple_wml::node::child_list::const_iterator g =
			std::find(games.begin(), games.end(), game_ptr->description());
	if (g != games.end()) {
		const size_t index = g - games.begin();
		gamelist->remove_child("game", index);
	} else {
		// Can happen when the game ends before the scenario was transferred.
		LOG_SERVER << "Could not find game (" << game_ptr->id()
				   << ") to delete in games_and_users_list_.\n";
	}
	delete game_ptr;
}

void server::handle_join_game(socket_ptr socket, simple_wml::node& join)
{
	const bool observer = join.attr("observe").to_bool();
	const std::string& password = join["password"].to_string();
	int game_id = join["id"].to_int();

	auto g_iter = player_connections_.get<game_t>().find(game_id);
	std::shared_ptr<game> g;
	if(g_iter != player_connections_.get<game_t>().end())
		g = g_iter->get_game();

	static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	if (!g) {
		WRN_SERVER << client_address(socket) << "\t" << player_connections_.find(socket)->info().name()
				   << "\tattempted to join unknown game:\t" << game_id << ".\n";
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "Attempt to join unknown game.");
		async_send_doc(socket, games_and_users_list_);
		return;
	} else if (!g->level_init()) {
		WRN_SERVER << client_address(socket) << "\t" << player_connections_.find(socket)->info().name()
				   << "\tattempted to join uninitialized game:\t\"" << g->name()
				   << "\" (" << game_id << ").\n";
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "Attempt to join an uninitialized game.");
		async_send_doc(socket, games_and_users_list_);
		return;
	} else if (player_connections_.find(socket)->info().is_moderator()) {
		// Admins are always allowed to join.
	} else if (g->registered_users_only() && !player_connections_.find(socket)->info().registered()) {
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "Only registered users are allowed to join this game.");
		async_send_doc(socket, games_and_users_list_);
		return;
	} else if (g->player_is_banned(socket)) {
		DBG_SERVER << client_address(socket) << "\tReject banned player: "
				   << player_connections_.find(socket)->info().name() << "\tfrom game:\t\"" << g->name()
				   << "\" (" << game_id << ").\n";
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "You are banned from this game.");
		async_send_doc(socket, games_and_users_list_);
		return;
	} else if(!observer && !g->password_matches(password)) {
		WRN_SERVER << client_address(socket) << "\t" << player_connections_.find(socket)->info().name()
				   << "\tattempted to join game:\t\"" << g->name() << "\" ("
				   << game_id << ") with bad password\n";
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "Incorrect password.");
		async_send_doc(socket, games_and_users_list_);
		return;
	}
	bool joined = g->add_player(socket, observer);
	if (!joined) {
		WRN_SERVER << client_address(socket) << "\t" << player_connections_.find(socket)->info().name()
				   << "\tattempted to observe game:\t\"" << g->name() << "\" ("
				   << game_id << ") which doesn't allow observers.\n";
		async_send_doc(socket, leave_game_doc);
		send_server_message(socket, "Attempt to observe a game that doesn't allow observers. (You probably joined the game shortly after it filled up.)");
		async_send_doc(socket, games_and_users_list_);
		return;
	}
	player_connections_.modify(
		player_connections_.find(socket),
		std::bind(&player_record::set_game, _1, player_connections_.get<game_t>().find(game_id)->get_game()));
	g->describe_slots();

	//send notification of changes to the game and user
	simple_wml::document diff;
	bool diff1 = make_change_diff(*games_and_users_list_.child("gamelist"),
								  "gamelist", "game", g->description(), diff);
	bool diff2 = make_change_diff(games_and_users_list_.root(), NULL,
								  "user", player_connections_.find(socket)->info().config_address(), diff);
	if (diff1 || diff2) {
		send_to_lobby(diff);
	}
}

void server::handle_player_in_game(socket_ptr socket, std::shared_ptr<simple_wml::document> doc) {
	DBG_SERVER << "in process_data_game...\n";

	auto p = player_connections_.find(socket);
	wesnothd::player& player = p->info();
	game& g = *(p->get_game());

	simple_wml::document& data = *doc;

	// If this is data describing the level for a game.
	if (doc->child("snapshot") || doc->child("scenario")) {
		if (!g.is_owner(socket)) {
			return;
		}
		// If this game is having its level data initialized
		// for the first time, and is ready for players to join.
		// We should currently have a summary of the game in g.level().
		// We want to move this summary to the games_and_users_list_, and
		// place a pointer to that summary in the game's description.
		// g.level() should then receive the full data for the game.
		if (!g.level_init()) {
			LOG_SERVER << client_address(socket) << "\t" << player.name()
					   << "\tcreated game:\t\"" << g.name() << "\" ("
					   << g.id() << ").\n";
			// Update our config object which describes the open games,
			// and save a pointer to the description in the new game.
			simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
			assert(gamelist != NULL);
			simple_wml::node& desc = gamelist->add_child("game");
			g.level().root().copy_into(desc);
			if (const simple_wml::node* m = doc->child("multiplayer")) {
				m->copy_into(desc);
			} else {
				WRN_SERVER << client_address(socket) << "\t" << player.name()
						   << "\tsent scenario data in game:\t\"" << g.name() << "\" ("
						   << g.id() << ") without a 'multiplayer' child.\n";
				// Set the description so it can be removed in delete_game().
				g.set_description(&desc);
				delete_game(g.id());
				send_server_message(socket, "The scenario data is missing the [multiplayer] tag which contains the game settings. Game aborted.");
				return;
			}

			g.set_description(&desc);
			desc.set_attr_dup("id", lexical_cast<std::string>(g.id()).c_str());
		} else {
			WRN_SERVER << client_address(socket) << "\t" << player.name()
					   << "\tsent scenario data in game:\t\"" << g.name() << "\" ("
					   << g.id() << ") although it's already initialized.\n";
			return;
		}

		assert(games_and_users_list_.child("gamelist")->children("game").empty() == false);

		simple_wml::node& desc = *g.description();
		// Update the game's description.
		// If there is no shroud, then tell players in the lobby
		// what the map looks like
		if (!data["mp_shroud"].to_bool()) {
			desc.set_attr_dup("map_data", (*wesnothd::game::starting_pos(data.root()))["map_data"]);
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
		for(const simple_wml::node* m : mlist) {
			desc.add_child_at("modification", 0);
			desc.child("modification")->set_attr_dup("id", m->attr("id"));
			desc.child("modification")->set_attr_dup("addon_id", m->attr("addon_id"));
			if (m->attr("require_modification").to_bool(false))
				desc.child("modification")->set_attr("require_modification", "yes");
		}

		// Record the full scenario in g.level()
		g.level().swap(data);
		// The host already put himself in the scenario so we just need
		// to update_side_data().
		//g.take_side(sock);
		g.update_side_data();
		g.describe_slots();

		assert(games_and_users_list_.child("gamelist")->children("game").empty() == false);

		// Send the update of the game description to the lobby.
		simple_wml::document diff;
		make_add_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", diff);
		send_to_lobby(diff);
		/** @todo FIXME: Why not save the level data in the history_? */
		return;
		// Everything below should only be processed if the game is already intialized.
	} else if (!g.level_init()) {
		WRN_SERVER << client_address(socket) << "\tReceived unknown data from: "
				   << player.name() << " (socket:" << socket
				   << ") while the scenario wasn't yet initialized.\n" << data.output();
		return;
		// If the host is sending the next scenario data.
	} else if (const simple_wml::node* scenario = data.child("store_next_scenario")) {
		if (!g.is_owner(socket)) return;
		if (!g.level_init()) {
			WRN_SERVER << client_address(socket) << "\tWarning: "
					   << player.name() << "\tsent [store_next_scenario] in game:\t\""
					   << g.name() << "\" (" << g.id()
					   << ") while the scenario is not yet initialized.";
			return;
		}
		g.save_replay();
		g.reset_last_synced_context_id();
		// Record the full scenario in g.level()
		g.level().clear();
		scenario->copy_into(g.level().root());

		if (g.description() == NULL) {
			ERR_SERVER << client_address(socket) << "\tERROR: \""
					   << g.name() << "\" (" << g.id()
					   << ") is initialized but has no description_.\n";
			return;
		}
		simple_wml::node& desc = *g.description();
		// Update the game's description.
		if (const simple_wml::node* m = scenario->child("multiplayer")) {
			m->copy_into(desc);
		} else {
			WRN_SERVER << client_address(socket) << "\t" << player.name()
					   << "\tsent scenario data in game:\t\"" << g.name() << "\" ("
					   << g.id() << ") without a 'multiplayer' child.\n";
			delete_game(g.id());
			send_server_message(socket, "The scenario data is missing the [multiplayer] tag which contains the game settings. Game aborted.");
			return;
		}

		// If there is no shroud, then tell players in the lobby
		// what the map looks like.
		const simple_wml::node& s = *wesnothd::game::starting_pos(g.level().root());
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
		g.send_data(notify_next_scenario, socket);

		// Send the update of the game description to the lobby.
		update_game_in_lobby(g);

		return;
	// A mp client sends a request for the next scenario of a mp campaign.
	} else if (data.child("load_next_scenario")) {
		g.load_next_scenario(socket);
		return;
	} else if (data.child("start_game")) {
		if (!g.is_owner(socket)) return;
		//perform controller tweaks, assigning sides as human for their owners etc.
		g.perform_controller_tweaks();
		// Send notification of the game starting immediately.
		// g.start_game() will send data that assumes
		// the [start_game] message has been sent
		g.send_data(data, socket);
		g.start_game(socket);

		//update the game having changed in the lobby
		update_game_in_lobby(g);
		return;
	} else if (data.child("update_game")) {
		g.update_game();
		update_game_in_lobby(g);
		return;
	} else if (data.child("leave_game")) {
		// May be better to just let remove_player() figure out when a game ends.
		if ((g.is_player(socket) && g.nplayers() == 1)
				|| (g.is_owner(socket) && (!g.started() || g.nplayers() == 0))) {
			// Remove the player in delete_game() with all other remaining
			// ones so he gets the updated gamelist.
			delete_game(g.id());
		} else {
			g.remove_player(socket);
			player_connections_.modify(player_connections_.find(socket), player_record::enter_lobby);
			g.describe_slots();

			// Send all other players in the lobby the update to the gamelist.
			simple_wml::document diff;
			bool diff1 = make_change_diff(*games_and_users_list_.child("gamelist"),
										  "gamelist", "game", g.description(), diff);
			bool diff2 = make_change_diff(games_and_users_list_.root(), NULL,
										  "user", player.config_address(), diff);
			if (diff1 || diff2) {
				send_to_lobby(diff, socket);
			}

			// Send the player who has quit the gamelist.
			send_to_player(socket, games_and_users_list_);
		}
		return;
	// If this is data describing side changes by the host.
	} else if (const simple_wml::node* scenario_diff = data.child("scenario_diff")) {
		if (!g.is_owner(socket)) return;
		g.level().root().apply_diff(*scenario_diff);
		const simple_wml::node* cfg_change = scenario_diff->child("change_child");
		if (cfg_change
				/**&& cfg_change->child("side") it is very likeley that
					the diff changes a side so this check isn't that important.
					Note that [side] is not at toplevel but inside
					[scenario] or [snapshot] **/) {
			g.update_side_data();
		}
		if (g.describe_slots()) {
			update_game_in_lobby(g);
		}
		g.send_data(data, socket);
		return;
	// If a player changes his faction.
	} else if (data.child("change_faction")) {
		g.send_data(data, socket);
		return;
	// If the owner of a side is changing the controller.
	} else if (const simple_wml::node *change = data.child("change_controller")) {
		g.transfer_side_control(socket, *change);
		if (g.describe_slots()) {
			update_game_in_lobby(g);
		}
		return;
	// If all observers should be muted. (toggles)
	} else if (data.child("muteall")) {
		if (!g.is_owner(socket)) {
			g.send_server_message("You cannot mute: not the game host.", socket);
			return;
		}
		g.mute_all_observers();
		return;
	// If an observer should be muted.
	} else if (const simple_wml::node* mute = data.child("mute")) {
		g.mute_observer(*mute, socket);
		return;
	// If an observer should be unmuted.
	} else if (const simple_wml::node* unmute = data.child("unmute")) {
		g.unmute_observer(*unmute, socket);
		return;
	// The owner is kicking/banning someone from the game.
	} else if (data.child("kick") || data.child("ban")) {
		bool ban = (data.child("ban") != NULL);
		const socket_ptr user =
				(ban ? g.ban_user(*data.child("ban"), socket)
					 : g.kick_member(*data.child("kick"), socket));
		if (user) {
			player_connections_.modify(player_connections_.find(socket), player_record::enter_lobby);
			if (g.describe_slots()) {
				update_game_in_lobby(g, user);
			}
			// Send all other players in the lobby the update to the gamelist.
			simple_wml::document gamelist_diff;
			make_change_diff(*games_and_users_list_.child("gamelist"),
							 "gamelist", "game", g.description(), gamelist_diff);
			make_change_diff(games_and_users_list_.root(), NULL, "user",
							 player_connections_.find(user)->info().config_address(), gamelist_diff);
			send_to_lobby(gamelist_diff, socket);
			// Send the removed user the lobby game list.
			send_to_player(user, games_and_users_list_);
		}
		return;
	} else if (const simple_wml::node* unban = data.child("unban")) {
		g.unban_user(*unban, socket);
		return;
	// If info is being provided about the game state.
	} else if (const simple_wml::node* info = data.child("info")) {
		if (!g.is_player(socket)) return;
		if ((*info)["type"] == "termination") {
			g.set_termination_reason((*info)["condition"].to_string());
			if ((*info)["condition"].to_string() == "out of sync") {
				g.send_server_message_to_all(player.name() + " reports out of sync errors.");
			}
		}
		return;
	} else if (data.child("turn")) {
		// Notify the game of the commands, and if it changes
		// the description, then sync the new description
		// to players in the lobby.
		if (g.process_turn(data, socket)) {
			update_game_in_lobby(g);
		}
		return;
	} else if (data.child("whiteboard")) {
		g.process_whiteboard(data,socket);
		return;
	} else if (data.child("change_turns_wml")) {
		g.process_change_turns_wml(data,socket);
		update_game_in_lobby(g);
		return;
	} else if (simple_wml::node* sch = data.child("request_choice")) {
		g.handle_choice(*sch, socket);
		return;
	} else if (data.child("message")) {
		g.process_message(data, socket);
		return;
	} else if (data.child("stop_updates")) {
		g.send_data(data, socket);
		return;
	// Data to ignore.
	} else if (data.child("error")
			   || data.child("side_secured")
			   || data.root().has_attr("failed")
			   || data.root().has_attr("side")) {
		return;
	}

	WRN_SERVER << client_address(socket) << "\tReceived unknown data from: "
			   << player.name() << " (socket:" << socket << ") in game: \""
			   << g.name() << "\" (" << g.id() << ")\n" << data.output();
}

typedef std::map<socket_ptr, std::deque<std::shared_ptr<simple_wml::document> > > SendQueue;
SendQueue send_queue;
void handle_send_to_player(socket_ptr socket);

void send_to_player(socket_ptr socket, simple_wml::document& doc)
{
	SendQueue::iterator iter = send_queue.find(socket);
	if(iter == send_queue.end()) {
		send_queue[socket];
		async_send_doc(socket, doc,
					   handle_send_to_player,
					   handle_send_to_player
					   );
	} else {
		send_queue[socket].push_back(std::shared_ptr<simple_wml::document>(doc.clone()));
	}
}

void handle_send_to_player(socket_ptr socket)
{
	if(send_queue[socket].empty()) {
		send_queue.erase(socket);
	} else {
		async_send_doc(socket, *(send_queue[socket].front()),
					   handle_send_to_player,
					   handle_send_to_player
					   );
		send_queue[socket].pop_front();
	}
}

void send_server_message(socket_ptr socket, const std::string& message)
{
	simple_wml::document server_message;
	simple_wml::node& msg = server_message.root().add_child("message");
	msg.set_attr("sender", "server");
	msg.set_attr_dup("message", message.c_str());
	send_to_player(socket, server_message);
}

void server::remove_player(socket_ptr socket)
{
	std::string ip = client_address(socket);

	auto iter = player_connections_.find(socket);
	if(iter == player_connections_.end())
		return;

	const std::shared_ptr<game> g = iter->get_game();
	if(g)
		g->remove_player(socket, true, false);

	const simple_wml::node::child_list& users = games_and_users_list_.root().children("user");
	const size_t index = std::find(users.begin(), users.end(), iter->info().config_address()) - users.begin();

	// Notify other players in lobby
	simple_wml::document diff;
	if(make_delete_diff(games_and_users_list_.root(), NULL, "user",
						iter->info().config_address(), diff)) {
		send_to_lobby(diff, socket);
	}
	games_and_users_list_.root().remove_child("user", index);

	LOG_SERVER << ip << "\t" << iter->info().name()
			   << "\twas logged off" << "\n";

	// Find the matching nick-ip pair in the log and update the sign off time
	connection_log ip_name = connection_log(iter->info().name(), ip, 0);
	std::deque<connection_log>::iterator i = std::find(ip_log_.begin(), ip_log_.end(), ip_name);
	if(i != ip_log_.end()) {
		i->log_off = time(nullptr);
	}

	player_connections_.erase(iter);

	if(socket->is_open())
		socket->close();
}

void server::send_to_lobby(simple_wml::document& data, socket_ptr exclude) const
{
	for(const auto& player : player_connections_.get<game_t>().equal_range(0))
		if(player.socket() != exclude)
			send_to_player(player.socket(), data);
}

void server::send_server_message_to_lobby(const std::string& message, socket_ptr exclude) const
{
	for(const auto& player : player_connections_.get<game_t>().equal_range(0)) {
		if(player.socket() != exclude)
			send_server_message(player.socket(), message);
	}
}

void server::send_server_message_to_all(const std::string& message, socket_ptr exclude) const
{
	for(const auto& player: player_connections_) {
		if(player.socket() != exclude)
			send_server_message(player.socket(), message);
	}
}

/*
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
							ERR_SERVER << "Player " << sock << " is in ghost_players_ but not in players_." << std::endl;
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
					WRN_SERVER << "received empty packet" << std::endl;
					continue;
				}

				const bool sample = request_sample_frequency >= 1 && (sample_counter++ % request_sample_frequency) == 0;

				const clock_t before_parsing = get_cpu_time(sample);

				char* buf_ptr = new char [buf.size()];
				memcpy(buf_ptr, &buf[0], buf.size());
				simple_wml::string_span compressed_buf(buf_ptr, buf.size());
				const std::unique_ptr<simple_wml::document> data_ptr;
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
			WRN_CONFIG << "Warning: error in received data: " << e.message << std::endl;
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
				ERR_SERVER << "network error: " << e.message << std::endl;
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
					WRN_SERVER << ip << "\tWarning: User disconnected right after the connection was accepted." << std::endl;
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
					for (t_games::const_iterator g = games_.begin(); g != games_.end(); ++g) {
						  // Note: This string is parsed by the client to identify lobby leave messages!
						  g->send_server_message_to_all(pl_it->second.name() + " has disconnected");
					}
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
				for (t_games::iterator g = games_.begin();
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
			ERR_SERVER << "Uncaught user_handler exception: " << e.message << std::endl;
		}
	}
	*/

void server::start_new_server() {
	if (restart_command.empty())
		return;

	// Example config line:
	// restart_command="./wesnothd-debug -d -c ~/.wesnoth1.5/server.cfg"
	// remember to make new one as a daemon or it will block old one
	if (std::system(restart_command.c_str())) {
		ERR_SERVER << "Failed to start new server with command: " << restart_command << std::endl;
	} else {
		LOG_SERVER << "New server started with command: " << restart_command << "\n";
	}
}

std::string server::process_command(std::string query, std::string issuer_name) {
	boost::trim(query);

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
			boost::trim(query);
		}
	}

	const std::string::iterator i = std::find(query.begin(), query.end(), ' ');

	try {

		const std::string command = utf8::lowercase(std::string(query.begin(), i));
		std::string parameters = (i == query.end() ? "" : std::string(i + 1, query.end()));
		boost::trim(parameters);

		std::ostringstream out;
		std::map<std::string, server::cmd_handler>::iterator handler_itor = cmd_handlers_.find(command);
		if(handler_itor == cmd_handlers_.end()) {
			out << "Command '" << command << "' is not recognized.\n" << help_msg;
		} else {
			const cmd_handler &handler = handler_itor->second;
			try {
				handler(issuer_name, query, parameters, &out);
			} catch (std::bad_function_call & ex) {
				ERR_SERVER << "While handling a command '" << command << "', caught a std::bad_function_call exception.\n";
				ERR_SERVER << ex.what() << std::endl;
				out << "An internal server error occurred (std::bad_function_call) while executing '" << command << "'\n";
			}
		}

		return out.str();

	} catch ( utf8::invalid_utf8_exception & e ) {
		std::string msg = "While handling a command, caught an invalid utf8 exception: ";
		msg += e.what();
		ERR_SERVER << msg << std::endl;
		return (msg + '\n');
	}
}

// Shutdown, restart and sample commands can only be issued via the socket.
void server::shut_down_handler(const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	if (issuer_name != "*socket*" && !allow_remote_shutdown_) {
		*out << denied_msg;
		return;
	}
	if (parameters == "now") {
		throw server_shutdown("shut down by admin command");
	} else {
		// Graceful shut down.
		graceful_restart = true;
		acceptor_.close();
		timer_.expires_from_now(boost::posix_time::seconds(10));
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, _1));
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
		acceptor_.close();
		timer_.expires_from_now(boost::posix_time::seconds(10));
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, _1));
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

	*out << "Number of games = " << games().size()
		 << "\nTotal number of users = " << player_connections_.size() << "\n";
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

void server::netstats_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream* /*out*/) {
	/*
	assert(out != NULL);

	network::pending_statistics stats = network::get_pending_stats();
	*out << "Network stats:\nPending send buffers: "
		<< stats.npending_sends << "\nBytes in buffers: "
		<< stats.nbytes_pending_sends << "\n";

	try {

	if (utf8::lowercase(parameters) == "all") {
		*out << network::get_bandwidth_stats_all();
	} else {
		*out << network::get_bandwidth_stats(); // stats from previuos hour
	}

	} catch ( utf8::invalid_utf8_exception & e ) {
		ERR_SERVER << "While handling a netstats command, caught an invalid utf8 exception: " << e.what() << std::endl;
	}
	*/
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
	for (const auto& player : player_connections_) {
		if (player.info().is_moderator()) {
			++n;
			send_to_player(player.socket(), data);
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
	boost::trim(message);
	if (message.empty()) {
		*out << "You must type a message.";
		return;
	}

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("whisper");
	// This string is parsed by the client!
	msg.set_attr_dup("sender", ("server message from " + sender).c_str());
	msg.set_attr_dup("message", message.c_str());
	for (const auto& player : player_connections_) {
		if (receiver != player.info().name().c_str()) {
			continue;
		}
		send_to_player(player.socket(), data);
		*out << "Message to " << receiver << " successfully sent.";
		return;
	}

	*out << "No such nick: " << receiver;
}

void server::msg_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != nullptr);

	if (parameters == "") {
		*out << "You must type a message.";
		return;
	}

	send_server_message_to_all(parameters);

	LOG_SERVER << "<server" << (parameters.find("/me ") == 0
								? std::string(parameters.begin() + 3, parameters.end()) + ">"
								: "> " + parameters) << "\n";

	*out << "message '" << parameters << "' relayed to players";
}

void server::lobbymsg_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != nullptr);

	if (parameters == "") {
		*out << "You must type a message.";
		return;
	}

	send_server_message_to_lobby(parameters);
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
		for (const auto& player : player_connections_) {
			if (parameters == player.info().name()) {
				parameters = client_address(player.socket());
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
	for (const auto& player : player_connections_) {
		if (parameters == "" || parameters == "*"
				|| (match_ip && utils::wildcard_string_match(client_address(player.socket()), parameters))
				|| (!match_ip && utils::wildcard_string_match(player.info().name(), parameters))) {
			found_something = true;
			*out << std::endl << player_status(player);
		}
	}
	if (!found_something) *out << "\nNo match found. You may want to check with 'searchlog'.";
}

void server::clones_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& /*parameters*/, std::ostringstream *out) {
	assert(out != NULL);

	*out << "CLONES STATUS REPORT";
	std::set<std::string> clones;
	for (player_connections::iterator it = player_connections_.begin(); it != player_connections_.end(); ++it) {
		if (clones.find(client_address(it->socket())) != clones.end()) continue;
		bool found = false;
		for (player_connections::iterator clone = std::next(it); clone != player_connections_.end(); ++clone) {
			if (client_address(it->socket()) == client_address(clone->socket())) {
				if (!found) {
					found = true;
					clones.insert(client_address(it->socket()));
					*out << std::endl << player_status(*it);
				}
				*out << std::endl << player_status(*clone);
			}
		}
	}
	if (clones.empty()) {
		*out << "No clones found.";
	}
}

void server::bans_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != nullptr);

	try
	{

		if (parameters.empty()) {
			ban_manager_.list_bans(*out);
		} else if (utf8::lowercase(parameters) == "deleted") {
			ban_manager_.list_deleted_bans(*out);
		} else if (utf8::lowercase(parameters).find("deleted") == 0) {
			std::string mask = parameters.substr(7);
			ban_manager_.list_deleted_bans(*out, boost::trim_copy(mask));
		} else {
			boost::trim(parameters);
			ban_manager_.list_bans(*out, parameters);
		}

	} catch ( utf8::invalid_utf8_exception & e ) {
		ERR_SERVER << "While handling bans, caught an invalid utf8 exception: " << e.what() << std::endl;
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
	boost::trim(reason);
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
		for (const auto& player : player_connections_)
		{
			if (utils::wildcard_string_match(player.info().name(), target)) {
				if (banned) *out << "\n";
				else banned = true;
				const std::string ip = client_address(player.socket());
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
	boost::trim(reason);
	if (reason.empty()) {
		*out << "You need to give a reason for the ban.";
		return;
	}

	std::string dummy_group;
	std::vector<socket_ptr> users_to_kick;

	// if we find a '.' consider it an ip mask
	/** @todo  FIXME: make a proper check for valid IPs. */
	if (std::count(target.begin(), target.end(), '.') >= 1) {
		banned = true;

		*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, dummy_group);

		for (const auto& player : player_connections_)
		{
			if (utils::wildcard_string_match(client_address(player.socket()), target)) {
				users_to_kick.push_back(player.socket());
			}
		}
	} else {
		for (const auto& player : player_connections_)
		{
			if (utils::wildcard_string_match(player.info().name(), target)) {
				if (banned) *out << "\n";
				else banned = true;
				const std::string ip = client_address(player.socket());
				users_to_kick.push_back(player.socket());
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

	for(const auto& user : users_to_kick) {
		*out << "\nKicked " << player_connections_.find(user)->info().name() << " ("
			 << client_address(user) << ").";
		async_send_error(user, "You have been banned. Reason: " + reason);
		remove_player(user);
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
	boost::trim(reason);
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
		for (const auto& player : player_connections_)
		{
			if (utils::wildcard_string_match(player.info().name(), target)) {
				if (banned) *out << "\n";
				else banned = true;
				const std::string ip = client_address(player.socket());
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
	std::vector<socket_ptr> users_to_kick;
	for (const auto& player : player_connections_)
	{
		if ((match_ip && utils::wildcard_string_match(client_address(player.socket()), kick_mask))
				|| (!match_ip && utils::wildcard_string_match(player.info().name(), kick_mask))) {
			users_to_kick.push_back(player.socket());
		}
	}
	for(const auto& socket : users_to_kick) {
		if (kicked) *out << "\n";
		else kicked = true;
		*out << "Kicked " << player_connections_.find(socket)->name() << " ("
			 << client_address(socket) << "). '"
			 << kick_message << "'";
		async_send_error(socket, kick_message);
		remove_player(socket);
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
			auto player = player_connections_.get<name_t>().find(username);
			if (player != player_connections_.get<name_t>().end() && client_address(player->socket()) == ip) {
				*out << std::endl << player_status(*player);
			} else {
				*out << "\n'" << username << "' @ " << ip << " last seen: " << lg::get_timestamp(i->log_off, "%H:%M:%S %d.%m.%Y");
			}
		}
	}
	if (!found_something) *out << "\nNo match found.";
}

void server::dul_handler(const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream *out) {
	assert(out != NULL);

	try {

		if (parameters == "") {
			*out << "Unregistered login is " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
		} else {
			deny_unregistered_login_ = (utf8::lowercase(parameters) == "yes");
			*out << "Unregistered login is now " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
		}

	} catch ( utf8::invalid_utf8_exception & e ) {
		ERR_SERVER << "While handling dul (deny unregistered logins), caught an invalid utf8 exception: " << e.what() << std::endl;
	}
}

void server::delete_game(int gameid) {
	std::shared_ptr<game> game_ptr = player_connections_.get<game_t>().find(gameid)->get_game();

	// Set the availability status for all quitting users.
	using titer = player_connections::index<game_t>::type::iterator;
	auto range_pair = player_connections_.get<game_t>().equal_range(gameid);
	//make a copy of the iterators so that we can change them while iterating over them.
	std::vector<titer> range_vctor;

	for (titer it = range_pair.first; it != range_pair.second; ++it) {
		range_vctor.push_back(it);
		it->info().mark_available();
		simple_wml::document udiff;
		if(make_change_diff(games_and_users_list_.root(), NULL,
							"user", it->info().config_address(), udiff)) {
			send_to_lobby(udiff);
		} else {
			ERR_SERVER << "ERROR: delete_game(): Could not find user in players_. (socket: "
					   << it->socket() << ")\n";
		}
	}

	//send users in the game a notification to leave the game since it has ended
	static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	game_ptr->send_data(leave_game_doc);
	// Put the remaining users back in the lobby.
	for (const titer& it : range_vctor) {
		player_connections_.get<game_t>().modify(it, player_record::enter_lobby);
	}
	game_ptr->send_data(games_and_users_list_);
}

void server::update_game_in_lobby(const wesnothd::game& g, const socket_ptr& exclude)
{
	simple_wml::document diff;
	if (make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", g.description(), diff)) {
		send_to_lobby(diff, exclude);
	}
}

} // namespace wesnothd

int main(int argc, char** argv) {
	int port = 15000;
	bool keep_alive = false;
	size_t min_threads = 5;
	size_t max_threads = 0;

	srand(static_cast<unsigned>(time(nullptr)));

	std::string config_file;

	// setting path to currentworking directory
	game_config::path = filesystem::get_cwd();

	// show 'info' by default
	lg::set_log_domain_severity("server", lg::info());
	lg::timestamps(true);

	for (int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if (val.empty()) {
			continue;
		}

		if ((val == "--config" || val == "-c") && arg+1 != argc) {
			config_file = argv[++arg];
		} else if (val == "--verbose" || val == "-v") {
			lg::set_log_domain_severity("all", lg::debug());
		} else if (val.substr(0, 6) == "--log-") {
			size_t p = val.find('=');
			if (p == std::string::npos) {
				std::cerr << "unknown option: " << val << '\n';
				return 2;
			}
			std::string s = val.substr(6, p - 6);
			int severity;
			if (s == "error") severity = lg::err().get_severity();
			else if (s == "warning") severity = lg::warn().get_severity();
			else if (s == "info") severity = lg::info().get_severity();
			else if (s == "debug") severity = lg::debug().get_severity();
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
		} else if (val == "--keepalive") {
			keep_alive = true;
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
					  << "  --keepalive                Enable TCP keepalive.\n"
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
			ERR_SERVER << "Running as a daemon is not supported on this platform" << std::endl;
			return -1;
#else
			const pid_t pid = fork();
			if (pid < 0) {
				ERR_SERVER << "Could not fork and run as a daemon" << std::endl;
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
			wesnothd::request_sample_frequency = atoi(argv[++arg]);
		} else {
			ERR_SERVER << "unknown option: " << val << std::endl;
			return 2;
		}
	}

	wesnothd::server(port, keep_alive, config_file, min_threads, max_threads).run();

	return 0;
}
