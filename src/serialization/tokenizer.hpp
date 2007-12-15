/* $Id$ */
/*
   Copyright (C) 2004 - 2007 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/tokenizer.hpp 
//!

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <istream>
#include <string>

class config;

struct token
{
	token() :
		type(END),
		leading_spaces(),
		value()
		{}

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

//! Abstract baseclass for the tokenizer
class tokenizer
{
public:
	tokenizer();
	virtual ~tokenizer() {}

	const token& next_token();
	const token& current_token() const;
	std::string get_line() const;
	std::string& textdomain();

protected:
	int current_;
	size_t lineno_;

	virtual void next_char() = 0;
	virtual int peek_char() const = 0;
private:
	bool is_space(const int c) const;
	bool is_alnum(const int c) const;
	void skip_comment();

	std::string textdomain_;
	std::string file_;
	size_t tokenstart_lineno_;
	token token_;
};

//! tokenizer which uses an istream as input
class tokenizer_stream : public tokenizer
{
public:
	tokenizer_stream(std::istream& in);

protected:
	void next_char();
	int peek_char() const;

private:
	std::istream& in_;
};

//! tokenizer which uses an string as input
class tokenizer_string : public tokenizer
{
public:
	tokenizer_string(std::string& in);

protected:
	void next_char();
	int peek_char() const;

private:
	std::string& in_;
	size_t offset_;
};
#endif

