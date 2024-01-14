/*
	Copyright (C) 2003 - 2024
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

/**
 * @file
 * Wesnoth-Server, for multiplayer-games.
 */

#include "server/wesnothd/server.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "multiplayer_error_codes.hpp"
#include "serialization/chrono.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "utils/iterable_pair.hpp"
#include "game_version.hpp"

#include "server/wesnothd/ban.hpp"
#include "server/wesnothd/game.hpp"
#include "server/wesnothd/metrics.hpp"
#include "server/wesnothd/player.hpp"
#include "server/wesnothd/player_network.hpp"
#include "server/common/simple_wml.hpp"
#include "server/common/user_handler.hpp"

#ifdef HAVE_MYSQLPP
#include "server/common/forum_user_handler.hpp"
#endif

#include <boost/algorithm/string.hpp>
#include <boost/scope_exit.hpp>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

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

using namespace std::chrono_literals;

namespace wesnothd
{
// we take profiling info on every n requests
int request_sample_frequency = 1;
version_info secure_version = version_info("1.14.4");

static void make_add_diff(
		const simple_wml::node& src, const char* gamelist, const char* type, simple_wml::document& out, int index = -1)
{
	if(!out.child("gamelist_diff")) {
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
	if(!out.child("gamelist_diff")) {
		out.root().add_child("gamelist_diff");
	}

	simple_wml::node* top = out.child("gamelist_diff");
	if(gamelist) {
		top = &top->add_child("change_child");
		top->set_attr_int("index", 0);
		top = &top->add_child("gamelist");
	}

	const simple_wml::node::child_list& children = src.children(type);
	const auto itor = std::find(children.begin(), children.end(), remove);

	if(itor == children.end()) {
		return false;
	}

	const int index = std::distance(children.begin(), itor);

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
	if(!out.child("gamelist_diff")) {
		out.root().add_child("gamelist_diff");
	}

	simple_wml::node* top = out.child("gamelist_diff");
	if(gamelist) {
		top = &top->add_child("change_child");
		top->set_attr_int("index", 0);
		top = &top->add_child("gamelist");
	}

	const simple_wml::node::child_list& children = src.children(type);
	const auto itor = std::find(children.begin(), children.end(), item);

	if(itor == children.end()) {
		return false;
	}

	simple_wml::node& diff = *top;
	simple_wml::node& del = diff.add_child("delete_child");

	const int index = std::distance(children.begin(), itor);

	del.set_attr_int("index", index);
	del.add_child(type);

	// inserts will be processed first by the client, so insert at index+1,
	// and then when the delete is processed we'll slide into the right position
	simple_wml::node& insert = diff.add_child("insert_child");
	insert.set_attr_int("index", index + 1);

	children[index]->copy_into(insert.add_child(type));
	return true;
}

static std::string player_status(const wesnothd::player_record& player)
{
	auto logged_on_time = std::chrono::steady_clock::now() - player.login_time;
	auto [d, h, m, s] = chrono::deconstruct_duration(chrono::format::days_hours_mins_secs, logged_on_time);
	std::ostringstream out;
	out << "'" << player.name() << "' @ " << player.client_ip()
		<< " logged on for "
		<< d.count() << " days, "
		<< h.count() << " hours, "
		<< m.count() << " minutes, "
		<< s.count() << " seconds";
	return out.str();
}

const std::string denied_msg = "You're not allowed to execute this command.";
const std::string help_msg =
	"Available commands are: adminmsg <msg>,"
	" ban <mask> <time> <reason>, bans [deleted] [<ipmask>], clones,"
	" dul|deny_unregistered_login [yes|no], kick <mask> [<reason>],"
	" k[ick]ban <mask> <time> <reason>, help, games, metrics,"
	" [lobby]msg <message>, motd [<message>],"
	" pm|privatemsg <nickname> <message>, requests, roll <sides>, sample, searchlog <mask>,"
	" signout, stats, status [<mask>], stopgame <nick> [<reason>], unban <ipmask>\n"
	"Specific strings (those not in between <> like the command names)"
	" are case insensitive.";

server::server(int port,
		bool keep_alive,
		const std::string& config_file)
	: server_base(port, keep_alive)
	, ban_manager_()
	, ip_log_()
	, failed_logins_()
	, user_handler_(nullptr)
	, die_(static_cast<unsigned>(std::time(nullptr)))
#ifndef _WIN32
	, input_path_()
#endif
	, uuid_("")
	, config_file_(config_file)
	, cfg_(read_config())
	, accepted_versions_()
	, redirected_versions_()
	, proxy_versions_()
	, disallowed_names_()
	, admin_passwd_()
	, motd_()
	, announcements_()
	, server_id_()
	, tournaments_()
	, information_()
	, default_max_messages_(0)
	, default_time_period_(0)
	, concurrent_connections_(0)
	, graceful_restart(false)
	, lan_server_(0)
	, restart_command()
	, max_ip_log_size_(0)
	, deny_unregistered_login_(false)
	, save_replays_(false)
	, replay_save_path_()
	, allow_remote_shutdown_(false)
	, client_sources_()
	, tor_ip_list_()
	, failed_login_limit_()
	, failed_login_ban_()
	, failed_login_buffer_size_()
	, version_query_response_("[version]\n[/version]\n", simple_wml::INIT_COMPRESSED)
	, login_response_("[mustlogin]\n[/mustlogin]\n", simple_wml::INIT_COMPRESSED)
	, games_and_users_list_("[gamelist]\n[/gamelist]\n", simple_wml::INIT_STATIC)
	, metrics_()
	, dump_stats_timer_(io_service_)
	, tournaments_timer_(io_service_)
	, cmd_handlers_()
	, timer_(io_service_)
	, lan_server_timer_(io_service_)
	, dummy_player_timer_(io_service_)
	, dummy_player_timer_interval_(30)
{
	setup_handlers();
	load_config();
	ban_manager_.read();

	start_server();

	start_dump_stats();
	start_tournaments_timer();
}

#ifndef _WIN32
void server::handle_sighup(const boost::system::error_code& error, int)
{
	assert(!error);

	WRN_SERVER << "SIGHUP caught, reloading config";

	cfg_ = read_config();
	load_config();

	sighup_.async_wait(std::bind(&server::handle_sighup, this, std::placeholders::_1, std::placeholders::_2));
}
#endif

void server::handle_graceful_timeout(const boost::system::error_code& error)
{
	assert(!error);

	if(games().empty()) {
		process_command("msg All games ended. Shutting down now. Reconnect to the new server instance.", "system");
		BOOST_THROW_EXCEPTION(server_shutdown("graceful shutdown timeout"));
	} else {
		timer_.expires_after(1s);
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, std::placeholders::_1));
	}
}

void server::start_lan_server_timer()
{
	lan_server_timer_.expires_after(lan_server_);
	lan_server_timer_.async_wait([this](const boost::system::error_code& ec) { handle_lan_server_shutdown(ec); });
}

void server::abort_lan_server_timer()
{
	lan_server_timer_.cancel();
}

void server::handle_lan_server_shutdown(const boost::system::error_code& error)
{
	if(error)
		return;

	BOOST_THROW_EXCEPTION(server_shutdown("lan server shutdown"));
}

void server::setup_fifo()
{
#ifndef _WIN32
	const int res = mkfifo(input_path_.c_str(), 0660);
	if(res != 0 && errno != EEXIST) {
		ERR_SERVER << "could not make fifo at '" << input_path_ << "' (" << strerror(errno) << ")";
		return;
	}
	int fifo = open(input_path_.c_str(), O_RDWR | O_NONBLOCK);
	input_.assign(fifo);
	LOG_SERVER << "opened fifo at '" << input_path_ << "'. Server commands may be written to this file.";
	read_from_fifo();
#endif
}

#ifndef _WIN32

void server::handle_read_from_fifo(const boost::system::error_code& error, std::size_t)
{
	if(error) {
		std::cout << error.message() << std::endl;
		return;
	}

	std::istream is(&admin_cmd_);
	std::string cmd;
	std::getline(is, cmd);

	LOG_SERVER << "Admin Command: type: " << cmd;

	const std::string res = process_command(cmd, "*socket*");

	// Only mark the response if we fake the issuer (i.e. command comes from IRC or so)
	if(!cmd.empty() && cmd.at(0) == '+') {
		LOG_SERVER << "[admin_command_response]\n"
				   << res << "\n"
				   << "[/admin_command_response]";
	} else {
		LOG_SERVER << res;
	}

	read_from_fifo();
}

#endif

