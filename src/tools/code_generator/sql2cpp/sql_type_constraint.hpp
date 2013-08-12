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

#ifndef SQL_TYPE_CONSTRAINT_HPP
#define SQL_TYPE_CONSTRAINT_HPP

#include <string>

namespace sql{

struct base_type_constraint{};

struct unique : base_type_constraint
{};

struct not_null : base_type_constraint
{};

struct auto_increment : base_type_constraint
{};

struct default_value : base_type_constraint
{
	typedef std::string value_type;
	
	default_value(const value_type& value)
	: value(value)
	{}

	value_type value;
};

} // namespace sql

#endif // SQL_TYPE_CONSTRAINT_HPP