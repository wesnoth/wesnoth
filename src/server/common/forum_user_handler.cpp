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
{
	try {
		mp_mod_group_ = std::stoi(c["mp_mod_group"].str());
	} catch(...) {
		ERR_UH << "Failed to convert the mp_mod_group value of '" << c["mp_mod_group"].str() << "' into an int!  Defaulting to " << mp_mod_group_ << ".";
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
	conn_.write_user_int("user_lastvisit", name, static_cast<int>(std::time(nullptr)));
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
	return conn_.get_user_int(db_extra_table_, "user_is_moderator", name) == 1 || (mp_mod_group_ != 0 && conn_.is_user_in_group(name, mp_mod_group_));
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {
	if(!user_exists(name)){
		return;
	}
	conn_.write_user_int("user_is_moderator", name, is_moderator);
}

fuh::ban_info fuh::user_is_banned(const std::string& name, const std::string& addr)
{
	ban_check b = conn_.get_ban_info(name, addr);
	switch(b.get_ban_type())
	{
		case BAN_NONE:
			return {};
		case BAN_IP:
			LOG_UH << "User '" << name << "' ip " << addr << " banned by IP address";
			return { BAN_IP, b.get_ban_duration() };
		case BAN_USER:
			LOG_UH << "User '" << name << "' uid " << b.get_user_id() << " banned by uid";
			return { BAN_USER, b.get_ban_duration() };
		case BAN_EMAIL:
			LOG_UH << "User '" << name << "' email " << b.get_email() << " banned by email address";
			return { BAN_EMAIL, b.get_ban_duration() };
		default:
			ERR_UH << "Invalid ban type '" << b.get_ban_type() << "' returned for user '" << name << "'";
			return {};
	}
}

std::string fuh::user_info(const std::string& name) {
	if(!user_exists(name)) {
		throw error("No user with the name '" + name + "' exists.");
	}

	std::time_t reg_date = get_registrationdate(name);
	std::time_t ll_date = get_lastlogin(name);

	std::string reg_string = ctime(&reg_date);
	std::string ll_string;

	if(ll_date) {
		ll_string = ctime(&ll_date);
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

std::time_t fuh::get_lastlogin(const std::string& user) {
	return std::time_t(conn_.get_user_int(db_extra_table_, "user_lastvisit", user));
}

std::time_t fuh::get_registrationdate(const std::string& user) {
	return std::time_t(conn_.get_user_int(db_users_table_, "user_regdate", user));
}

std::string fuh::get_uuid(){
	return conn_.get_uuid();
}

std::string fuh::get_tournaments(){
	return conn_.get_tournaments();
}

void fuh::async_get_and_send_game_history(boost::asio::io_service& io_service, wesnothd::server& s, wesnothd::player_iterator player, int player_id, int offset, std::string& search_game_name, int search_content_type, std::string& search_content) {
	boost::asio::post([this, &s, player, player_id, offset, &io_service, search_game_name, search_content_type, search_content] {
		boost::asio::post(io_service, [player, &s, doc = conn_.get_game_history(player_id, offset, search_game_name, search_content_type, search_content)]{
			s.send_to_player(player, *doc);
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

void fuh::async_test_query(boost::asio::io_service& io_service, int limit) {
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

void fuh::db_insert_addon_authors(const std::string& instance_version, const std::string& id, const std::string& primary_author, const std::vector<std::string>& secondary_authors) {
	conn_.insert_addon_author(instance_version, id, primary_author, 1);

	// ignore any duplicate authors
	std::set<std::string> inserted_authors;
	inserted_authors.emplace(primary_author);

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

#endif //HAVE_MYSQLPP
