/* $Id$ */
/*
   Copyright (C) 2004 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/tokenizer.cpp 
//!

#include "global.hpp"

#include "util.hpp"
#include "serialization/tokenizer.hpp"
#include "serialization/string_utils.hpp"

#include <iostream>
#include <sstream>

tokenizer::tokenizer(std::istream& in) :
	current_(EOF),
	lineno_(1),
	textdomain_(),
	file_(),
	tokenstart_lineno_(),
	token_(),
	in_(in)
{
	next_char_fast();
}

void tokenizer::skip_comment()
{
	next_char_fast();
	if(current_ != '\n' && current_ != EOF) {
		if(current_ == 't') {
			// When the string 'textdomain[ |\t] is matched the rest of the line is
			// the textdomain to switch to. If we at any point fail to match we break
			// out of the loop and eat the rest of the line without testing.
			size_t i = 0;
			static const std::string match = "extdomain";
			this->next_char_fast();
			while(current_ != '\n' && current_ != EOF) {
				if(i < 9) {
					if(current_ != match[i]) {
						break;
					}
					++i;
				} else if(i == 9) { 
					if(current_ != ' ' && current_ != '\t') {
						break;
					}
					++i;
					textdomain_ = "";
				} else {
					textdomain_ += current_;
				}
				this->next_char_fast();
			}
			while(current_ != '\n' && current_ != EOF) {
				this->next_char_fast();
			}

		} else if(current_ == 'l') {
			// Basically the same as textdomain but we match 'line[ |\t]d*[ |\t]s*
			// d* is the line number 
			// s* is the file name
			// It inherited the * instead of + from the previous implementation.
			size_t i = 0;
			static const std::string match = "ine";
			this->next_char_fast();
			bool found = false;
			std::string lineno;
			while(current_ != '\n' && current_ != EOF) {
				if(i < 3) {
					if(current_ != match[i]) {
						break;
					}
					++i;
				} else if(i == 3) { 
					if(current_ != ' ' && current_ != '\t') {
						break;
					}
					++i;
				} else {
					if(!found) {
						if(current_ == ' ' || current_ == '\t') {
							found = true;
							lineno_ = lexical_cast<size_t>(lineno);
							file_ = "";
						} else {
							lineno += current_;
						}
					} else {
						file_ += current_;
					}
				}
				this->next_char_fast();
			}
			while(current_ != '\n' && current_ != EOF) {
				this->next_char_fast();
			}
		} else {
			// Neither a textdomain or line comment skip it.
			while(current_ != '\n' && current_ != EOF) {
				this->next_char_fast();
			}
		}
	}
}

const token& tokenizer::next_token()
{
	token_.value = "";
	token_.leading_spaces = "";

	// Dump spaces and inlined comments
	for(;;) {
		while (is_space(current_)) {
			token_.leading_spaces += current_;
			next_char_fast();
		}
		if (current_ != 254)
			break;
		skip_comment();
		--lineno_;
		next_char();
	}

	if (current_ == '#')
		skip_comment();

	tokenstart_lineno_ = lineno_;

	switch(current_) {
	case EOF:
		token_.type = token::END;
		break;
	case '"':
		token_.type = token::QSTRING;
		while (1) {
			next_char();

			if(current_ == EOF) {
				token_.type = token::UNTERMINATED_QSTRING;
				break;
			}
			if(current_ == '"' && peek_char() != '"')
				break;
			if(current_ == '"' && peek_char() == '"')
				next_char_fast();
			if (current_ == 254) {
				skip_comment();
				--lineno_;
				continue;
			}

			token_.value += current_;
		};
		break;
	case '[': case ']': case '/': case '\n': case '=': case ',': case '+':
		token_.type = token::token_type(current_);
		token_.value = current_;
		break;
	default:
		if(is_alnum(current_)) {
			token_.type = token::STRING;
			token_.value += current_;
			while(is_alnum(peek_char())) {
				next_char_fast();
				token_.value += current_;
			}
		} else {
			token_.type = token::MISC;
			token_.value += current_;
		}
		if(token_.value == "_")
			token_.type = token::token_type('_');
	}

	if(current_ != EOF)
		next_char();

	return token_;
}

const token& tokenizer::current_token() const
{
	return token_;
}


bool tokenizer::is_space(const int c) const
{
	return c == ' ' || c == '\t';
}

bool tokenizer::is_alnum(const int c) const
{
	return (c >= 'a' && c <= 'z') 
		|| (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

std::string tokenizer::get_line() const
{
	std::ostringstream s;
	s << tokenstart_lineno_ << ' ' << file_;
	return s.str();
}

std::string& tokenizer::textdomain()
{
	return textdomain_;
}



