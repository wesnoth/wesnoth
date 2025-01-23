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

#include "server/common/user_handler.hpp"
#include "server/common/dbconn.hpp"

#include <chrono>
#include <vector>
#include <memory>

/**
 * A class to handle the non-SQL logic for connecting to the phpbb forum database.
 */
class fuh : public user_handler
{
public:
	/**
	 * Reads wesnothd's config for the data needed to initialize this class and @ref dbconn.
	 */
	fuh(const config& c);

	/**
	 * Retrieves the player's hashed password from the phpbb forum database and checks if it matches the hashed password sent by the client.
	 *
	 * @param name The username used to login.
	 * @param password The hashed password sent by the client.
	 *             @see server::send_password_request().
	 * @return Whether the hashed password sent by the client matches the hash retrieved from the phpbb database.
	 */
	bool login(const std::string& name, const std::string& password);

	/**
	 * Needed because the hashing algorithm used by phpbb requires some info
	 * from the original hash to recreate the same hash
	 *
	 * @return the salt, or an empty string if an error occurs.
	 */
	std::string extract_salt(const std::string& name);

	/**
	 * Sets the last login time to the current time.
	 *
	 * @param name The player's username.
	 */
	void user_logged_in(const std::string& name);

	/**
	 * @param name The player's username.
	 * @return Whether the player's username is exists in the forum database.
	 */
	bool user_exists(const std::string& name);

	/**
	 * @param name The player's username.
	 * @return The phpbb USER_ID value created when the player registers on the forums.
	 * @note wesnothd allows the same player to login with multiple clients using the same username but with different case letters (ie: abc and ABC).
	 *       This means that this value is not necessarily unique among all connected clients.
	 */
	long get_forum_id(const std::string& name);

	/**
	 * @param name The player's username.
	 * @return Whether the username has been activated.
	 */
	bool user_is_active(const std::string& name);

	/**
	 * @param name The player's username.
	 * @return Whether the user is a moderator or not.
	 * @note This can be either from the extra table or whether the player is a member of the MP Moderators groups.
	 */
	bool user_is_moderator(const std::string& name);

	/**
	 * Sets or unsets whether the player should be considered a moderator in the extra table.
	 *
	 * @param name The player's username.
	 * @param is_moderator The moderator value to set.
	 */
	void set_is_moderator(const std::string& name, const bool& is_moderator);

	/**
	 * @param name The player's username.
	 * @param addr The IP address being checked.
	 * @return Whether the user is banned, and if so then how long. See also @ref user_handler::ban_info().
	 * @note This checks for bans by username, the email associated to the username, and IP address.
	 * @note Glob IP and email address bans are NOT supported yet since they require a different kind of query that isn't supported
	 *       by our prepared SQL statement API right now. However, they are basically never used on forums.wesnoth.org,
	 *       so this shouldn't be a problem.
	 */
	ban_info user_is_banned(const std::string& name, const std::string& addr);

	/**
	 * @param name The player's username.
	 * @return A string containing basic information about the player.
	 */
	std::string user_info(const std::string& name);

	/**
	 * @return A unique UUID from the backing database.
	 */
	std::string get_uuid();

	/**
	 * @return A list of active tournaments pulled from the Tournaments subforum.
	 */
	std::string get_tournaments();

	/**
	 * Runs an asynchronous query to fetch the user's game history data.
	 * The result is then posted back to the main boost::asio thread to be sent to the requesting player.
	 *
	 * @param io_service The boost io_service to use to post the query results back to the main boost::asio thread.
	 * @param s The server instance the player is connected to.
	 * @param socket The socket used to communicate with the player's client.
	 * @param player_id The forum ID of the player to get the game history for.
	 * @param offset Where to start returning rows to the client from the query results.
	 * @param search_game_name Query for games matching this name. Supports leading and/or trailing wildcards.
	 * @param search_content_type The content type to query for (ie: scenario)
	 * @param search_content Query for games using this content ID. Supports leading and/or trailing wildcards.
	 */
	void async_get_and_send_game_history(boost::asio::io_context& io_service, wesnothd::server& s, any_socket_ptr socket, int player_id, int offset, std::string& search_game_name, int search_content_type, std::string& search_content);

