/* $Id$ */
/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2010 - 2012 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

	/**
	 * Fills the cache.
	 *
	 * @warning This function must be called before @ref peek_char(). (Not
	 * required before @ref next_char_fast(), but it can't hurt.) It must also
	 * be called after consuming a character from the cache.
	 */
	void fill_cache()
	{
		if(UNLIKELY(cache_offset_ >= cache_size_)) {
			/*
			 * This does not only test for the EOF, but also makes sure the
			 * data is available in the buffer. Without it readsome will read
			 * nothing, after its first call, even if the EOF has not been
			 * reached.
			 */
			if(UNLIKELY(in_.rdbuf()->sgetc() == EOF)) {
				read_eof_ = true;
			} else {
				cache_offset_ = 0;
				cache_size_ = in_.readsome(cache_, sizeof(cache_));
			}
		}
	}

	void next_char_fast()
	{
		do {
			if(UNLIKELY(read_eof_)) {
				current_ = EOF;
			} else {
				/*
				 * The data needs to be casted to an unsigned value before it
				 * is promoted to an int. The char might be signed and contain
				 * a negative value, resulting in a negative result, and cause
				 * problems. (Using gcc on x86 has this issue.)
				 */
				current_ = static_cast<unsigned char>(cache_[cache_offset_]);
				++cache_offset_;
				fill_cache();
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
		if(UNLIKELY(read_eof_)) {
			return EOF;
		} else {
			/* See next_char_fast() */
			return static_cast<unsigned char>(cache_[cache_offset_]);
		}
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

	std::string textdomain_;
	std::string file_;
	token token_;
#ifdef DEBUG
	token previous_token_;
#endif
	std::istream &in_;
	char char_types_[128];

	/**
	 * Read cache.
	 *
	 * Reading from @ref std::istream isn't to fast, especially not a byte at a
	 * time. This cache is used to buffer x bytes at a time. The size of the
	 * buffer is determined experimentally.
	 */
	char cache_[1024];

	/**
	 * The size of @ref cache_.
	 *
	 * When buffering the data there might be less data in the stream as in the
	 * buffer. This variable contains the exact size of the buffer.
	 */
	unsigned cache_size_;

	/**
	 * The offset of the current character in the buffer.
	 *
	 * @ref cache_[cache_offset_] is the current character, and can be peaked
	 * or consumed.
	 */
	unsigned cache_offset_;

	/** Is the end of file reached? */
	bool read_eof_;
};

#endif

