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

#include "tokenizer.hpp"
#include "string_utils.hpp"
#include "filesystem.hpp"

#include <iostream>
#include <sstream>

tokenizer::tokenizer(std::istream& in) :
	in_(in),
	lineno_(0),
	colno_(0)
{
	if(in_.good()) {
		current_ = in_.get();
	} else {
		current_ = EOF;
	}
}

const token& tokenizer::next_token()
{
	tokenstart_lineno_ = lineno_;
	tokenstart_colno_ = colno_;
	token_.value = "";
	token_.leading_spaces = "";

	// Dump spaces
	while(is_space(current_)) {
		token_.leading_spaces += current_;
		next_char();
	}

	// Dump comments up to \n
	if(current_ == '#') {
		std::string comment;
		do {
			comment += current_;
			next_char();
		} while(current_ != EOF && current_ != '\n');

		// Identifies and processes tokenizer directives
		std::vector<std::string> comment_line = utils::split(comment, ' ');
		if ((comment_line.size() == 2 || comment_line.size() == 3)
				&& comment_line[0] == "#textdomain") {
			textdomain_ = comment_line[1];
			std::string path;
			if (comment_line.size() == 3)
				path = comment_line[2];
			textdomain_init(textdomain_, path);
		}
	} 

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
	if(current_ == '\n') {
		colno_ = 0;
		lineno_++;
	} else {
		colno_++;
	}

	if(in_.good()) {
		current_ = in_.get();
	} else {
		current_ = EOF;
	}
}

int tokenizer::peek_char()
{
	return in_.peek();
}

bool tokenizer::is_space(int c)
{
	return c == ' ' || c == '\t' || c == '\r';
}

bool tokenizer::is_alnum(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

const size_t tokenizer::get_line()
{
	return tokenstart_lineno_;
}

const size_t tokenizer::get_column()
{
	return tokenstart_colno_;
}

std::string& tokenizer::textdomain()
{
	return textdomain_;
}

void tokenizer::textdomain_init(const std::string& domain, const std::string& path)
{
	bindtextdomain(domain.c_str(), path.empty() ? get_intl_dir().c_str() : path.c_str());
	bind_textdomain_codeset(domain.c_str(), "UTF-8");
}
