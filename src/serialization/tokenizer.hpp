/* $Id$ */
/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2010 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "util.hpp"
#include "token.hpp"

#include <istream>
#include <string>

class config;

namespace {
//static configuration tokens
static const n_token::t_token z_empty("", false);
}

struct token {
	token()
		: type(END)
		, value_()
		, t_token_()
		, is_parsed_(false)
		, buffer_()
	{
	}

	token(token const & a)
		: type(a.type)
		, value_()
		, t_token_(a.t_token_)
		, is_parsed_(a.is_parsed_)
		, buffer_()
	{
	}

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
	mutable std::string value_;
	
	mutable n_token::t_token t_token_;
	mutable bool is_parsed_;

	std::vector<char> buffer_;

	n_token::t_token const & value() const {
		if(!is_parsed_){
			if(!buffer_.empty()){
				value_.assign(buffer_.begin(), buffer_.end());
				t_token_ = n_token::t_token(value_);
			}else {
				t_token_ = z_empty;
			}

			is_parsed_ = true;
		}
		return t_token_;
	}
	
	void set_token(n_token::t_token const & t){
		t_token_ = t;
		is_parsed_=true;
	}

	void set_start() {
		buffer_.clear();
		is_parsed_ = false;
	}
	void add_char(char const c) {
		buffer_.push_back(c);
		is_parsed_ = false;
	} 
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

	int peek_char() const
	{
		return in_.peek();
	}

	enum
	{
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
		return char_type(c) & TOK_SPACE;
	}

	bool is_num(int c) const
	{
		return char_type(c) & TOK_NUMERIC;
	}

	bool is_alnum(int c) const
	{
		return char_type(c) & (TOK_ALPHA | TOK_NUMERIC);
	}

	void skip_comment();

	/**
	 * Returns true if the next characters are the one from @a cmd
	 * followed by a space. Skips all the matching characters.
	 */
	bool skip_command(char const *cmd);
	bool fill_char_types();

	std::string textdomain_;
	std::string file_;
	token token_;
#ifdef DEBUG
	token previous_token_;
#endif
	std::istream &in_;
	static char char_types_[128];
};

#endif

