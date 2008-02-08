/* $Id$ */
/*
   Copyright (C) 2004 - 2007 by Philippe Plantier <ayin@anathas.org>
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
#include <list>

tokenizer::tokenizer() :
	current_(EOF),
	lineno_(1),
	textdomain_(),
	file_(),
	tokenstart_lineno_(),
	token_()
{
}
const size_t matching_comments = 2;
const std::string comment[] = {"textdomain","line"};

void tokenizer::skip_comment()
{
	std::list<int> matching;
	std::list<int>::iterator index;
	size_t n;
	for (n = 0; n < matching_comments; ++n)
	{
		matching.push_back(n);
	}
	n = 0;
	this->next_char_fast();
 	while (current_ != '\n' && current_ != EOF) {
		for (index = matching.begin(); index != matching.end();)
		{
			if(UNLIKELY(comment[*index][n] != static_cast<unsigned char>(current_)))
			{
				index = matching.erase(index);
			}
			else
			{
				if (n+1 == comment[*index].size())
				{
					// We have a match
					switch(*index)
					{
					case 0:
						do {
							this->next_char_fast();
						} while (current_ == ' ' || current_ == '\t');
						textdomain_ = "";
						while(current_ != '\n' && current_ != EOF)
						{
							textdomain_ += current_;
							this->next_char_fast();
						}
						std::cerr << textdomain_ << " ";
						return;
					case 1:
						do {
							this->next_char_fast();
						} while (current_ == ' ' || current_ == '\t');
						std::string lineno;
						while(current_ != '\n' && current_ != EOF)
						{
							if (UNLIKELY(current_ == '\n') || UNLIKELY(current_ == EOF))
							{
								return;
							}
							if (UNLIKELY(current_ == ' ') || UNLIKELY(current_ == '\t'))
							{
								break;
							}
							lineno += current_;
							this->next_char_fast();
						}


						do {
							this->next_char_fast();
						} while (current_ == ' ' || current_ == '\t');
						file_ = "";
						while (current_ != '\n' && current_ != EOF)
						{
							file_ += current_;
							this->next_char_fast();
						}
						lineno_ = lexical_cast<size_t>(lineno);
						std::cerr << lineno_ << " " << file_ << " ";

						return;
					}
				}
				++index;
			}
		}
		++n;
		if (UNLIKELY(!matching.empty()))
		{
			break;
		}
		this->next_char_fast();
 	}
 
	while (current_ != '\n' && current_ != EOF)
	{
		this->next_char_fast();
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
			this->next_char_fast();
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
				this->next_char_fast();
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
				this->next_char_fast();
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


tokenizer_string::tokenizer_string(std::string& in) :
	in_(in),
	offset_(0)
{
	this->next_char_fast();
}


tokenizer_stream::tokenizer_stream(std::istream& in) :
	in_(in)
{
	this->next_char_fast();
}

void tokenizer_stream::next_char_fast()
{
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
}

int tokenizer_stream::peek_char() const
{
	return in_.peek();
}


void tokenizer_string::next_char_fast()
{

	if(LIKELY(offset_ < in_.size())) {
		current_ = in_[offset_++];
		if (UNLIKELY(current_ == '\r'))
		{
			if(LIKELY(offset_ < in_.size())) {
				current_ = in_[offset_++];
			} else {
				current_ = EOF;
			}
		}
	} else {
		current_ = EOF;
	}
	
}

int tokenizer_string::peek_char() const
{
	return in_[offset_];
}

