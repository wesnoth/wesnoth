/* $Id$ */
/*
   Copyright (C) 2004 - 2010 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file serialization/tokenizer.hpp */

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
		leading_spaces(),
		value()
		{}

	enum token_type {
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
	} type;

	void reset() {
		value.clear();
		leading_spaces.clear();
	}

	std::string leading_spaces;
	std::string value;
};

/** Abstract baseclass for the tokenizer. */
class tokenizer
{
	public:
		tokenizer(std::istream& in);
		~tokenizer();

		const token& next_token()
		{
#if DEBUG
			previous_token_ = token_;
#endif
			token_.reset();

			// Dump spaces and inlined comments
			for(;;) {
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
					while (true) {
						next_char();
						if(current_ == EOF) {
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
						if (current_ == 254 ) {
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
				case '_':
					if (!is_alnum(peek_char())) {
						token_.type = token::token_type(current_);
						token_.value = current_;
						break;
					}
					// no break
				default:
					if(is_alnum(current_)) {
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

			if(current_ != EOF)
				next_char();

			return token_;
		}

		const token& current_token() const
		{
			return token_;
		}
#ifdef DEBUG
		const token& previous_token() const;
#endif

		std::string& textdomain()
		{
			return textdomain_;
		}

		const std::string& get_file() const
		{
			return file_;
		}

		int get_start_line() const
		{
			return startlineno_;
		}

	protected:
		tokenizer();
		int current_;
		size_t lineno_;
		size_t startlineno_;

		inline void next_char()
		{
			if (UNLIKELY(current_ == '\n'))
				lineno_++;
			this->next_char_fast();
		}

		inline void next_char_fast()
		{
			do {
				if (LIKELY(in_.good()))
				{
					current_ = in_.get();
				}
				else
				{
					current_ = EOF;
					return;
				}
			}while (UNLIKELY(current_ == '\r'));
#if 0
			// @todo: disabled untill campaign server is fixed
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

		inline int peek_char() const
		{
			return in_.peek();
		}

	private:
		bool is_space(const int c) const
		{
			return c == ' ' || c == '\t';
		}
		bool is_alnum(const int c) const
		{
			return (c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
		}
		void skip_comment()
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

		std::string textdomain_;
		std::string file_;
		token token_;
#ifdef DEBUG
		token previous_token_;
#endif
		std::istream& in_;
};

#endif

