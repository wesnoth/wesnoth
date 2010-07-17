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

/** @file serialization/tokenizer.cpp */

#include "global.hpp"

#include "wesconfig.h"
#include "serialization/tokenizer.hpp"


tokenizer::tokenizer(std::istream& in) :
	current_(EOF),
	lineno_(1),
	startlineno_(0),
	textdomain_(PACKAGE),
	file_(),
	token_(),
	in_(in)
{
	in_.exceptions(std::ios_base::badbit);
	next_char_fast();
}

tokenizer::~tokenizer()
{
	in_.clear(std::ios_base::goodbit);
	in_.exceptions(std::ios_base::goodbit);
}

const token &tokenizer::next_token()
{
#if DEBUG
	previous_token_ = token_;
#endif
	token_.reset();

	// Dump spaces and inlined comments
	for(;;)
	{
		while (is_space(current_)) {
			token_.leading_spaces += current_;
			next_char_fast();
		}
		if (current_ != 254)
			break;
		skip_comment();
		// skip the line end
		next_char_fast();
	}

	if (current_ == '#')
		skip_comment();

	startlineno_ = lineno_;

	switch(current_) {
	case EOF:
		token_.type = token::END;
		break;

	case '<':
		if (peek_char() != '<') {
			token_.type = token::MISC;
			token_.value += current_;
			break;
		}
		token_.type = token::QSTRING;
		next_char_fast();
		for (;;) {
			next_char();
			if (current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			}
			if (current_ == '>' && peek_char() == '>') {
				next_char_fast();
				break;
			}
			token_.value += current_;
		}
		break;

	case '"':
		token_.type = token::QSTRING;
		for (;;) {
			next_char();
			if (current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			}
			if (current_ == '"') {
				if (peek_char() != '"') break;
				next_char_fast();
			}
			if (current_ == 254 ) {
				skip_comment();
				--lineno_;
				continue;
			}
			token_.value += current_;
		}
		break;

	case '[': case ']': case '/': case '\n': case '=': case ',': case '+':
		token_.type = token::token_type(current_);
		token_.value = current_;
		break;

	case '_':
		if (!is_alnum(peek_char())) {
			token_.type = token::token_type(current_);
			token_.value = current_;
			break;
		}
		// no break

	default:
		if (is_alnum(current_)) {
			token_.type = token::STRING;
			do {
				token_.value += current_;
				next_char_fast();
			} while (is_alnum(current_));
		} else {
			token_.type = token::MISC;
			token_.value += current_;
			next_char();
		}
		return token_;
	}

	if (current_ != EOF)
		next_char();

	return token_;
}

void tokenizer::skip_comment()
{
	next_char_fast();
	if (current_ == '\n' || current_ == EOF) return;

	if (current_ == 't')
	{
		// When the string 'textdomain[ |\t] is matched the rest of the line is
		// the textdomain to switch to. If we at any point fail to match we break
		// out of the loop and eat the rest of the line without testing.
		size_t i = 0;
		static const std::string match = "extdomain";
		next_char_fast();
		while (current_ != '\n' && current_ != EOF) {
			if (i < 9) {
				if (current_ != match[i]) break;
				++i;
			} else if (i == 9) {
				if (current_ != ' ' && current_ != '\t') break;
				++i;
				textdomain_.clear();
			} else {
				textdomain_ += current_;
			}
			next_char_fast();
		}
		while (current_ != '\n' && current_ != EOF) {
			next_char_fast();
		}
	}
	else if (current_ == 'l')
	{
		// Basically the same as textdomain but we match 'line[ |\t]d*[ |\t]s*
		// d* is the line number
		// s* is the file name
		// It inherited the * instead of + from the previous implementation.
		size_t i = 0;
		static const std::string match = "ine";
		next_char_fast();
		bool found = false;
		std::string lineno;
		while (current_ != '\n' && current_ != EOF) {
			if (i < 3) {
				if (current_ != match[i]) break;
				++i;
			} else if(i == 3) {
				if (current_ != ' ' && current_ != '\t') break;
				++i;
			} else {
				if (!found) {
					if (current_ == ' ' || current_ == '\t') {
						found = true;
						lineno_ = lexical_cast<int>(lineno);
						file_.clear();
					} else {
						lineno += current_;
					}
				} else {
					file_ += current_;
				}
			}
			next_char_fast();
		}
		while (current_ != '\n' && current_ != EOF) {
			next_char_fast();
		}
	}
	else
	{
		// Neither a textdomain or line comment skip it.
		while (current_ != '\n' && current_ != EOF) {
			next_char_fast();
		}
	}
}