void server::setup_handlers()
{
#define SETUP_HANDLER(name, function)                                                                                  \
	cmd_handlers_[name] = std::bind(function, this,                                                                    \
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	SETUP_HANDLER("shut_down", &server::shut_down_handler);
	SETUP_HANDLER("restart", &server::restart_handler);
	SETUP_HANDLER("sample", &server::sample_handler);
	SETUP_HANDLER("help", &server::help_handler);
	SETUP_HANDLER("stats", &server::stats_handler);
	SETUP_HANDLER("version", &server::version_handler);
	SETUP_HANDLER("metrics", &server::metrics_handler);
	SETUP_HANDLER("requests", &server::requests_handler);
	SETUP_HANDLER("roll", &server::roll_handler);
	SETUP_HANDLER("games", &server::games_handler);
	SETUP_HANDLER("wml", &server::wml_handler);
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
	SETUP_HANDLER("stopgame", &server::stopgame);

#undef SETUP_HANDLER
}

config server::read_config() const
{
	config configuration;

	if(config_file_.empty()) {
		return configuration;
	}

	try {
		// necessary to avoid assert since preprocess_file() goes through filesystem::get_short_wml_path()
		filesystem::set_user_data_dir(std::string());
		filesystem::scoped_istream stream = preprocess_file(config_file_);
		read(configuration, *stream);
		LOG_SERVER << "Server configuration from file: '" << config_file_ << "' read.";
	} catch(const config::error& e) {
		ERR_CONFIG << "ERROR: Could not read configuration file: '" << config_file_ << "': '" << e.message << "'.";
	}

	return configuration;
}

void server::load_config()
{
#ifndef _WIN32
#ifndef FIFODIR
#warning No FIFODIR set
#define FIFODIR "/var/run/wesnothd"
#endif
	const std::string fifo_path
			= (cfg_["fifo_path"].empty() ? std::string(FIFODIR) + "/socket" : std::string(cfg_["fifo_path"]));
	// Reset (replace) the input stream only if the FIFO path changed.
	if(fifo_path != input_path_) {
		input_.close();
		input_path_ = fifo_path;
		setup_fifo();
	}
#endif

	save_replays_ = cfg_["save_replays"].to_bool();
	replay_save_path_ = cfg_["replay_save_path"].str();

	tor_ip_list_ = utils::split(cfg_["tor_ip_list_path"].empty()
		? ""
		: filesystem::read_file(cfg_["tor_ip_list_path"]), '\n');

	admin_passwd_ = cfg_["passwd"].str();
	motd_ = cfg_["motd"].str();
	information_ = cfg_["information"].str();
	announcements_ = cfg_["announcements"].str();
	server_id_ = cfg_["id"].str();
	lan_server_ = chrono::parse_duration(cfg_["lan_server"], 0s);

	deny_unregistered_login_ = cfg_["deny_unregistered_login"].to_bool();

	allow_remote_shutdown_ = cfg_["allow_remote_shutdown"].to_bool();

	for(const std::string& source : utils::split(cfg_["client_sources"].str())) {
		client_sources_.insert(source);
	}

	disallowed_names_.clear();
	if(cfg_["disallow_names"].empty()) {
		disallowed_names_.push_back("*admin*");
		disallowed_names_.push_back("*admln*");
		disallowed_names_.push_back("*server*");
		disallowed_names_.push_back("player");
		disallowed_names_.push_back("network");
		disallowed_names_.push_back("human");
		disallowed_names_.push_back("computer");
		disallowed_names_.push_back("ai");
		disallowed_names_.push_back("ai?");
		disallowed_names_.push_back("*moderator*");
	} else {
		disallowed_names_ = utils::split(cfg_["disallow_names"]);
	}

	default_max_messages_ = cfg_["max_messages"].to_int(4);
	default_time_period_ = chrono::parse_duration(cfg_["messages_time_period"], 10s);
	concurrent_connections_ = cfg_["connections_allowed"].to_int(5);
	max_ip_log_size_ = cfg_["max_ip_log_size"].to_int(500);

	failed_login_limit_ = cfg_["failed_logins_limit"].to_int(10);
	failed_login_ban_ = chrono::parse_duration(cfg_["failed_logins_ban"], 3600s);
	failed_login_buffer_size_ = cfg_["failed_logins_buffer_size"].to_int(500);

	// Example config line:
	// restart_command="./wesnothd-debug -d -c ~/.wesnoth1.5/server.cfg"
	// remember to make new one as a daemon or it will block old one
	restart_command = cfg_["restart_command"].str();

	recommended_version_ = cfg_["recommended_version"].str();
	accepted_versions_.clear();
	const std::string& versions = cfg_["versions_accepted"];
	if(versions.empty() == false) {
		accepted_versions_ = utils::split(versions);
	} else {
		accepted_versions_.push_back(game_config::wesnoth_version.str());
		accepted_versions_.push_back("test");
	}

	redirected_versions_.clear();
	for(const config& redirect : cfg_.child_range("redirect")) {
		for(const std::string& version : utils::split(redirect["version"])) {
			redirected_versions_[version] = redirect;
		}
	}

	proxy_versions_.clear();
	for(const config& proxy : cfg_.child_range("proxy")) {
		for(const std::string& version : utils::split(proxy["version"])) {
			proxy_versions_[version] = proxy;
		}
	}

	ban_manager_.load_config(cfg_);

	// If there is a [user_handler] tag in the config file
	// allow nick registration, otherwise we set user_handler_
	// to nullptr. Thus we must check user_handler_ for not being
	// nullptr every time we want to use it.
	user_handler_.reset();

#ifdef HAVE_MYSQLPP
	if(auto user_handler = cfg_.optional_child("user_handler")) {
		if(server_id_ == "") {
			ERR_SERVER << "The server id must be set when database support is used";
			exit(1);
		}

		user_handler_.reset(new fuh(*user_handler));
		uuid_ = user_handler_->get_uuid();
		tournaments_ = user_handler_->get_tournaments();
	}
#endif

	load_tls_config(cfg_);

	if(cfg_["dummy_player_count"].to_int() > 0) {
		for(int i = 0; i < cfg_["dummy_player_count"].to_int(); i++) {
			simple_wml::node& dummy_user = games_and_users_list_.root().add_child_at("user", i);
			dummy_user.set_attr_dup("available", "yes");
			dummy_user.set_attr_int("forum_id", i);
			dummy_user.set_attr_int("game_id", 0);
			dummy_user.set_attr_dup("location", "");
			dummy_user.set_attr_dup("moderator", "no");
			dummy_user.set_attr_dup("name", ("player"+std::to_string(i)).c_str());
			dummy_user.set_attr_dup("registered", "yes");
			dummy_user.set_attr_dup("status", "lobby");
		}
		if(cfg_["dummy_player_timer_interval"].to_int() > 0) {
			dummy_player_timer_interval_ = chrono::parse_duration(cfg_["dummy_player_timer_interval"], 0s);
		}
		start_dummy_player_updates();
	}
}

bool server::ip_exceeds_connection_limit(const std::string& ip) const
{
	if(concurrent_connections_ == 0) {
		return false;
	}

	std::size_t connections = 0;
	for(const auto& player : player_connections_) {
		if(player.client_ip() == ip) {
			++connections;
		}
	}

	return connections >= concurrent_connections_;
}

utils::optional<server_base::login_ban_info> server::is_ip_banned(const std::string& ip)
{
	if(utils::contains(tor_ip_list_, ip)) {
		return login_ban_info{ MP_SERVER_IP_BAN_ERROR, "TOR IP", {} };
	}

	if(auto server_ban_info = ban_manager_.get_ban_info(ip)) {
		return login_ban_info{
			MP_SERVER_IP_BAN_ERROR,
			server_ban_info->get_reason(),
			server_ban_info->get_remaining_ban_time()
		};
	}

	return {};
}

void server::start_dump_stats()
{
	dump_stats_timer_.expires_after(5min);
	dump_stats_timer_.async_wait([this](const boost::system::error_code& ec) { dump_stats(ec); });
}

void server::dump_stats(const boost::system::error_code& ec)
{
	if(ec) {
		ERR_SERVER << "Error waiting for dump stats timer: " << ec.message();
		return;
	}
	LOG_SERVER << "Statistics:"
	           << "\tnumber_of_games = " << games().size()
	           << "\tnumber_of_users = " << player_connections_.size();
	start_dump_stats();
}

void server::start_dummy_player_updates()
{
	dummy_player_timer_.expires_after(dummy_player_timer_interval_);
	dummy_player_timer_.async_wait([this](const boost::system::error_code& ec) { dummy_player_updates(ec); });
}

void server::dummy_player_updates(const boost::system::error_code& ec)
{
	if(ec) {
		ERR_SERVER << "Error waiting for dummy player timer: " << ec.message();
		return;
	}

	int size = games_and_users_list_.root().children("user").size();
	LOG_SERVER << "player count: " << size;
	if(size % 2 == 0) {
		simple_wml::node* dummy_user = games_and_users_list_.root().children("user").at(size-1);

		simple_wml::document diff;
		if(make_delete_diff(games_and_users_list_.root(), nullptr, "user", dummy_user, diff)) {
			send_to_lobby(diff);
		}

		games_and_users_list_.root().remove_child("user", size-1);
	} else {
		simple_wml::node& dummy_user = games_and_users_list_.root().add_child_at("user", size-1);
		dummy_user.set_attr_dup("available", "yes");
		dummy_user.set_attr_int("forum_id", size-1);
		dummy_user.set_attr_int("game_id", 0);
		dummy_user.set_attr_dup("location", "");
		dummy_user.set_attr_dup("moderator", "no");
		dummy_user.set_attr_dup("name", ("player"+std::to_string(size-1)).c_str());
		dummy_user.set_attr_dup("registered", "yes");
		dummy_user.set_attr_dup("status", "lobby");

		simple_wml::document diff;
		make_add_diff(games_and_users_list_.root(), nullptr, "user", diff);
		send_to_lobby(diff);
	}

	start_dummy_player_updates();
}

void server::start_tournaments_timer()
{
	tournaments_timer_.expires_after(60min);
	tournaments_timer_.async_wait([this](const boost::system::error_code& ec) { refresh_tournaments(ec); });
}

void server::refresh_tournaments(const boost::system::error_code& ec)
{
	if(ec) {
		ERR_SERVER << "Error waiting for tournament refresh timer: " << ec.message();
		return;
	}
	if(user_handler_) {
		tournaments_ = user_handler_->get_tournaments();
		start_tournaments_timer();
	}
}

void server::handle_new_client(socket_ptr socket)
{
	boost::asio::spawn(io_service_, [socket, this](boost::asio::yield_context yield) { login_client(std::move(yield), socket); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);
}

void server::handle_new_client(tls_socket_ptr socket)
{
	boost::asio::spawn(io_service_, [socket, this](boost::asio::yield_context yield) { login_client(std::move(yield), socket); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);
}

template<class SocketPtr>
void server::login_client(boost::asio::yield_context yield, SocketPtr socket)
{
	coro_send_doc(socket, version_query_response_, yield);

	auto doc { coro_receive_doc(socket, yield) };
	if(!doc) return;

	std::string client_version, client_source;
	if(const simple_wml::node* const version = doc->child("version")) {
		const simple_wml::string_span& version_str_span = (*version)["version"];
		client_version = std::string { version_str_span.begin(), version_str_span.end() };

		const simple_wml::string_span& source_str_span = (*version)["client_source"];
		client_source = std::string { source_str_span.begin(), source_str_span.end() };

		// Check if it is an accepted version.
		auto accepted_it = std::find_if(accepted_versions_.begin(), accepted_versions_.end(),
			std::bind(&utils::wildcard_string_match, client_version, std::placeholders::_1));

		if(accepted_it != accepted_versions_.end()) {
			LOG_SERVER << log_address(socket) << "\tplayer joined using accepted version " << client_version
					   << ":\ttelling them to log in.";
			coro_send_doc(socket, login_response_, yield);
		} else {
			simple_wml::document response;

			// Check if it is a redirected version
			for(const auto& redirect_version : redirected_versions_) {
				if(utils::wildcard_string_match(client_version, redirect_version.first)) {
					LOG_SERVER << log_address(socket) << "\tplayer joined using version " << client_version
						   << ":\tredirecting them to " << redirect_version.second["host"] << ":"
						   << redirect_version.second["port"];

					simple_wml::node& redirect = response.root().add_child("redirect");
					for(const auto& attr : redirect_version.second.attribute_range()) {
						redirect.set_attr_dup(attr.first.c_str(), attr.second.str().c_str());
					}

					async_send_doc_queued(socket, response);
					return;
				}
			}

			LOG_SERVER << log_address(socket) << "\tplayer joined using unknown version " << client_version
				   << ":\trejecting them";

			// For compatibility with older clients
			response.set_attr_dup("version", accepted_versions_.begin()->c_str());

			simple_wml::node& reject = response.root().add_child("reject");
			reject.set_attr_dup("accepted_versions", utils::join(accepted_versions_).c_str());
			async_send_doc_queued(socket, response);
			return;
		}
	} else {
		LOG_SERVER << log_address(socket) << "\tclient didn't send its version: rejecting";
		return;
	}

	std::string username;
	bool registered, is_moderator;

	while(true) {
		auto login_response { coro_receive_doc(socket, yield) };
		if(!login_response) return;

		if(const simple_wml::node* const login = login_response->child("login")) {
			username = (*login)["username"].to_string();

			if(is_login_allowed(yield, socket, login, username, registered, is_moderator)) {
				break;
			} else continue;
		}

		async_send_error(socket, "You must login first.", MP_MUST_LOGIN);
	}

	simple_wml::node& player_cfg = games_and_users_list_.root().add_child("user");
	wesnothd::player player_data {
			username,
			player_cfg,
			user_handler_ ? user_handler_->get_forum_id(username) : 0,
			registered,
			client_version,
			client_source,
			user_handler_ ? user_handler_->db_insert_login(username, client_address(socket), client_version) : 0,
			default_max_messages_,
			default_time_period_,
			is_moderator
	};
	bool inserted;
	player_iterator new_player;
	std::tie(new_player, inserted) = player_connections_.insert(player_connections::value_type(socket, player_data));
	assert(inserted && "unexpected duplicate username");

	simple_wml::document join_lobby_response;
	join_lobby_response.root().add_child("join_lobby").set_attr("is_moderator", is_moderator ? "yes" : "no");
	join_lobby_response.root().child("join_lobby")->set_attr_dup("profile_url_prefix", "https://r.wesnoth.org/u");
	coro_send_doc(socket, join_lobby_response, yield);

	boost::asio::spawn(io_service_,
		[this, socket, new_player](boost::asio::yield_context yield) { handle_player(yield, socket, new_player); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);

	LOG_SERVER << log_address(socket) << "\t" << username << "\thas logged on"
			   << (registered ? " to a registered account" : "");

	std::shared_ptr<game> last_sent;
	for(const auto& record : player_connections_.get<game_t>()) {
		auto g_ptr = record.get_game();
		if(g_ptr != last_sent) {
			// Note: This string is parsed by the client to identify lobby join messages!
			g_ptr->send_server_message_to_all(username + " has logged into the lobby");
			last_sent = g_ptr;
		}
	}

	// Log the IP
	if(!user_handler_) {
		connection_log ip_name { username, client_address(socket), {} };

		if(std::find(ip_log_.begin(), ip_log_.end(), ip_name) == ip_log_.end()) {
			ip_log_.push_back(ip_name);

			// Remove the oldest entry if the size of the IP log exceeds the maximum size
			if(ip_log_.size() > max_ip_log_size_) {
				ip_log_.pop_front();
			}
		}
	}
}

template<class SocketPtr> bool server::is_login_allowed(boost::asio::yield_context yield, SocketPtr socket, const simple_wml::node* const login, const std::string& username, bool& registered, bool& is_moderator)
{
	// Check if the username is valid (all alpha-numeric plus underscore and hyphen)
	if(!utils::isvalid_username(username)) {
		async_send_error(socket,
			"The nickname '" + username + "' contains invalid "
			"characters. Only alpha-numeric characters, underscores and hyphens are allowed.",
			MP_INVALID_CHARS_IN_NAME_ERROR
		);

		return false;
	}

	if(username.size() > 20) {
		async_send_error(socket, "The nickname '" + username + "' is too long. Nicks must be 20 characters or less.",
			MP_NAME_TOO_LONG_ERROR);

		return false;
	}

	// Check if the username is allowed.
	for(const std::string& d : disallowed_names_) {
		if(utils::wildcard_string_match(utf8::lowercase(username), utf8::lowercase(d))) {
			async_send_error(socket, "The nickname '" + username + "' is reserved and cannot be used by players",
				MP_NAME_RESERVED_ERROR);

			return false;
		}
	}

	// Check the username isn't already taken
	auto p = player_connections_.get<name_t>().find(username);
	bool name_taken = p != player_connections_.get<name_t>().end();

	// Check for password

	if(!authenticate(socket, username, (*login)["password"].to_string(), name_taken, registered))
		return false;

	// If we disallow unregistered users and this user is not registered send an error
	if(user_handler_ && !registered && deny_unregistered_login_) {
		async_send_error(socket,
			"The nickname '" + username + "' is not registered. This server disallows unregistered nicknames.",
			MP_NAME_UNREGISTERED_ERROR
		);

		return false;
	}

	is_moderator = user_handler_ && user_handler_->user_is_moderator(username);
	user_handler::ban_info auth_ban;

	if(user_handler_) {
		auth_ban = user_handler_->user_is_banned(username, client_address(socket));
	}

	if(auth_ban.type) {
		std::string ban_type_desc;
		std::string ban_reason;
		const char* msg_numeric;
		std::string ban_duration = std::to_string(auth_ban.duration.count());

		switch(auth_ban.type) {
		case user_handler::BAN_USER:
			ban_type_desc = "account";
			msg_numeric = MP_NAME_AUTH_BAN_USER_ERROR;
			ban_reason = "a ban has been issued on your user account.";
			break;
		case user_handler::BAN_IP:
			ban_type_desc = "IP address";
			msg_numeric = MP_NAME_AUTH_BAN_IP_ERROR;
			ban_reason = "a ban has been issued on your IP address.";
			break;
		case user_handler::BAN_EMAIL:
			ban_type_desc = "email address";
			msg_numeric = MP_NAME_AUTH_BAN_EMAIL_ERROR;
			ban_reason = "a ban has been issued on your email address.";
			break;
		default:
			ban_type_desc = "<unknown ban type>";
			msg_numeric = "";
			ban_reason = ban_type_desc;
		}

		ban_reason += " (" + ban_duration + ")";

		if(!is_moderator) {
			LOG_SERVER << log_address(socket) << "\t" << username << "\tis banned by user_handler (" << ban_type_desc
					   << ")";
			if(auth_ban.duration > 0s) {
				// Temporary ban
				async_send_error(socket, "You are banned from this server: " + ban_reason, msg_numeric, {{"duration", ban_duration}});
			} else {
				// Permanent ban
				async_send_error(socket, "You are banned from this server: " + ban_reason, msg_numeric);
			}
			return false;
		} else {
			LOG_SERVER << log_address(socket) << "\t" << username << "\tis banned by user_handler (" << ban_type_desc
					   << "), " << "ignoring due to moderator flag";
		}
	}

	if(name_taken) {
		if(registered) {
			// If there is already a client using this username kick it
			process_command("kick " + username + " autokick by registered user", username);
			// need to wait for it to process
			while(player_connections_.get<name_t>().count(username) > 0) {
				boost::asio::post(yield);
			}
		} else {
			async_send_error(socket, "The nickname '" + username + "' is already taken.", MP_NAME_TAKEN_ERROR);
			return false;
		}
	}

	if(auth_ban.type) {
		send_server_message(socket, "You are currently banned by the forum administration.", "alert");
	}

	return true;
}

template<class SocketPtr> bool server::authenticate(
		SocketPtr socket, const std::string& username, const std::string& password, bool name_taken, bool& registered)
{
	// Current login procedure  for registered nicks is:
	// - Client asks to log in with a particular nick
	// - Server sends client a password request (if TLS/database support is enabled)
	// - Client sends the plaintext password
	// - Server receives plaintext password, hashes it, and compares it to the password in the forum database

	registered = false;

	if(user_handler_) {
		const bool exists = user_handler_->user_exists(username);

		// This name is registered but the account is not active
		if(exists && !user_handler_->user_is_active(username)) {
			async_send_warning(socket,
				"The nickname '" + username + "' is inactive. You cannot claim ownership of this "
				"nickname until you activate your account via email or ask an administrator to do it for you.",
				MP_NAME_INACTIVE_WARNING);
		} else if(exists) {
			const std::string salt = user_handler_->extract_salt(username);
			if(salt.empty()) {
				async_send_error(socket,
					"Even though your nickname is registered on this server you "
					"cannot log in due to an error in the hashing algorithm. "
					"Logging into your forum account on https://forums.wesnoth.org "
					"may fix this problem.");
				return false;
			}
			const std::string hashed_password = hash_password(password, salt, username);

			// This name is registered and no password provided
			if(password.empty()) {
				if(!name_taken) {
					send_password_request(socket, "The nickname '" + username + "' is registered on this server.", MP_PASSWORD_REQUEST);
				} else {
					send_password_request(socket,
						"The nickname '" + username + "' is registered on this server."
						"\n\nWARNING: There is already a client using this username, "
						"logging in will cause that client to be kicked!",
						MP_PASSWORD_REQUEST_FOR_LOGGED_IN_NAME, true
					);
				}

				return false;
			}

			// hashing the password failed
			// note: this could be due to other related problems other than *just* the hashing step failing
			if(hashed_password.empty()) {
				async_send_error(socket, "Password hashing failed.", MP_HASHING_PASSWORD_FAILED);
				return false;
			}
			// This name is registered and an incorrect password provided
			else if(!(user_handler_->login(username, hashed_password))) {
				const auto steady_now = std::chrono::steady_clock::now();

				login_log login_ip { client_address(socket), 0, steady_now };
				auto i = std::find(failed_logins_.begin(), failed_logins_.end(), login_ip);

				if(i == failed_logins_.end()) {
					failed_logins_.push_back(login_ip);
					i = --failed_logins_.end();

					// Remove oldest entry if maximum size is exceeded
					if(failed_logins_.size() > failed_login_buffer_size_) {
						failed_logins_.pop_front();
					}
				}

				if(i->first_attempt + failed_login_ban_ < steady_now) {
					// Clear and move to the beginning
					failed_logins_.erase(i);
					failed_logins_.push_back(login_ip);
					i = --failed_logins_.end();
				}

				i->attempts++;

				if(i->attempts > failed_login_limit_) {
					LOG_SERVER << ban_manager_.ban(login_ip.ip, std::chrono::system_clock::now() + failed_login_ban_,
						"Maximum login attempts exceeded", "automatic", "", username);

					async_send_error(socket, "You have made too many failed login attempts.", MP_TOO_MANY_ATTEMPTS_ERROR);
				} else {
					send_password_request(socket,
						"The password you provided for the nickname '" + username + "' was incorrect.",
						MP_INCORRECT_PASSWORD_ERROR);
				}

				// Log the failure
				LOG_SERVER << log_address(socket) << "\t"
						   << "Login attempt with incorrect password for nickname '" << username << "'.";
				return false;
			}

			// This name exists and the password was neither empty nor incorrect
			registered = true;
			user_handler_->user_logged_in(username);
		}
	}

	return true;
}

template<class SocketPtr> void server::send_password_request(SocketPtr socket,
		const std::string& msg,
		const char* error_code,
		bool force_confirmation)
{
	simple_wml::document doc;
	simple_wml::node& e = doc.root().add_child("error");
	e.set_attr_dup("message", msg.c_str());
	e.set_attr("password_request", "yes");
	e.set_attr("force_confirmation", force_confirmation ? "yes" : "no");

	if(*error_code != '\0') {
		e.set_attr("error_code", error_code);
	}

	async_send_doc_queued(socket, doc);
}

template<class SocketPtr> void server::handle_player(boost::asio::yield_context yield, SocketPtr socket, player_iterator player)
{
	if(lan_server_ > 0s)
		abort_lan_server_timer();

	BOOST_SCOPE_EXIT_ALL(this, &player) {
		if(!destructed) {
			remove_player(player);
		}
	};

	async_send_doc_queued(socket, games_and_users_list_);

	if(!motd_.empty()) {
		send_server_message(player, motd_+'\n'+announcements_+tournaments_, "motd");
	}
	send_server_message(player, information_, "server_info");
	send_server_message(player, announcements_+tournaments_, "announcements");
	if(version_info(player->info().version()) < secure_version ){
		send_server_message(player, "You are using version " + player->info().version() + " which has known security issues that can be used to compromise your computer. We strongly recommend updating to a Wesnoth version " + secure_version.str() + " or newer!", "alert");
	}
	if(version_info(player->info().version()) < version_info(recommended_version_)) {
		send_server_message(player, "A newer Wesnoth version, " + recommended_version_ + ", is out!", "alert");
	}

	// Send other players in the lobby the update that the player has joined
	simple_wml::document diff;
	make_add_diff(games_and_users_list_.root(), nullptr, "user", diff);
	send_to_lobby(diff, player);

	while(true) {
		auto doc { coro_receive_doc(socket, yield) };
		if(!doc) return;

		// DBG_SERVER << client_address(socket) << "\tWML received:\n" << doc->output();
		if(doc->child("refresh_lobby")) {
			async_send_doc_queued(socket, games_and_users_list_);
			continue;
		}

		if(simple_wml::node* whisper = doc->child("whisper")) {
			handle_whisper(player, *whisper);
			continue;
		}

		if(simple_wml::node* query = doc->child("query")) {
			handle_query(player, *query);
			continue;
		}

		if(simple_wml::node* nickserv = doc->child("nickserv")) {
			handle_nickserv(player, *nickserv);
			continue;
		}

		if(!player_is_in_game(player)) {
			handle_player_in_lobby(player, *doc);
		} else {
			handle_player_in_game(player, *doc);
		}
	}
}

void server::handle_player_in_lobby(player_iterator player, simple_wml::document& data)
{
	if(simple_wml::node* message = data.child("message")) {
		handle_message(player, *message);
		return;
	}

	if(simple_wml::node* create_game = data.child("create_game")) {
		handle_create_game(player, *create_game);
		return;
	}

	if(simple_wml::node* join = data.child("join")) {
		handle_join_game(player, *join);
		return;
	}

 	if(simple_wml::node* request = data.child("game_history_request")) {
		if(user_handler_) {
			int offset = request->attr("offset").to_int();
			int player_id = 0;

			// if search_for attribute for offline player -> query the forum database for the forum id
			// if search_for attribute for online player -> get the forum id from wesnothd's player info
			if(request->has_attr("search_player") && request->attr("search_player").to_string() != "") {
				std::string player_name = request->attr("search_player").to_string();
				auto player_ptr = player_connections_.get<name_t>().find(player_name);
				if(player_ptr == player_connections_.get<name_t>().end()) {
					player_id = user_handler_->get_forum_id(player_name);
				} else {
					player_id = player_ptr->info().config_address()->attr("forum_id").to_int();
				}
			}

			std::string search_game_name = request->attr("search_game_name").to_string();
			int search_content_type = request->attr("search_content_type").to_int();
			std::string search_content = request->attr("search_content").to_string();
			LOG_SERVER << "Querying game history requested by player `" << player->info().name() << "` for player id `" << player_id << "`."
					   << "Searching for game name `" << search_game_name << "`, search content type `" << search_content_type << "`, search content `" << search_content << "`.";
			user_handler_->async_get_and_send_game_history(io_service_, *this, player->socket(), player_id, offset, search_game_name, search_content_type, search_content);
		}
		return;
	}
}

void server::handle_whisper(player_iterator player, simple_wml::node& whisper)
{
	if((whisper["receiver"].empty()) || (whisper["message"].empty())) {
		static simple_wml::document data(
			"[message]\n"
			"message=\"Invalid number of arguments\"\n"
			"sender=\"server\"\n"
			"[/message]\n",
			simple_wml::INIT_COMPRESSED
		);

		send_to_player(player, data);
		return;
	}

	whisper.set_attr_dup("sender", player->name().c_str());

	auto receiver_iter = player_connections_.get<name_t>().find(whisper["receiver"].to_string());
	if(receiver_iter == player_connections_.get<name_t>().end()) {
		send_server_message(player, "Can't find '" + whisper["receiver"].to_string() + "'.", "error");
		return;
	}

	auto g = player->get_game();
	if(g && g->started() && g->is_player(player_connections_.project<0>(receiver_iter))) {
		send_server_message(player, "You cannot send private messages to players in a running game you observe.", "error");
		return;
	}

	simple_wml::document cwhisper;

	simple_wml::node& trunc_whisper = cwhisper.root().add_child("whisper");
	whisper.copy_into(trunc_whisper);

	const simple_wml::string_span& msg = trunc_whisper["message"];
	chat_message::truncate_message(msg, trunc_whisper);

	send_to_player(player_connections_.project<0>(receiver_iter), cwhisper);
}

void server::handle_query(player_iterator iter, simple_wml::node& query)
{
	wesnothd::player& player = iter->info();

	const std::string command(query["type"].to_string());
	std::ostringstream response;

	const std::string& query_help_msg =
		"Available commands are: adminmsg <msg>, help, games, metrics,"
		" motd, requests, roll <sides>, sample, stats, status, version, wml.";

	// Commands a player may issue.
	if(command == "status") {
		response << process_command(command + " " + player.name(), player.name());
	} else if(
		command.compare(0, 8, "adminmsg") == 0 ||
		command.compare(0, 6, "report") == 0 ||
		command == "games" ||
		command == "metrics" ||
		command == "motd" ||
		command.compare(0, 7, "version") == 0 ||
		command == "requests" ||
		command.compare(0, 4, "roll") == 0 ||
		command == "sample" ||
		command == "stats" ||
		command == "status " + player.name() ||
		command == "wml"
	) {
		response << process_command(command, player.name());
	} else if(player.is_moderator()) {
		if(command == "signout") {
			LOG_SERVER << "Admin signed out: IP: " << iter->client_ip() << "\tnick: " << player.name();
			player.set_moderator(false);
			// This string is parsed by the client!
			response << "You are no longer recognized as an administrator.";
			if(user_handler_) {
				user_handler_->set_is_moderator(player.name(), false);
			}
		} else {
			LOG_SERVER << "Admin Command: type: " << command << "\tIP: " << iter->client_ip()
					   << "\tnick: " << player.name();
			response << process_command(command, player.name());
			LOG_SERVER << response.str();
		}
	} else if(command == "help" || command.empty()) {
		response << query_help_msg;
	} else if(command == "admin" || command.compare(0, 6, "admin ") == 0) {
		if(admin_passwd_.empty()) {
			send_server_message(iter, "No password set.", "error");
			return;
		}

		std::string passwd;
		if(command.size() >= 6) {
			passwd = command.substr(6);
		}

		if(passwd == admin_passwd_) {
			LOG_SERVER << "New Admin recognized: IP: " << iter->client_ip() << "\tnick: " << player.name();
			player.set_moderator(true);
			// This string is parsed by the client!
			response << "You are now recognized as an administrator.";

			if(user_handler_) {
				user_handler_->set_is_moderator(player.name(), true);
			}
		} else {
			WRN_SERVER << "FAILED Admin attempt with password: '" << passwd << "'\tIP: " << iter->client_ip()
					   << "\tnick: " << player.name();
			response << "Error: wrong password";
		}
	} else {
		response << "Error: unrecognized query: '" << command << "'\n" << query_help_msg;
	}

	send_server_message(iter, response.str(), "info");
}

void server::handle_nickserv(player_iterator player, simple_wml::node& nickserv)
{
	// Check if this server allows nick registration at all
	if(!user_handler_) {
		send_server_message(player, "This server does not allow username registration.", "error");
		return;
	}

	// A user requested a list of which details can be set
	if(nickserv.child("info")) {
		try {
			std::string res = user_handler_->user_info((*nickserv.child("info"))["name"].to_string());
			send_server_message(player, res, "info");
		} catch(const user_handler::error& e) {
			send_server_message(player,
				"There was an error looking up the details of the user '"
				+ (*nickserv.child("info"))["name"].to_string() + "'. "
				+ " The error message was: " + e.message, "error"
			);
		}

		return;
	}
}

void server::handle_message(player_iterator user, simple_wml::node& message)
{
	if(user->info().is_message_flooding()) {
		send_server_message(user,
			"Warning: you are sending too many messages too fast. Your message has not been relayed.", "error");
		return;
	}

	simple_wml::document relay_message;
	message.set_attr_dup("sender", user->name().c_str());

	simple_wml::node& trunc_message = relay_message.root().add_child("message");
	message.copy_into(trunc_message);

	const simple_wml::string_span& msg = trunc_message["message"];
	chat_message::truncate_message(msg, trunc_message);

	if(msg.size() >= 3 && simple_wml::string_span(msg.begin(), 4) == "/me ") {
		LOG_SERVER << user->client_ip() << "\t<" << user->name()
				   << simple_wml::string_span(msg.begin() + 3, msg.size() - 3) << ">";
	} else {
		LOG_SERVER << user->client_ip() << "\t<" << user->name() << "> " << msg;
	}

	send_to_lobby(relay_message, user);
}

void server::handle_create_game(player_iterator player, simple_wml::node& create_game)
{
	if(graceful_restart) {
		static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
		send_to_player(player, leave_game_doc);

		send_server_message(player,
			"This server is shutting down. You aren't allowed to make new games. Please "
			"reconnect to the new server.", "error");

		send_to_player(player, games_and_users_list_);
		return;
	}

	const std::string game_name = create_game["name"].to_string();
	const std::string game_password = create_game["password"].to_string();
	const std::string initial_bans = create_game["ignored"].to_string();

	DBG_SERVER << player->client_ip() << "\t" << player->info().name()
			   << "\tcreates a new game: \"" << game_name << "\".";

	// Create the new game, remove the player from the lobby
	// and set the player as the host/owner.
	player_connections_.modify(player, [this, player, &game_name](player_record& host_record) {
		host_record.get_game().reset(
			new wesnothd::game(*this, player_connections_, player, game_name, save_replays_, replay_save_path_),
			std::bind(&server::cleanup_game, this, std::placeholders::_1)
		);
	});

	wesnothd::game& g = *player->get_game();

	DBG_SERVER << "initial bans: " << initial_bans;
	if(initial_bans != "") {
		g.set_name_bans(utils::split(initial_bans,','));
	}

	if(game_password.empty() == false) {
		g.set_password(game_password);
	}

	create_game.copy_into(g.level().root());
}

void server::cleanup_game(game* game_ptr)
{
	metrics_.game_terminated(game_ptr->termination_reason());

	if(user_handler_){
		user_handler_->db_update_game_end(uuid_, game_ptr->db_id(), game_ptr->get_replay_filename());
	}

	simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
	assert(gamelist != nullptr);

	// Send a diff of the gamelist with the game deleted to players in the lobby
	simple_wml::document diff;
	if(!destructed && make_delete_diff(*gamelist, "gamelist", "game", game_ptr->description(), diff)) {
		send_to_lobby(diff);
	}

	// Delete the game from the games_and_users_list_.
	const simple_wml::node::child_list& games = gamelist->children("game");
	const auto g = std::find(games.begin(), games.end(), game_ptr->description());

	if(g != games.end()) {
		const std::size_t index = std::distance(games.begin(), g);
		gamelist->remove_child("game", index);
	} else {
		// Can happen when the game ends before the scenario was transferred.
		LOG_SERVER << "Could not find game (" << game_ptr->id() << ", " << game_ptr->db_id() << ") to delete in games_and_users_list_.";
	}

	if(destructed) game_ptr->emergency_cleanup();

	delete game_ptr;
}

void server::handle_join_game(player_iterator player, simple_wml::node& join)
{
	const bool observer = join.attr("observe").to_bool();
	const std::string& password = join["password"].to_string();
	int game_id = join["id"].to_int();

	auto g_iter = player_connections_.get<game_t>().find(game_id);

	std::shared_ptr<game> g;
	if(g_iter != player_connections_.get<game_t>().end()) {
		g = g_iter->get_game();
	}

	static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	if(!g) {
		WRN_SERVER << player->client_ip() << "\t" << player->info().name()
				   << "\tattempted to join unknown game:\t" << game_id << ".";
		send_to_player(player, leave_game_doc);
		send_server_message(player, "Attempt to join unknown game.", "error");
		send_to_player(player, games_and_users_list_);
		return;
	} else if(!g->level_init()) {
		WRN_SERVER << player->client_ip() << "\t" << player->info().name()
				   << "\tattempted to join uninitialized game:\t\"" << g->name() << "\" (" << game_id << ").";
		send_to_player(player, leave_game_doc);
		send_server_message(player, "Attempt to join an uninitialized game.", "error");
		send_to_player(player, games_and_users_list_);
		return;
	} else if(player->info().is_moderator()) {
		// Admins are always allowed to join.
	} else if(g->player_is_banned(player, player->info().name())) {
		DBG_SERVER << player->client_ip()
				   << "\tReject banned player: " << player->info().name()
				   << "\tfrom game:\t\"" << g->name() << "\" (" << game_id << ").";
		send_to_player(player, leave_game_doc);
		send_server_message(player, "You are banned from this game.", "error");
		send_to_player(player, games_and_users_list_);
		return;
	} else if(!g->password_matches(password)) {
		WRN_SERVER << player->client_ip() << "\t" << player->info().name()
				   << "\tattempted to join game:\t\"" << g->name() << "\" (" << game_id << ") with bad password";
		send_to_player(player, leave_game_doc);
		send_server_message(player, "Incorrect password.", "error");
		send_to_player(player, games_and_users_list_);
		return;
	}

	bool joined = g->add_player(player, observer);
	if(!joined) {
		WRN_SERVER << player->client_ip() << "\t" << player->info().name()
				   << "\tattempted to observe game:\t\"" << g->name() << "\" (" << game_id
				   << ") which doesn't allow observers.";
		send_to_player(player, leave_game_doc);

		send_server_message(player,
			"Attempt to observe a game that doesn't allow observers. (You probably joined the "
			"game shortly after it filled up.)", "error");

		send_to_player(player, games_and_users_list_);
		return;
	}

	player_connections_.modify(player,
		std::bind(&player_record::set_game, std::placeholders::_1, g));

	g->describe_slots();

	// send notification of changes to the game and user
	simple_wml::document diff;
	bool diff1 = make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", g->changed_description(), diff);
	bool diff2 = make_change_diff(games_and_users_list_.root(), nullptr, "user",
		player->info().config_address(), diff);

	if(diff1 || diff2) {
		send_to_lobby(diff);
	}
}

void server::handle_player_in_game(player_iterator p, simple_wml::document& data)
{
	DBG_SERVER << "in process_data_game...";

	wesnothd::player& player { p->info() };

	game& g = *(p->get_game());
	std::weak_ptr<game> g_ptr{p->get_game()};

	// If this is data describing the level for a game.
	if(data.child("snapshot") || data.child("scenario")) {
		if(!g.is_owner(p)) {
			return;
		}

		// If this game is having its level data initialized
		// for the first time, and is ready for players to join.
		// We should currently have a summary of the game in g.level().
		// We want to move this summary to the games_and_users_list_, and
		// place a pointer to that summary in the game's description.
		// g.level() should then receive the full data for the game.
		if(!g.level_init()) {
			LOG_SERVER << p->client_ip() << "\t" << player.name() << "\tcreated game:\t\"" << g.name() << "\" ("
					   << g.id() << ", " << g.db_id() << ").";
			// Update our config object which describes the open games,
			// and save a pointer to the description in the new game.
			simple_wml::node* const gamelist = games_and_users_list_.child("gamelist");
			assert(gamelist != nullptr);

			simple_wml::node& desc = gamelist->add_child("game");
			g.level().root().copy_into(desc);

			if(const simple_wml::node* m = data.child("multiplayer")) {
				m->copy_into(desc);
			} else {
				WRN_SERVER << p->client_ip() << "\t" << player.name() << "\tsent scenario data in game:\t\""
						   << g.name() << "\" (" << g.id() << ", " << g.db_id() << ") without a 'multiplayer' child.";
				// Set the description so it can be removed in delete_game().
				g.set_description(&desc);
				delete_game(g.id());

				send_server_message(p,
					"The scenario data is missing the [multiplayer] tag which contains the "
					"game settings. Game aborted.", "error");
				return;
			}

			g.set_description(&desc);
			desc.set_attr_dup("id", std::to_string(g.id()).c_str());
		} else {
			WRN_SERVER << p->client_ip() << "\t" << player.name() << "\tsent scenario data in game:\t\""
					   << g.name() << "\" (" << g.id() << ", " << g.db_id() << ") although it's already initialized.";
			return;
		}

		assert(games_and_users_list_.child("gamelist")->children("game").empty() == false);

		simple_wml::node& desc = *g.description_for_writing();

		// Update the game's description.
		// If there is no shroud, then tell players in the lobby
		// what the map looks like
		const simple_wml::node& s = *wesnothd::game::starting_pos(data.root());
		// fixme: the hanlder of [store_next_scenario] below searches for 'mp_shroud' in [scenario]
		//        at least of the these cosed is likely wrong.
		if(!data["mp_shroud"].to_bool()) {
			desc.set_attr_dup("map_data", s["map_data"]);
		}

		if(const simple_wml::node* e = data.child("era")) {
			if(!e->attr("require_era").to_bool(true)) {
				desc.set_attr("require_era", "no");
			}
		}

		if(s["require_scenario"].to_bool(false)) {
			desc.set_attr("require_scenario", "yes");
		}

		const simple_wml::node::child_list& mlist = data.children("modification");
		for(const simple_wml::node* m : mlist) {
			desc.add_child_at("modification", 0);
			desc.child("modification")->set_attr_dup("id", m->attr("id"));
			desc.child("modification")->set_attr_dup("name", m->attr("name"));
			desc.child("modification")->set_attr_dup("addon_id", m->attr("addon_id"));
			desc.child("modification")->set_attr_dup("require_modification", m->attr("require_modification"));
		}

		// Record the full scenario in g.level()
		g.level().swap(data);

		// The host already put himself in the scenario so we just need
		// to update_side_data().
		// g.take_side(sock);
		g.update_side_data();
		g.describe_slots();

		// Send the update of the game description to the lobby.
		simple_wml::document diff;
		make_add_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", diff);
		make_change_diff(games_and_users_list_.root(), nullptr, "user", p->info().config_address(), diff);

		send_to_lobby(diff);

		/** @todo FIXME: Why not save the level data in the history_? */
		return;
		// Everything below should only be processed if the game is already initialized.
	} else if(!g.level_init()) {
		WRN_SERVER << p->client_ip() << "\tReceived unknown data from: " << player.name()
				   << " while the scenario wasn't yet initialized."
				   << data.output();
		return;
		// If the host is sending the next scenario data.
	} else if(const simple_wml::node* scenario = data.child("store_next_scenario")) {
		if(!g.is_owner(p)) {
			return;
		}

		if(!g.level_init()) {
			WRN_SERVER << p->client_ip() << "\tWarning: " << player.name()
					   << "\tsent [store_next_scenario] in game:\t\"" << g.name() << "\" (" << g.id()
					   << ", " << g.db_id() << ") while the scenario is not yet initialized.";
			return;
		}

		g.save_replay();
		if(user_handler_){
			user_handler_->db_update_game_end(uuid_, g.db_id(), g.get_replay_filename());
		}

		g.new_scenario(p);
		g.reset_last_synced_context_id();

		// Record the full scenario in g.level()
		g.level().clear();
		scenario->copy_into(g.level().root());
		g.next_db_id();

		if(g.description() == nullptr) {
			ERR_SERVER << p->client_ip() << "\tERROR: \"" << g.name() << "\" (" << g.id()
					   << ", " << g.db_id() << ") is initialized but has no description_.";
			return;
		}

		simple_wml::node& desc = *g.description_for_writing();

		// Update the game's description.
		if(const simple_wml::node* m = scenario->child("multiplayer")) {
			m->copy_into(desc);
		} else {
			WRN_SERVER << p->client_ip() << "\t" << player.name() << "\tsent scenario data in game:\t\""
					   << g.name() << "\" (" << g.id() << ", " << g.db_id() << ") without a 'multiplayer' child.";

			delete_game(g.id());

			send_server_message(p,
				"The scenario data is missing the [multiplayer] tag which contains the game "
				"settings. Game aborted.", "error");
			return;
		}

		// If there is no shroud, then tell players in the lobby
		// what the map looks like.
		const simple_wml::node& s = *wesnothd::game::starting_pos(g.level().root());
		desc.set_attr_dup("map_data", s["mp_shroud"].to_bool() ? "" : s["map_data"]);

		if(const simple_wml::node* e = data.child("era")) {
			if(!e->attr("require_era").to_bool(true)) {
				desc.set_attr("require_era", "no");
			}
		}

		if(s["require_scenario"].to_bool(false)) {
			desc.set_attr("require_scenario", "yes");
		}

		// Tell everyone that the next scenario data is available.
		static simple_wml::document notify_next_scenario(
			"[notify_next_scenario]\n[/notify_next_scenario]\n", simple_wml::INIT_COMPRESSED);
		g.send_data(notify_next_scenario, p);

		// Send the update of the game description to the lobby.
		update_game_in_lobby(g);
		return;
		// A mp client sends a request for the next scenario of a mp campaign.
	} else if(data.child("load_next_scenario")) {
		g.load_next_scenario(p);
		return;
	} else if(data.child("start_game")) {
		if(!g.is_owner(p)) {
			return;
		}

		// perform controller tweaks, assigning sides as human for their owners etc.
		g.perform_controller_tweaks();

		// Send notification of the game starting immediately.
		// g.start_game() will send data that assumes
		// the [start_game] message has been sent
		g.send_data(data, p);
		g.start_game(p);

		if(user_handler_) {
			const simple_wml::node& m = *g.level().root().child("multiplayer");
			DBG_SERVER << simple_wml::node_to_string(m);
			// [addon] info handling
			std::set<std::string> primary_keys;
			for(const auto& addon : m.children("addon")) {
				for(const auto& content : addon->children("content")) {
					std::string key = uuid_+"-"+std::to_string(g.db_id())+"-"+content->attr("type").to_string()+"-"+content->attr("id").to_string()+"-"+addon->attr("id").to_string();
					if(primary_keys.count(key) == 0) {
						primary_keys.emplace(key);
						unsigned long long rows_inserted = user_handler_->db_insert_game_content_info(uuid_, g.db_id(), content->attr("type").to_string(), content->attr("name").to_string(), content->attr("id").to_string(), addon->attr("id").to_string(), addon->attr("version").to_string());
						if(rows_inserted == 0) {
							WRN_SERVER << "Did not insert content row for [addon] data with uuid '" << uuid_ << "', game ID '" << g.db_id() << "', type '" << content->attr("type").to_string() << "', and content ID '" << content->attr("id").to_string() << "'";
						}
					}
				}
			}
			if(m.children("addon").size() == 0) {
				WRN_SERVER << "Game content info missing for game with uuid '" << uuid_ << "', game ID '" << g.db_id() << "', named '" << g.name() << "'";
			}

			user_handler_->db_insert_game_info(uuid_, g.db_id(), server_id_, g.name(), g.is_reload(), m["observer"].to_bool(), !m["private_replay"].to_bool(), g.has_password());

			const simple_wml::node::child_list& sides = g.get_sides_list();
			for(unsigned side_index = 0; side_index < sides.size(); ++side_index) {
				const simple_wml::node& side = *sides[side_index];
				const auto player = player_connections_.get<name_t>().find(side["player_id"].to_string());
				std::string version;
				std::string source;

				// if "Nobody" is chosen for a side, for example
				if(player == player_connections_.get<name_t>().end()){
					version = "";
					source = "";
				} else {
					version = player->info().version();
					source = player->info().source();

					if(client_sources_.count(source) == 0) {
						source = "Default";
					}
				}

				// approximately determine leader(s) for the side like the client does
				// useful generally to know how often leaders are used vs other leaders
				// also as an indication for which faction was chosen if a custom recruit list is provided since that results in "Custom" in the faction field
				std::vector<std::string> leaders;
				// if a type= attribute is specified for the side, add it
				if(side.attr("type") != "") {
					leaders.emplace_back(side.attr("type").to_string());
				}
				// add each [unit] in the side that has canrecruit=yes
				for(const auto unit : side.children("unit")) {
					if(unit->attr("canrecruit") == "yes") {
						leaders.emplace_back(unit->attr("type").to_string());
					}
				}
				// add any [leader] specified for the side
				for(const auto leader : side.children("leader")) {
					leaders.emplace_back(leader->attr("type").to_string());
				}

				user_handler_->db_insert_game_player_info(uuid_, g.db_id(), side["player_id"].to_string(), side["side"].to_int(), side["is_host"].to_bool(), side["faction"].to_string(), version, source, side["current_player"].to_string(), utils::join(leaders));
			}
		}

		// update the game having changed in the lobby
		update_game_in_lobby(g);
		return;
	} else if(data.child("leave_game")) {
		if(g.remove_player(p)) {
			delete_game(g.id());
		} else {
			bool has_diff = false;
			simple_wml::document diff;

			// After this line, the game object may be destroyed. Don't use `g`!
			player_connections_.modify(p, std::bind(&player_record::enter_lobby, std::placeholders::_1));

			// Only run this if the game object is still valid
			if(auto gStrong = g_ptr.lock()) {
				gStrong->describe_slots();
				//Don't update the game if it no longer exists.
				has_diff |= make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", gStrong->description(), diff);
			}

			// Send all other players in the lobby the update to the gamelist.
			has_diff |= make_change_diff(games_and_users_list_.root(), nullptr, "user", player.config_address(), diff);

			if(has_diff) {
				send_to_lobby(diff, p);
			}

			// Send the player who has quit the gamelist.
			send_to_player(p, games_and_users_list_);
		}

		return;
		// If this is data describing side changes by the host.
	} else if(const simple_wml::node* scenario_diff = data.child("scenario_diff")) {
		if(!g.is_owner(p)) {
			return;
		}

		g.level().root().apply_diff(*scenario_diff);
		const simple_wml::node* cfg_change = scenario_diff->child("change_child");

		if(cfg_change) {
			g.update_side_data();
		}

		g.describe_slots();
		update_game_in_lobby(g);

		g.send_data(data, p);
		return;
		// If a player changes his faction.
	} else if(data.child("change_faction")) {
		g.send_data(data, p);
		return;
		// If the owner of a side is changing the controller.
	} else if(const simple_wml::node* change = data.child("change_controller")) {
		g.transfer_side_control(p, *change);
		g.describe_slots();
		update_game_in_lobby(g);

		return;
		// If all observers should be muted. (toggles)
	} else if(data.child("muteall")) {
		if(!g.is_owner(p)) {
			g.send_server_message("You cannot mute: not the game host.", p);
			return;
		}

		g.mute_all_observers();
		return;
		// If an observer should be muted.
	} else if(const simple_wml::node* mute = data.child("mute")) {
		g.mute_observer(*mute, p);
		return;
		// If an observer should be unmuted.
	} else if(const simple_wml::node* unmute = data.child("unmute")) {
		g.unmute_observer(*unmute, p);
		return;
		// The owner is kicking/banning someone from the game.
	} else if(data.child("kick") || data.child("ban")) {
		bool ban = (data.child("ban") != nullptr);
		auto user { ban
			? g.ban_user(*data.child("ban"), p)
			: g.kick_member(*data.child("kick"), p)};

		if(user) {
			player_connections_.modify(*user, std::bind(&player_record::enter_lobby, std::placeholders::_1));
			g.describe_slots();

			update_game_in_lobby(g, user);

			// Send all other players in the lobby the update to the gamelist.
			simple_wml::document gamelist_diff;
			make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", g.description(), gamelist_diff);
			make_change_diff(games_and_users_list_.root(), nullptr, "user", (*user)->info().config_address(), gamelist_diff);

			send_to_lobby(gamelist_diff, p);

			// Send the removed user the lobby game list.
			send_to_player(*user, games_and_users_list_);
		}

		return;
	} else if(const simple_wml::node* unban = data.child("unban")) {
		g.unban_user(*unban, p);
		return;
		// If info is being provided about the game state.
	} else if(const simple_wml::node* info = data.child("info")) {
		if(!g.is_player(p)) {
			return;
		}

		if((*info)["type"] == "termination") {
			g.set_termination_reason((*info)["condition"].to_string());
			if((*info)["condition"].to_string() == "out of sync") {
				g.send_and_record_server_message(player.name() + " reports out of sync errors.");
				if(user_handler_){
					user_handler_->db_set_oos_flag(uuid_, g.db_id());
				}
			}
		}

		return;
	} else if(data.child("turn")) {
		// Notify the game of the commands, and if it changes
		// the description, then sync the new description
		// to players in the lobby.
		g.process_turn(data, p);
		update_game_in_lobby(g);

		return;
	} else if(data.child("whiteboard")) {
		g.process_whiteboard(data, p);
		return;
	} else if(data.child("change_turns_wml")) {
		g.process_change_turns_wml(data, p);
		update_game_in_lobby(g);
		return;
	} else if(simple_wml::node* sch = data.child("request_choice")) {
		g.handle_choice(*sch, p);
		return;
	} else if(data.child("message")) {
		g.process_message(data, p);
		return;
	} else if(data.child("stop_updates")) {
		g.send_data(data, p);
		return;
	// Data to ignore.
	} else if(
		data.child("error") ||
		data.child("side_secured") ||
		data.root().has_attr("failed") ||
		data.root().has_attr("side")
	) {
		return;
	}

	WRN_SERVER << p->client_ip() << "\tReceived unknown data from: " << player.name()
			   << " in game: \"" << g.name() << "\" (" << g.id() << ", " << g.db_id() << ")\n"
			   << data.output();
}

template<class SocketPtr> void server::send_server_message(SocketPtr socket, const std::string& message, const std::string& type)
{
	simple_wml::document server_message;
	simple_wml::node& msg = server_message.root().add_child("message");
	msg.set_attr("sender", "server");
	msg.set_attr_dup("message", message.c_str());
	msg.set_attr_dup("type", type.c_str());

	async_send_doc_queued(socket, server_message);
}

void server::disconnect_player(player_iterator player)
{
	utils::visit([](auto&& socket) {
		if constexpr (utils::decayed_is_same<tls_socket_ptr, decltype(socket)>) {
			socket->async_shutdown([socket](...) {});
			const char buffer[] = "";
			async_write(*socket, boost::asio::buffer(buffer), [socket](...) { socket->lowest_layer().close(); });
		} else {
			socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
		}
	}, player->socket());
}

void server::remove_player(player_iterator iter)
{
	std::string ip = iter->client_ip();

	const std::shared_ptr<game> g = iter->get_game();
	bool game_ended = false;
	if(g) {
		game_ended = g->remove_player(iter, true, false);
	}

	const simple_wml::node::child_list& users = games_and_users_list_.root().children("user");
	const std::size_t index =
		std::distance(users.begin(), std::find(users.begin(), users.end(), iter->info().config_address()));

	// Notify other players in lobby
	simple_wml::document diff;
	if(make_delete_diff(games_and_users_list_.root(), nullptr, "user", iter->info().config_address(), diff)) {
		send_to_lobby(diff, iter);
	}

	games_and_users_list_.root().remove_child("user", index);

	LOG_SERVER << ip << "\t" << iter->info().name() << "\thas logged off";

	// Find the matching nick-ip pair in the log and update the sign off time
	if(user_handler_) {
		user_handler_->db_update_logout(iter->info().get_login_id());
	} else {
		connection_log ip_name { iter->info().name(), ip, {} };

		auto i = std::find(ip_log_.begin(), ip_log_.end(), ip_name);
		if(i != ip_log_.end()) {
			i->log_off = std::chrono::system_clock::now();
		}
	}

	player_connections_.erase(iter);

	if(lan_server_ > 0s && player_connections_.size() == 0)
		start_lan_server_timer();

	if(game_ended) delete_game(g->id());
}

void server::send_to_lobby(simple_wml::document& data, utils::optional<player_iterator> exclude)
{
	for(const auto& p : player_connections_.get<game_t>().equal_range(0)) {
		auto player { player_connections_.iterator_to(p) };
		if(player != exclude) {
			send_to_player(player, data);
		}
	}
}

void server::send_server_message_to_lobby(const std::string& message, utils::optional<player_iterator> exclude)
{
	for(const auto& p : player_connections_.get<game_t>().equal_range(0)) {
		auto player { player_connections_.iterator_to(p) };
		if(player != exclude) {
			send_server_message(player, message, "alert");
		}
	}
}

void server::send_server_message_to_all(const std::string& message, utils::optional<player_iterator> exclude)
{
	for(auto player = player_connections_.begin(); player != player_connections_.end(); ++player) {
		if(player != exclude) {
			send_server_message(player, message, "alert");
		}
	}
}

void server::start_new_server()
{
	if(restart_command.empty()) {
		return;
	}

	// Example config line:
	// restart_command="./wesnothd-debug -d -c ~/.wesnoth1.5/server.cfg"
	// remember to make new one as a daemon or it will block old one
	if(std::system(restart_command.c_str())) {
		ERR_SERVER << "Failed to start new server with command: " << restart_command;
	} else {
		LOG_SERVER << "New server started with command: " << restart_command;
	}
}

std::string server::process_command(std::string query, std::string issuer_name)
{
	boost::trim(query);

	if(issuer_name == "*socket*" && !query.empty() && query.at(0) == '+') {
		// The first argument might be "+<issuer>: ".
		// In that case we use +<issuer>+ as the issuer_name.
		// (Mostly used for communication with IRC.)
		auto issuer_end = std::find(query.begin(), query.end(), ':');

		std::string issuer(query.begin() + 1, issuer_end);
		if(!issuer.empty()) {
			issuer_name = "+" + issuer + "+";
			query = std::string(issuer_end + 1, query.end());
			boost::trim(query);
		}
	}

	const auto i = std::find(query.begin(), query.end(), ' ');

	try {
		const std::string command = utf8::lowercase(std::string(query.begin(), i));

		std::string parameters = (i == query.end() ? "" : std::string(i + 1, query.end()));
		boost::trim(parameters);

		std::ostringstream out;
		auto handler_itor = cmd_handlers_.find(command);

		if(handler_itor == cmd_handlers_.end()) {
			out << "Command '" << command << "' is not recognized.\n" << help_msg;
		} else {
			const cmd_handler& handler = handler_itor->second;
			try {
				handler(issuer_name, query, parameters, &out);
			} catch(const std::bad_function_call& ex) {
				ERR_SERVER << "While handling a command '" << command
						   << "', caught a std::bad_function_call exception.";
				ERR_SERVER << ex.what();
				out << "An internal server error occurred (std::bad_function_call) while executing '" << command
					<< "'\n";
			}
		}

		return out.str();

	} catch(const utf8::invalid_utf8_exception& e) {
		std::string msg = "While handling a command, caught an invalid utf8 exception: ";
		msg += e.what();
		ERR_SERVER << msg;
		return (msg + '\n');
	}
}

// Shutdown, restart and sample commands can only be issued via the socket.
void server::shut_down_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	if(issuer_name != "*socket*" && !allow_remote_shutdown_) {
		*out << denied_msg;
		return;
	}

	if(parameters == "now") {
		BOOST_THROW_EXCEPTION(server_shutdown("shut down by admin command"));
	} else {
		// Graceful shut down.
		graceful_restart = true;
		acceptor_v6_.close();
		acceptor_v4_.close();

		timer_.expires_after(10s);
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, std::placeholders::_1));

		process_command(
			"msg The server is shutting down. You may finish your games but can't start new ones. Once all "
			"games have ended the server will exit.",
			issuer_name
		);

		*out << "Server is doing graceful shut down.";
	}
}

void server::restart_handler(const std::string& issuer_name,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(issuer_name != "*socket*" && !allow_remote_shutdown_) {
		*out << denied_msg;
		return;
	}

	if(restart_command.empty()) {
		*out << "No restart_command configured! Not restarting.";
	} else {
		graceful_restart = true;
		acceptor_v6_.close();
		acceptor_v4_.close();
		timer_.expires_after(10s);
		timer_.async_wait(std::bind(&server::handle_graceful_timeout, this, std::placeholders::_1));

		start_new_server();

		process_command(
			"msg The server has been restarted. You may finish current games but can't start new ones and "
			"new players can't join this (old) server instance. (So if a player of your game disconnects "
			"you have to save, reconnect and reload the game on the new server instance. It is actually "
			"recommended to do that right away.)",
			issuer_name
		);

		*out << "New server started.";
	}
}

void server::sample_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "Current sample frequency: " << request_sample_frequency;
		return;
	} else if(issuer_name != "*socket*") {
		*out << denied_msg;
		return;
	}

	request_sample_frequency = atoi(parameters.c_str());
	if(request_sample_frequency <= 0) {
		*out << "Sampling turned off.";
	} else {
		*out << "Sampling every " << request_sample_frequency << " requests.";
	}
}

