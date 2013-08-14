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

// Class sql2cpp_type_visitor.

std::string sql2cpp_type_visitor::add_unsigned_qualifier(const sql::type::numeric_type& num_type)
{
	return (num_type.is_unsigned) ? "u":"";
}

sql2cpp_type_visitor::sql2cpp_type_visitor(std::string& res)
: res_(res)
{}

void sql2cpp_type_visitor::visit(const sql::type::smallint& s)
{
	res_ = "boost::" + add_unsigned_qualifier(s) + "int16_t";
}

void sql2cpp_type_visitor::visit(const sql::type::integer& i)
{
	res_ = "boost::" + add_unsigned_qualifier(i) + "int32_t";
}

void sql2cpp_type_visitor::visit(const sql::type::text&)
{
	res_ = "std::string";
}

void sql2cpp_type_visitor::visit(const sql::type::date&)
{
	res_ = "boost::posix_time::ptime";
}

void sql2cpp_type_visitor::visit(const sql::type::varchar& v)
{
	res_ = "boost::array<char, " + boost::lexical_cast<std::string>(v.length) + ">";
}

// Class sql2cpp_header_type_visitor.

sql2cpp_header_type_visitor::sql2cpp_header_type_visitor(std::string& res)
: res_(res)
{}

void sql2cpp_header_type_visitor::visit(const sql::type::smallint&)
{
	res_ = "#include <boost/cstdint.hpp>";
}

void sql2cpp_header_type_visitor::visit(const sql::type::integer&)
{
	res_ = "#include <boost/cstdint.hpp>";
}

void sql2cpp_header_type_visitor::visit(const sql::type::text&)
{
	res_ = "#include <string>";
}

void sql2cpp_header_type_visitor::visit(const sql::type::date&)
{
	res_ = "#include <boost/date_time/posix_time/posix_time.hpp>";
}

void sql2cpp_header_type_visitor::visit(const sql::type::varchar&)
{
	res_ = "#include <boost/array.hpp>";
}

} // namespace cpp
