/*
	Copyright (C) 2008 - 2024
	by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifdef HAVE_MYSQLPP

#include "server/common/forum_user_handler.hpp"
#include "server/wesnothd/server.hpp"
#include "serialization/chrono.hpp"
#include "hash.hpp"
#include "log.hpp"
#include "config.hpp"

#include <boost/asio/post.hpp>

#include <cstdlib>
#include <sstream>

static lg::log_domain log_mp_user_handler("mp_user_handler");
#define ERR_UH LOG_STREAM(err, log_mp_user_handler)
#define WRN_UH LOG_STREAM(warn, log_mp_user_handler)
#define LOG_UH LOG_STREAM(info, log_mp_user_handler)
#define DBG_UH LOG_STREAM(debug, log_mp_user_handler)

namespace {
	const int USER_INACTIVE = 1;
	const int USER_IGNORE = 2;
}

fuh::fuh(const config& c)
	: conn_(c)
	, db_users_table_(c["db_users_table"].str())
	, db_extra_table_(c["db_extra_table"].str())
	, mp_mod_group_(0)
	, site_admin_group_(0)
	, forum_admin_group_(0)
{
	try {
		mp_mod_group_ = std::stoi(c["mp_mod_group"].str());
	} catch(...) {
		ERR_UH << "Failed to convert the mp_mod_group value of '" << c["mp_mod_group"].str() << "' into an int!  Defaulting to " << mp_mod_group_ << ".";
	}
	try {
		site_admin_group_ = std::stoi(c["site_admin_group"].str());
	} catch(...) {
		ERR_UH << "Failed to convert the site_admin_group_ value of '" << c["site_admin_group"].str() << "' into an int!  Defaulting to " << site_admin_group_ << ".";
	}
	try {
		forum_admin_group_ = std::stoi(c["forum_admin_group"].str());
	} catch(...) {
		ERR_UH << "Failed to convert the forum_admin_group_ value of '" << c["forum_admin_group"].str() << "' into an int!  Defaulting to " << forum_admin_group_ << ".";
	}
}

bool fuh::login(const std::string& name, const std::string& password) {
	// Retrieve users' password as hash
	try {
		std::string hash = get_hashed_password_from_db(name);

		if(utils::md5::is_valid_hash(hash) || utils::bcrypt::is_valid_prefix(hash)) { // md5 hash
			return password == hash;
		} else {
			ERR_UH << "Invalid hash for user '" << name << "'";
			return false;
		}
	} catch (const error& e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message;
		return false;
	}
}

std::string fuh::extract_salt(const std::string& name) {

	// Some double security, this should never be needed
	if(!(user_exists(name))) {
		return "";
	}

	std::string hash;

	try {
		hash = get_hashed_password_from_db(name);
	} catch (const error& e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message;
		return "";
	}

	if(utils::md5::is_valid_hash(hash))
		return hash.substr(0,12);

	if(utils::bcrypt::is_valid_prefix(hash)) {
		try {
			return utils::bcrypt::from_hash_string(hash).get_salt();
		} catch(const utils::hash_error& err) {
			ERR_UH << "Error getting salt from hash of user '" << name << "': " << err.what();
			return "";
		}
	}

	return "";
}

void fuh::user_logged_in(const std::string& name) {
	auto now = chrono::serialize_timestamp(std::chrono::system_clock::now());
	conn_.write_user_int("user_lastvisit", name, static_cast<int>(now));
}

bool fuh::user_exists(const std::string& name) {
	return conn_.user_exists(name);
}

long fuh::get_forum_id(const std::string& name) {
	return conn_.get_forum_id(name);
}

bool fuh::user_is_active(const std::string& name) {
	int user_type = conn_.get_user_int(db_users_table_, "user_type", name);
	return user_type != USER_INACTIVE && user_type != USER_IGNORE;
}

bool fuh::user_is_moderator(const std::string& name) {
	if(!user_exists(name)){
		return false;
	}
	return conn_.get_user_int(db_extra_table_, "user_is_moderator", name) == 1 || (mp_mod_group_ != 0 && conn_.is_user_in_groups(name, { mp_mod_group_ }));
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {
	if(!user_exists(name)){
		return;
	}
	conn_.write_user_int("user_is_moderator", name, is_moderator);
}

fuh::ban_info fuh::user_is_banned(const std::string& name, const std::string& addr)
{
	config b = conn_.get_ban_info(name, addr);

	std::chrono::seconds ban_duration(0);
	if(b["ban_end"].to_unsigned() != 0) {
		auto time_remaining = chrono::parse_timestamp(b["ban_end"].to_unsigned()) - std::chrono::system_clock::now();
		ban_duration = std::chrono::duration_cast<std::chrono::seconds>(time_remaining);
	}

	switch(b["ban_type"].to_int())
	{
		case BAN_NONE:
			return {};
		case BAN_IP:
			LOG_UH << "User '" << name << "' ip " << addr << " banned by IP address";
			return { BAN_IP, ban_duration };
		case BAN_USER:
			LOG_UH << "User '" << name << "' uid " << b["user_id"].str() << " banned by uid";
			return { BAN_USER, ban_duration };
		case BAN_EMAIL:
			LOG_UH << "User '" << name << "' email " << b["email"].str() << " banned by email address";
			return { BAN_EMAIL, ban_duration };
		default:
			ERR_UH << "Invalid ban type '" << b["ban_type"].to_int() << "' returned for user '" << name << "'";
			return {};
	}
}

std::string fuh::user_info(const std::string& name) {
	if(!user_exists(name)) {
		throw error("No user with the name '" + name + "' exists.");
	}

	auto reg_date = get_registrationdate(name);
	auto ll_date = get_lastlogin(name);

	static constexpr std::string_view format = "%a %b %d %T %Y"; // equivalent to std::ctime
	std::string reg_string = chrono::format_local_timestamp(reg_date, format);
	std::string ll_string;

	if(ll_date > decltype(ll_date){}) {
		ll_string = chrono::format_local_timestamp(ll_date, format);
	} else {
		ll_string = "Never\n";
	}

	std::stringstream info;
	info << "Name: " << name << "\n"
		 << "Registered: " << reg_string
		 << "Last login: " << ll_string;
	if(!user_is_active(name)) {
		info << "This account is currently inactive.\n";
	}

	return info.str();
}

std::string fuh::get_hashed_password_from_db(const std::string& user) {
	return conn_.get_user_string(db_users_table_, "user_password", user);
}

std::string fuh::get_user_email(const std::string& user) {
	return conn_.get_user_string(db_users_table_, "user_email", user);
}

void fuh::db_update_addon_download_count(const std::string& instance_version, const std::string& id, const std::string& version) {
	return conn_.update_addon_download_count(instance_version, id, version);
}

std::chrono::system_clock::time_point fuh::get_lastlogin(const std::string& user) {
	return chrono::parse_timestamp(conn_.get_user_int(db_extra_table_, "user_lastvisit", user));
}

std::chrono::system_clock::time_point fuh::get_registrationdate(const std::string& user) {
	return chrono::parse_timestamp(conn_.get_user_int(db_users_table_, "user_regdate", user));
}

std::string fuh::get_uuid(){
	return conn_.get_uuid();
}

std::string fuh::get_tournaments(){
	return conn_.get_tournaments();
}

void fuh::async_get_and_send_game_history(boost::asio::io_context& io_service, wesnothd::server& s, any_socket_ptr socket, int player_id, int offset, std::string& search_game_name, int search_content_type, std::string& search_content) {
	boost::asio::post([this, &s, socket, player_id, offset, &io_service, search_game_name, search_content_type, search_content] {
		boost::asio::post(io_service, [socket, &s, doc = conn_.get_game_history(player_id, offset, search_game_name, search_content_type, search_content)]{
			s.send_to_player(socket, *doc);
		});
	 });
}

void fuh::db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, int reload, int observers, int is_public, int has_password){
	conn_.insert_game_info(uuid, game_id, version, name, reload, observers, is_public, has_password);
}

void fuh::db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location){
	conn_.update_game_end(uuid, game_id, replay_location);
}

void fuh::db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user, const std::string& leaders){
	conn_.insert_game_player_info(uuid, game_id, username, side_number, is_host, faction, version, source, current_user, leaders);
}

unsigned long long fuh::db_insert_game_content_info(const std::string& uuid, int game_id, const std::string& type, const std::string& name, const std::string& id, const std::string& addon_id, const std::string& addon_version){
	return conn_.insert_game_content_info(uuid, game_id, type, name, id, addon_id, addon_version);
}

void fuh::db_set_oos_flag(const std::string& uuid, int game_id){
	conn_.set_oos_flag(uuid, game_id);
}

void fuh::async_test_query(boost::asio::io_context& io_service, int limit) {
	boost::asio::post([this, limit, &io_service] {
		ERR_UH << "async test query starts!";
		int i = conn_.async_test_query(limit);
		boost::asio::post(io_service, [i]{ ERR_UH << "async test query output: " << i; });
	 });
}

bool fuh::db_topic_id_exists(int topic_id) {
	return conn_.topic_id_exists(topic_id);
}

void fuh::db_insert_addon_info(const std::string& instance_version, const std::string& id, const std::string& name, const std::string& type, const std::string& version, bool forum_auth, int topic_id, const std::string uploader) {
	conn_.insert_addon_info(instance_version, id, name, type, version, forum_auth, topic_id, uploader);
}

unsigned long long fuh::db_insert_login(const std::string& username, const std::string& ip, const std::string& version) {
	return conn_.insert_login(username, ip, version);
}

void fuh::db_update_logout(unsigned long long login_id) {
	conn_.update_logout(login_id);
}

void fuh::get_users_for_ip(const std::string& ip, std::ostringstream* out) {
	conn_.get_users_for_ip(ip, out);
}

void fuh::get_ips_for_user(const std::string& username, std::ostringstream* out) {
	conn_.get_ips_for_user(username, out);
}

bool fuh::db_is_user_primary_author(const std::string& instance_version, const std::string& id, const std::string& username) {
	return conn_.is_user_author(instance_version, id, username, 1);
}

bool fuh::db_is_user_secondary_author(const std::string& instance_version, const std::string& id, const std::string& username) {
	return conn_.is_user_author(instance_version, id, username, 0);
}

void fuh::db_delete_addon_authors(const std::string& instance_version, const std::string& id) {
	conn_.delete_addon_authors(instance_version, id);
}

void fuh::db_insert_addon_authors(const std::string& instance_version, const std::string& id, const std::vector<std::string>& primary_authors, const std::vector<std::string>& secondary_authors) {
	// ignore any duplicate authors
	std::set<std::string> inserted_authors;

	for(const std::string& primary_author : primary_authors) {
		if(inserted_authors.count(primary_author) == 0) {
			inserted_authors.emplace(primary_author);
			conn_.insert_addon_author(instance_version, id, primary_author, 1);
		}
	}
	for(const std::string& secondary_author : secondary_authors) {
		if(inserted_authors.count(secondary_author) == 0) {
			inserted_authors.emplace(secondary_author);
			conn_.insert_addon_author(instance_version, id, secondary_author, 0);
		}
	}
}

bool fuh::db_do_any_authors_exist(const std::string& instance_version, const std::string& id) {
	return conn_.do_any_authors_exist(instance_version, id);
}

config fuh::db_get_addon_downloads_info(const std::string& instance_version, const std::string& id) {
	return conn_.get_addon_downloads_info(instance_version, id);
}

config fuh::db_get_forum_auth_usage(const std::string& instance_version) {
	return conn_.get_forum_auth_usage(instance_version);
}

config fuh::db_get_addon_admins() {
	return conn_.get_addon_admins(site_admin_group_, forum_admin_group_);
}

bool fuh::user_is_addon_admin(const std::string& name) {
	return conn_.is_user_in_groups(name, { site_admin_group_, forum_admin_group_ });
}

#endif //HAVE_MYSQLPP
