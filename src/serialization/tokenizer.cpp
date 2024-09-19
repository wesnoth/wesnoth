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
	for (int c = 0; c < START_EXTENDED_ASCII; ++c)
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

	// skip comments on their own line
	if (current_ == token::POUND)
		skip_comment();

	// set the line number the next token will start on
	startlineno_ = lineno_;

	switch(current_) {
	// we reached the end of the file being read
	case EOF:
		token_.type = token::END;
		break;

	// handle open/closed angle brackets
	// most commonly used for enclosing lua code
	// more generally is used to indicate the preprocessor should skip over a particular block of text
	case token::LEFT_ANGLE_BRACKET:
		// if there aren't double left angle brackets, there is no extra handling needed - this is just a regular left angle bracket
		if (peek_char() != token::LEFT_ANGLE_BRACKET) {
			token_.type = token::MISC;
			token_.value += current_;
			break;
		}

		// else, treat this like a quoted string
		token_.type = token::QSTRING;
		next_char_fast();

		// keep getting characters and appending them to the current token's value until either the file ends or double right angle brackets are found
		// finding the end of the file first is an error since double left angle brackets must always be closed by double right angle brackets
		for (;;) {
			next_char();
			if (current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			} else if (current_ == token::RIGHT_ANGLE_BRACKET && peek_char() == token::RIGHT_ANGLE_BRACKET) {
				next_char_fast();
				break;
			}
			token_.value += current_;
		}
		break;

	// very similar to the double left+right angle bracket handling
	// the main difference is the need to handle INLINED_PREPROCESS_DIRECTIVE_CHAR since double quotes don't affect the preprocessor
	case token::DOUBLE_QUOTE:
		token_.type = token::QSTRING;

		for (;;) {
			next_char();
			if (current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			} else if (current_ == token::DOUBLE_QUOTE) {
				if (peek_char() != token::DOUBLE_QUOTE) {
					break;
				} else {
					next_char_fast();
				}
			}

			// ignore this line and decrement the current line number
			if (current_ == INLINED_PREPROCESS_DIRECTIVE_CHAR) {
				skip_comment();
				--lineno_;
				continue;
			}

			token_.value += current_;
		}
		break;

	// tag name delimiters
	case token::OPEN_BRACKET:
	case token::CLOSE_BRACKET:
	// closing tag
	case token::SLASH:
	case token::NEWLINE:
	case token::EQUALS:
	// handles multiple attributes on the same line
	// ie: x,y = 5,5
	case token::COMMA:
	// tag merge aka node append
	case token::PLUS:
		token_.type = static_cast<token::token_type>(current_);
		token_.value = current_;
		break;

	// when in front of a QSTRING, indicates that the string is translatable
	case token::UNDERSCORE:
		// this check seems off - there are certainly other non-alphanumeric characters that shouldn't mean anything - but it looks like the parser handles those cases
		if (!is_alnum(peek_char())) {
			token_.type = token::UNDERSCORE;
			token_.value = current_;
			break;
		}
		[[fallthrough]];

	// everything else
	default:
		// if alphanumeric (regular text) or the dollar sign (variable)
		// not quite sure how this works with non-ascii text particularly since the parser doesn't reference token_type::MISC
		// but maybe the default handling does what's needed
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

	// if this isn't the end of the file, get the next character in preparation for the next call to this method
	if (current_ != EOF) {
		next_char();
	}

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