void server::help_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	*out << help_msg;
}

void server::stats_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);

	*out << "Number of games = " << games().size() << "\nTotal number of users = " << player_connections_.size();
}

void server::metrics_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	*out << metrics_;
}

void server::requests_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	metrics_.requests(*out);
}

void server::roll_handler(const std::string& issuer_name,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);
	if(parameters.empty()) {
		return;
	}

	int N;
	try {
		N = std::stoi(parameters);
	} catch(const std::invalid_argument&) {
		*out << "The number of die sides must be a number!";
		return;
	} catch(const std::out_of_range&) {
		*out << "The number of sides is too big for the die!";
		return;
	}

	if(N < 1) {
		*out << "The die cannot have less than 1 side!";
		return;
	}
	std::uniform_int_distribution<int> dice_distro(1, N);
	std::string value = std::to_string(dice_distro(die_));

	*out << "You rolled a die [1 - " + parameters + "] and got a " + value + ".";

	auto player_ptr = player_connections_.get<name_t>().find(issuer_name);
	if(player_ptr == player_connections_.get<name_t>().end()) {
		return;
	}

	auto g_ptr = player_ptr->get_game();
	if(g_ptr) {
		g_ptr->send_server_message_to_all(issuer_name + " rolled a die [1 - " + parameters + "] and got a " + value + ".", player_connections_.project<0>(player_ptr));
	} else {
		*out << " (The result is shown to others only in a game.)";
	}
}

