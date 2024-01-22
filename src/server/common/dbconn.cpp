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

#ifdef HAVE_MYSQLPP

#include "server/common/dbconn.hpp"
#include "server/common/resultsets/tournaments.hpp"
#include "server/common/resultsets/ban_check.hpp"
#include "server/common/resultsets/game_history.hpp"

#include "log.hpp"
#include "serialization/unicode.hpp"
#include "serialization/string_utils.hpp"

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
	, db_game_content_info_table_(c["db_game_content_info_table"].str())
	, db_user_group_table_(c["db_user_group_table"].str())
	, db_tournament_query_(c["db_tournament_query"].str())
	, db_topics_table_(c["db_topics_table"].str())
	, db_addon_info_table_(c["db_addon_info_table"].str())
	, db_connection_history_table_(c["db_connection_history_table"].str())
	, db_addon_authors_table_(c["db_addon_authors_table"].str())
{
	try
	{
		account_ = mariadb::account::create(c["db_host"].str(), c["db_user"].str(), c["db_password"].str());
		account_->set_connect_option(mysql_option::MYSQL_SET_CHARSET_NAME, std::string("utf8mb4"));
		account_->set_schema(c["db_name"].str());
		// initialize the connection used to run synchronous queries.
		connection_ = create_connection();
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
			<< "error id: " << e.error_id();
}

mariadb::connection_ref dbconn::create_connection()
{
	return mariadb::connection::create(account_);
}

//
// queries
//
int dbconn::async_test_query(int limit)
{
	std::string sql = "with recursive TEST(T) as "
	                  "( "
					  "select 1 "
					  "union all "
					  "select T+1 from TEST where T < ? "
					  ") "
					  "select count(*) from TEST";
	int t = get_single_long(create_connection(), sql, { limit });
	return t;
}

std::string dbconn::get_uuid()
{
	try
	{
		return get_single_string(connection_, "SELECT UUID()", {});
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
		get_complex_results(connection_, t, db_tournament_query_, {});
		return t.str();
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Could not retrieve the tournaments!", e);
		return "";
	}
}

std::unique_ptr<simple_wml::document> dbconn::get_game_history(int player_id, int offset, std::string search_game_name, int search_content_type, std::string search_content)
{
	try
	{
		// if no parameters populated, return an error
		if(player_id == 0 && search_game_name.empty() && search_content.empty())
		{
			ERR_SQL << "Skipping game history query due to lack of search parameters.";
			auto doc = std::make_unique<simple_wml::document>();
			doc->set_attr("error", "No search parameters provided.");
			return doc;
		}

		sql_parameters params;

		std::string game_history_query = "select "
"  game.GAME_NAME, "
"  game.START_TIME, "
"  GROUP_CONCAT(CONCAT(player.USER_NAME, ':', player.FACTION)) as PLAYERS, "
"  IFNULL(scenario.NAME, '') as SCENARIO_NAME, "
"  IFNULL(era.NAME, '') as ERA_NAME, "
"  IFNULL(GROUP_CONCAT(distinct mods.NAME), '') as MODIFICATION_NAMES, "
"  case "
"  when game.PUBLIC = 1 and game.INSTANCE_VERSION != 'trunk' "
"  then concat('https://replays.wesnoth.org/', substring(game.INSTANCE_VERSION, 1, 4), '/', year(game.END_TIME), '/', lpad(month(game.END_TIME), 2, '0'), '/', lpad(day(game.END_TIME), 2, '0'), '/', game.REPLAY_NAME) "
"  when game.PUBLIC = 1 and game.INSTANCE_VERSION = 'trunk' "
"  then concat('https://replays.wesnoth.org/', game.INSTANCE_VERSION, '/', year(game.END_TIME), '/', lpad(month(game.END_TIME), 2, '0'), '/', lpad(day(game.END_TIME), 2, '0'), '/', game.REPLAY_NAME) "
"  else '' "
"  end as REPLAY_URL, "
"  case "
"  when game.INSTANCE_VERSION != 'trunk' "
"  then SUBSTRING(game.INSTANCE_VERSION, 1, 4) "
"  else 'trunk' "
"  end as VERSION "
"from "+db_game_player_info_table_+" player , "+db_game_content_info_table_+" scenario , "+db_game_content_info_table_+" era , "+db_game_info_table_+" game ";
	// using a left join when searching by modification won't exclude anything
	game_history_query += search_content_type == 2 && !search_content.empty() ? "inner join " : "left join ";
	game_history_query += db_game_content_info_table_+" mods "
"   on mods.TYPE = 'modification' "
"  and mods.INSTANCE_UUID = game.INSTANCE_UUID "
"  and mods.GAME_ID = game.GAME_ID ";
	// modification id optional parameter
	if(search_content_type == 2)
	{
		if(!search_content.empty())
		{
			game_history_query += "and mods.ID like ? ";

			utils::to_sql_wildcards(search_content, false);
			params.emplace_back(search_content);
		}
	}

	game_history_query += "where exists "
"  ( "
"    select 1 "
"    from "+db_game_player_info_table_+" player1 "
"    where game.INSTANCE_UUID = player1.INSTANCE_UUID "
"      and game.GAME_ID = player1.GAME_ID ";

	if(player_id != 0)
	{
		game_history_query += " and player1.USER_ID = ? ";
		params.emplace_back(player_id);
	}

	game_history_query += "  ) "
"  and game.INSTANCE_UUID = player.INSTANCE_UUID "
"  and game.GAME_ID = player.GAME_ID "
"  and player.USER_ID != -1 "
"  and game.END_TIME is not NULL "
"  and scenario.TYPE = 'scenario' "
"  and scenario.INSTANCE_UUID = game.INSTANCE_UUID "
"  and scenario.GAME_ID = game.GAME_ID "
"  and era.TYPE = 'era' "
"  and era.INSTANCE_UUID = game.INSTANCE_UUID "
"  and era.GAME_ID = game.GAME_ID ";
	// game name optional paramenter
	if(!search_game_name.empty())
	{
		game_history_query += "and game.GAME_NAME like ? ";

		utils::to_sql_wildcards(search_game_name, false);
		params.emplace_back(search_game_name);
	}

	// scenario id optional parameter
	if(search_content_type == 0)
	{
		if(!search_content.empty())
		{
			game_history_query += "and scenario.ID like ? ";

			utils::to_sql_wildcards(search_content, false);
			params.emplace_back(search_content);
		}
	}

	// era id optional parameter
	if(search_content_type == 1)
	{
		if(!search_content.empty())
		{
			game_history_query += "and era.ID like ? ";

			utils::to_sql_wildcards(search_content, false);
			params.emplace_back(search_content);
		}
	}

	game_history_query += "group by game.INSTANCE_UUID, game.GAME_ID "
"order by game.START_TIME desc "
"limit 11 offset ? ";
		params.emplace_back(offset);

		DBG_SQL << "before getting connection for game history query for player " << player_id;

		mariadb::connection_ref connection = create_connection();

		DBG_SQL << "game history query text for player " << player_id << ": " << game_history_query;

		game_history gh;
		get_complex_results(connection, gh, game_history_query, params);

		DBG_SQL << "after game history query for player " << player_id;

		auto doc = gh.to_doc();

		DBG_SQL << "after parsing results of game history query for player " << player_id;

		return doc;
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Could not retrieve the game history for forum ID `"+std::to_string(player_id)+"`!", e);
		auto doc = std::make_unique<simple_wml::document>();
		doc->set_attr("error", "Error retrieving game history.");
		return doc;
	}
}

bool dbconn::user_exists(const std::string& name)
{
	try
	{
		return exists(connection_, "SELECT 1 FROM `"+db_users_table_+"` WHERE UPPER(username)=UPPER(?)", { name });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check if user row for '"+name+"' exists!", e);
		return false;
	}
}

long dbconn::get_forum_id(const std::string& name)
{
	try
	{
		return get_single_long(connection_, "SELECT IFNULL((SELECT user_id FROM `"+db_users_table_+"` WHERE UPPER(username)=UPPER(?)), 0)", { name });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to get user_id for '"+name+"'!", e);
		return 0;
	}
}

bool dbconn::extra_row_exists(const std::string& name)
{
	try
	{
		return exists(connection_, "SELECT 1 FROM `"+db_extra_table_+"` WHERE UPPER(username)=UPPER(?)", { name });
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
		return exists(connection_, "SELECT 1 FROM `"+db_users_table_+"` u, `"+db_user_group_table_+"` ug WHERE UPPER(u.username)=UPPER(?) AND u.USER_ID = ug.USER_ID AND ug.GROUP_ID = ?",
		{ name, group_id });
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
		get_complex_results(connection_, b, "select ban_userid, ban_email, case when ban_ip != '' then 1 when ban_userid != 0 then 2 when ban_email != '' then 3 end as ban_type, ban_end from `"+db_banlist_table_+"` where (ban_ip = ? or ban_userid = (select user_id from `"+db_users_table_+"` where UPPER(username) = UPPER(?)) or UPPER(ban_email) = (select UPPER(user_email) from `"+db_users_table_+"` where UPPER(username) = UPPER(?))) AND ban_exclude = 0 AND (ban_end = 0 OR ban_end >= ?)",
			{ ip, name, name, std::time(nullptr) });
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
		return get_single_string(connection_, "SELECT `"+column+"` from `"+table+"` WHERE UPPER(username)=UPPER(?)", { name });
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
		return static_cast<int>(get_single_long(connection_, "SELECT `"+column+"` from `"+table+"` WHERE UPPER(username)=UPPER(?)", { name }));
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
			modify(connection_, "INSERT INTO `"+db_extra_table_+"` VALUES(?,?,'0')", { name, value });
		}
		modify(connection_, "UPDATE `"+db_extra_table_+"` SET "+column+"=? WHERE UPPER(username)=UPPER(?)", { value, name });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to write `"+std::to_string(value)+"` to column `"+column+"` on table `"+db_extra_table_+"` for user `"+name+"`", e);
	}
}

