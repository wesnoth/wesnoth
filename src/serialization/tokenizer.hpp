/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <istream>
#include <string>

class config;

struct token 
{
	enum token_type {
		STRING,
		QSTRING,
		UNTERMINATED_QSTRING,
		MISC,
		
		LF = '\n',
		EQUALS = '=',
		COMMA = ',',
		PLUS = '+',
		SLASH = '/',
		OPEN_BRACKET = '[',
		CLOSE_BRACKED = ']',
		UNDERSCORE = '_',
		END
	} type;

	std::string leading_spaces;
	std::string value;
};

class tokenizer 
{
public:
	tokenizer(std::istream& in);

	const token& next_token();
	const token& current_token();
	std::string get_line();
	std::string& textdomain();
private:
	void next_char();
	int get_char();
	int peek_char();
	bool is_space(int c);
	bool is_alnum(int c);
	void skip_comment();

	std::istream& in_;
	int current_;

	std::string textdomain_;
	std::string file_;
	size_t tokenstart_lineno_;
	size_t lineno_;
	token token_;
};

#endif