void server::games_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	metrics_.games(*out);
}

void server::wml_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	*out << simple_wml::document::stats();
}

void server::adminmsg_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must type a message.";
		return;
	}

	const std::string& sender = issuer_name;
	const std::string& message = parameters;
	LOG_SERVER << "Admin message: <" << sender
			   << (message.find("/me ") == 0 ? std::string(message.begin() + 3, message.end()) + ">" : "> " + message);

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("whisper");
	msg.set_attr_dup("sender", ("admin message from " + sender).c_str());
	msg.set_attr_dup("message", message.c_str());

	int n = 0;
	for(const auto& player : player_connections_) {
		if(player.info().is_moderator()) {
			++n;
			send_to_player(player_connections_.iterator_to(player), data);
		}
	}

	bool is_admin = false;

	for(const auto& player : player_connections_) {
		if(issuer_name == player.info().name() && player.info().is_moderator()) {
			is_admin = true;
			break;
		}
	}

	if(!is_admin) {
		*out << "Your report has been logged and sent to the server administrators. Thanks!";
		return;
	}

	*out << "Your report has been logged and sent to " << n << " online administrators. Thanks!";
}

void server::pm_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	auto first_space = std::find(parameters.begin(), parameters.end(), ' ');
	if(first_space == parameters.end()) {
		*out << "You must name a receiver.";
		return;
	}

	const std::string& sender = issuer_name;
	const std::string receiver(parameters.begin(), first_space);

	std::string message(first_space + 1, parameters.end());
	boost::trim(message);

	if(message.empty()) {
		*out << "You must type a message.";
		return;
	}

	simple_wml::document data;
	simple_wml::node& msg = data.root().add_child("whisper");

	// This string is parsed by the client!
	msg.set_attr_dup("sender", ("server message from " + sender).c_str());
	msg.set_attr_dup("message", message.c_str());

	for(const auto& player : player_connections_) {
		if(receiver != player.info().name().c_str()) {
			continue;
		}

		send_to_player(player_connections_.iterator_to(player), data);
		*out << "Message to " << receiver << " successfully sent.";
		return;
	}

	*out << "No such nick: " << receiver;
}

