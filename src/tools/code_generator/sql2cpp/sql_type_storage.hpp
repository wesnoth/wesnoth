/*
	 Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.
	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY.

	 See the COPYING file for more details.
*/

#ifndef SQL_TYPE_STORAGE_HPP
#define SQL_TYPE_STORAGE_HPP

#include <string>
#include <boost/cstdint.hpp>

#include "tools/code_generator/sql2cpp/sql_type.hpp"

namespace sql{
namespace type{

template <class Type>
struct type_storage
{};

template <>
class type_storage<smallint>
{
	typedef int16_t storage_type;
};

template <>
class type_storage<integer>
{
	typedef int32_t storage_type;
};

template <>
class type_storage<text>
{
	typedef std::string storage_type;
};

template <>
class type_storage<date>
{
	typedef std::string storage_type;
};

template <>
class type_storage<varchar>
{
	typedef std::string storage_type;
};

}} // namespace sql::type

#endif // SQL_TYPE_HPP