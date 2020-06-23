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

#ifdef HAVE_MYSQLPP

#include "dbconn.hpp"
#include "resultsets/tournaments.hpp"
#include "resultsets/ban_check.hpp"
#include "log.hpp"

static lg::log_domain log_sql_handler("sql_executor");
#define ERR_SQL LOG_STREAM(err, log_sql_handler)
#define WRN_SQL LOG_STREAM(warn, log_sql_handler)
#define LOG_SQL LOG_STREAM(info, log_sql_handler)
#define DBG_SQL LOG_STREAM(debug, log_sql_handler)

dbconn::dbconn(const config& c)
	: db_users_table_(c["db_users_table"].str())
	, db_banlist_table_(c["db_banlist_table"].str())
	, db_extra_table_(c["db_extra_table"].str())
	, db_game_info_table_(c["db_game_info_table"].str())
	, db_game_player_info_table_(c["db_game_player_info_table"].str())
	, db_game_modification_info_table_(c["db_game_modification_info_table"].str())
	, db_user_group_table_(c["db_user_group_table"].str())
	, db_tournament_query_(c["db_tournament_query"].str())
{
	try
	{
		// NOTE: settings put on the connection, rather than the account, are NOT kept if a reconnect occurs!
		account_ = mariadb::account::create(c["db_host"].str(), c["db_user"].str(), c["db_password"].str());
		account_->set_connect_option(mysql_option::MYSQL_SET_CHARSET_NAME, std::string("utf8mb4"));
		account_->set_schema(c["db_name"].str());
		connection_ = mariadb::connection::create(account_);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to connect to the database!", e);
	}
}

void dbconn::log_sql_exception(const std::string& text, const mariadb::exception::base& e)
{
	ERR_SQL << text << '\n'
			<< "what: " << e.what() << '\n'
			<< "error id: " << e.error_id() << std::endl;
}

//
// queries
//
std::string dbconn::get_uuid()
{
	try
	{
		return get_single_string("SELECT UUID()");
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Could not retrieve a UUID!", e);
		return "";
	}
}

std::string dbconn::get_tournaments()
{
	if(db_tournament_query_ == "")
	{
		return "";
	}

	try
	{
		tournaments t;
		get_complex_results(t, db_tournament_query_);
		return t.str();
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Could not retrieve the tournaments!", e);
		return "";
	}
}

bool dbconn::user_exists(const std::string& name)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_users_table_+"` WHERE UPPER(username)=UPPER(?)", name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check if user row for '"+name+"' exists!", e);
		return false;
	}
}

bool dbconn::extra_row_exists(const std::string& name)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_extra_table_+"` WHERE UPPER(username)=UPPER(?)", name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check if extra row for '"+name+"' exists!", e);
		return false;
	}
}

bool dbconn::is_user_in_group(const std::string& name, int group_id)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_users_table_+"` u, `"+db_user_group_table_+"` ug WHERE UPPER(u.username)=UPPER(?) AND u.USER_ID = ug.USER_ID AND ug.GROUP_ID = ?", name, group_id);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check if the user '"+name+"' is in group '"+std::to_string(group_id)+"'!", e);
		return false;
	}
}

ban_check dbconn::get_ban_info(const std::string& name, const std::string& ip)
{
	try
	{
		// selected ban_type value must be part of user_handler::BAN_TYPE
		ban_check b;
		get_complex_results(b, "select ban_userid, ban_email, case when ban_ip != '' then 1 when ban_userid != 0 then 2 when ban_email != '' then 3 end as ban_type, ban_end from `"+db_banlist_table_+"` where (ban_ip = ? or ban_userid = (select user_id from `"+db_users_table_+"` where UPPER(username) = UPPER(?)) or UPPER(ban_email) = (select UPPER(user_email) from `"+db_users_table_+"` where UPPER(username) = UPPER(?))) AND ban_exclude = 0 AND (ban_end = 0 OR ban_end >= ?)", ip, name, name, std::time(nullptr));
		return b;
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to check ban info for user '"+name+"' connecting from ip '"+ip+"'!", e);
		return ban_check();
	}
}