void server::msg_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must type a message.";
		return;
	}

	send_server_message_to_all(parameters);

	LOG_SERVER << "<server"
			   << (parameters.find("/me ") == 0
			   		? std::string(parameters.begin() + 3, parameters.end()) + ">"
					: "> " + parameters);

	*out << "message '" << parameters << "' relayed to players";
}

void server::lobbymsg_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must type a message.";
		return;
	}

	send_server_message_to_lobby(parameters);
	LOG_SERVER << "<server"
			   << (parameters.find("/me ") == 0
					? std::string(parameters.begin() + 3, parameters.end()) + ">"
					: "> " + parameters);

	*out << "message '" << parameters << "' relayed to players";
}

void server::version_handler(
		const std::string& /*issuer_name*/, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "Server version is " << game_config::wesnoth_version.str();
		return;
	}

	for(const auto& player : player_connections_) {
		if(parameters == player.info().name()) {
			*out << "Player " << parameters << " is using wesnoth " << player.info().version();
			return;
		}
	}

	*out << "Player '" << parameters << "' not found.";
}

void server::status_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	*out << "STATUS REPORT for '" << parameters << "'";
	bool found_something = false;

	// If a simple username is given we'll check for its IP instead.
	if(utils::isvalid_username(parameters)) {
		for(const auto& player : player_connections_) {
			if(parameters == player.name()) {
				parameters = player.client_ip();
				found_something = true;
				break;
			}
		}

		if(!found_something) {
			// out << "\nNo match found. You may want to check with 'searchlog'.";
			// return out.str();
			*out << process_command("searchlog " + parameters, issuer_name);
			return;
		}
	}

	const bool match_ip = ((std::count(parameters.begin(), parameters.end(), '.') >= 1) || (std::count(parameters.begin(), parameters.end(), ':') >= 1));
	for(const auto& player : player_connections_) {
		if(parameters.empty() || parameters == "*" ||
			(match_ip  && utils::wildcard_string_match(player.client_ip(), parameters)) ||
			(!match_ip && utils::wildcard_string_match(utf8::lowercase(player.info().name()), utf8::lowercase(parameters)))
		) {
			found_something = true;
			*out << std::endl << player_status(player);
		}
	}

	if(!found_something) {
		*out << "\nNo match found. You may want to check with 'searchlog'.";
	}
}

