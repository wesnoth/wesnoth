/* $Id$ */
/*
   Copyright (C) 2004 - 2008 by Philippe Plantier <ayin@anathas.org>
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

#include "util.hpp"

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
	tokenizer(std::istream& in);
	~tokenizer() {}

	const token& next_token();
	const token& current_token() const;
	std::string get_line() const;
	std::string& textdomain();

protected:
	tokenizer();
	int current_;
	size_t lineno_;

	inline void next_char()
	{
		if (UNLIKELY(current_ == '\n'))
			lineno_++;
		this->next_char_fast();
	}

	inline void next_char_fast()
	{
		if(LIKELY(in_.good())) {
			current_ = in_.get();
			if (UNLIKELY(current_ == '\r'))
			{
				// we assume that there is only one '\r'
				if(LIKELY(in_.good())) {
					current_ = in_.get();
				} else {
					current_ = EOF;
				}
			}
		} else {
			current_ = EOF;
		}
	}

	inline int peek_char() const
	{
		return in_.peek();
	}

private:
	bool is_space(const int c) const;
	bool is_alnum(const int c) const;
	void skip_comment();

	std::string textdomain_;
	std::string file_;
	size_t tokenstart_lineno_;
	token token_;
	std::istream& in_;
};

#endif

