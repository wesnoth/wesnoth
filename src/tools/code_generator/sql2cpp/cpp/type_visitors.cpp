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

#include "tools/code_generator/sql2cpp/cpp/type_visitors.hpp"

#include <boost/lexical_cast.hpp>

namespace cpp{

// Class type2string_visitor.

std::string type2string_visitor::add_unsigned_qualifier(const sql::type::numeric_type& num_type)
{
	return (num_type.is_unsigned) ? "u":"";
}

type2string_visitor::type2string_visitor(std::string& res)
: res_(res)
{}

void type2string_visitor::visit(const sql::type::smallint& s)
{
	res_ = "boost::" + add_unsigned_qualifier(s) + "int16_t";
}

void type2string_visitor::visit(const sql::type::integer& i)
{
	res_ = "boost::" + add_unsigned_qualifier(i) + "int32_t";
}

void type2string_visitor::visit(const sql::type::text&)
{
	res_ = "std::string";
}

void type2string_visitor::visit(const sql::type::date&)
{
	res_ = "boost::posix_time::ptime";
}

void type2string_visitor::visit(const sql::type::varchar& v)
{
	res_ = "boost::array<char, " + boost::lexical_cast<std::string>(v.length) + ">";
}

// Class type2header_visitor.

type2header_visitor::type2header_visitor(std::string& res)
: res_(res)
{}

void type2header_visitor::visit(const sql::type::smallint&)
{
	res_ = "#include <boost/cstdint.hpp>";
}

void type2header_visitor::visit(const sql::type::integer&)
{
	res_ = "#include <boost/cstdint.hpp>";
}

void type2header_visitor::visit(const sql::type::text&)
{
	res_ = "#include <string>";
}

void type2header_visitor::visit(const sql::type::date&)
{
	res_ = "#include <boost/date_time/posix_time/posix_time.hpp>";
}

void type2header_visitor::visit(const sql::type::varchar&)
{
	res_ = "#include <boost/array.hpp>";
}

} // namespace cpp
