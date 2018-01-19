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

#pragma once

#include "user_handler.hpp"

#include <vector>
#include <memory>

#include <mysql/mysql.h>

// The [user_handler] section in the server configuration
// file could look like this:
//
//[user_handler]
//	db_name=phpbb3
//	db_host=localhost
//	db_user=root
//	db_password=secret
//	db_users_table=users
//	db_extra_table=extra_data
//[/user_handler]

/**
 * A user_handler implementation to link the server with a phpbb3 forum.
 */
class fuh : public user_handler {
	public:
		fuh(const config& c);
		~fuh();

		// Throws user_handler::error
		void add_user(const std::string& name, const std::string& mail, const std::string& password);

		// Throws user_handler::error
		void remove_user(const std::string& name);

		void clean_up() {}

		bool login(const std::string& name, const std::string& password, const std::string& seed);

		/**
		 * Needed because the hashing algorithm used by phpbb requires some info
		 * from the original hash to recreate the same hash
		 *
		 * Return an empty string if an error occurs
		 */
		std::string create_pepper(const std::string& name);

		void user_logged_in(const std::string& name);

		bool user_exists(const std::string& name);

		bool user_is_active(const std::string& name);

		bool user_is_moderator(const std::string& name);
		void set_is_moderator(const std::string& name, const bool& is_moderator);

		// Throws user_handler::error
		void password_reminder(const std::string& name);

		// Throws user_handler::error
		std::string user_info(const std::string& name);

		// Throws user_handler::error
		void set_user_detail(const std::string& user, const std::string& detail, const std::string& value);
		std::string get_valid_details();

		bool use_phpbb_encryption() const { return true; }

	private:
		std::string get_hash(const std::string& user);
		std::string get_mail(const std::string& user);
		/*std::vector<std::string> get_friends(const std::string& user);
		std::vector<std::string> get_ignores(const std::string& user);*/
		time_t get_lastlogin(const std::string& user);
		time_t get_registrationdate(const std::string& user);
		bool is_inactive(const std::string& user);

		void set_lastlogin(const std::string& user, const time_t& lastlogin);

		std::string db_name_, db_host_, db_user_, db_password_, db_users_table_, db_extra_table_;

		typedef std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> mysql_result;

		MYSQL *conn;

		template<typename T, typename... Args>
		inline T prepared_statement(const std::string& sql, Args&&...);
		// Query a detail for a particular user from the database
		template<typename T>
		T get_detail_for_user(const std::string& name, const std::string& detail);
		template<typename T>
		T get_writable_detail_for_user(const std::string& name, const std::string& detail);

		// Write something to the write table
		template<typename T>
		void write_detail(const std::string& name, const std::string& detail, T&& value);

		// Same as user_exists() but checks if we have a row for this user in the extra table
		bool extra_row_exists(const std::string& name);
};
