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

#include "server/forum_user_handler.hpp"
#include "server/mysql_prepared_statement.ipp"
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
	: db_name_(c["db_name"].str())
	, db_host_(c["db_host"].str())
	, db_user_(c["db_user"].str())
	, db_password_(c["db_password"].str())
	, db_users_table_(c["db_users_table"].str())
	, db_banlist_table_(c["db_banlist_table"].str())
	, db_extra_table_(c["db_extra_table"].str())
	, db_game_info_table_(c["db_game_info_table"].str())
	, db_game_player_info_table_(c["db_game_player_info_table"].str())
	, db_game_modification_info_table_(c["db_game_modification_info_table"].str())
	, conn(mysql_init(nullptr))
{
	mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
	if(!conn || !mysql_real_connect(conn, db_host_.c_str(),  db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), 0, nullptr, 0)) {
		ERR_UH << "Could not connect to database: " << mysql_errno(conn) << ": " << mysql_error(conn) << std::endl;
	}
}

fuh::~fuh() {
	mysql_close(conn);
}

void fuh::add_user(const std::string& /*name*/, const std::string& /*mail*/, const std::string& /*password*/) {
	throw error("For now please register at https://forums.wesnoth.org");
}

void fuh::remove_user(const std::string& /*name*/) {
	throw error("'Dropping your nickname is currently impossible");
}

// The hashing code is basically taken from forum_auth.cpp
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
	set_lastlogin(name, time(nullptr));
}

bool fuh::user_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return prepared_statement<bool>("SELECT 1 FROM `" + db_users_table_ + "` WHERE UPPER(username)=UPPER(?)", name);
	} catch (const sql_error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		// If the database is down just let all usernames log in
		return false;
	}
}