void server::clones_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& /*parameters*/,
		std::ostringstream* out)
{
	assert(out != nullptr);
	*out << "CLONES STATUS REPORT";

	std::set<std::string> clones;

	for(auto it = player_connections_.begin(); it != player_connections_.end(); ++it) {
		if(clones.find(it->client_ip()) != clones.end()) {
			continue;
		}

		bool found = false;
		for(auto clone = std::next(it); clone != player_connections_.end(); ++clone) {
			if(it->client_ip() == clone->client_ip()) {
				if(!found) {
					found = true;
					clones.insert(it->client_ip());
					*out << std::endl << player_status(*it);
				}

				*out << std::endl << player_status(*clone);
			}
		}
	}

	if(clones.empty()) {
		*out << std::endl << "No clones found.";
	}
}

void server::bans_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	try {
		if(parameters.empty()) {
			ban_manager_.list_bans(*out);
		} else if(utf8::lowercase(parameters) == "deleted") {
			ban_manager_.list_deleted_bans(*out);
		} else if(utf8::lowercase(parameters).find("deleted") == 0) {
			std::string mask = parameters.substr(7);
			ban_manager_.list_deleted_bans(*out, boost::trim_copy(mask));
		} else {
			boost::trim(parameters);
			ban_manager_.list_bans(*out, parameters);
		}

	} catch(const utf8::invalid_utf8_exception& e) {
		ERR_SERVER << "While handling bans, caught an invalid utf8 exception: " << e.what();
	}
}

