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

#ifndef CPP_GENERATOR_HPP
#define CPP_GENERATOR_HPP

#include "tools/code_generator/sql2cpp/preprocessor_rule_helper.hpp"
#include "tools/code_generator/sql2cpp/cpp/semantic_actions.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace cpp{

namespace karma = boost::spirit::karma;
namespace phx = boost::phoenix;

template <typename OutputIterator>
struct grammar 
: karma::grammar<OutputIterator, sql::ast::schema()>
{
	typedef OutputIterator iterator_type;

	grammar(semantic_actions& sa);

private:
	semantic_actions &sa_;

	KA_RULE(sql::ast::schema(), schema);
	KA_RULE(sql::ast::table(), file);
	KA_RULE(sql::ast::table(), structure);
	KA_RULE(sql::ast::table(), header);
	KA_RULE_LOC(std::string(), std::string, define_header);
	KA_RULE_LOC(sql::ast::column_list(), std::set<std::string>, includes);

	KA_RULE0(footer);
	KA_RULE0(define_footer);
	KA_RULE0(license_header);
	KA_RULE0(namespace_open);
	KA_RULE0(namespace_close);
	KA_RULE0(do_not_modify);

	KA_RULE(sql::ast::column_list(), members);
	KA_RULE(sql::ast::column(), member);
	KA_RULE(sql::ast::column_type_ptr(), member_type);
};

template <typename OutputIterator>
bool generate(OutputIterator& sink, sql::ast::schema const& sql_ast, std::ofstream& generated, const std::string& output_dir)
{
	semantic_actions sa("../", generated, output_dir);
	grammar<OutputIterator> cpp_grammar(sa);
	return karma::generate(sink, cpp_grammar, sql_ast);
}

} // namespace cpp
#endif // CPP_GENERATOR_HPP