	/**
	 * Inserts game related information.
	 *
	 * @param uuid The value returned by @ref get_uuid().
	 * @param game_id The game's db_id.
	 * @param version The version of wesnothd running this game.
	 * @param name The game's name as entered by the user.
	 * @param reload Whether this game was loaded from the save of a previous game.
	 * @param observers Whether observers are allowed.
	 * @param is_public Whether the game's replay will be publicly available.
	 * @param has_password Whether the game has a password.
	 */
	void db_insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, int reload, int observers, int is_public, int has_password);

	/**
	 * Update the game related information when the game ends.
	 *
	 * @param uuid The value returned by @ref get_uuid().
	 * @param game_id The game's db_id.
	 * @param replay_location The location of the game's publicly available replay.
	 */
	void db_update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);

	/**
	 * Inserts player information per side.
	 *
	 * @param uuid The value returned by @ref get_uuid().
	 * @param game_id The game's db_id.
	 * @param username The username of the player who owns this side.
	 * @param side_number This side's side number.
	 * @param is_host Whether this player is the host.
	 * @param faction The name of this side's faction.
	 * @param version The version of Wesnoth this player is using.
	 * @param source The source where this player downloaded Wesnoth (ie: Steam, SourceForge, etc).
	 * @param current_user The player currently in control of this side.
	 * @param leaders The comma-delimited list of leader unit types for that side.
	 */
	void db_insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user, const std::string& leaders);

	/**
	 * Inserts information about the content being played.
	 *
	 * @param uuid The value returned by @ref get_uuid().
	 * @param game_id The game's db_id.
	 * @param type The add-on content's type (ie: era, scenario, etc).
	 * @param name The name of the content.
	 * @param id The id of the content.
	 * @param addon_id The id of the addon that the content is from.
	 * @param addon_version The version of the add-on.
	 * @return The number of rows inserted which should always be 1.
	 */
	unsigned long long db_insert_game_content_info(const std::string& uuid, int game_id, const std::string& type, const std::string& name, const std::string& id, const std::string& addon_id, const std::string& addon_version);

	/**
	 * Sets the OOS flag in the database if wesnothd is told by a client it has detected an OOS error.
	 *
	 * @param uuid The value returned by @ref get_uuid().
	 * @param game_id The game's db_id.
	 */
	void db_set_oos_flag(const std::string& uuid, int game_id);

	/**
	 * A simple test query for running a query asynchronously.
	 * The main point is that it takes a meaningful amount of time to complete so that it's easy to see that multiple are running at once and are finishing out of order.
	 *
	 * @param io_service The boost io_service to use to post the query results back to the main boost::asio thread.
	 * @param limit How many recursions to make in the query.
	 */
	void async_test_query(boost::asio::io_context& io_service, int limit);

	/**
	 * Checks whether a forum thread with @a topic_id exists.
	 *
	 * @param topic_id The topic id to check for.
	 * @return True if the thread exists or there was a database failure, false if the topic wasn't found.
	 */
	bool db_topic_id_exists(int topic_id);

	/**
	 * Inserts information about an uploaded add-on into the database.
	 *
	 * @param instance_version The version of campaignd the add-on was uploaded to.
	 * @param id The add-on's ID (aka directory name).
	 * @param name The add-on's name from the pbl.
	 * @param type The add-on's type from the pbl.
	 * @param version The add-on's version from the pbl.
	 * @param forum_auth Whether the provided author and password should be matched a forum account or not.
	 * @param topic_id The forum topic ID of the add-on's feedback thread, 0 if not present.
	 * @param uploader The person uploading this version of the add-on.
	 */
	void db_insert_addon_info(const std::string& instance_version, const std::string& id, const std::string& name, const std::string& type, const std::string& version, bool forum_auth, int topic_id, const std::string uploader);

	/**
	 * Inserts into the database for when a player logs in.
	 *
	 * @param username The username of who logged in. The username is converted to lower case when inserting in order to allow index usage when querying.
	 * @param ip The ip address of who logged in.
	 * @param version The version of the client that logged in.
	 */
	unsigned long long db_insert_login(const std::string& username, const std::string& ip, const std::string& version);

	/**
	 * Updates the database for when a player logs out.
	 *
	 * @param login_id The generated ID that uniquely identifies the row to be updated.
	 */
	void db_update_logout(unsigned long long login_id);

	/**
	 * Searches for all players that logged in using the ip address.
	 * The '%' wildcard can be used to search for partial ip addresses.
	 *
	 * @param ip The ip address to search for.
	 * @param out Where to output the results.
	 */
	void get_users_for_ip(const std::string& ip, std::ostringstream* out);

	/**
	 * Searches for all ip addresses used by the player.
	 * The username is converted to lower case to allow a case insensitive select query to be executed while still using an index.
	 * The '%' wildcard can be used to search for partial usernames.
	 *
	 * @param username The username to search for.
	 * @param out Where to output the results.
	 */
	void get_ips_for_user(const std::string& username, std::ostringstream* out);

	/**
	 * @param user The player's username.
	 * @return The player's email address from the phpbb forum database.
	 */
	std::string get_user_email(const std::string& user);

	/**
	 * Increments the download count for this add-on for the specific version.
	 *
	 * @param instance_version The version of campaignd the add-on was uploaded to.
	 * @param id The add-on's ID (aka directory name).
	 * @param version The version of the add-on being downloaded. May not be the most recent version.
	 */
	void db_update_addon_download_count(const std::string& instance_version, const std::string& id, const std::string& version);

	/**
	 * Checks whether the provided username is the primary author of the add-on.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 * @param username The username attempting to do something with the add-on.
	 * @return true if the user is the primary author of an addon, false otherwise.
	 */
	bool db_is_user_primary_author(const std::string& instance_version, const std::string& id, const std::string& username);

	/**
	 * Checks whether the provided username is a secondary author of the add-on.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 * @param username The username attempting to do something with the add-on.
	 * @return true if the user is a secondary author of an addon, false otherwise.
	 */
	bool db_is_user_secondary_author(const std::string& instance_version, const std::string& id, const std::string& username);

	/**
	 * Removes the authors information from addon_author for a particular addon and version.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 */
	void db_delete_addon_authors(const std::string& instance_version, const std::string& id);

	/**
	 * Inserts rows for the primary and secondary authors for a particular addon and version.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 * @param primary_authors The primary authors of the add-on.
	 * @param secondary_authors The secondary authors of the add-on.
	 */
	void db_insert_addon_authors(const std::string& instance_version, const std::string& id, const std::vector<std::string>& primary_authors, const std::vector<std::string>& secondary_authors);

	/**
	 * Checks whether any author information exists for a particular addon and version, since if there's no author information then of course no primary or secondary authors will ever be found.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 * @return true if any author information exists for this addon, false otherwise.
	 */
	bool db_do_any_authors_exist(const std::string& instance_version, const std::string& id);

	/**
	 * Gets a list of download count by version for add-ons.
	 *
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @param id The ID of the add-on.
	 * @return The results of the query.
	 */
	config db_get_addon_downloads_info(const std::string& instance_version, const std::string& id);

	/**
	 * @param instance_version Which major version this is for (1.16, 1.17, etc).
	 * @return The total count of add-ons amd the count of add-ons using forum_auth.
	 */
	config db_get_forum_auth_usage(const std::string& instance_version);

	/**
	 * @return the list of account names that have admin abilities, ie deleting or hiding add-ons
	 */
	config db_get_addon_admins();

	/**
	 * @param name The provided username.
	 * @return Whether the username is in any groups specified as admins.
	 */
	bool user_is_addon_admin(const std::string& name);

private:
	/** An instance of the class responsible for executing the queries and handling the database connection. */
	dbconn conn_;
	/** The name of the phpbb users table */
	std::string db_users_table_;
	/** The name of the extras custom table, not part of a phpbb database */
	std::string db_extra_table_;
	/** The group ID of the forums MP Moderators group */
	int mp_mod_group_;
	/** The group ID of the forums Site Administrators group */
	int site_admin_group_;
	/** The group ID of the forums Forum Administrators group */
	int forum_admin_group_;

	/**
	 * @param user The player's username.
	 * @return The player's hashed password from the phpbb forum database.
	 */
	std::string get_hashed_password_from_db(const std::string& user);

	/**
	 * @param user The player's username.
	 * @return The player's last login time.
	 */
	std::chrono::system_clock::time_point get_lastlogin(const std::string& user);

	/**
	 * @param user The player's username.
	 * @return The player's forum registration date.
	 */
	std::chrono::system_clock::time_point get_registrationdate(const std::string& user);
};