void server::ban_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	bool banned = false;
	auto first_space = std::find(parameters.begin(), parameters.end(), ' ');

	if(first_space == parameters.end()) {
		*out << ban_manager_.get_ban_help();
		return;
	}

	auto second_space = std::find(first_space + 1, parameters.end(), ' ');
	const std::string target(parameters.begin(), first_space);
	const std::string duration(first_space + 1, second_space);
	auto [success, parsed_time] = ban_manager_.parse_time(duration, std::chrono::system_clock::now());

	if(!success) {
		*out << "Failed to parse the ban duration: '" << duration << "'\n" << ban_manager_.get_ban_help();
		return;
	}

	if(second_space == parameters.end()) {
		--second_space;
	}

	std::string reason(second_space + 1, parameters.end());
	boost::trim(reason);

	if(reason.empty()) {
		*out << "You need to give a reason for the ban.";
		return;
	}

	std::string dummy_group;

	// if we find a '.' consider it an ip mask
	/** @todo  FIXME: make a proper check for valid IPs. */
	if(std::count(target.begin(), target.end(), '.') >= 1) {
		banned = true;

		*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, dummy_group);
	} else {
		for(const auto& player : player_connections_) {
			if(utils::wildcard_string_match(player.info().name(), target)) {
				if(banned) {
					*out << "\n";
				} else {
					banned = true;
				}

				const std::string ip = player.client_ip();
				*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, dummy_group, target);
			}
		}

		if(!banned) {
			*out << "Nickname mask '" << target << "' did not match, no bans set.";
		}
	}
}

void server::kickban_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	bool banned = false;
	auto first_space = std::find(parameters.begin(), parameters.end(), ' ');
	if(first_space == parameters.end()) {
		*out << ban_manager_.get_ban_help();
		return;
	}

	auto second_space = std::find(first_space + 1, parameters.end(), ' ');
	const std::string target(parameters.begin(), first_space);
	const std::string duration(first_space + 1, second_space);
	auto [success, parsed_time] = ban_manager_.parse_time(duration, std::chrono::system_clock::now());

	if(!success) {
		*out << "Failed to parse the ban duration: '" << duration << "'\n" << ban_manager_.get_ban_help();
		return;
	}

	if(second_space == parameters.end()) {
		--second_space;
	}

	std::string reason(second_space + 1, parameters.end());
	boost::trim(reason);

	if(reason.empty()) {
		*out << "You need to give a reason for the ban.";
		return;
	}

	std::string dummy_group;
	std::vector<player_iterator> users_to_kick;

	// if we find a '.' consider it an ip mask
	/** @todo  FIXME: make a proper check for valid IPs. */
	if(std::count(target.begin(), target.end(), '.') >= 1) {
		banned = true;

		*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, dummy_group);

		for(player_iterator player = player_connections_.begin(); player != player_connections_.end(); ++player) {
			if(utils::wildcard_string_match(player->client_ip(), target)) {
				users_to_kick.push_back(player);
			}
		}
	} else {
		for(player_iterator player = player_connections_.begin(); player != player_connections_.end(); ++player) {
			if(utils::wildcard_string_match(player->info().name(), target)) {
				if(banned) {
					*out << "\n";
				} else {
					banned = true;
				}

				const std::string ip = player->client_ip();
				*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, dummy_group, target);
				users_to_kick.push_back(player);
			}
		}

		if(!banned) {
			*out << "Nickname mask '" << target << "' did not match, no bans set.";
		}
	}

	for(auto user : users_to_kick) {
		*out << "\nKicked " << user->info().name() << " (" << user->client_ip() << ").";
		utils::visit([this,reason](auto&& socket) { async_send_error(socket, "You have been banned. Reason: " + reason); }, user->socket());
		disconnect_player(user);
	}
}

