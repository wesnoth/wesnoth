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

	tokens();
};

namespace detail{
// Some typedef for the use of the lexertl engine.
template <class BaseIterator>
struct lexer_helper
{
	typedef BaseIterator base_iterator_type;

	typedef lex::lexertl::token<
		base_iterator_type, boost::mpl::vector<lex::omit, int, std::size_t, std::string> 
	> token_type;

	typedef lex::lexertl::actor_lexer<token_type> type;
};

// iterator type used to expose the underlying input stream
typedef std::string::iterator lexer_base_iterator_type;

// We use the default lexer engine.
typedef lexer_helper<lexer_base_iterator_type>::type lexer_type;

} // namespace detail

typedef tokens<detail::lexer_type> tokens_type;

} // namespace sql
#endif // SQL_LEXER_HPP