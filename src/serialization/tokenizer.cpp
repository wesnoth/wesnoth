/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "tokenizer.hpp"
#include "string_utils.hpp"

#include <iostream>
#include <sstream>

tokenizer::tokenizer(std::istream& in) :
	in_(in),
	lineno_(1)
{
	if(in_.good()) {
		current_ = in_.get();
	} else {
		current_ = EOF;
	}
}

void tokenizer::skip_comment()
{
	// Dump comments up to \n
	std::string comment;
	next_char();
	while (current_ != EOF && current_ != '\n') {
		comment += current_;
		next_char();
	}

	// Identifies and processes tokenizer directives
	std::vector<std::string> comment_line = utils::split(comment, ' ');
	if (comment_line.size() == 2 && comment_line[0] == "textdomain")
		textdomain_ = comment_line[1];
	else if (comment_line.size() > 3 && comment_line[0] == "line") {
		lineno_ = atoi(comment_line[1].c_str());
		comment_line.erase(comment_line.begin(), comment_line.begin() + 2);
		file_ = ' ' + utils::join(comment_line, ' ');
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
			next_char();
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
				next_char();
			if (current_ == 254) {
				skip_comment();
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
				next_char();
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

const token& tokenizer::current_token()
{
	return token_;
}

void tokenizer::next_char()
{
	if (current_ == '\n')
		lineno_++;

	do {
		if(in_.good()) {
			current_ = in_.get();
		} else {
			current_ = EOF;
		}
	} while(current_ == '\r');
}

int tokenizer::peek_char()
{
	return in_.peek();
}

bool tokenizer::is_space(int c)
{
	return c == ' ' || c == '\t';
}

bool tokenizer::is_alnum(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

std::string tokenizer::get_line()
{
	std::ostringstream s;
	s << tokenstart_lineno_ << file_;
	return s.str();
}

std::string& tokenizer::textdomain()
{
	return textdomain_;
}
