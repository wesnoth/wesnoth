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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace sql{

namespace bs = boost::spirit;
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

// Grammar definition, define a little part of the SQL language.
template <typename Iterator>
struct sql_grammar 
	: qi::grammar<Iterator, sql::ast::schema()>
{
	typedef Iterator iterator_type;

	template <typename TokenDef>
	sql_grammar(TokenDef const& tok)
		: sql_grammar::base_type(schema, "schema")
	{
		RULE_DEF(schema,
			%=  (statement(qi::_val) % tok.semi_colon) >> *tok.semi_colon
			);

		RULE_DEF(statement,
			=   create_statement 	[phx::push_back(qi::_r1, qi::_1)]
			|		alter_statement(qi::_r1)
			);

		RULE_DEF(create_statement,
			%=   tok.kw_create >> create_table
			);

		RULE_DEF(alter_statement,
			=	 tok.kw_alter >> alter_table(qi::_r1)
			);

		RULE_NDEF(alter_table,
			=	 tok.kw_table
			>> tok.identifier [phx::bind(&semantic_actions::get_table_by_name, &sa_, qi::_a, qi::_r1, qi::_1, bs::_pass)]
			>> (alter_table_add(*qi::_a) % tok.comma)
			);

		RULE_DEF(alter_table_add,
			=	 tok.kw_add >> constraint_definition
			);

		RULE_DEF(create_table,
			=	tok.kw_table >> tok.identifier[phx::at_c<0>(qi::_val) = qi::_1] >> tok.paren_open >> create_table_columns [phx::at_c<1>(qi::_val) = qi::_1] 
				>> -(tok.comma >> table_constraints) [phx::at_c<2>(qi::_val) = qi::_1] >> tok.paren_close
			);

		RULE_DEF(table_constraints,
			%= 	constraint_definition % tok.comma
			);

		RULE_DEF(constraint_definition,
			= tok.kw_constraint >> tok.identifier [qi::_a = qi::_1] >> 
			(	primary_key_constraint(qi::_a) 
			|	foreign_key_constraint(qi::_a)
			) [qi::_val = qi::_1]
			);

		RULE_DEF(primary_key_constraint,
			= tok.kw_primary_key >> tok.paren_open >> (tok.identifier % tok.comma) [phx::bind(&semantic_actions::make_pk_constraint, &sa_, qi::_val, qi::_r1, qi::_1)]
			>> tok.paren_close
			);

		RULE_DEF(foreign_key_constraint,
			=	(tok.kw_foreign_key >> tok.paren_open >> (tok.identifier % tok.comma) >> tok.paren_close >> reference_definition)
				[phx::bind(&semantic_actions::make_fk_constraint, &sa_, qi::_val, qi::_r1, qi::_1, qi::_2)]
			);

		RULE_DEF(reference_definition,
			%=	tok.kw_references >> tok.identifier >> tok.paren_open >> (tok.identifier % tok.comma) >> tok.paren_close
			);

		RULE_DEF(create_table_columns,
			%=   column_definition % tok.comma
			);

		RULE_DEF(column_definition,
			%=   tok.identifier >> column_type >> *type_constraint
			);

		RULE_DEF(type_constraint,
			=   tok.kw_not_null		[phx::bind(&semantic_actions::make_type_constraint<sql::not_null>, &sa_, qi::_val)]
			|   tok.kw_auto_increment	[phx::bind(&semantic_actions::make_type_constraint<sql::auto_increment>, &sa_, qi::_val)]
			|   tok.kw_unique  		[phx::bind(&semantic_actions::make_type_constraint<sql::unique>, &sa_, qi::_val)]
			|   default_value 		[phx::bind(&semantic_actions::make_default_value_constraint, &sa_, qi::_val, qi::_1)]
			);

		RULE_DEF(default_value,
			%=   tok.kw_default > tok.quoted_string
			);

		RULE_DEF(column_type,
			=   numeric_type	[qi::_val = qi::_1]
			|		(tok.type_varchar > tok.paren_open > tok.unsigned_digit > tok.paren_close) 
															[phx::bind(&semantic_actions::make_varchar_type, &sa_, qi::_val, qi::_1)]
			|   tok.type_text 			[phx::bind(&semantic_actions::make_column_type<sql::type::text>, &sa_, qi::_val)]
			|   tok.type_date			  [phx::bind(&semantic_actions::make_column_type<sql::type::date>, &sa_, qi::_val)]
			);

		RULE_DEF(numeric_type,
			=
			(		tok.type_smallint		[phx::bind(&semantic_actions::make_numeric_type<sql::type::smallint>, &sa_, qi::_val)]
			| 	tok.type_int 				[phx::bind(&semantic_actions::make_numeric_type<sql::type::integer>, &sa_, qi::_val)]
			) 
				>> -tok.kw_unsigned		[phx::bind(&semantic_actions::make_unsigned_numeric, &sa_, qi::_val)]
			);

		using namespace qi::labels;
		qi::on_error<qi::fail>
		(
			schema,
			std::cout
				<< phx::val("Error! Expecting ")
				<< bs::_4                               // what failed?
				<< phx::val(" here: \"")
				<< phx::construct<std::string>(bs::_3, bs::_2)   // iterators to error-pos, end
				<< phx::val("\"")
				<< std::endl
		);
	}

private:
	semantic_actions sa_;

	QI_RULE(ast::schema(), schema);
	QI_RULE(void(ast::schema&), statement);

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
	QI_RULE(std::string(), default_value);
	QI_RULE(ast::numeric_type_ptr(), numeric_type);

	// Alter rules.
	QI_RULE(void(ast::schema&), alter_statement);
	QI_RULE_LOC(void(ast::schema&), ast::schema::iterator, alter_table);
	QI_RULE(void(ast::table&), alter_table_add);
};

} // namespace sql
#endif // SQL_PARSER_HPP