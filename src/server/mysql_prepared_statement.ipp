/*
   Copyright (C) 2016 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MYSQL_PREPARED_STATEMENT_IPP
#define MYSQL_PREPARED_STATEMENT_IPP

#include <array>
#include <utility>
#include <memory>
#include <string>
#include <string.h>

#define BOOST_SCOPE_EXIT_CONFIG_USE_LAMBDAS
#include <boost/scope_exit.hpp>

#include <mysql/mysql.h>

#include "exceptions.hpp"

struct sql_error : public game::error
{
	sql_error(const std::string& message, const std::string& sql)
		: game::error("Error evaluating SQL statement: '" + sql + "': " + message) {}
	sql_error(const std::string& message) : game::error(message) {}
};

// make_bind functions embed pointers to their arguments in the
// MYSQL_BIND structure returned. It's caller's responsibility
// to ensure that argument's lifetime doesn't end before mysql
// is done with those MYSQL_BINDs
MYSQL_BIND make_bind(const std::string& str, my_bool* is_null = 0)
{
	MYSQL_BIND result;
	memset(&result, 0, sizeof (MYSQL_BIND));
	result.buffer_type = MYSQL_TYPE_STRING;
	result.buffer = const_cast<void*>(static_cast<const void*>(str.c_str()));
	result.buffer_length = str.size();
	result.is_unsigned = 0;
	result.is_null = is_null;
	result.length = 0;
	return result;
}

MYSQL_BIND make_bind(char* str, std::size_t* len, my_bool* is_null = 0)
{
	MYSQL_BIND result;
	memset(&result, 0, sizeof (MYSQL_BIND));
	result.buffer_type = MYSQL_TYPE_STRING;
	result.buffer = static_cast<void*>(str);
	result.buffer_length = *len;
	result.is_unsigned = 0;
	result.is_null = is_null;
	result.length = len;
	return result;
}

MYSQL_BIND make_bind(int& i, my_bool* is_null = 0)
{
	MYSQL_BIND result;
	memset(&result, 0, sizeof (MYSQL_BIND));
	result.buffer_type = MYSQL_TYPE_LONG;
	result.buffer = static_cast<void*>(&i);
	result.is_unsigned = 0;
	result.is_null = is_null;
	return result;
}

MYSQL_BIND make_bind(int&& i, my_bool* is_null = 0)
{
	MYSQL_BIND result;
	memset(&result, 0, sizeof (MYSQL_BIND));
	result.buffer_type = MYSQL_TYPE_LONG;
	result.buffer = static_cast<void*>(&i);
	result.is_unsigned = 0;
	result.is_null = is_null;
	return result;
}

template<typename... Args> constexpr auto make_binds(Args&&... args)
	-> std::array<MYSQL_BIND, sizeof...(Args)>
{
	return { { (make_bind(std::forward<Args>(args))) ... } };
}

template<typename T> T fetch_result(MYSQL_STMT* stmt, const std::string& sql);
template<> std::string fetch_result<std::string>(MYSQL_STMT* stmt, const std::string& sql)
{
	char* buf = nullptr;
	std::string result;
	std::size_t len = 0;
	my_bool is_null;
	MYSQL_BIND result_bind[1] = { make_bind(buf, &len, &is_null) };

	if(mysql_stmt_bind_result(stmt, result_bind) != 0)
		throw sql_error(mysql_stmt_error(stmt), sql);

	BOOST_SCOPE_EXIT(&stmt) {
		mysql_stmt_free_result(stmt);
	} ;

	mysql_stmt_store_result(stmt);

	int res = mysql_stmt_fetch(stmt);
	if(res == MYSQL_NO_DATA)
		throw sql_error("no data returned", sql);
	if(is_null)
		throw sql_error("null value returned", sql);
	if(res != MYSQL_DATA_TRUNCATED)
		throw sql_error(mysql_stmt_error(stmt), sql);
	if(len > 0) {
		buf = new char[len];
		result_bind[0].buffer = buf;
		result_bind[0].buffer_length = len;
		res = mysql_stmt_fetch_column(stmt, result_bind, 0, 0);
		result = std::string(buf, len);
		delete[] buf;
	}
	if(res == MYSQL_NO_DATA)
		throw sql_error("no data returned", sql);
	if(res != 0)
		throw sql_error("mysql_stmt_fetch_column failed", sql);
	return result;
}

template<> int fetch_result<int>(MYSQL_STMT* stmt, const std::string& sql)
{
	int result;
	my_bool is_null;
	MYSQL_BIND result_bind[1] = { make_bind(result, &is_null) };

	if(mysql_stmt_bind_result(stmt, result_bind) != 0)
		throw sql_error(mysql_stmt_error(stmt), sql);

	BOOST_SCOPE_EXIT(&stmt) {
		mysql_stmt_free_result(stmt);
	} ;

	int res = mysql_stmt_fetch(stmt);
	if(res == MYSQL_NO_DATA)
		throw sql_error("no data returned", sql);
	if(is_null)
		throw sql_error("null value returned", sql);
	if(res != 0)
		throw sql_error(mysql_stmt_error(stmt), sql);
	return result;
}

template<> bool fetch_result<bool>(MYSQL_STMT* stmt, const std::string& sql)
{
	int result;
	my_bool is_null;
	MYSQL_BIND result_bind[1] = { make_bind(result, &is_null) };

	if(mysql_stmt_bind_result(stmt, result_bind) != 0)
		throw sql_error(mysql_stmt_error(stmt), sql);

	BOOST_SCOPE_EXIT(&stmt) {
		mysql_stmt_free_result(stmt);
	} ;

	int res = mysql_stmt_fetch(stmt);
	if(res == MYSQL_NO_DATA)
		return false;
	if(is_null)
		throw sql_error("null value returned", sql);
	if(res != 0)
		throw sql_error(mysql_stmt_error(stmt), sql);
	return true;
}

template<> void fetch_result<void>(MYSQL_STMT*, const std::string&)
{
}

/**
 * Execute an sql query using mysql prepared statements API
 * This function can convert its arguments and results to appropriate
 * MYSQL_BIND structures automatically based on their C++ type
 * though each type requires explicit support. For now only ints and
 * std::strings are supported.
 * Setting return type to bool causes this function to do a test query
 * and return true if there is any data in result set, false otherwise
 */
template<typename R, typename... Args>
R prepared_statement(MYSQL* conn, const std::string& sql, Args&&... args)
{
	auto arg_binds = make_binds(args...);

	std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt{mysql_stmt_init(conn), mysql_stmt_close};
	if(stmt == nullptr)
		throw sql_error("mysql_stmt_init failed", sql);

	if(mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.size()) != 0)
		throw sql_error(mysql_stmt_error(stmt.get()), sql);

	if(mysql_stmt_bind_param(stmt.get(), arg_binds.data()) != 0)
		throw sql_error(mysql_stmt_error(stmt.get()), sql);

	if(mysql_stmt_execute(stmt.get()) != 0)
		throw sql_error(mysql_stmt_error(stmt.get()), sql);

	return fetch_result<R>(stmt.get(), sql);
}

#endif // MYSQL_PREPARED_STATEMENT_IPP
