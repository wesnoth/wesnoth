/*
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
#include "resultsets/rs_base.hpp"
#include "resultsets/ban_check.hpp"

#include <mysql/mysql.h>
#include <mariadb++/account.hpp>
#include <mariadb++/connection.hpp>
#include <mariadb++/statement.hpp>
#include <mariadb++/result_set.hpp>
#include <mariadb++/exceptions.hpp>

#include <vector>
#include <unordered_map>

class dbconn
{
	public:
		dbconn(const config& c);
		std::string get_uuid();
		std::string get_tournaments();
		bool user_exists(const std::string& name);
		bool extra_row_exists(const std::string& name);
		bool is_user_in_group(const std::string& name, int group_id);
		std::string get_user_string(const std::string& table, const std::string& column, const std::string& name);
		int get_user_int(const std::string& table, const std::string& column, const std::string& name);
		void write_user_int(const std::string& column, const std::string& name, int value);
		ban_check get_ban_info(const std::string& name, const std::string& ip);
		void insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password, const std::string& map_source, const std::string& map_version, const std::string& era_source, const std::string& era_version);
		void update_game_end(const std::string& uuid, int game_id, const std::string& replay_location);
		void insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user);
		void insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name, const std::string& modification_source, const std::string& modification_version);
		void set_oos_flag(const std::string& uuid, int game_id);
	
	private:
		mariadb::account_ref account_;
		mariadb::connection_ref connection_;

		std::string db_users_table_;
		std::string db_banlist_table_;
		std::string db_extra_table_;
		std::string db_game_info_table_;
		std::string db_game_player_info_table_;
		std::string db_game_modification_info_table_;
		std::string db_user_group_table_;
		std::string db_tournament_query_;

		void log_sql_exception(const std::string& text, const mariadb::exception::base& e);

		template<typename... Args>
		void get_complex_results(rs_base& base, const std::string& sql, Args&&... args);

		template<typename... Args>
		std::string get_single_string(const std::string& sql, Args&&... args);

		template<typename... Args>
		int get_single_int(const std::string& sql, Args&&... args);

		template<typename... Args>
		bool exists(const std::string& sql, Args&&... args);

		template<typename... Args>
		mariadb::result_set_ref select(const std::string& sql, Args&&... args);

		template<typename... Args>
		int modify(const std::string& sql, Args&&... args);

		template<typename... Args>
		mariadb::statement_ref query(const std::string& sql, Args&&... args);

		template<typename Arg, typename... Args>
		void prepare(mariadb::statement_ref stmt, int i, Arg arg, Args&&... args);

		template<typename Arg>
		int prepare(mariadb::statement_ref stmt, int i, Arg arg);

		void prepare(mariadb::statement_ref stmt, int i);
};
