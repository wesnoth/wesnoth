/*
	Copyright (C) 2020 - 2024
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

#include "config.hpp"
#include "server/common/simple_wml.hpp"

#include <mysql/mysql.h>
#include "mariadb++/account.hpp"
#include "mariadb++/connection.hpp"
#include "mariadb++/statement.hpp"
#include "mariadb++/result_set.hpp"
#include "mariadb++/exceptions.hpp"

#include <vector>
#include <unordered_map>

typedef std::vector<std::variant<bool, int, long, unsigned long long, std::string, const char*>> sql_parameters;

/**
 * This class is responsible for handling the database connections as well as executing queries and handling any results.
 * @note The account and connection should never ever be provided to anything outside of this class.
 * @note !DO NOT APPEND VALUES DIRECTLY TO THE SQL TEXT IN ANY QUERY!
 */
class dbconn
{
public:
	/**
	 * Initializes the synchronous query connection as well as the account object that has the connection settings.
	 *
	 * @param c The config object to read information from.
	 */
	dbconn(const config& c);

	/**
	 * @see forum_user_handler::async_test_query().
	 */
	int async_test_query(int limit);

	/**
	 * @see forum_user_handler::get_uuid().
	 */
	std::string get_uuid();

	/**
	 * @see forum_user_handler::get_tournaments().
	 */
	std::string get_tournaments();

	/**
	 * This is an asynchronous query that is executed on a separate connection to retrieve the game history for the
	 * provided player.
	 *
	 * @param player_id The forum ID of the player to get the game history for.
	 * @param offset The offset to provide to the database for where to start returning rows from.
	 * @param search_game_name Query for games matching this name. Supports leading and/or trailing wildcards.
	 * @param search_content_type The content type to query for (ie: scenario)
	 * @param search_content Query for games using this content ID. Supports leading and/or trailing wildcards.
	 * @return The simple_wml document containing the query results, or simply the @a error attribute if an exception is
	 * thrown.
	 */
	std::unique_ptr<simple_wml::document> get_game_history(
		int player_id, int offset, std::string search_game_name, int search_content_type, std::string search_content);

	/**
	 * @see forum_user_handler::user_exists().
	 */
	bool user_exists(const std::string& name);

	/**
	 * @see forum_user_handler::get_forum_id().
	 */
	long get_forum_id(const std::string& name);

	/**
	 * @param name The player's username.
	 * @return Whether the player has a row in the extra table.
	 */
	bool extra_row_exists(const std::string& name);

	/**
	 * @param name The player's username.
	 * @param group_ids The forum group IDs to check if the user is part of.
	 * @return Whether the user is a member of the forum group.
	 */
	bool is_user_in_groups(const std::string& name, const std::vector<int>& group_ids);

	/**
	 * @param table The table that will be queried.
	 * @param column The column that will be selected.
	 * @param name The player's username.
	 * @return The string value in the provided table and column for the provided username.
	 */
	std::string get_user_string(const std::string& table, const std::string& column, const std::string& name);

	/**
	 * @param table The table that will be queried.
	 * @param column The column that will be selected.
	 * @param name The player's username.
	 * @return The int value in the provided table and column for the provided username.
	 */
	int get_user_int(const std::string& table, const std::string& column, const std::string& name);

	/**
	 * The provided value is updated if a row exists or a new row inserted otherwise.
	 *
	 * @param column The column that the value will be put into.
	 * @param name The player's username.
	 * @param value The value to be put into the column.
	 */
	void write_user_int(const std::string& column, const std::string& name, int value);

	/**
	 * @see forum_user_handler::user_is_banned().
	 */
	config get_ban_info(const std::string& name, const std::string& ip);

	/**
	 * @see forum_user_handler::db_insert_game_info().
	 */
	void insert_game_info(
		const std::string& uuid,
		int game_id,
		const std::string& version,
		const std::string& name,
		int reload,
		int observers,
		int is_public,
		int has_password);

