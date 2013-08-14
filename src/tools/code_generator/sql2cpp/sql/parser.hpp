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

#ifndef SQL_PARSER_HPP
#define SQL_PARSER_HPP

#include "tools/code_generator/sql2cpp/sql/semantic_actions.hpp"
#include "tools/code_generator/sql2cpp/preprocessor_rule_helper.hpp"
#include "tools/code_generator/sql2cpp/sql/lexer.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace sql{

namespace bs = boost::spirit;
namespace qi = boost::spirit::qi;

// Grammar definition, define a little part of the SQL language.
template <typename Iterator>
struct grammar 
	: qi::grammar<Iterator, sql::ast::schema()>
{
	typedef Iterator iterator_type;

	template <typename TokenDef>
	grammar(TokenDef const& tok);

private:
	semantic_actions sa_;

	QI_RULE(ast::schema(), schema);
	QI_RULE(void(ast::schema&), statement);

	// Generic rules.
	QI_RULE(ast::id_list(), identifier_list);

	// Create rules.
	QI_RULE(ast::table(), create_statement);
	QI_RULE(ast::table(), create_table);
	QI_RULE(ast::column_list(), create_table_columns);
	QI_RULE(ast::column(), column_definition);
	QI_RULE(ast::constraint_list(), table_constraints);

	// Constraint rules.
	QI_RULE_LOC(ast::constraint_ptr(), std::string, constraint_definition);
	QI_RULE(ast::constraint_ptr(std::string), primary_key_constraint);
	QI_RULE(ast::constraint_ptr(std::string), foreign_key_constraint);
	QI_RULE(ast::key_references(), reference_definition);

	// Type rules.
	QI_RULE(ast::column_type_ptr(), column_type);
	QI_RULE(ast::type_constraint_ptr(), type_constraint);
	QI_RULE(std::string(), default_value_constraint);
	QI_RULE(ast::numeric_type_ptr(), numeric_type);

	// Alter rules.
	QI_RULE(void(ast::schema&), alter_statement);
	QI_RULE_LOC(void(ast::schema&), ast::schema::iterator, alter_table);
	QI_RULE(void(ast::table&), alter_table_add);
};

// this is the iterator type exposed by the lexer 
typedef tokens_type::iterator_type token_iterator_type;

// this is the type of the grammar to parse
typedef sql::grammar<token_iterator_type> grammar_type;

} // namespace sql
#endif // SQL_PARSER_HPP