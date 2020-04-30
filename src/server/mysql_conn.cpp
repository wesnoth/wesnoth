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

#include "mysql_conn.hpp"

#include "log.hpp"

static lg::log_domain log_sql_handler("sql_executor");
#define ERR_SQL LOG_STREAM(err, log_sql_handler)
#define WRN_SQL LOG_STREAM(warn, log_sql_handler)
#define LOG_SQL LOG_STREAM(info, log_sql_handler)
#define DBG_SQL LOG_STREAM(debug, log_sql_handler)

mysql_conn::mysql_conn(const config& c)
	: db_users_table_(c["db_users_table"].str())
	, db_banlist_table_(c["db_banlist_table"].str())
	, db_extra_table_(c["db_extra_table"].str())
	, db_game_info_table_(c["db_game_info_table"].str())
	, db_game_player_info_table_(c["db_game_player_info_table"].str())
	, db_game_modification_info_table_(c["db_game_modification_info_table"].str())
	, db_user_group_table_(c["db_user_group_table"].str())
	, db_tournament_query_(c["db_tournament_query"].str())
	, mp_mod_group_(0)
{
	try
	{
		// NOTE: settings put on the connection are NOT kept if a reconnect occurs!
		account_ = mariadb::account::create(c["db_host"].str(), c["db_user"].str(), c["db_password"].str());
		account_->set_connect_option(mysql_option::MYSQL_SET_CHARSET_NAME, std::string("utf8mb4"));
		account_->set_schema(c["db_name"].str());
		connection_ = mariadb::connection::create(account_);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to connect to the database!", e);
	}

	try
	{
		mp_mod_group_ = std::stoi(c["mp_mod_group"].str());
	}
	catch(...)
	{
		ERR_SQL << "Failed to convert the mp_mod_group value of '" << c["mp_mod_group"].str() << "' into an int!  Defaulting to " << mp_mod_group_ << "." << std::endl;
	}
}

void mysql_conn::log_sql_exception(const std::string& text, const mariadb::exception::base& e)
{
	ERR_SQL << text << '\n'
			<< "what: " << e.what() << '\n'
			<< "error id: " << e.error_id() << std::endl;
}

//
// queries
//
std::string mysql_conn::get_uuid()
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

std::string mysql_conn::get_tournaments()
{
	if(db_tournament_query_ == "")
	{
		return "";
	}

	try
	{
		std::string text;
		for(const auto& row : get_string_data(db_tournament_query_))
		{
			text += "\nThe tournament "+row.at("TITLE")+" is "+row.at("STATUS")+". More information can be found at "+row.at("URL");
		}
		return text;
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Could not retrieve the tournaments!", e);
		return "";
	}
}

bool mysql_conn::user_exists(const std::string& name)
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

bool mysql_conn::extra_row_exists(const std::string& name)
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

bool mysql_conn::is_user_in_group(const std::string& name, int group_id)
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

// NOTE: A ban end time of 0 is a permanent ban.
bool mysql_conn::ip_is_banned(const std::string& ip)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_banlist_table_+"` WHERE UPPER(ban_ip) = UPPER(?) AND ban_exclude = 0 AND (ban_end = 0 OR ban_end >="+std::to_string(std::time(nullptr)) + ")", ip);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to check IP ban for address `"+ip+"`!", e);
		return false;
	}
}
bool mysql_conn::user_is_banned(int userid)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_banlist_table_+"` WHERE ban_userid = ? AND ban_exclude = 0 AND (ban_end = 0 OR ban_end >="+std::to_string(std::time(nullptr)) + ")", userid);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to check user ID ban for ID `"+std::to_string(userid)+"`!", e);
		return false;
	}
}
bool mysql_conn::email_is_banned(const std::string& email)
{
	try
	{
		return exists("SELECT 1 FROM `"+db_banlist_table_+"` WHERE UPPER(ban_email) = UPPER(?) AND ban_exclude = 0 AND (ban_end = 0 OR ban_end >=" + std::to_string(std::time(nullptr)) + ")", email);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to check email address ban for address `"+email+"`!", e);
		return false;
	}
}

