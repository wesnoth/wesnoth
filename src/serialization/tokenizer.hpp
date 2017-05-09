/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2010 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

//#define DEBUG_TOKENIZER

#include "buffered_istream.hpp"

#include <istream>
#include <string>

class config;

struct token
{
	token() :
		type(END),
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

#ifdef DEBUG_TOKENIZER
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
			current_ = in_.get();
		} while (UNLIKELY(current_ == '\r'));
#if 0
			/// @todo disabled untill campaign server is fixed
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

	int peek_char()
	{
		return in_.peek();
	}

	enum
	{
		TOK_NONE = 0,
		TOK_SPACE = 1,
		TOK_NUMERIC = 2,
		TOK_ALPHA = 4
	};

	int char_type(unsigned c) const
	{
		return c < 128 ? char_types_[c] : 0;
	}

	bool is_space(int c) const
	{
		return (char_type(c) & TOK_SPACE) == TOK_SPACE;
	}

	bool is_num(int c) const
	{
		return (char_type(c) & TOK_NUMERIC) == TOK_NUMERIC;
	}

	bool is_alnum(int c) const
	{
		return (char_type(c) & (TOK_ALPHA | TOK_NUMERIC)) != TOK_NONE;
	}

	void skip_comment();

	/**
	 * Returns true if the next characters are the one from @a cmd
	 * followed by a space. Skips all the matching characters.
	 */
	bool skip_command(char const *cmd);

	std::string textdomain_;
	std::string file_;
	token token_;
#ifdef DEBUG_TOKENIZER
	token previous_token_;
#endif
	buffered_istream in_;
	char char_types_[128];
};