void server::gban_handler(
		const std::string& issuer_name, const std::string& /*query*/, std::string& parameters, std::ostringstream* out)
{
	assert(out != nullptr);

	bool banned = false;
	auto first_space = std::find(parameters.begin(), parameters.end(), ' ');
	if(first_space == parameters.end()) {
		*out << ban_manager_.get_ban_help();
		return;
	}

	auto second_space = std::find(first_space + 1, parameters.end(), ' ');
	const std::string target(parameters.begin(), first_space);

	std::string group = std::string(first_space + 1, second_space);
	first_space = second_space;
	second_space = std::find(first_space + 1, parameters.end(), ' ');

	const std::string duration(first_space + 1, second_space);
	auto [success, parsed_time] = ban_manager_.parse_time(duration, std::chrono::system_clock::now());

	if(!success) {
		*out << "Failed to parse the ban duration: '" << duration << "'\n" << ban_manager_.get_ban_help();
		return;
	}

	if(second_space == parameters.end()) {
		--second_space;
	}

	std::string reason(second_space + 1, parameters.end());
	boost::trim(reason);

	if(reason.empty()) {
		*out << "You need to give a reason for the ban.";
		return;
	}

	// if we find a '.' consider it an ip mask
	/** @todo  FIXME: make a proper check for valid IPs. */
	if(std::count(target.begin(), target.end(), '.') >= 1) {
		banned = true;

		*out << ban_manager_.ban(target, parsed_time, reason, issuer_name, group);
	} else {
		for(const auto& player : player_connections_) {
			if(utils::wildcard_string_match(player.info().name(), target)) {
				if(banned) {
					*out << "\n";
				} else {
					banned = true;
				}

				const std::string ip = player.client_ip();
				*out << ban_manager_.ban(ip, parsed_time, reason, issuer_name, group, target);
			}
		}

		if(!banned) {
			*out << "Nickname mask '" << target << "' did not match, no bans set.";
		}
	}
}

void server::unban_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must enter an ipmask to unban.";
		return;
	}

	ban_manager_.unban(*out, parameters);
}

void server::ungban_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must enter an ipmask to ungban.";
		return;
	}

	ban_manager_.unban_group(*out, parameters);
}

void server::kick_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must enter a mask to kick.";
		return;
	}

	auto i = std::find(parameters.begin(), parameters.end(), ' ');
	const std::string kick_mask = std::string(parameters.begin(), i);
	const std::string kick_message = (i == parameters.end()
		? "You have been kicked."
		: "You have been kicked. Reason: " + std::string(i + 1, parameters.end()));

	bool kicked = false;

	// if we find a '.' consider it an ip mask
	const bool match_ip = (std::count(kick_mask.begin(), kick_mask.end(), '.') >= 1);

	std::vector<player_iterator> users_to_kick;
	for(player_iterator player = player_connections_.begin(); player != player_connections_.end(); ++player) {
		if((match_ip && utils::wildcard_string_match(player->client_ip(), kick_mask)) ||
		  (!match_ip && utils::wildcard_string_match(player->info().name(), kick_mask))
		) {
			users_to_kick.push_back(player);
		}
	}

	for(const auto& player : users_to_kick) {
		if(kicked) {
			*out << "\n";
		} else {
			kicked = true;
		}

		*out << "Kicked " << player->name() << " (" << player->client_ip() << "). '"
			 << kick_message << "'";

		utils::visit([this, &kick_message](auto&& socket) { async_send_error(socket, kick_message); }, player->socket());
		disconnect_player(player);
	}

	if(!kicked) {
		*out << "No user matched '" << kick_mask << "'.";
	}
}

void server::motd_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		if(!motd_.empty()) {
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

void server::searchlog_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	if(parameters.empty()) {
		*out << "You must enter a mask to search for.";
		return;
	}

	*out << "IP/NICK LOG for '" << parameters << "'";

	// If this looks like an IP look up which nicks have been connected from it
	// Otherwise look for the last IP the nick used to connect
	const bool match_ip = (std::count(parameters.begin(), parameters.end(), '.') >= 1);

	if(!user_handler_) {
		bool found_something = false;

		for(const auto& i : ip_log_) {
			const std::string& username = i.nick;
			const std::string& ip = i.ip;

			if((match_ip && utils::wildcard_string_match(ip, parameters)) ||
			(!match_ip && utils::wildcard_string_match(utf8::lowercase(username), utf8::lowercase(parameters)))
			) {
				found_something = true;
				auto player = player_connections_.get<name_t>().find(username);

				if(player != player_connections_.get<name_t>().end() && player->client_ip() == ip) {
					*out << std::endl << player_status(*player);
				} else {
					*out << "\n'" << username << "' @ " << ip
						<< " last seen: " << chrono::format_local_timestamp(i.log_off, "%H:%M:%S %d.%m.%Y");
				}
			}
		}

		if(!found_something) {
			*out << "\nNo match found.";
		}
	} else {
		if(!match_ip) {
			utils::to_sql_wildcards(parameters);
			user_handler_->get_ips_for_user(parameters, out);
		} else {
			user_handler_->get_users_for_ip(parameters, out);
		}
	}
}

void server::dul_handler(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	assert(out != nullptr);

	try {
		if(parameters.empty()) {
			*out << "Unregistered login is " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
		} else {
			deny_unregistered_login_ = (utf8::lowercase(parameters) == "yes");
			*out << "Unregistered login is now " << (deny_unregistered_login_ ? "disallowed" : "allowed") << ".";
		}

	} catch(const utf8::invalid_utf8_exception& e) {
		ERR_SERVER << "While handling dul (deny unregistered logins), caught an invalid utf8 exception: " << e.what();
	}
}

void server::stopgame(const std::string& /*issuer_name*/,
		const std::string& /*query*/,
		std::string& parameters,
		std::ostringstream* out)
{
	const std::string nick = parameters.substr(0, parameters.find(' '));
	const std::string reason = parameters.length() > nick.length()+1 ? parameters.substr(nick.length()+1) : "";
	auto player = player_connections_.get<name_t>().find(nick);

	if(player != player_connections_.get<name_t>().end()){
		std::shared_ptr<game> g = player->get_game();
		if(g){
			*out << "Player '" << nick << "' is in game with id '" << g->id() << ", " << g->db_id() << "' named '" << g->name() << "'.  Ending game for reason: '" << reason << "'...";
			delete_game(g->id(), reason);
		} else {
			*out << "Player '" << nick << "' is not currently in a game.";
		}
	} else {
		*out << "Player '" << nick << "' is not currently logged in.";
	}
}

void server::delete_game(int gameid, const std::string& reason)
{
	// Set the availability status for all quitting users.
	auto range_pair = player_connections_.get<game_t>().equal_range(gameid);

	// Make a copy of the iterators so that we can change them while iterating over them.
	// We can use pair::first_type since equal_range returns a pair of iterators.
	std::vector<decltype(range_pair)::first_type> range_vctor;

	for(auto it = range_pair.first; it != range_pair.second; ++it) {
		range_vctor.push_back(it);
		it->info().mark_available();

		simple_wml::document udiff;
		if(make_change_diff(games_and_users_list_.root(), nullptr, "user", it->info().config_address(), udiff)) {
			send_to_lobby(udiff);
		} else {
			ERR_SERVER << "ERROR: delete_game(): Could not find user in players_.";
		}
	}

	// Put the remaining users back in the lobby.
	// This will call cleanup_game() deleter since there won't
	// be any references to that game from player_connections_ anymore
	for(const auto& it : range_vctor) {
		player_connections_.get<game_t>().modify(it, std::bind(&player_record::enter_lobby, std::placeholders::_1));
	}

	// send users in the game a notification to leave the game since it has ended
	static simple_wml::document leave_game_doc("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);

	for(const auto& it : range_vctor) {
		player_iterator p { player_connections_.project<0>(it) };
		if(reason != "") {
			simple_wml::document leave_game_doc_reason("[leave_game]\n[/leave_game]\n", simple_wml::INIT_STATIC);
			leave_game_doc_reason.child("leave_game")->set_attr_dup("reason", reason.c_str());
			send_to_player(p, leave_game_doc_reason);
		} else {
			send_to_player(p, leave_game_doc);
		}
		send_to_player(p, games_and_users_list_);
	}
}

void server::update_game_in_lobby(wesnothd::game& g, utils::optional<player_iterator> exclude)
{
	simple_wml::document diff;
	if(auto p_desc = g.changed_description()) {
		if(make_change_diff(*games_and_users_list_.child("gamelist"), "gamelist", "game", p_desc, diff)) {
			send_to_lobby(diff, exclude);
		}
	}
}

} // namespace wesnothd

int main(int argc, char** argv)
{
	int port = 15000;
	bool keep_alive = false;

	srand(static_cast<unsigned>(std::time(nullptr)));

	std::string config_file;

	// setting path to currentworking directory
	game_config::path = filesystem::get_cwd();

	// show 'info' by default
	lg::set_log_domain_severity("server", lg::info());
	lg::timestamps(true);

	for(int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if((val == "--config" || val == "-c") && arg + 1 != argc) {
			config_file = argv[++arg];
		} else if(val == "--verbose" || val == "-v") {
			lg::set_log_domain_severity("all", lg::debug());
		} else if(val == "--dump-wml" || val == "-w") {
			dump_wml = true;
		} else if(val.substr(0, 6) == "--log-") {
			std::size_t p = val.find('=');
			if(p == std::string::npos) {
				PLAIN_LOG << "unknown option: " << val;
				return 2;
			}

			std::string s = val.substr(6, p - 6);
			lg::severity severity;

			if(s == "error") {
				severity = lg::err().get_severity();
			} else if(s == "warning") {
				severity = lg::warn().get_severity();
			} else if(s == "info") {
				severity = lg::info().get_severity();
			} else if(s == "debug") {
				severity = lg::debug().get_severity();
			} else {
				PLAIN_LOG << "unknown debug level: " << s;
				return 2;
			}

			while(p != std::string::npos) {
				std::size_t q = val.find(',', p + 1);
				s = val.substr(p + 1, q == std::string::npos ? q : q - (p + 1));

				if(!lg::set_log_domain_severity(s, severity)) {
					PLAIN_LOG << "unknown debug domain: " << s;
					return 2;
				}

				p = q;
			}
		} else if((val == "--port" || val == "-p") && arg + 1 != argc) {
			port = atoi(argv[++arg]);
		} else if(val == "--keepalive") {
			keep_alive = true;
		} else if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
					  << " [-dvwV] [-c path] [-p port]\n"
					  << "  -c, --config <path>        Tells wesnothd where to find the config file to use.\n"
					  << "  -d, --daemon               Runs wesnothd as a daemon.\n"
					  << "  -h, --help                 Shows this usage message.\n"
					  << "  --log-<level>=<domain1>,<domain2>,...\n"
					  << "                             sets the severity level of the debug domains.\n"
					  << "                             'all' can be used to match any debug domain.\n"
					  << "                             Available levels: error, warning, info, debug.\n"
					  << "  -p, --port <port>          Binds the server to the specified port.\n"
					  << "  --keepalive                Enable TCP keepalive.\n"
					  << "  -v  --verbose              Turns on more verbose logging.\n"
					  << "  -V, --version              Returns the server version.\n"
					  << "  -w, --dump-wml             Print all WML sent to clients to stdout.\n";
			return 0;
		} else if(val == "--version" || val == "-V") {
			std::cout << "Battle for Wesnoth server " << game_config::wesnoth_version.str() << "\n";
			return 0;
		} else if(val == "--daemon" || val == "-d") {
#ifdef _WIN32
			ERR_SERVER << "Running as a daemon is not supported on this platform";
			return -1;
#else
			const pid_t pid = fork();
			if(pid < 0) {
				ERR_SERVER << "Could not fork and run as a daemon";
				return -1;
			} else if(pid > 0) {
				std::cout << "Started wesnothd as a daemon with process id " << pid << "\n";
				return 0;
			}

			setsid();
#endif
		} else if(val == "--request_sample_frequency" && arg + 1 != argc) {
			wesnothd::request_sample_frequency = atoi(argv[++arg]);
		} else {
			ERR_SERVER << "unknown option: " << val;
			return 2;
		}
	}

	return wesnothd::server(port, keep_alive, config_file).run();
}