//
// NOTE:
// If retrieving the duration fails to fetch the ban row, odds are the ban was
// lifted in the meantime (it's meant to be called by user_is_banned(), so we
// assume the ban expires in one second instead of returning 0 (permanent ban)
// just to err on the safe side (returning BAN_NONE would be a terrible idea,
// for that matter).
//
int mysql_conn::ban_duration_by_int_column(const std::string& column, int value)
{
	try
	{
		return get_single_int("SELECT `ban_end` FROM `"+db_banlist_table_+"` WHERE "+column+" = ?", value);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to retrieve ban duration using column `"+column+"` and value `"+std::to_string(value)+"`!", e);
		return 1;
	}
}
int mysql_conn::ban_duration_by_string_column(const std::string& column, const std::string& value)
{
	try
	{
		return get_single_int("SELECT `ban_end` FROM `"+db_banlist_table_+"` WHERE UPPER("+column+") = UPPER(?)", value);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to retrieve ban duration using column `"+column+"` and value `"+value+"`!", e);
		return 1;
	}
}

std::string mysql_conn::get_user_string(const std::string& table, const std::string& column, const std::string& name)
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
int mysql_conn::get_user_int(const std::string& table, const std::string& column, const std::string& name)
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
void mysql_conn::write_user_int(const std::string& column, const std::string& name, int value)
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

void mysql_conn::insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, const std::string& map_name, const std::string& era_name, int reload, int observers, int is_public, int has_password)
{
	try
	{
		modify("INSERT INTO `"+db_game_info_table_+"`(INSTANCE_UUID, GAME_ID, INSTANCE_VERSION, GAME_NAME, MAP_NAME, ERA_NAME, RELOAD, OBSERVERS, PUBLIC, PASSWORD) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
		uuid, game_id, version, name, map_name, era_name, reload, observers, is_public, has_password);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void mysql_conn::update_game_end(const std::string& uuid, int game_id, const std::string& replay_location)
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
void mysql_conn::insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user)
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
void mysql_conn::insert_modification_info(const std::string& uuid, int game_id, const std::string& modification_name)
{
	try
	{
		modify("INSERT INTO `"+db_game_modification_info_table_+"`(INSTANCE_UUID, GAME_ID, MODIFICATION_NAME) VALUES(?, ?, ?)",
		uuid, game_id, modification_name);
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game modification info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void mysql_conn::set_oos_flag(const std::string& uuid, int game_id)
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
// get multiple rows of multiple string values
//
template<typename... Args>
std::vector<std::unordered_map<std::string, std::string>> mysql_conn::get_string_data(const std::string& sql, Args&&... args)
{
	std::vector<std::unordered_map<std::string, std::string>> data;
	mariadb::result_set_ref rslt = select(sql, args...);

	while(rslt->next())
	{
		std::unordered_map<std::string, std::string> row;
		for(unsigned int i = 0; i < rslt->column_count(); i++)
		{
			row.emplace(rslt->column_name(i), rslt->get_string(i));
		}
		data.push_back(row);
	}

	return data;
}
//
// get single values
//
template<typename... Args>
std::string mysql_conn::get_single_string(const std::string& sql, Args&&... args)
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
int mysql_conn::get_single_int(const std::string& sql, Args&&... args)
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
bool mysql_conn::exists(const std::string& sql, Args&&... args)
{
	mariadb::result_set_ref rslt = select(sql, args...);
	return rslt->next();
}

//
// select or modify values
//
template<typename... Args>
mariadb::result_set_ref mysql_conn::select(const std::string& sql, Args&&... args)
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
int mysql_conn::modify(const std::string& sql, Args&&... args)
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
mariadb::statement_ref mysql_conn::query(const std::string& sql, Args&&... args)
{
	mariadb::statement_ref stmt = connection_->create_statement(sql);
	prepare(stmt, 0, args...);
	return stmt;
}
// split off the next parameter
template<typename Arg, typename... Args>
void mysql_conn::prepare(mariadb::statement_ref stmt, int i, Arg arg, Args&&... args)
{
	i = prepare(stmt, i, arg);
	prepare(stmt, i, args...);
}
// template specialization for supported parameter types
// there are other parameter setters, but so far there hasn't been a reason to add them
template<>
int mysql_conn::prepare(mariadb::statement_ref stmt, int i, int arg)
{
	stmt->set_signed32(i++, arg);
	return i;
}
template<>
int mysql_conn::prepare(mariadb::statement_ref stmt, int i, const char* arg)
{
	stmt->set_string(i++, arg);
	return i;
}
template<>
int mysql_conn::prepare(mariadb::statement_ref stmt, int i, std::string arg)
{
	stmt->set_string(i++, arg);
	return i;
}
// no more parameters, nothing left to do
void mysql_conn::prepare(mariadb::statement_ref, int){}

#endif //HAVE_MYSQLPP
