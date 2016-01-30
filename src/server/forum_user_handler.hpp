/*
   Copyright (C) 2008 - 2016 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORUM_USER_HANDLER_HPP_INCLUDED
#define FORUM_USER_HANDLER_HPP_INCLUDED

#include "user_handler.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>
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

		// std::unique_ptr would be better, as the object isn't actually shared
		// boost::scoped_ptr cannot be returned, so we can't use that
		// TODO C++11: switch to std::unique_ptr
		typedef boost::shared_ptr<MYSQL_RES> mysql_result;

		// Throws user_handler::error
		mysql_result db_query(const std::string& query);

		// Throws user_handler::error via db_query()
		std::string db_query_to_string(const std::string& query);
		MYSQL *conn;

		// Query a detail for a particular user from the database
		std::string get_detail_for_user(const std::string& name, const std::string& detail);
		std::string get_writable_detail_for_user(const std::string& name, const std::string& detail);

		// Write something to the write table
		void write_detail(const std::string& name, const std::string& detail, const std::string& value);

		// Same as user_exists() but checks if we have a row for this user in the extra table
		bool extra_row_exists(const std::string& name);
};

#endif //FORUM_USER_HANDLER_HPP_INCLUDED