	/**
	 * @see forum_user_handler::db_update_game_end().
	 */
	void update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);

	/**
	 * @see forum_user_handler::db_insert_game_player_info().
	 */
	void insert_game_player_info(
		const std::string& uuid,
		int game_id,
		const std::string& username,
		int side_number,
		int is_host,
		const std::string& faction,
		const std::string& version,
		const std::string& source,
		const std::string& current_user,
		const std::string& leaders);

	/**
	 * @see forum_user_handler::db_insert_game_content_info().
	 */
	unsigned long long insert_game_content_info(
		const std::string& uuid,
		int game_id,
		const std::string& type,
		const std::string& name,
		const std::string& id,
		const std::string& addon_id,
		const std::string& addon_version);

	/**
	 * @see forum_user_handler::db_set_oos_flag().
	 */
	void set_oos_flag(const std::string& uuid, int game_id);

	/**
	 * @see forum_user_handler::db_topic_id_exists().
	 */
	bool topic_id_exists(int topic_id);

	/**
	 * @see forum_user_handler::db_insert_addon_info().
	 */
	void insert_addon_info(
		const std::string& instance_version,
		const std::string& id,
		const std::string& name,
		const std::string& type,
		const std::string& version,
		bool forum_auth,
		int topic_id,
		const std::string& uploader);

	/**
	 * @see forum_user_handler::db_insert_login().
	 */
	unsigned long long insert_login(const std::string& username, const std::string& ip, const std::string& version);

	/**
	 * @see forum_user_handler::db_update_logout().
	 */
	void update_logout(unsigned long long login_id);

	/**
	 * @see forum_user_handler::get_users_for_ip().
	 */
	void get_users_for_ip(const std::string& ip, std::ostringstream* out);

	/**
	 * @see forum_user_handler::get_ips_for_users().
	 */
	void get_ips_for_user(const std::string& username, std::ostringstream* out);

	/**
	 * @see forum_user_handler::db_update_addon_download_count().
	 */
	void update_addon_download_count(
		const std::string& instance_version, const std::string& id, const std::string& version);

	/**
	 * @see forum_user_handler::is_user_primary_author().
	 * @see forum_user_handler::is_user_secondary_author().
	 */
	bool is_user_author(
		const std::string& instance_version, const std::string& id, const std::string& username, int is_primary);

	/**
	 * @see forum_user_handler::db_delete_addon_authors().
	 */
	void delete_addon_authors(const std::string& instance_version, const std::string& id);

	/**
	 * @see forum_user_handler::db_insert_addon_authors().
	 */
	void insert_addon_author(
		const std::string& instance_version, const std::string& id, const std::string& author, int is_primary);

	/**
	 * @see forum_user_handler::do_any_authors_exist().
	 */
	bool do_any_authors_exist(const std::string& instance_version, const std::string& id);

	/**
	 * @see forum_user_handler::get_addon_downloads_info().
	 */
	config get_addon_downloads_info(const std::string& instance_version, const std::string& id);

	/**
	 * @see forum_user_handler::get_forum_auth_usage().
	 */
	config get_forum_auth_usage(const std::string& instance_version);

	/**
	 * @see forum_user_handler::get_addon_admins().
	 */
	config get_addon_admins(int site_admin_group, int forum_admin_group);

private:
	/**
	 * The account used to connect to the database.
	 * Also contains the connection settings.
	 * @note settings put on the connection, rather than the account, are NOT kept if a reconnect occurs!
	 */
	mariadb::account_ref account_;
	/** The actual connection to the database. */
	mariadb::connection_ref connection_;

	/** The name of the table that contains forum user information. */
	std::string db_users_table_;
	/** The name of the table that contains forum ban information. */
	std::string db_banlist_table_;
	/** The name of the table that contains additional user information. */
	std::string db_extra_table_;
	/** The name of the table that contains game-level information. */
	std::string db_game_info_table_;
	/** The name of the table that contains player-level information per game. */
	std::string db_game_player_info_table_;
	/** The name of the table that contains game content information. */
	std::string db_game_content_info_table_;
	/** The name of the table that contains forum group information. */
	std::string db_user_group_table_;
	/** The text of the SQL query to use to retrieve any currently active tournaments. */
	std::string db_tournament_query_;
	/** The name of the table that contains phpbb forum thread information */
	std::string db_topics_table_;
	/** The name of the table that contains add-on information. */
	std::string db_addon_info_table_;
	/** The name of the table that contains user connection history. */
	std::string db_connection_history_table_;
	/** The name of the table that contains the add-on authors information */
	std::string db_addon_authors_table_;

	/**
	 * This is used to write out error text when an SQL-related exception occurs.
	 *
	 * @param text Some custom text to log.
	 * @param e The exception that occurred which has information about what went wrong.
	 */
	void log_sql_exception(const std::string& text, const mariadb::exception::base& e);

	/**
	 * Creates a new connection object from the account.
	 */
	mariadb::connection_ref create_connection();

	/**
	 * @param connection The database connection that will be used to execute the query.
	 * @param handler The lambda that will handle reading the results into a config.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 */
	template <typename F>
	config get_complex_results(
		const mariadb::connection_ref& connection, F* handler, const std::string& sql, const sql_parameters& params);

	/**
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return The single string value queried.
	 * @throws mariadb::exception::base when the query finds no value to be retrieved.
	 */
	std::string get_single_string(
		const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return The single long value queried.
	 * @throws mariadb::exception::base when the query finds no value to be retrieved.
	 */
	long get_single_long(const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return True if any data was returned by the query, otherwise false.
	 */
	bool exists(const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * Executes a select statement.
	 *
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return A result set containing the results of the select statement executed.
	 */
	mariadb::result_set_ref select(
		const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * Executes non-select statements (ie: insert, update, delete).
	 *
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return The number of rows modified.
	 */
	unsigned long long modify(const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * Executes non-select statements (ie: insert, update, delete), but primarily intended for inserts that return a
	 * generated ID.
	 *
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return The value of an AUTO_INCREMENT column on the table being modified.
	 */
	unsigned long long modify_get_id(
		const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);

	/**
	 * For a given connection, set the provided SQL and parameters on a statement.
	 *
	 * @param connection The database connection that will be used to execute the query.
	 * @param sql The SQL text to be executed.
	 * @param params The parameterized values to be inserted into the query.
	 * @return A statement ready to be executed.
	 */
	mariadb::statement_ref query(
		const mariadb::connection_ref& connection, const std::string& sql, const sql_parameters& params);
};