bool fuh::user_is_active(const std::string& name) {
	try {
		int user_type = get_detail_for_user<int>(name, "user_type");
		return user_type != USER_INACTIVE && user_type != USER_IGNORE;
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve user type for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

bool fuh::user_is_moderator(const std::string& name) {

	if(!user_exists(name)) return false;

	try {
		return get_writable_detail_for_user<int>(name, "user_is_moderator") == 1;
	} catch (const sql_error& e) {
		ERR_UH << "Could not query user_is_moderator for user '" << name << "' :" << e.message << std::endl;
		// If the database is down mark nobody as a mod
		return false;
	}
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {

	if(!user_exists(name)) return;

	try {
		write_detail(name, "user_is_moderator", static_cast<int>(is_moderator));
	} catch (const sql_error& e) {
		ERR_UH << "Could not set is_moderator for user '" << name << "' :" << e.message << std::endl;
	}
}

fuh::ban_info fuh::user_is_banned(const std::string& name, const std::string& addr)
{
	//
	// NOTE: glob IP and email address bans are NOT supported yet since they
	//       require a different kind of query that isn't supported by our
	//       prepared SQL statement API right now. However, they are basically
	//       never used on forums.wesnoth.org, so this shouldn't be a problem
	//       for the time being.
	//

	// NOTE: A ban end time of 0 is a permanent ban.
	const std::string& is_extant_ban_sql =
		"ban_exclude = 0 AND (ban_end = 0 OR ban_end >=" + std::to_string(std::time(nullptr)) + ")";

	// TODO: retrieve full ban info in a single statement instead of issuing
	//       separate queries to check for a ban's existence and its duration.

	try {
		if(!addr.empty() && prepared_statement<bool>("SELECT 1 FROM `" + db_banlist_table_ + "` WHERE UPPER(ban_ip) = UPPER(?) AND " + is_extant_ban_sql, addr)) {
			LOG_UH << "User '" << name << "' ip " << addr << " banned by IP address\n";
			return retrieve_ban_info(BAN_IP, addr);
		}
	} catch(const sql_error& e) {
		ERR_UH << "Could not check forum bans on address '" << addr << "' :" << e.message << '\n';
		return {};
	}

	if(!user_exists(name)) return {};

	try {
		auto uid = get_detail_for_user<unsigned int>(name, "user_id");

		if(uid == 0) {
			ERR_UH << "Invalid user id for user '" << name << "'\n";
		} else if(prepared_statement<bool>("SELECT 1 FROM `" + db_banlist_table_ + "` WHERE ban_userid = ? AND " + is_extant_ban_sql, uid)) {
			LOG_UH << "User '" << name << "' uid " << uid << " banned by uid\n";
			return retrieve_ban_info(BAN_USER, uid);
		}

		auto email = get_detail_for_user<std::string>(name, "user_email");

		if(!email.empty() && prepared_statement<bool>("SELECT 1 FROM `" + db_banlist_table_ + "` WHERE UPPER(ban_email) = UPPER(?) AND " + is_extant_ban_sql, email)) {
			LOG_UH << "User '" << name << "' email " << email << " banned by email address\n";
			return retrieve_ban_info(BAN_EMAIL, email);
		}

	} catch(const sql_error& e) {
		ERR_UH << "Could not check forum bans on user '" << name << "' :" << e.message << '\n';
	}

	return {};
}

template<typename T>
fuh::ban_info fuh::retrieve_ban_info(fuh::BAN_TYPE type, T detail)
{
	std::string col;

	switch(type) {
	case BAN_USER:
		col = "ban_userid";
		break;
	case BAN_EMAIL:
		col = "ban_email";
		break;
	case BAN_IP:
		col = "ban_ip";
		break;
	default:
		return {};
	}

	try {
		return { type, retrieve_ban_duration_internal(col, detail) };
	} catch(const sql_error& e) {
		//
		// NOTE:
		// If retrieve_ban_internal() fails to fetch the ban row, odds are the ban was
		// lifted in the meantime (it's meant to be called by user_is_banned(), so we
		// assume the ban expires in one second instead of returning 0 (permanent ban)
		// just to err on the safe side (returning BAN_NONE would be a terrible idea,
		// for that matter).
		//
		return { type, 1 };
	}
}

std::time_t fuh::retrieve_ban_duration_internal(const std::string& col, const std::string& detail)
{
	const std::time_t end_time = prepared_statement<int>("SELECT `ban_end` FROM `" + db_banlist_table_ + "` WHERE UPPER(" + col + ") = UPPER(?)", detail);
	return end_time ? end_time - std::time(nullptr) : 0;
}

std::time_t fuh::retrieve_ban_duration_internal(const std::string& col, unsigned int detail)
{
	const std::time_t end_time = prepared_statement<int>("SELECT `ban_end` FROM `" + db_banlist_table_ + "` WHERE " + col + " = ?", detail);
	return end_time ? end_time - std::time(nullptr) : 0;
}

std::string fuh::user_info(const std::string& name) {
	if(!user_exists(name)) {
		throw error("No user with the name '" + name + "' exists.");
	}

	time_t reg_date = get_registrationdate(name);
	time_t ll_date = get_lastlogin(name);

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

void fuh::set_user_detail(const std::string& /*user*/, const std::string& /*detail*/, const std::string& /*value*/) {
	throw error("For now this is a 'read-only' user_handler");
}

std::string fuh::get_valid_details() {
	return "For now this is a 'read-only' user_handler";
}

std::string fuh::get_hash(const std::string& user) {
	try {
		return get_detail_for_user<std::string>(user, "user_password");
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve password for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

std::string fuh::get_mail(const std::string& user) {
	try {
		return get_detail_for_user<std::string>(user, "user_email");
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve email for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

time_t fuh::get_lastlogin(const std::string& user) {
	try {
		int time_int = get_writable_detail_for_user<int>(user, "user_lastvisit");
		return time_t(time_int);
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve last visit for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

time_t fuh::get_registrationdate(const std::string& user) {
	try {
		int time_int = get_detail_for_user<int>(user, "user_regdate");
		return time_t(time_int);
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve registration date for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

void fuh::set_lastlogin(const std::string& user, const time_t& lastlogin) {

	try {
		write_detail(user, "user_lastvisit", static_cast<int>(lastlogin));
	} catch (const sql_error& e) {
		ERR_UH << "Could not set last visit for user '" << user << "' :" << e.message << std::endl;
	}
}

template<typename T, typename... Args>
inline T fuh::prepared_statement(const std::string& sql, Args&&... args)
{
	try {
		return ::prepared_statement<T>(conn, sql, std::forward<Args>(args)...);
	} catch (const sql_error& e) {
		WRN_UH << "caught sql error: " << e.message << std::endl;
		WRN_UH << "trying to reconnect and retry..." << std::endl;
		//Try to reconnect and execute query again
		mysql_close(conn);
		conn = mysql_init(nullptr);
		if(!conn || !mysql_real_connect(conn, db_host_.c_str(),  db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), 0, nullptr, 0)) {
			ERR_UH << "Could not connect to database: " << mysql_errno(conn) << ": " << mysql_error(conn) << std::endl;
			throw sql_error("Error querying database.");
		}
	}
	return ::prepared_statement<T>(conn, sql, std::forward<Args>(args)...);
}

template<typename T>
T fuh::get_detail_for_user(const std::string& name, const std::string& detail) {
	return prepared_statement<T>(
		"SELECT `" + detail + "` FROM `" + db_users_table_ + "` WHERE UPPER(username)=UPPER(?)",
		name);
}

template<typename T>
T fuh::get_writable_detail_for_user(const std::string& name, const std::string& detail) {
	if(!extra_row_exists(name)) throw sql_error("row doesn't exist");
	return prepared_statement<T>(
		"SELECT `" + detail + "` FROM `" + db_extra_table_ + "` WHERE UPPER(username)=UPPER(?)",
		name);
}

template<typename T>
void fuh::write_detail(const std::string& name, const std::string& detail, T&& value) {
	try {
		// Check if we do already have a row for this user in the extra table
		if(!extra_row_exists(name)) {
			// If not create the row
			prepared_statement<void>("INSERT INTO `" + db_extra_table_ + "` VALUES(?,?,'0')", name, std::forward<T>(value));
		}
		prepared_statement<void>("UPDATE `" + db_extra_table_ + "` SET " + detail + "=? WHERE UPPER(username)=UPPER(?)", std::forward<T>(value), name);
	} catch (const sql_error& e) {
		ERR_UH << "Could not set detail for user '" << name << "': " << e.message << std::endl;
	}
}

bool fuh::extra_row_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return prepared_statement<bool>("SELECT 1 FROM `" + db_extra_table_ + "` WHERE UPPER(username)=UPPER(?)", name);
	} catch (const sql_error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

std::string fuh::get_uuid(){
	try {
		return prepared_statement<std::string>("SELECT UUID()");
	} catch (const sql_error& e) {
		ERR_UH << "Could not retrieve a UUID:" << e.message << std::endl;
		return "";
	}
}

void fuh::db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name){
	try {
		prepared_statement<void>("insert into `" + db_game_info_table_ + "`(INSTANCE_UUID, GAME_ID, INSTANCE_VERSION, GAME_NAME) values(?, ?, ?, ?)",
		uuid, game_id, version, name);
	} catch (const sql_error& e) {
		ERR_UH << "Could not insert into table `" + db_game_info_table_ + "`:" << e.message << std::endl;
	}
}

void fuh::db_update_game_start(const std::string& uuid, int game_id, const std::string& map_name, const std::string& era_name){
	try {
		prepared_statement<void>("update `" + db_game_info_table_ + "` set START_TIME = CURRENT_TIMESTAMP, MAP_NAME = ?, ERA_NAME = ? where INSTANCE_UUID = ? and GAME_ID = ?",
		map_name, era_name, uuid, game_id);
	} catch (const sql_error& e) {
		ERR_UH << "Could not update the game's starting information on table `" + db_game_info_table_ + "`:" << e.message << std::endl;
	}
}

void fuh::db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location){
	try {
		prepared_statement<void>("update `" + db_game_info_table_ + "` set END_TIME = CURRENT_TIMESTAMP, REPLAY_NAME = ? where INSTANCE_UUID = ? and GAME_ID = ?",
		replay_location, uuid, game_id);
	} catch (const sql_error& e) {
		ERR_UH << "Could not update the game's ending information on table `" + db_game_info_table_ + "`:" << e.message << std::endl;
	}
}

void fuh::db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, const std::string& is_host, const std::string& faction){
	try {
		prepared_statement<void>("insert into `" + db_game_player_info_table_ + "`(INSTANCE_UUID, GAME_ID, USER_ID, SIDE_NUMBER, IS_HOST, FACTION) values(?, ?, IFNULL((select user_id from `"+db_users_table_+"` where username = ?), -1), ?, ?, ?)",
		uuid, game_id, username, side_number, is_host, faction);
	} catch (const sql_error& e) {
		ERR_UH << "Could not insert the game's player information on table `" + db_game_player_info_table_ + "`:" << e.message << std::endl;
	}
}

void fuh::db_insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name){
	try {
		prepared_statement<void>("insert into `" + db_game_modification_info_table_ + "`(INSTANCE_UUID, GAME_ID, MODIFICATION_NAME) values(?, ?, ?)",
		uuid, game_id, modification_name);
	} catch (const sql_error& e) {
		ERR_UH << "Could not insert the game's modification information on table `" + db_game_modification_info_table_ + "`:" << e.message << std::endl;
	}
}

void fuh::db_set_oos_flag(const std::string& uuid, int game_id){
	try {
		prepared_statement<void>("UPDATE `" + db_game_info_table_ + "` SET OOS = 'Y' WHERE INSTANCE_UUID = ? AND GAME_ID = ?",
		uuid, game_id);
	} catch (const sql_error& e) {
		ERR_UH << "Could not update the game's OOS flag on table `" + db_game_info_table_ + "`:" << e.message << std::endl;
	}
}

#endif //HAVE_MYSQLPP
