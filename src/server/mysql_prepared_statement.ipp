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
#include <string>
#include <string.h>
#include <iostream>

#include <mysql/mysql.h>

#include "exceptions.hpp"

struct sql_error : public game::error
{
	sql_error(const std::string& message) : game::error(message) {}
};

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
	return { (make_bind(std::forward<Args>(args)))... };
}

template<typename T> T fetch_result(MYSQL_STMT* stmt);
template<> std::string fetch_result<std::string>(MYSQL_STMT* stmt)
{
	char* buf = new char[200];
	std::size_t len = 200;
	my_bool is_null;
	MYSQL_BIND result_bind[1] = { make_bind(buf, &len, &is_null) };

	if(mysql_stmt_bind_result(stmt, result_bind) != 0)
		throw sql_error(mysql_stmt_error(stmt));

	int res = mysql_stmt_fetch(stmt);
	if(res == MYSQL_NO_DATA)
		throw sql_error("no data returned");
	if(is_null)
		throw sql_error("null value returned");
	if(res != 0)
		throw sql_error(mysql_stmt_error(stmt));
	mysql_stmt_free_result(stmt);
	mysql_stmt_close(stmt);
	std::cout << "Result: " << buf << std::endl;
	return buf;
}

template<> int fetch_result<int>(MYSQL_STMT* stmt)
{
	int result;
	my_bool is_null;
	MYSQL_BIND result_bind[1] = { make_bind(result, &is_null) };

	if(mysql_stmt_bind_result(stmt, result_bind) != 0)
		throw sql_error(mysql_stmt_error(stmt));

	int res = mysql_stmt_fetch(stmt);
	if(res == MYSQL_NO_DATA)
		throw sql_error("no data returned");
	if(is_null)
		throw sql_error("null value returned");
	if(res != 0)
		throw sql_error(mysql_stmt_error(stmt));
	mysql_stmt_free_result(stmt);
	mysql_stmt_close(stmt);
	std::cout << "Result: " << result << std::endl;
	return result;
}

template<> void fetch_result<void>(MYSQL_STMT*)
{
}

template<typename R, typename... Args>
R prepared_statement(MYSQL* conn, const std::string& sql, Args&&... args)
{
	MYSQL_STMT* stmt;
	auto arg_binds = make_binds(args...);

	stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw sql_error("mysql_stmt_init failed");

	if(mysql_stmt_prepare(stmt, sql.c_str(), sql.size()) != 0)
		throw sql_error(mysql_stmt_error(stmt));

	if(mysql_stmt_bind_param(stmt, arg_binds.data()) != 0)
		throw sql_error(mysql_stmt_error(stmt));

	if(mysql_stmt_execute(stmt) != 0)
		throw sql_error(mysql_stmt_error(stmt));

	std::cout << "SQL: " << sql << std::endl;

	return fetch_result<R>(stmt);
}

#endif // MYSQL_PREPARED_STATEMENT_IPP
