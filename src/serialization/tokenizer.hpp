/* $Id$ */
/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file serialization/tokenizer.hpp */

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

	enum token_type
	{
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
		CLOSE_BRACKET = ']',
		UNDERSCORE = '_',
		END
	};

	token_type type;

	void reset()
	{
		value.clear();
		leading_spaces.clear();
	}

	std::string leading_spaces;
	std::string value;
};

/** Abstract baseclass for the tokenizer. */
class tokenizer
{
public:
	tokenizer(std::istream& in);
	~tokenizer();

	const token &next_token();

	const token &current_token() const
	{
		return token_;
	}

#ifdef DEBUG
	const token &previous_token() const
	{
		return previous_token_;
	}
#endif

	const std::string &textdomain() const
	{
		return textdomain_;
	}

	const std::string &get_file() const
	{
		return file_;
	}

	int get_start_line() const
	{
		return startlineno_;
	}

private:
	tokenizer();
	int current_;
	int lineno_;
	int startlineno_;

	void next_char()
	{
		if (UNLIKELY(current_ == '\n'))
			++lineno_;
		next_char_fast();
	}

	void next_char_fast()
	{
		do {
			if (LIKELY(in_.good())) {
				current_ = in_.get();
			} else {
				current_ = EOF;
				return;
			}
		} while (UNLIKELY(current_ == '\r'));
#if 0
			// @todo: disabled untill campaign server is fixed
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
#endif
	}

	int peek_char() const
	{
		return in_.peek();
	}

	bool is_space(const int c) const
	{
		return c == ' ' || c == '\t';
	}

	bool is_alnum(const int c) const
	{
		return (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	}

	void skip_comment();

	std::string textdomain_;
	std::string file_;
	token token_;
#ifdef DEBUG
	token previous_token_;
#endif
	std::istream &in_;
};

#endif

