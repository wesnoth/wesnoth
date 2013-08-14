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

#ifndef CPP_TYPE_VISITORS_HPP
#define CPP_TYPE_VISITORS_HPP

#include "tools/code_generator/sql2cpp/sql/type.hpp"

namespace cpp{

struct sql2cpp_type_visitor : sql::type::type_visitor
{
private:
	std::string add_unsigned_qualifier(const sql::type::numeric_type& num_type);
public:

	sql2cpp_type_visitor(std::string& res);
	virtual void visit(const sql::type::smallint& s);
	virtual void visit(const sql::type::integer& i);
	virtual void visit(const sql::type::text&);
	virtual void visit(const sql::type::date&);
	virtual void visit(const sql::type::varchar& v);

private:
	std::string& res_;
};

struct sql2cpp_header_type_visitor : sql::type::type_visitor
{
	sql2cpp_header_type_visitor(std::string& res);

	virtual void visit(const sql::type::smallint&);
	virtual void visit(const sql::type::integer&);
	virtual void visit(const sql::type::text&);
	virtual void visit(const sql::type::date&);
	virtual void visit(const sql::type::varchar&);

private:
	std::string& res_;
};

} // namespace cpp
#endif // CPP_TYPE_VISITORS_HPP