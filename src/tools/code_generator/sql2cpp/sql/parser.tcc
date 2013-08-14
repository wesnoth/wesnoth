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

#ifndef SQL_PARSER_DEF_HPP
#define SQL_PARSER_DEF_HPP

#include "tools/code_generator/sql2cpp/sql/parser.hpp"

namespace sql{

namespace bs = boost::spirit;
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

template <typename Iterator>
template <typename TokenDef>
grammar<Iterator>::grammar(TokenDef const& tok)
	: grammar::base_type(schema, "schema")
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

	RULE_DEF(identifier_list,
		%= tok.paren_open >> (tok.identifier % tok.comma) >> tok.paren_close
		);

	RULE_DEF(primary_key_constraint,
		= tok.kw_primary_key >> identifier_list 
			[phx::bind(&semantic_actions::make_ptr<primary_key, base_constraint, std::string, ast::id_list>,
		  &sa_, qi::_val, qi::_r1, qi::_1)]
		);

	RULE_DEF(foreign_key_constraint,
		=	(tok.kw_foreign_key >> identifier_list >> reference_definition)
			[phx::bind(&semantic_actions::make_ptr<foreign_key, base_constraint, std::string, ast::id_list, ast::key_references>, 
				&sa_, qi::_val, qi::_r1, qi::_1, qi::_2)]
		);

	RULE_DEF(reference_definition,
		%=	tok.kw_references >> tok.identifier >> identifier_list
		);

	RULE_DEF(create_table_columns,
		%=   column_definition % tok.comma
		);

	RULE_DEF(column_definition,
		%=   tok.identifier >> column_type >> *type_constraint
		);

	RULE_DEF(type_constraint,
		=   tok.kw_not_null					[phx::bind(&semantic_actions::make_ptr<not_null, base_type_constraint>, &sa_, qi::_val)]
		|   tok.kw_auto_increment		[phx::bind(&semantic_actions::make_ptr<auto_increment, base_type_constraint>, &sa_, qi::_val)]
		|   tok.kw_unique  					[phx::bind(&semantic_actions::make_ptr<unique, base_type_constraint>, &sa_, qi::_val)]
		|   default_value_constraint[phx::bind(&semantic_actions::make_ptr<default_value, base_type_constraint, std::string>, 
														&sa_, qi::_val, qi::_1)]
		);

	RULE_DEF(default_value_constraint,
		%=   tok.kw_default > tok.quoted_string
		);

	RULE_DEF(column_type,
		=   numeric_type	[qi::_val = qi::_1]
		|		(tok.type_varchar > tok.paren_open > tok.unsigned_digit > tok.paren_close) 
														[phx::bind(&semantic_actions::make_ptr<type::varchar, type::base_type, std::size_t>, &sa_, qi::_val, qi::_1)]
		|   tok.type_text 			[phx::bind(&semantic_actions::make_ptr<type::text, type::base_type>, &sa_, qi::_val)]
		|   tok.type_date			  [phx::bind(&semantic_actions::make_ptr<type::date, type::base_type>, &sa_, qi::_val)]
		);

	RULE_DEF(numeric_type,
		=
		(		tok.type_smallint		[phx::bind(&semantic_actions::make_ptr<type::smallint, type::numeric_type>, &sa_, qi::_val)]
		| 	tok.type_int 				[phx::bind(&semantic_actions::make_ptr<type::integer, type::numeric_type>, &sa_, qi::_val)]
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

} // namespace sql
#endif // SQL_PARSER_DEF_HPP