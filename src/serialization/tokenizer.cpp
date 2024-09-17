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
	for (int c = 0; c < std::numeric_limits<unsigned char>::max(); ++c)
	{
		token_category t = TOK_NONE;
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
#ifdef DEBUG_TOKENIZER
	previous_token_ = token_;
#endif
	token_.value.clear();

	// Dump spaces and inlined comments
	while(true)
	{
		while (is_space(current_)) {
			next_char_fast();
		}
		if (current_ != INLINED_PREPROCESS_DIRECTIVE_CHAR)
			break;
		skip_comment();
		// skip the line end
		next_char_fast();
	}

	if (current_ == token::POUND)
		skip_comment();

	startlineno_ = lineno_;

	switch(current_) {
	case EOF:
		token_.type = token::END;
		break;

	case token::LESS_THAN:
		if (peek_char() != token::LESS_THAN) {
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
			if (current_ == token::GREATER_THAN && peek_char() == token::GREATER_THAN) {
				next_char_fast();
				break;
			}
			token_.value += current_;
		}
		break;

	case token::DOUBLE_QUOTE:
		token_.type = token::QSTRING;
		for (;;) {
			next_char();
			if (current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			}
			if (current_ == token::DOUBLE_QUOTE) {
				if (peek_char() != token::DOUBLE_QUOTE) break;
				next_char_fast();
			}
			if (current_ == INLINED_PREPROCESS_DIRECTIVE_CHAR) {
				skip_comment();
				--lineno_;
				continue;
			}
			token_.value += current_;
		}
		break;

	case token::OPEN_BRACKET:
	case token::CLOSE_BRACKET:
	case token::SLASH:
	case token::NEWLINE:
	case token::EQUALS:
	case token::COMMA:
	case token::PLUS:
		token_.type = token::token_type(current_);
		token_.value = current_;
		break;

	case token::UNDERSCORE:
		if (!is_alnum(peek_char())) {
			token_.type = token::token_type(current_);
			token_.value = current_;
			break;
		}
		[[fallthrough]];

	default:
		if (is_alnum(current_) || current_ == token::DOLLAR) {
			token_.type = token::STRING;
			do {
				token_.value += current_;
				next_char_fast();
				while (current_ == INLINED_PREPROCESS_DIRECTIVE_CHAR) {
					skip_comment();
					next_char_fast();
				}
			} while (is_alnum(current_) || current_ == token::DOLLAR);
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
	if (current_ == token::NEWLINE || current_ == EOF) return;
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
		while (current_ != token::NEWLINE && current_ != EOF) {
			next_char_fast();
		}
		return;
	}

	dst->clear();
	while (current_ != token::NEWLINE && current_ != EOF) {
		*dst += current_;
		next_char_fast();
	}
}
