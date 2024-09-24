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

#include <array>
#include <istream>
#include <string>

// use of illegal utf8 character for this purpose was added in a76be7ef1e921dabacd99f16ef440bf9673b8d98
// added by the preprocessor to allow special handling for #line and #textdomain commands
constexpr unsigned char INLINED_PREPROCESS_DIRECTIVE_CHAR = 254;

// normal ascii is 0-127
// extended ascii is from 128-255, none of which need any special handling
constexpr int END_STANDARD_ASCII = 128;

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

	/**
	 * used for a token's type field
	 */
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

		/** set when EOF is returned by the input stream */
		END = 256
	};

	/**
	 * not used for a token's type field
	 */
	enum source_chars
	{
		POUND = '#',
		LEFT_ANGLE_BRACKET = '<',
		RIGHT_ANGLE_BRACKET = '>',
		DOUBLE_QUOTE = '"',
		DOLLAR = '$',
	};

	token_type type;
	/** the token's value, can be either a single character or multiple characters */
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

	/**
	 * Reads characters off of @a in_ to return the next token type and its value.
	 */
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

	/**
	 * increments the line number if the current character is a newline
	 * set current_ to the next character that's not `\r`
	 */
	void next_char()
	{
		if (current_ == token::NEWLINE)
			++lineno_;
		next_char_skip_cr();
	}

	/**
	 * set current_ to the next character
	 * skip the `\r` in the `\r\n` Windows-style line endings
	 * the test_cvs_2018_1999023_2.cfg file also uses `\r\n` line endings for some reason - otherwise that check isn't needed on non-Windows platforms since `\r` characters are removed from cfg files on upload
	 */
	void next_char_skip_cr()
	{
		current_ = in_.get();
		if(current_ == '\r') {
			current_ = in_.get();
		}
	}

	/**
	 * return the next character without incrementing the current position in the istream
	 */
	int peek_char()
	{
		return in_.peek();
	}

	/**
	 * the different types of characters while parsing
	 * TOK_NONE is also the default for anything beyond standard ascii
	 */
	enum character_type
	{
		TOK_NONE = 0,
		TOK_SPACE = 1,
		TOK_NUMERIC = 2,
		TOK_ALPHA = 3
	};

	character_type char_type(unsigned c) const
	{
		return c < END_STANDARD_ASCII ? char_types_[c] : TOK_NONE;
	}

	bool is_space(int c) const
	{
		return char_type(c) == TOK_SPACE;
	}

	bool is_num(int c) const
	{
		return char_type(c) == TOK_NUMERIC;
	}

	bool is_alnum(int c) const
	{
		return char_type(c) > TOK_SPACE;
	}

	/**
	 * handles skipping over comments (inline and on a separate line) as well as the special processing needed for \#textdomain and \#line
	 */
	void skip_comment();

	/**
	 * Returns true if the next characters are the one from @a cmd followed by a space. Skips all the matching characters.
	 * Currently only used by \#textdomain (specified by the WML) and \#line (added by the preprocessor)
	 */
	bool skip_command(char const *cmd);

	std::string textdomain_;
	std::string file_;
	token token_;
#ifdef DEBUG_TOKENIZER
	token previous_token_;
#endif
	buffered_istream in_;
	std::array<character_type, END_STANDARD_ASCII> char_types_;
};
