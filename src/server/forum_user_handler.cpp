/*
   Copyright (C) 2008 - 2018 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
	, db_extra_table_(c["db_extra_table"].str())
	, conn(mysql_init(nullptr))
{
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
	} catch (error& e) {
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

std::string fuh::create_pepper(const std::string& name) {

	// Some double security, this should never be needed
	if(!(user_exists(name))) {
		return "";
	}

	std::string hash;

	try {
		hash = get_hash(name);
	} catch (error& e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message << std::endl;
		return "";
	}

	if(utils::md5::is_valid_hash(hash))
		return hash.substr(0,12);

	if(utils::bcrypt::is_valid_prefix(hash)) {
		try {
			return utils::bcrypt::from_hash_string(hash).get_salt();
		} catch(utils::hash_error& err) {
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
	} catch (sql_error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		// If the database is down just let all usernames log in
		return false;
	}
}

bool fuh::user_is_active(const std::string& name) {
	try {
		int user_type = get_detail_for_user<int>(name, "user_type");
		return user_type != USER_INACTIVE && user_type != USER_IGNORE;
	} catch (sql_error& e) {
		ERR_UH << "Could not retrieve user type for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

bool fuh::user_is_moderator(const std::string& name) {

	if(!user_exists(name)) return false;

	try {
		return get_writable_detail_for_user<int>(name, "user_is_moderator") == 1;
	} catch (sql_error& e) {
		ERR_UH << "Could not query user_is_moderator for user '" << name << "' :" << e.message << std::endl;
		// If the database is down mark nobody as a mod
		return false;
	}
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {

	if(!user_exists(name)) return;

	try {
		write_detail(name, "user_is_moderator", int(is_moderator));
	} catch (sql_error& e) {
		ERR_UH << "Could not set is_moderator for user '" << name << "' :" << e.message << std::endl;
	}
}

void fuh::password_reminder(const std::string& /*name*/) {
	throw error("For now please use the password recovery "
		"function provided at https://forums.wesnoth.org");
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
	} catch (sql_error& e) {
		ERR_UH << "Could not retrieve password for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

std::string fuh::get_mail(const std::string& user) {
	try {
		return get_detail_for_user<std::string>(user, "user_email");
	} catch (sql_error& e) {
		ERR_UH << "Could not retrieve email for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

time_t fuh::get_lastlogin(const std::string& user) {
	try {
		int time_int = get_writable_detail_for_user<int>(user, "user_lastvisit");
		return time_t(time_int);
	} catch (sql_error& e) {
		ERR_UH << "Could not retrieve last visit for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

time_t fuh::get_registrationdate(const std::string& user) {
	try {
		int time_int = get_detail_for_user<int>(user, "user_regdate");
		return time_t(time_int);
	} catch (sql_error& e) {
		ERR_UH << "Could not retrieve registration date for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

void fuh::set_lastlogin(const std::string& user, const time_t& lastlogin) {

	try {
		write_detail(user, "user_lastvisit", int(lastlogin));
	} catch (sql_error& e) {
		ERR_UH << "Could not set last visit for user '" << user << "' :" << e.message << std::endl;
	}
}

template<typename T, typename... Args>
inline T fuh::prepared_statement(const std::string& sql, Args&&... args)
{
	try {
		return ::prepared_statement<T>(conn, sql, std::forward<Args>(args)...);
	} catch (sql_error& e) {
		WRN_UH << "caught sql error: " << e.message << std::endl;
		WRN_UH << "trying to reconnect and retry..." << std::endl;
		//Try to reconnect and execute query again
		if(!mysql_real_connect(conn, db_host_.c_str(),  db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), 0, nullptr, 0)) {
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
	} catch (sql_error& e) {
		ERR_UH << "Could not set detail for user '" << name << "': " << e.message << std::endl;
	}
}

bool fuh::extra_row_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return prepared_statement<bool>("SELECT 1 FROM `" + db_extra_table_ + "` WHERE UPPER(username)=UPPER(?)", name);
	} catch (sql_error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

#endif //HAVE_MYSQLPP
