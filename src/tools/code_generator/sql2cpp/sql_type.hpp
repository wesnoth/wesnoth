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

#ifndef SQL_TYPE_HPP
#define SQL_TYPE_HPP

#include <string>
#include <boost/cstdint.hpp>

namespace sql{
namespace type{

struct base_type{};

struct smallint : base_type
{};

struct integer : base_type
{};

struct text : base_type
{};

struct date : base_type
{};

struct varchar : base_type
{
	std::size_t length;

	varchar(std::size_t length)
	: length(length)
	{}
};

}} // namespace sql::type

#endif // SQL_TYPE_HPP