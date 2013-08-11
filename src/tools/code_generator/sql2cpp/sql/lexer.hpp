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

#ifndef SQL_LEXER_HPP
#define SQL_LEXER_HPP

#include <boost/spirit/include/lex_lexertl.hpp>

namespace sql{

// Namespace shortcut.
namespace lex = boost::spirit::lex;

// Token definition base, defines all tokens for the base grammar below
template <typename Lexer>
struct tokens : lex::lexer<Lexer>
{
public:
	// Tokens with no attributes.
	lex::token_def<lex::omit> type_smallint, type_int, type_varchar, type_text, type_date;
	lex::token_def<lex::omit> kw_not_null, kw_auto_increment, kw_unique, kw_default, kw_create,
		kw_table, kw_constraint, kw_primary_key, kw_alter, kw_add, kw_unsigned, kw_foreign_key,
		kw_references;
  
	lex::token_def<lex::omit> comma, semi_colon, paren_open, paren_close;
  lex::token_def<lex::omit>	ws, comment, cstyle_comment;

	// Attributed tokens. (If you add a new type, don't forget to add it to the lex::lexertl::token definition too below).
	lex::token_def<int> signed_digit;
	lex::token_def<std::size_t> unsigned_digit;
	lex::token_def<std::string> identifier;
	lex::token_def<std::string> quoted_string;

	tokens()
	{
		// Column data types.
		type_smallint = "(?i:smallint)";
		type_int = "(?i:int)";
		type_varchar = "(?i:varchar)";
		type_text = "(?i:text)";
		type_date = "(?i:date)";

		// Keywords.
		kw_not_null = "(?i:not +null)";
		kw_auto_increment = "(?i:auto_increment)";
		kw_unique = "(?i:unique)";
		kw_default = "(?i:default)";
		kw_create = "(?i:create)";
		kw_table = "(?i:table)";
		kw_constraint = "(?i:constraint)";
		kw_primary_key = "(?i:primary +key)";
		kw_foreign_key = "(?i:foreign +key)";
		kw_alter = "(?i:alter)";
		kw_add = "(?i:add)";
		kw_unsigned = "(?i:unsigned)";
		kw_references = "(?i:references)";

		// Values.
		signed_digit = "[-+]?[0-9]+";
		unsigned_digit = "[0-9]+";
		quoted_string = "\\\"(\\\\.|[^\\\"])*\\\""; // \"(\\.|[^\"])*\"

		// Identifier.
		identifier = "[a-zA-Z][a-zA-Z0-9_]*";

		// Separator.
		comma = ',';
		semi_colon = ';';
		paren_open = '(';
		paren_close = ')';

		// White spaces/comments.
		ws = "[ \\t\\n]+";
		comment = "--[^\\n]*\\n";  // Single line comments with --
		cstyle_comment = "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"; // C-style comments

		// The token must be added in priority order.
		this->self += comma | semi_colon | paren_open | paren_close;
		this->self += type_smallint | type_int | type_varchar | type_text |
									type_date;
		this->self += kw_not_null | kw_auto_increment | kw_unique | kw_default |
									kw_create | kw_table | kw_constraint | kw_primary_key | kw_alter |
									kw_add | kw_unsigned | kw_foreign_key | kw_references;
		this->self += identifier | unsigned_digit | signed_digit | quoted_string;

		this->self += ws 		[ lex::_pass = lex::pass_flags::pass_ignore ] 
				| comment 			[ lex::_pass = lex::pass_flags::pass_ignore ]
				| cstyle_comment[ lex::_pass = lex::pass_flags::pass_ignore ]
				;
	}
};

// Some typedef for the use of the lexertl engine.
template <class BaseIterator>
struct lexer
{
	typedef BaseIterator base_iterator_type;

	typedef lex::lexertl::token<
		base_iterator_type, boost::mpl::vector<lex::omit, int, std::size_t, std::string> 
	> token_type;

	typedef lex::lexertl::actor_lexer<token_type> type;
};

} // namespace sql
#endif // SQL_LEXER_HPP