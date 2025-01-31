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

#pragma once

class config;

#include "exceptions.hpp"

#include <string>

#include <boost/asio/io_context.hpp>

#include "server/wesnothd/player_connection.hpp"

namespace wesnothd
{
	class server;
}

/**
 * An interface class to handle nick registration
 * To activate it put a [user_handler] section into the
 * server configuration file
 */
class user_handler
{
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
	virtual bool login(const std::string& name, const std::string& password) = 0;

	/** Executed when the user with the given name logged in. */
	virtual void user_logged_in(const std::string& name) = 0;

	/**
	 * Returns a string containing info like the last login of this user.
	 *
	 * Formatted for user readable output.
	 */
	virtual std::string user_info(const std::string& name) = 0;

	/** Returns true if a user with the given name exists. */
	virtual bool user_exists(const std::string& name) = 0;

	/** Returns the forum user id for the given username */
	virtual long get_forum_id(const std::string& name) = 0;

	/** Returns the user's email from the forum database */
	virtual std::string get_user_email(const std::string& user) = 0;

	/** Returns true if the specified user account is usable for logins. */
	virtual bool user_is_active(const std::string& name) = 0;

	/** Returns true if this user is a moderator on this server */
	virtual bool user_is_moderator(const std::string& name) = 0;

	/** Mark this user as a moderator */
	virtual void set_is_moderator(const std::string& name, const bool& is_moderator) = 0;

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
		BAN_TYPE type = BAN_NONE;			/**< Ban type */
		std::chrono::seconds duration{0};	/**< Ban duration (0 if permanent) */
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

	/**
	 * Create custom salt.
	 */
	virtual std::string extract_salt(const std::string& username) = 0;

	virtual std::string get_uuid() = 0;
	virtual std::string get_tournaments() = 0;
	virtual void async_get_and_send_game_history(boost::asio::io_context& io_service, wesnothd::server& s, any_socket_ptr socket, int player_id, int offset, std::string& search_game_name, int search_content_type, std::string& search_content) =0;
	virtual void db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, int reload, int observers, int is_public, int has_password) = 0;
	virtual void db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location) = 0;
	virtual void db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user, const std::string& leaders) = 0;
	virtual unsigned long long db_insert_game_content_info(const std::string& uuid, int game_id, const std::string& type, const std::string& name, const std::string& id, const std::string& addon_id, const std::string& addon_version) = 0;
	virtual void db_set_oos_flag(const std::string& uuid, int game_id) = 0;
	virtual void async_test_query(boost::asio::io_context& io_service, int limit) = 0;
	virtual bool db_topic_id_exists(int topic_id) = 0;
	virtual void db_insert_addon_info(const std::string& instance_version, const std::string& id, const std::string& name, const std::string& type, const std::string& version, bool forum_auth, int topic_id, const std::string uploader) = 0;
	virtual unsigned long long db_insert_login(const std::string& username, const std::string& ip, const std::string& version) = 0;
	virtual void db_update_logout(unsigned long long login_id) = 0;
	virtual void get_users_for_ip(const std::string& ip, std::ostringstream* out) = 0;
	virtual void get_ips_for_user(const std::string& username, std::ostringstream* out) = 0;
	virtual void db_update_addon_download_count(const std::string& instance_version, const std::string& id, const std::string& version) = 0;
	virtual bool db_is_user_primary_author(const std::string& instance_version, const std::string& id, const std::string& username) = 0;
	virtual bool db_is_user_secondary_author(const std::string& instance_version, const std::string& id, const std::string& username) = 0;
	virtual void db_delete_addon_authors(const std::string& instance_version, const std::string& id) = 0;
	virtual void db_insert_addon_authors(const std::string& instance_version, const std::string& id, const std::vector<std::string>& primary_authors, const std::vector<std::string>& secondary_authors) = 0;
	virtual bool db_do_any_authors_exist(const std::string& instance_version, const std::string& id) = 0;
	virtual config db_get_addon_downloads_info(const std::string& instance_version, const std::string& id) = 0;
	virtual config db_get_forum_auth_usage(const std::string& instance_version) = 0;
	virtual config db_get_addon_admins() = 0;
	virtual bool user_is_addon_admin(const std::string& name) = 0;
};