void dbconn::insert_game_info(const std::string& uuid, int game_id, const std::string& version, const std::string& name, int reload, int observers, int is_public, int has_password)
{
	try
	{
		modify(connection_, "INSERT INTO `"+db_game_info_table_+"`(INSTANCE_UUID, GAME_ID, INSTANCE_VERSION, GAME_NAME, RELOAD, OBSERVERS, PUBLIC, PASSWORD) VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
			{ uuid, game_id, version, name, reload, observers, is_public, has_password });
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
		modify(connection_, "UPDATE `"+db_game_info_table_+"` SET END_TIME = CURRENT_TIMESTAMP, REPLAY_NAME = ? WHERE INSTANCE_UUID = ? AND GAME_ID = ?",
			{ replay_location, uuid, game_id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to update the game end for game info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
void dbconn::insert_game_player_info(const std::string& uuid, int game_id, const std::string& username, int side_number, int is_host, const std::string& faction, const std::string& version, const std::string& source, const std::string& current_user, const std::string& leaders)
{
	try
	{
		modify(connection_, "INSERT INTO `"+db_game_player_info_table_+"`(INSTANCE_UUID, GAME_ID, USER_ID, SIDE_NUMBER, IS_HOST, FACTION, CLIENT_VERSION, CLIENT_SOURCE, USER_NAME, LEADERS) VALUES(?, ?, IFNULL((SELECT user_id FROM `"+db_users_table_+"` WHERE username = ?), -1), ?, ?, ?, ?, ?, ?, ?)",
			{ uuid, game_id, username, side_number, is_host, faction, version, source, current_user, leaders });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game player info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}
unsigned long long dbconn::insert_game_content_info(const std::string& uuid, int game_id, const std::string& type, const std::string& name, const std::string& id, const std::string& addon_id, const std::string& addon_version)
{
	try
	{
		return modify(connection_, "INSERT INTO `"+db_game_content_info_table_+"`(INSTANCE_UUID, GAME_ID, TYPE, NAME, ID, ADDON_ID, ADDON_VERSION) VALUES(?, ?, ?, ?, ?, ?, ?)",
			{ uuid, game_id, type, name, id, addon_id, addon_version });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to insert game content info row for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
		return 0;
	}
}
void dbconn::set_oos_flag(const std::string& uuid, int game_id)
{
	try
	{
		modify(connection_, "UPDATE `"+db_game_info_table_+"` SET OOS = 1 WHERE INSTANCE_UUID = ? AND GAME_ID = ?",
			{ uuid, game_id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Failed to set the OOS flag for UUID `"+uuid+"` and game ID `"+std::to_string(game_id)+"`", e);
	}
}

bool dbconn::topic_id_exists(int topic_id) {
	try
	{
		return exists(connection_, "SELECT 1 FROM `"+db_topics_table_+"` WHERE TOPIC_ID = ?",
			{ topic_id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check whether `"+std::to_string(topic_id)+"` exists.", e);
		return true;
	}
}

void dbconn::insert_addon_info(const std::string& instance_version, const std::string& id, const std::string& name, const std::string& type, const std::string& version, bool forum_auth, int topic_id, const std::string uploader)
{
	try
	{
		modify(connection_, "INSERT INTO `"+db_addon_info_table_+"`(INSTANCE_VERSION, ADDON_ID, ADDON_NAME, TYPE, VERSION, FORUM_AUTH, FEEDBACK_TOPIC, UPLOADER) VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
			{ instance_version, id, name, type, version, forum_auth, topic_id, uploader });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to insert add-on info for add-on `"+id+"` for instance `"+instance_version+"`.", e);
	}
}

unsigned long long dbconn::insert_login(const std::string& username, const std::string& ip, const std::string& version)
{
	try
	{
		return modify_get_id(connection_, "INSERT INTO `"+db_connection_history_table_+"`(USER_NAME, IP, VERSION) values(lower(?), ?, ?)",
			{ username, ip, version });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to insert login row user `"+username+"` and ip `"+ip+"`.", e);
		return 0;
	}
}

void dbconn::update_logout(unsigned long long login_id)
{
	try
	{
		modify(connection_, "UPDATE `"+db_connection_history_table_+"` SET LOGOUT_TIME = CURRENT_TIMESTAMP WHERE LOGIN_ID = ?",
			{ login_id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to update login row `"+std::to_string(login_id)+"`.", e);
	}
}

void dbconn::get_users_for_ip(const std::string& ip, std::ostringstream* out)
{
	try
	{
		mariadb::result_set_ref rslt = select(connection_, "SELECT USER_NAME, IP, date_format(LOGIN_TIME, '%Y/%m/%d %h:%i:%s'), coalesce(date_format(LOGOUT_TIME, '%Y/%m/%d %h:%i:%s'), '(not set)') FROM `"+db_connection_history_table_+"` WHERE IP LIKE ? order by LOGIN_TIME",
			{ ip });

		*out << "\nCount of results for ip: " << rslt->row_count();

		while(rslt->next())
		{
			*out << "\nFound user " << rslt->get_string(0) << " with ip " << rslt->get_string(1)
			     << ", logged in at " << rslt->get_string(2) << " and logged out at " << rslt->get_string(3);
		}
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to select rows for ip `"+ip+"`.", e);
	}
}

void dbconn::get_ips_for_user(const std::string& username, std::ostringstream* out)
{
	try
	{
		mariadb::result_set_ref rslt = select(connection_, "SELECT USER_NAME, IP, date_format(LOGIN_TIME, '%Y/%m/%d %h:%i:%s'), coalesce(date_format(LOGOUT_TIME, '%Y/%m/%d %h:%i:%s'), '(not set)') FROM `"+db_connection_history_table_+"` WHERE USER_NAME LIKE ? order by LOGIN_TIME",
			{ utf8::lowercase(username) });

		*out << "\nCount of results for user: " << rslt->row_count();

		while(rslt->next())
		{
			*out << "\nFound user " << rslt->get_string(0) << " with ip " << rslt->get_string(1)
			     << ", logged in at " << rslt->get_string(2) << " and logged out at " << rslt->get_string(3);
		}
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to select rows for player `"+username+"`.", e);
	}
}

void dbconn::update_addon_download_count(const std::string& instance_version, const std::string& id, const std::string& version)
{
	try
	{
		modify(connection_, "UPDATE `"+db_addon_info_table_+"` SET DOWNLOAD_COUNT = DOWNLOAD_COUNT+1 WHERE INSTANCE_VERSION = ? AND ADDON_ID = ? AND VERSION = ?",
			{ instance_version, id, version });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to update download count for add-on "+id+" with version "+version+".", e);
	}
}

bool dbconn::is_user_author(const std::string& instance_version, const std::string& id, const std::string& username, int is_primary) {
	try
	{
		return exists(connection_, "SELECT 1 FROM `"+db_addon_authors_table_+"` WHERE INSTANCE_VERSION = ? AND ADDON_ID = ? AND AUTHOR = ? AND IS_PRIMARY = ?",
			{ instance_version, id, username, is_primary });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check whether `"+username+"` is an author of "+id+" for version "+instance_version+".", e);
		return false;
	}
}

void dbconn::delete_addon_authors(const std::string& instance_version, const std::string& id) {
	try
	{
		modify(connection_, "DELETE FROM `"+db_addon_authors_table_+"` WHERE INSTANCE_VERSION = ? AND ADDON_ID = ?",
			{ instance_version, id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to delete addon authors for "+id+" and version "+instance_version+".", e);
	}
}

void dbconn::insert_addon_author(const std::string& instance_version, const std::string& id, const std::string author, int is_primary) {
	try
	{
		modify(connection_, "INSERT INTO `"+db_addon_authors_table_+"`(INSTANCE_VERSION, ADDON_ID, AUTHOR, IS_PRIMARY) VALUES(?,?,?,?)",
			{ instance_version, id, author, is_primary });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to delete addon authors for "+id+" and version "+instance_version+".", e);
	}
}

bool dbconn::do_any_authors_exist(const std::string& instance_version, const std::string& id) {
	try
	{
		return exists(connection_, "SELECT 1 FROM `"+db_addon_authors_table_+"` WHERE INSTANCE_VERSION = ? AND ADDON_ID = ?",
			{ instance_version, id });
	}
	catch(const mariadb::exception::base& e)
	{
		log_sql_exception("Unable to check whether authors exist for "+id+" for version "+instance_version+".", e);
		return true;
	}
}

//
// handle complex query results
//
void dbconn::get_complex_results(mariadb::connection_ref connection, rs_base& base, const std::string& sql, const sql_parameters& params)
{
	mariadb::result_set_ref rslt = select(connection, sql, params);
	base.read(rslt);
}
//
// handle single values
//
std::string dbconn::get_single_string(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	mariadb::result_set_ref rslt = select(connection, sql, params);
	if(rslt->next())
	{
		return rslt->get_string(0);
	}
	else
	{
		throw mariadb::exception::base("No string value found in the database!");
	}
}
long dbconn::get_single_long(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	mariadb::result_set_ref rslt = select(connection, sql, params);
	if(rslt->next())
	{
		// mariadbpp checks for strict integral equivalence, but we don't care
		// so check the type beforehand, call the associated getter, and let it silently get upcast to a long if needed
		// subselects also apparently return a decimal
		switch(rslt->column_type(0))
		{
			case mariadb::value::type::decimal:
				return static_cast<long>(rslt->get_decimal(0).float32());
			case mariadb::value::type::unsigned8:
			case mariadb::value::type::signed8:
				return rslt->get_signed8(0);
			case mariadb::value::type::unsigned16:
			case mariadb::value::type::signed16:
				return rslt->get_signed16(0);
			case mariadb::value::type::unsigned32:
			case mariadb::value::type::signed32:
				return rslt->get_signed32(0);
			case mariadb::value::type::unsigned64:
			case mariadb::value::type::signed64:
				return rslt->get_signed64(0);
			default:
				throw mariadb::exception::base("Value retrieved was not a long!");
		}
	}
	else
	{
		throw mariadb::exception::base("No long value found in the database!");
	}
}
bool dbconn::exists(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	mariadb::result_set_ref rslt = select(connection, sql, params);
	return rslt->next();
}

//
// select or modify values
//
mariadb::result_set_ref dbconn::select(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	try
	{
		mariadb::statement_ref stmt = query(connection, sql, params);
		mariadb::result_set_ref rslt = mariadb::result_set_ref(stmt->query());
		return rslt;
	}
	catch(const mariadb::exception::base& e)
	{
		ERR_SQL << "SQL query failed for query: `"+sql+"`";
		throw e;
	}
}
unsigned long long dbconn::modify(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	try
	{
		mariadb::statement_ref stmt = query(connection, sql, params);
		unsigned long long count = stmt->execute();
		return count;
	}
	catch(const mariadb::exception::base& e)
	{
		ERR_SQL << "SQL query failed for query: `"+sql+"`";
		throw e;
	}
}
unsigned long long dbconn::modify_get_id(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	try
	{
		mariadb::statement_ref stmt = query(connection, sql, params);
		unsigned long long count = stmt->insert();
		return count;
	}
	catch(const mariadb::exception::base& e)
	{
		ERR_SQL << "SQL query failed for query: `"+sql+"`";
		throw e;
	}
}

mariadb::statement_ref dbconn::query(mariadb::connection_ref connection, const std::string& sql, const sql_parameters& params)
{
	mariadb::statement_ref stmt = connection->create_statement(sql);

	unsigned i = 0;
	for(const auto& param : params)
	{
		if(std::holds_alternative<bool>(param))
		{
			stmt->set_boolean(i, std::get<bool>(param));
		}
		else if(std::holds_alternative<int>(param))
		{
			stmt->set_signed32(i, std::get<int>(param));
		}
		else if(std::holds_alternative<unsigned long long>(param))
		{
			stmt->set_signed64(i, std::get<unsigned long long>(param));
		}
		else if(std::holds_alternative<std::string>(param))
		{
			stmt->set_string(i, std::get<std::string>(param));
		}
		else if(std::holds_alternative<const char*>(param))
		{
			stmt->set_string(i, std::get<const char*>(param));
		}
		i++;
	}

	return stmt;
}

#endif //HAVE_MYSQLPP
