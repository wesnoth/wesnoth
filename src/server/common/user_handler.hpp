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

class config;

#include "exceptions.hpp"

#include <ctime>
#include <string>

/**
 * An interface class to handle nick registration
 * To activate it put a [user_handler] section into the
 * server configuration file
 */
class user_handler {

	// public functions are called by the server
	//
	// private functions are for convenience as they
	// will probably be the same for all user_handler
	// implementations

	public:
		user_handler()
		{
		}

		virtual ~user_handler()
		{
		}

		/**
		 * Return true if the given password matches the password for the given user.
		 *
		 * Password could also be a hash
		 * Seed is not needed for clear text log ins
		 * Currently the login procedure in the server and client code is hardcoded
		 * for the forum_user_handler implementation
		 */
		virtual bool login(const std::string& name, const std::string& password, const std::string& seed) =0;

		/** Executed when the user with the given name logged in. */
		virtual void user_logged_in(const std::string& name) =0;

		/**
		 * Returns a string containing info like the last login of this user.
		 *
		 * Formatted for user readable output.
		 */
		virtual std::string user_info(const std::string& name) =0;

		/** Returns true if a user with the given name exists. */
		virtual bool user_exists(const std::string& name) =0;

		/** Returns true if the specified user account is usable for logins. */
		virtual bool user_is_active(const std::string& name) =0;

		/** Returns true if this user is a moderator on this server */
		virtual bool user_is_moderator(const std::string& name) =0;

		/** Mark this user as a moderator */
		virtual void set_is_moderator(const std::string& name, const bool& is_moderator) =0;

		/** Ban type values */
		enum BAN_TYPE
		{
			BAN_NONE = 0,	/**< Not a ban */
			BAN_IP = 1, 	/**< IP address ban */
			BAN_USER = 2,	/**< User account/name ban */
			BAN_EMAIL = 3,	/**< Account email address ban */
		};

		/** Ban status description */
		struct ban_info
		{
			BAN_TYPE type;			/**< Ban type */
			std::time_t duration;	/**< Ban duration (0 if permanent) */

			ban_info()
				: type(BAN_NONE)
				, duration(0)
			{
			}

			ban_info(BAN_TYPE ptype, std::time_t pduration)
				: type(ptype)
				, duration(pduration)
			{
			}
		};

		/**
		 * Returns true if this user account or IP address is banned.
		 *
		 * @note The IP address is only used by the @a forum_user_handler
		 *       subclass. Regular IP ban checks are done by @a server_base
		 *       instead.
		 */
		virtual ban_info user_is_banned(const std::string& name, const std::string& addr="") = 0;

		struct error : public game::error {
			error(const std::string& message) : game::error(message) {}
		};

		/** Create a random string of digits for password encryption. */
		std::string create_unsecure_nonce(int length = 8);
		std::string create_secure_nonce();

		/**
		 * Create custom salt.
		 *
		 * If not needed let it return and empty string or whatever you feel like.
		 */
		virtual std::string extract_salt(const std::string& username) =0;

		/**
		 * Does this user_handler want passwords passed encrypted using phpbb's algorithm?
		 *
		 * Let it return true if it does and false if it does not.
		 */
		virtual bool use_phpbb_encryption() const =0;

		virtual std::string get_uuid() =0;
		virtual std::string get_tournaments() =0;
		virtual void db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password, const std::string& map_source, const std::string& map_version, const std::string& era_source, const std::string& era_version) =0;
		virtual void db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location) =0;
		virtual void db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user) =0;
		virtual void db_insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name, const std::string& modification_source, const std::string& modification_version) =0;
		virtual void db_set_oos_flag(const std::string& uuid, int game_id) =0;
};
