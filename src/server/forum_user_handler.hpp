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

#pragma once

#include "server/user_handler.hpp"

#include <vector>
#include <memory>
#include <ctime>

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
//	db_banlist_table=banlist
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
		std::string extract_salt(const std::string& name);

		void user_logged_in(const std::string& name);

		bool user_exists(const std::string& name);

		bool user_is_active(const std::string& name);

		bool user_is_moderator(const std::string& name);
		void set_is_moderator(const std::string& name, const bool& is_moderator);

		ban_info user_is_banned(const std::string& name, const std::string& addr);

		// Throws user_handler::error
		std::string user_info(const std::string& name);

		// Throws user_handler::error
		void set_user_detail(const std::string& user, const std::string& detail, const std::string& value);
		std::string get_valid_details();

		bool use_phpbb_encryption() const { return true; }

		std::string get_uuid();
		void db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name);
		void db_update_game_start(const std::string& uuid, int game_id, const std::string& map_name, const std::string& era_name);
		void db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);
		void db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, const std::string& is_host, const std::string& faction);
		void db_insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name);
		void db_set_oos_flag(const std::string& uuid, int game_id);

	private:
		std::string get_hash(const std::string& user);
		std::time_t get_lastlogin(const std::string& user);
		std::time_t get_registrationdate(const std::string& user);
		bool is_inactive(const std::string& user);

		void set_lastlogin(const std::string& user, const std::time_t& lastlogin);

		template<typename T>
		ban_info retrieve_ban_info(BAN_TYPE, T detail);

		std::time_t retrieve_ban_duration_internal(const std::string& col, const std::string& detail);
		std::time_t retrieve_ban_duration_internal(const std::string& col, unsigned int detail);

		std::string db_name_, db_host_, db_user_, db_password_, db_users_table_, db_banlist_table_, db_extra_table_, db_game_info_table_, db_game_player_info_table_, db_game_modification_info_table_, db_group_table_;
		unsigned int mp_mod_group_;

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
		
		bool is_user_in_group(const std::string& name, unsigned int group_id);
};

