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

#include "tools/code_generator/sql2cpp/sql/type.hpp"
#include "tools/code_generator/sql2cpp/sql/type_constraint.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

// This struct is needed by the sql_constraint file (more specifically by the foreign_key class).
namespace sql{
namespace ast{
	struct key_references
	{
		std::string ref_table;
		std::vector<std::string> refs;
	};
}} // namespace sql::ast

#include "tools/code_generator/sql2cpp/sql/constraint.hpp"

namespace sql{
namespace ast{

struct column;
struct table;

typedef std::vector<table> schema;
typedef std::vector<column> column_list;
typedef boost::shared_ptr<base_constraint> constraint_ptr;
typedef boost::shared_ptr<base_type_constraint> type_constraint_ptr;
typedef boost::shared_ptr<type::base_type> column_type_ptr;
typedef boost::shared_ptr<type::numeric_type> numeric_type_ptr;

typedef std::vector<constraint_ptr> constraint_list;
typedef std::vector<type_constraint_ptr> type_constraint_list;
typedef std::vector<std::string> id_list;

struct table
{
	std::string table_identifier;
	column_list columns;
	constraint_list constraints;
};

struct column
{
	std::string column_identifier;
	column_type_ptr sql_type;
	type_constraint_list constraints;
};

}} // namespace sql::ast

// Fusion AST adaptation.
BOOST_FUSION_ADAPT_STRUCT(
	sql::ast::column,
	(std::string, column_identifier)
	(sql::ast::column_type_ptr, sql_type)
	(sql::ast::type_constraint_list, constraints)
);

BOOST_FUSION_ADAPT_STRUCT(
	sql::ast::table,
	(std::string, table_identifier)
	(sql::ast::column_list, columns)
	(sql::ast::constraint_list, constraints)
);

BOOST_FUSION_ADAPT_STRUCT(
	sql::ast::key_references,
	(std::string, ref_table)
	(std::vector<std::string>, refs)
);

#endif // SQL_AST_HPP