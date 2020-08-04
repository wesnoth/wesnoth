/*
   Copyright (C) 2008 - 2018 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
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
#include "hash.hpp"
#include "log.hpp"
#include "config.hpp"

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
	: conn(c)
	, db_users_table_(c["db_users_table"].str())
	, db_extra_table_(c["db_extra_table"].str())
	, mp_mod_group_(0)
{
	try {
		mp_mod_group_ = std::stoi(c["mp_mod_group"].str());
	} catch(...) {
		ERR_UH << "Failed to convert the mp_mod_group value of '" << c["mp_mod_group"].str() << "' into an int!  Defaulting to " << mp_mod_group_ << "." << std::endl;
	}
}

bool fuh::login(const std::string& name, const std::string& password, const std::string& seed) {
	// Retrieve users' password as hash
	std::string hash;

	try {
		hash = get_hash(name);
	} catch (const error& e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message << std::endl;
		return false;
	}

	std::string valid_hash;

	if(utils::md5::is_valid_hash(hash)) { // md5 hash
		valid_hash = utils::md5(hash.substr(12,34), seed).base64_digest();
	} else if(utils::bcrypt::is_valid_prefix(hash)) { // bcrypt hash
		valid_hash = utils::md5(hash, seed).base64_digest();
	} else {
		ERR_UH << "Invalid hash for user '" << name << "'" << std::endl;
		return false;
	}

	if(password == valid_hash) return true;

	return false;
}

std::string fuh::extract_salt(const std::string& name) {

	// Some double security, this should never be needed
	if(!(user_exists(name))) {
		return "";
	}

	std::string hash;

	try {
		hash = get_hash(name);
	} catch (const error& e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message << std::endl;
		return "";
	}

	if(utils::md5::is_valid_hash(hash))
		return hash.substr(0,12);

	if(utils::bcrypt::is_valid_prefix(hash)) {
		try {
			return utils::bcrypt::from_hash_string(hash).get_salt();
		} catch(const utils::hash_error& err) {
			ERR_UH << "Error getting salt from hash of user '" << name << "': " << err.what() << std::endl;
			return "";
		}
	}

	return "";
}

void fuh::user_logged_in(const std::string& name) {
	set_lastlogin(name, std::time(nullptr));
}

bool fuh::user_exists(const std::string& name) {
	return conn.user_exists(name);
}

bool fuh::user_is_active(const std::string& name) {
	int user_type = conn.get_user_int(db_users_table_, "user_type", name);
	return user_type != USER_INACTIVE && user_type != USER_IGNORE;
}

bool fuh::user_is_moderator(const std::string& name) {
	if(!user_exists(name)){
		return false;
	}
	return conn.get_user_int(db_extra_table_, "user_is_moderator", name) == 1 || (mp_mod_group_ != 0 && conn.is_user_in_group(name, mp_mod_group_));
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {
	if(!user_exists(name)){
		return;
	}
	conn.write_user_int("user_is_moderator", name, is_moderator);
}

fuh::ban_info fuh::user_is_banned(const std::string& name, const std::string& addr)
{
	//
	// NOTE: glob IP and email address bans are NOT supported yet since they
	//       require a different kind of query that isn't supported by our
	//       prepared SQL statement API right now. However, they are basically
	//       never used on forums.wesnoth.org, so this shouldn't be a problem
	//       for the time being.
	ban_check b = conn.get_ban_info(name, addr);
	switch(b.get_ban_type())
	{
		case BAN_NONE:
			return {};
		case BAN_IP:
			LOG_UH << "User '" << name << "' ip " << addr << " banned by IP address\n";
			return { BAN_IP, b.get_ban_duration() };
		case BAN_USER:
			LOG_UH << "User '" << name << "' uid " << b.get_user_id() << " banned by uid\n";
			return { BAN_USER, b.get_ban_duration() };
		case BAN_EMAIL:
			LOG_UH << "User '" << name << "' email " << b.get_email() << " banned by email address\n";
			return { BAN_EMAIL, b.get_ban_duration() };
		default:
			ERR_UH << "Invalid ban type '" << b.get_ban_type() << "' returned for user '" << name << "'\n";
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

std::string fuh::get_hash(const std::string& user) {
	return conn.get_user_string(db_users_table_, "user_password", user);
}

std::time_t fuh::get_lastlogin(const std::string& user) {
	return std::time_t(conn.get_user_int(db_extra_table_, "user_lastvisit", user));
}

std::time_t fuh::get_registrationdate(const std::string& user) {
	return std::time_t(conn.get_user_int(db_users_table_, "user_regdate", user));
}

void fuh::set_lastlogin(const std::string& user, const std::time_t& lastlogin) {
	conn.write_user_int("user_lastvisit", user, static_cast<int>(lastlogin));
}

std::string fuh::get_uuid(){
	return conn.get_uuid();
}

std::string fuh::get_tournaments(){
	return conn.get_tournaments();
}

void fuh::db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password, const std::string& map_source, const std::string& map_version, const std::string& era_source, const std::string& era_version){
	conn.insert_game_info(uuid, game_id, version, name, map_name, era_name, reload, observers, is_public, has_password, map_source, map_version, era_source, era_version);
}

void fuh::db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location){
	conn.update_game_end(uuid, game_id, replay_location);
}

void fuh::db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user){
	conn.insert_game_player_info(uuid, game_id, username, side_number, is_host, faction, version, source, current_user);
}

void fuh::db_insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name, const std::string& modification_source, const std::string& modification_version){
	conn.insert_modification_info(uuid, game_id, modification_name, modification_source, modification_version);
}

void fuh::db_set_oos_flag(const std::string& uuid, int game_id){
	conn.set_oos_flag(uuid, game_id);
}

#endif //HAVE_MYSQLPP
