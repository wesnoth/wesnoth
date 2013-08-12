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

#ifndef SQL_AST_HPP
#define SQL_AST_HPP

#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp> 

#include <boost/shared_ptr.hpp>

#include "tools/code_generator/sql2cpp/sql_type.hpp"
#include "tools/code_generator/sql2cpp/sql_type_constraint.hpp"
#include "tools/code_generator/sql2cpp/sql_constraint.hpp"


#include <vector>

namespace sql{
namespace ast{

struct column
{
	std::string column_identifier;
	boost::shared_ptr<sql::type::base_type> sql_type;
	std::vector<boost::shared_ptr<sql::base_type_constraint> > constraints;
};

struct table
{
	std::string table_identifier;
	std::vector<column> columns;
	std::vector<boost::shared_ptr<sql::constraint::base_constraint> > constraints;
};

}} // namespace sql::ast

// Fusion AST adaptation.
BOOST_FUSION_ADAPT_STRUCT(
	sql::ast::column,
	(std::string, column_identifier)
	(boost::shared_ptr<sql::type::base_type>, sql_type)
	(std::vector<boost::shared_ptr<sql::base_type_constraint> >, constraints)
);

BOOST_FUSION_ADAPT_STRUCT(
	sql::ast::table,
	(std::string, table_identifier)
	(std::vector<sql::ast::column>, columns)
	(std::vector<boost::shared_ptr<sql::constraint::base_constraint> >, constraints)
);

#endif // SQL_AST_HPP