std::string dbconn::get_user_string(const std::string& table, const std::string& column, const std::string& name)
{
	try
	{
		return get_single_string("SELECT `"+column+"` from `"+table+"` WHERE UPPER(username)=UPPER(?)", name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to query column `"+column+"` from table `"+table+"` for user `"+name+"`", e);
		return "";
	}
}
int dbconn::get_user_int(const std::string& table, const std::string& column, const std::string& name)
{
	try
	{
		return get_single_int("SELECT `"+column+"` from `"+table+"` WHERE UPPER(username)=UPPER(?)", name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to query column `"+column+"` from table `"+table+"` for user `"+name+"`", e);
		return 0;
	}
}
void dbconn::write_user_int(const std::string& column, const std::string& name, int value)
{
	try
	{
		if(!extra_row_exists(name))
		{
			modify("INSERT INTO `"+db_extra_table_+"` VALUES(?,?,'0')", name, value);
		}
		modify("UPDATE `"+db_extra_table_+"` SET "+column+"=? WHERE UPPER(username)=UPPER(?)", value, name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to write `"+std::to_string(value)+"` to column `"+column+"` on table `"+db_extra_table_+"` for user `"+name+"`", e);
	}
}

void dbconn::insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password, const std::string& map_source, const std::string& map_version, const std::string& era_source, const std::string& era_version)
{
	try
	{
		modify("INSERT INTO `"+db_game_info_table_+"`(INSTANCE_UUID, GAME_ID, INSTANCE_VERSION, GAME_NAME, MAP_NAME, ERA_NAME, RELOAD, OBSERVERS, PUBLIC, PASSWORD, MAP_SOURCE_ADDON, MAP_VERSION, ERA_SOURCE_ADDON, ERA_VERSION) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
		uuid, game_id, version, name, map_name, era_name, reload, observers, is_public, has_password, map_source, map_version, era_source, era_version);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void dbconn::update_game_end(const std::string& uuid, int game_id, const std::string& replay_location)
{
	try
	{
		modify("UPDATE `"+db_game_info_table_+"` SET END_TIME = CURRENT_TIMESTAMP, REPLAY_NAME = ? WHERE INSTANCE_UUID = ? AND GAME_ID = ?",
		replay_location, uuid, game_id);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to update the game end for game info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void dbconn::insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user)
{
	try
	{
		modify("INSERT INTO `"+db_game_player_info_table_+"`(INSTANCE_UUID, GAME_ID, USER_ID, SIDE_NUMBER, IS_HOST, FACTION, CLIENT_VERSION, CLIENT_SOURCE, USER_NAME) VALUES(?, ?, IFNULL((SELECT user_id FROM `"+db_users_table_+"` WHERE username = ?), -1), ?, ?, ?, ?, ?, ?)",
		uuid, game_id, username, side_number, is_host, faction, version, source, current_user);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game player info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void dbconn::insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name, const std::string& modification_source, const std::string& modification_version)
{
	try
	{
		modify("INSERT INTO `"+db_game_modification_info_table_+"`(INSTANCE_UUID, GAME_ID, MODIFICATION_NAME, SOURCE_ADDON, VERSION) VALUES(?, ?, ?, ?, ?)",
		uuid, game_id, modification_name, modification_source, modification_version);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game modification info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void dbconn::set_oos_flag(const std::string& uuid, int game_id)
{
	try
	{
		modify("UPDATE `"+db_game_info_table_+"` SET OOS = 1 WHERE INSTANCE_UUID = ? AND GAME_ID = ?",
		uuid, game_id);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to set the OOS flag for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}

//
// queries can return data with various types that can't be easily fit into a pre-determined structure
// therefore for queries that can return multiple rows of multiple columns, implement a class to define how the results should be read
//
template<typename... Args>
void dbconn::get_complex_results(rs_base& base, const std::string& sql, Args&&... args)
{
	mariadb::result_set_ref rslt = select(sql, args...);
	base.read(rslt);
}
//
// get single values
//
template<typename... Args>
std::string dbconn::get_single_string(const std::string& sql, Args&&... args)
{
	mariadb::result_set_ref rslt = select(sql, args...);
	if(rslt->next())
	{
		return rslt->get_string(0);
	}
	else
	{
		throw mariadb::exception::base("No string value found in the database!");
	}
}
template<typename... Args>
int dbconn::get_single_int(const std::string& sql, Args&&... args)
{
	mariadb::result_set_ref rslt = select(sql, args...);
	if(rslt->next())
	{
		// mariadbpp checks for strict integral equivalence, but we don't care
		// so check the type beforehand, call the associated getter, and let it silently get upcast to an int if needed
		switch(rslt->column_type(0))
		{
			case mariadb::value::type::unsigned8:
			case mariadb::value::type::signed8:
				return rslt->get_signed8(0);
			case mariadb::value::type::unsigned16:
			case mariadb::value::type::signed16:
				return rslt->get_signed16(0);
			case mariadb::value::type::unsigned32:
			case mariadb::value::type::signed32:
				return rslt->get_signed32(0);
			default:
				throw mariadb::exception::base("Value retrieved was not an int!");
		}
	}
	else
	{
		throw mariadb::exception::base("No int value found in the database!");
	}
}
template<typename... Args>
bool dbconn::exists(const std::string& sql, Args&&... args)
{
	mariadb::result_set_ref rslt = select(sql, args...);
	return rslt->next();
}

//
// select or modify values
//
template<typename... Args>
mariadb::result_set_ref dbconn::select(const std::string& sql, Args&&... args)
{
	try
	{
		mariadb::statement_ref stmt = query(sql, args...);
		return mariadb::result_set_ref(stmt->query());
	}
	catch(const mariadb::exception::base& e)
	{
		if(!connection_->connected())
		{
			ERR_SQL << "Connection is invalid!" << std::endl;
		}
		ERR_SQL << "SQL query failed for query: `"+sql+"`!" << std::endl;
		throw e;
	}
}
template<typename... Args>
int dbconn::modify(const std::string& sql, Args&&... args)
{
	try
	{
		mariadb::statement_ref stmt = query(sql, args...);
		return stmt->insert();
	}
	catch(const mariadb::exception::base& e)
	{
		if(!connection_->connected())
		{
			ERR_SQL << "Connection is invalid!" << std::endl;
		}
		ERR_SQL << "SQL query failed for query: `"+sql+"`!" << std::endl;
		throw e;
	}
}

//
// start of recursive unpacking of variadic template in order to be able to call correct parameterized setters on query
//
template<typename... Args>
mariadb::statement_ref dbconn::query(const std::string& sql, Args&&... args)
{
	mariadb::statement_ref stmt = connection_->create_statement(sql);
	prepare(stmt, 0, args...);
	return stmt;
}
// split off the next parameter
template<typename Arg, typename... Args>
void dbconn::prepare(mariadb::statement_ref stmt, int i, Arg arg, Args&&... args)
{
	i = prepare(stmt, i, arg);
	prepare(stmt, i, args...);
}
// template specialization for supported parameter types
// there are other parameter setters, but so far there hasn't been a reason to add them
template<>
int dbconn::prepare(mariadb::statement_ref stmt, int i, int arg)
{
	stmt->set_signed32(i++, arg);
	return i;
}
template<>
int dbconn::prepare(mariadb::statement_ref stmt, int i, long arg)
{
	stmt->set_signed64(i++, arg);
	return i;
}
template<>
int dbconn::prepare(mariadb::statement_ref stmt, int i, const char* arg)
{
	stmt->set_string(i++, arg);
	return i;
}
template<>
int dbconn::prepare(mariadb::statement_ref stmt, int i, std::string arg)
{
	stmt->set_string(i++, arg);
	return i;
}
// no more parameters, nothing left to do
void dbconn::prepare(mariadb::statement_ref, int){}

#endif //HAVE_MYSQLPP
