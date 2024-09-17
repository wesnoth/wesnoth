/*
	Copyright (C) 2010 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <limits>
#include <istream>
#include <string>

// use of illegal utf8 character for this purpose was added in a76be7ef1e921dabacd99f16ef440bf9673b8d98
// has something to do with the result of the preprocessor whose format is essentially undocumented and I don't intend to delve into that as of writing this comment
#define INLINED_PREPROCESS_DIRECTIVE_CHAR 254

/**
 * contains the current text being parsed as well as the token_type of what's being parsed.
 * multi-character token types will have a value that's a string with zero or more characters in it.
 * single character token types are a single character with special meaning for a config
 */
struct token
{
	token() :
		type(END),
		value()
	{}

	enum token_type
	{
		// multi-character
		/** unquoted text */
		STRING,
		/** quoted string, contained within double quotes or by less than/greater than symbols */
		QSTRING,
		/** reached end of file without finding the closing character for a QSTRING */
		UNTERMINATED_QSTRING,
		/** any characters that don't have special meaning */
		MISC,

		// single characters
		NEWLINE = '\n',
		EQUALS = '=',
		COMMA = ',',
		PLUS = '+',
		SLASH = '/',
		OPEN_BRACKET = '[',
		CLOSE_BRACKET = ']',
		UNDERSCORE = '_',
		POUND = '#',
		LESS_THAN = '<',
		GREATER_THAN = '>',
		DOUBLE_QUOTE = '"',
		DOLLAR = '$',

		/** set when EOF is returned by the input stream */
		END = 256
	};

	token_type type;
	std::string value;
};

/**
 * class responsible for parsing the provided text into tokens and tracking information about the current token.
 * can also track the previous token when built with the DEBUG_TOKENIZER compiler define.
 * does not otherwise keep track of the processing history.
 */
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
		if (current_ == token::NEWLINE)
			++lineno_;
		next_char_fast();
	}

	void next_char_fast()
	{
		do {
			current_ = in_.get();
		} while (current_ == '\r');
	}

	int peek_char()
	{
		return in_.peek();
	}

	enum token_category
	{
		TOK_NONE = 0,
		TOK_SPACE = 1,
		TOK_NUMERIC = 2,
		TOK_ALPHA = 3
	};

	bool is_space(int c) const
	{
		return char_types_[c] == TOK_SPACE;
	}

	bool is_num(int c) const
	{
		return char_types_[c] == TOK_NUMERIC;
	}

	bool is_alnum(int c) const
	{
		return char_types_[c] > TOK_SPACE;
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
	token_category char_types_[std::numeric_limits<unsigned char>::max()];
};
