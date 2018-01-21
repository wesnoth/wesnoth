/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2010 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "global.hpp"
#include "serialization/tokenizer.hpp"
#include "wesconfig.h"

tokenizer::tokenizer(std::istream& in) :
	current_(EOF),
	lineno_(1),
	startlineno_(0),
	textdomain_(PACKAGE),
	file_(),
	token_(),
	in_(in)
{
	for (int c = 0; c < 128; ++c)
	{
		int t = 0;
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
			t = TOK_ALPHA;
		} else if (c >= '0' && c <= '9') {
			t = TOK_NUMERIC;
		} else if (c == ' ' || c == '\t') {
			t = TOK_SPACE;
		}
		char_types_[c] = t;
	}
	in_.stream().exceptions(std::ios_base::badbit);
	next_char_fast();
}

tokenizer::~tokenizer()
{
	in_.stream().clear(std::ios_base::goodbit);
	in_.stream().exceptions(std::ios_base::goodbit);
}

const token &tokenizer::next_token()
{
#if DEBUG_TOKENIZER
	previous_token_ = token_;
#endif
	token_.value.clear();

	// Dump spaces and inlined comments
	for(;;)
	{
		while (is_space(current_)) {
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
			if (current_ == 254) {
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
		FALLTHROUGH;

	default:
		if (is_alnum(current_)) {
			token_.type = token::STRING;
			do {
				token_.value += current_;
				next_char_fast();
				while (current_ == 254) {
					skip_comment();
					next_char_fast();
				}
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

bool tokenizer::skip_command(char const *cmd)
{
	for (; *cmd; ++cmd) {
		next_char_fast();
		if (current_ != *cmd) return false;
	}
	next_char_fast();
	if (!is_space(current_)) return false;
	next_char_fast();
	return true;
}

void tokenizer::skip_comment()
{
	next_char_fast();
	if (current_ == '\n' || current_ == EOF) return;
	std::string *dst = nullptr;

	if (current_ == 't')
	{
		if (!skip_command("extdomain")) goto fail;
		dst = &textdomain_;
	}
	else if (current_ == 'l')
	{
		if (!skip_command("ine")) goto fail;
		lineno_ = 0;
		while (is_num(current_)) {
			lineno_ = lineno_ * 10 + (current_ - '0');
			next_char_fast();
		}
		if (!is_space(current_)) goto fail;
		next_char_fast();
		dst = &file_;
	}
	else
	{
		fail:
		while (current_ != '\n' && current_ != EOF) {
			next_char_fast();
		}
		return;
	}

	dst->clear();
	while (current_ != '\n' && current_ != EOF) {
		*dst += current_;
		next_char_fast();
	}
}
