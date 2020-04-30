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
#include "mysql_conn.hpp"

#include <vector>
#include <memory>
#include <ctime>

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

		bool use_phpbb_encryption() const { return true; }

		std::string get_uuid();
		std::string get_tournaments();
		void db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password);
		void db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);
		void db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user);
		void db_insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name);
		void db_set_oos_flag(const std::string& uuid, int game_id);

	private:
		mysql_conn conn;
		std::string db_users_table_;
		std::string db_extra_table_;
		int mp_mod_group_;

		std::string get_hash(const std::string& user);
		std::time_t get_lastlogin(const std::string& user);
		std::time_t get_registrationdate(const std::string& user);
		bool is_inactive(const std::string& user);

		void set_lastlogin(const std::string& user, const std::time_t& lastlogin);

		template<typename T>
		ban_info retrieve_ban_info(BAN_TYPE, T detail);

		std::time_t retrieve_ban_duration_internal(const std::string& col, const std::string& detail);
		std::time_t retrieve_ban_duration_internal(const std::string& col, unsigned int detail);

		bool is_user_in_group(const std::string& name, int group_id);
};

