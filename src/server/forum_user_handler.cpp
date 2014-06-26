/*
   Copyright (C) 2008 - 2014 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
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

#include "forum_user_handler.hpp"
#include "../hash.hpp"
#include "log.hpp"
#include "config.hpp"

#include <stdlib.h>
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
	, conn(mysql_init(NULL))
{
	if(!conn || !mysql_real_connect(conn, db_host_.c_str(),  db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), 0, NULL, 0)) {
		ERR_UH << "Could not connect to database: " << mysql_errno(conn) << ": " << mysql_error(conn) << std::endl;
	}
}

fuh::~fuh() {
	mysql_close(conn);
}

void fuh::add_user(const std::string& /*name*/, const std::string& /*mail*/, const std::string& /*password*/) {
	throw error("For now please register at http://forum.wesnoth.org");
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

	// Check hash prefix, if different than $H$ hash is invalid
	if(!util::is_valid_hash(hash)) {
		ERR_UH << "Invalid hash for user '" << name << "'" << std::endl;
		return false;
	}

	std::string valid_hash = util::create_hash(hash.substr(12,34), seed);

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

	if(!util::is_valid_hash(hash)) return "";

	return hash.substr(0,12);
}

void fuh::user_logged_in(const std::string& name) {
	set_lastlogin(name, time(NULL));
}

bool fuh::user_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return mysql_fetch_row(db_query("SELECT username FROM " + db_users_table_ + " WHERE UPPER(username)=UPPER('" + name + "')"));
	} catch (error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		// If the database is down just let all usernames log in
		return false;
	}
}

bool fuh::user_is_active(const std::string& name) {
	try {
		int user_type = atoi(get_detail_for_user(name, "user_type").c_str());
		return user_type != USER_INACTIVE && user_type != USER_IGNORE;
	} catch (error& e) {
		ERR_UH << "Could not retrieve user type for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

bool fuh::user_is_moderator(const std::string& name) {

	if(!user_exists(name)) return false;

	try {
		return get_writable_detail_for_user(name, "user_is_moderator") == "1";
	} catch (error& e) {
		ERR_UH << "Could not query user_is_moderator for user '" << name << "' :" << e.message << std::endl;
		// If the database is down mark nobody as a mod
		return false;
	}
}

void fuh::set_is_moderator(const std::string& name, const bool& is_moderator) {

	if(!user_exists(name)) return;

	try {
		write_detail(name, "user_is_moderator", is_moderator ? "1" : "0");
	} catch (error& e) {
		ERR_UH << "Could not set is_moderator for user '" << name << "' :" << e.message << std::endl;
	}
}

void fuh::password_reminder(const std::string& /*name*/) {
	throw error("For now please use the password recovery "
		"function provided at http://forum.wesnoth.org");
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
		return get_detail_for_user(user, "user_password");
	} catch (error& e) {
		ERR_UH << "Could not retrieve password for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

std::string fuh::get_mail(const std::string& user) {
	try {
		return get_detail_for_user(user, "user_email");
	} catch (error& e) {
		ERR_UH << "Could not retrieve email for user '" << user << "' :" << e.message << std::endl;
		return "";
	}
}

time_t fuh::get_lastlogin(const std::string& user) {
	try {
		int time_int = atoi(get_writable_detail_for_user(user, "user_lastvisit").c_str());
		return time_t(time_int);
	} catch (error& e) {
		ERR_UH << "Could not retrieve last visit for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

time_t fuh::get_registrationdate(const std::string& user) {
	try {
		int time_int = atoi(get_detail_for_user(user, "user_regdate").c_str());
		return time_t(time_int);
	} catch (error& e) {
		ERR_UH << "Could not retrieve registration date for user '" << user << "' :" << e.message << std::endl;
		return time_t(0);
	}
}

void fuh::set_lastlogin(const std::string& user, const time_t& lastlogin) {

	std::stringstream ss;
	ss << lastlogin;

	try {
		write_detail(user, "user_lastvisit", ss.str());
	} catch (error& e) {
		ERR_UH << "Could not set last visit for user '" << user << "' :" << e.message << std::endl;
	}
}

MYSQL_RES* fuh::db_query(const std::string& sql) {
	if(mysql_query(conn, sql.c_str())) {
		WRN_UH << "not connected to database, reconnecting..." << std::endl;
		//Try to reconnect and execute query again
		if(!mysql_real_connect(conn, db_host_.c_str(),  db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), 0, NULL, 0)
				|| mysql_query(conn, sql.c_str())) {
			ERR_UH << "Could not connect to database: " << mysql_errno(conn) << ": " << mysql_error(conn) << std::endl;
			throw error("Error querying database.");
		}
	}
	return mysql_store_result(conn);
}

std::string fuh::db_query_to_string(const std::string& sql) {
	return std::string(mysql_fetch_row(db_query(sql))[0]);
}


std::string fuh::get_detail_for_user(const std::string& name, const std::string& detail) {
	return db_query_to_string("SELECT " + detail + " FROM " + db_users_table_ + " WHERE UPPER(username)=UPPER('" + name + "')");
}

std::string fuh::get_writable_detail_for_user(const std::string& name, const std::string& detail) {
	if(!extra_row_exists(name)) return "";
	return db_query_to_string("SELECT " + detail + " FROM " + db_extra_table_ + " WHERE UPPER(username)=UPPER('" + name + "')");
}

void fuh::write_detail(const std::string& name, const std::string& detail, const std::string& value) {
	try {
		// Check if we do already have a row for this user in the extra table
		if(!extra_row_exists(name)) {
			// If not create the row
			db_query("INSERT INTO " + db_extra_table_ + " VALUES('" + name + "','" + value + "','0')");
		}
		db_query("UPDATE " + db_extra_table_ + " SET " + detail + "='" + value + "' WHERE UPPER(username)=UPPER('" + name + "')");
	} catch (error& e) {
		ERR_UH << "Could not set detail for user '" << name << "': " << e.message << std::endl;
	}
}

bool fuh::extra_row_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return mysql_fetch_row(db_query("SELECT username FROM " + db_extra_table_ + " WHERE UPPER(username)=UPPER('" + name + "')"));
	} catch (error& e) {
		ERR_UH << "Could not execute test query for user '" << name << "' :" << e.message << std::endl;
		return false;
	}
}

#endif //HAVE_MYSQLPP
