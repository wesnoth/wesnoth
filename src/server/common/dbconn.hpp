/*
	Copyright (C) 2020 - 2022
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
#include "server/common/resultsets/rs_base.hpp"
#include "server/common/resultsets/ban_check.hpp"
#include "server/common/simple_wml.hpp"

#include <mysql/mysql.h>
#include "mariadb++/account.hpp"
#include "mariadb++/connection.hpp"
#include "mariadb++/statement.hpp"
#include "mariadb++/result_set.hpp"
#include "mariadb++/exceptions.hpp"

#include <vector>
#include <unordered_map>

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
		 * This is an asynchronous query that is executed on a separate connection to retrieve the game history for the provided player.
		 *
		 * @param player_id The forum ID of the player to get the game history for.
		 * @param offset The offset to provide to the database for where to start returning rows from.
		 * @return The simple_wml document containing the query results, or simply the @a error attribute if an exception is thrown.
		 */
		std::unique_ptr<simple_wml::document> get_game_history(int player_id, int offset);

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
		 * @see forum_user_handler::is_user_in_group().
		 */
		bool is_user_in_group(const std::string& name, int group_id);

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
		ban_check get_ban_info(const std::string& name, const std::string& ip);

		/**
		 * @see forum_user_handler::db_insert_game_info().
		 */
		void insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, int reload, int observers, int is_public, int has_password);

		/**
		 * @see forum_user_handler::db_update_game_end().
		 */
		void update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);

		/**
		 * @see forum_user_handler::db_insert_game_player_info().
		 */
		void insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user);

		/**
		 * @see forum_user_handler::db_insert_game_content_info().
		 */
		unsigned long long insert_game_content_info(const std::string& uuid, int game_id, const std::string& type, const std::string& name, const std::string& id, const std::string& source, const std::string& version);

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
		void insert_addon_info(const std::string& instance_version, const std::string& id, const std::string& name, const std::string& type, const std::string& version, bool forum_auth, int topic_id);

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
		void update_addon_download_count(const std::string& instance_version, const std::string& id, const std::string& version);

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
		 * Queries can return data with various types that can't be easily fit into a pre-determined structure.
		 * Therefore for queries that can return multiple rows with multiple columns, a class that extends @ref rs_base handles reading the results.
		 *
		 * @param connection The database connecion that will be used to execute the query.
		 * @param base The class that will handle reading the results.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 */
		template<typename... Args>
		void get_complex_results(mariadb::connection_ref connection, rs_base& base, const std::string& sql, Args&&... args);

		/**
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return The single string value queried.
		 * @throws mariadb::exception::base when the query finds no value to be retrieved.
		 */
		template<typename... Args>
		std::string get_single_string(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return The single long value queried.
		 * @throws mariadb::exception::base when the query finds no value to be retrieved.
		 */
		template<typename... Args>
		long get_single_long(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return True if any data was returned by the query, otherwise false.
		 */
		template<typename... Args>
		bool exists(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * Executes a select statement.
		 *
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return A result set containing the results of the select statement executed.
		 */
		template<typename... Args>
		mariadb::result_set_ref select(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * Executes non-select statements (ie: insert, update, delete).
		 *
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return The number of rows modified.
		 */
		template<typename... Args>
		unsigned long long modify(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * Executes non-select statements (ie: insert, update, delete), but primarily intended for inserts that return a generated ID.
		 *
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return The value of an AUTO_INCREMENT column on the table being modified.
		 */
		template<typename... Args>
		unsigned long long modify_get_id(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * Begins recursively unpacking of the parameter pack in order to be able to call the correct parameterized setters on the query.
		 *
		 * @param connection The database connecion that will be used to execute the query.
		 * @param sql The SQL text to be executed.
		 * @param args The parameterized values to be inserted into the query.
		 * @return A statement object with all parameterized values added. This will then be executed by either @ref modify() or @ref select().
		 */
		template<typename... Args>
		mariadb::statement_ref query(mariadb::connection_ref connection, const std::string& sql, Args&&... args);

		/**
		 * The next parameter to be added is split off from the parameter pack.
		 *
		 * @param stmt The statement that will have parameterized values set on it.
		 * @param i The index of the current parameterized value.
		 * @param arg The next parameter to be added.
		 * @param args The remaining parameters to be added.
		 */
		template<typename Arg, typename... Args>
		void prepare(mariadb::statement_ref stmt, int i, Arg arg, Args&&... args);

		/**
		 * Specializations for each type of value to be parameterized.
		 * There are other parameter setters than those currently implemented, but so far there hasn't been a reason to add them.
		 *
		 * @param stmt The statement that will have parameterized values set on it.
		 * @param i The index of the current parameterized value.
		 * @param arg The next parameter to be added.
		 * @return The index of the next parameter to add.
		 */
		template<typename Arg>
		int prepare(mariadb::statement_ref stmt, int i, Arg arg);

		/**
		 * Nothing left to parameterize, so break out of the recursion.
		 */
		void prepare(mariadb::statement_ref stmt, int i);
};
