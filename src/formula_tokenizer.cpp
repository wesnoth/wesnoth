/*
   Copyright (C) 2007 - 2015 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <sstream>

#include "formula_tokenizer.hpp"

namespace formula_tokenizer
{

namespace {

void raise_exception(iterator& i1, iterator i2, std::string str) {
	std::ostringstream expr;
	while( (i1 != i2) && (*i1 != '\n') ) {
		if( (*i1 != '\t') )
			expr << *i1;
		++i1;
	}

	if( str.empty() )
		throw token_error("Unrecognized token", expr.str() );
	else
		throw token_error(str, expr.str() );
}

}

token get_token(iterator& i1, iterator i2) {

	iterator it = i1;
	if( *i1 >= 'A' ) {
		//current character is >= 'A', limit search to the upper-half of the ASCII table

		// check if we parse now TOKEN_IDENTIFIER or TOKEN_OPERATOR/KEYWORD based on string
		if( *i1 <= 'Z' || ( *i1 >= 'a' && *it <= 'z' ) || *i1 == '_' ) {

			while( i1 != i2 && ( ( *i1 >= 'a' && *i1 <= 'z' ) || *i1 == '_' || ( *i1 >= 'A' && *i1 <= 'Z' ) ) )
				++i1;

			int diff = i1 - it;
			TOKEN_TYPE t = TOKEN_IDENTIFIER;

			//check if this string matches any keyword or an operator
			//possible operators and keywords:
			// d, or, def, and, not, fai, where, faiend, functions
			if( diff == 1 ) {
				if( *it == 'd' )
					t = TOKEN_OPERATOR;
			} else if( diff == 2 ) {
				if( *it == 'o' && *(it+1) == 'r' )
					t = TOKEN_OPERATOR;
			} else if( diff == 3 ) {
				if( *it == 'd' ) { //def
					if( *(it+1) == 'e' && *(it+2) == 'f' )
						t = TOKEN_KEYWORD;
				} else if( *it == 'a' ) { //and
					if( *(it+1) == 'n' && *(it+2) == 'd' )
						t = TOKEN_OPERATOR;
				} else if( *it == 'n' ) { //not
					if( *(it+1) == 'o' && *(it+2) == 't' )
						t = TOKEN_OPERATOR;
				} else if( *it == 'f' ) { //fai
					if( *(it+1) == 'a' && *(it+2) == 'i' )
						t = TOKEN_KEYWORD;
				}
			} else if( diff == 5 ) {
				std::string s(it, i1);
				if( s == "where" )
					t = TOKEN_OPERATOR;
			} else if( diff == 6 ) {
				std::string s(it, i1);
				if( s == "faiend" )
					t = TOKEN_KEYWORD;
			} else if( diff == 9 ) {
				std::string s(it, i1);
				if( s == "functions" )
					t = TOKEN_KEYWORD;
			}

			return token( it, i1, t);
		} else {
			//at this point only 3 chars left to check:
			if( *i1 == '[' )
				return token( it, ++i1, TOKEN_LSQUARE );

			if( *i1 == ']' )
				return token( it, ++i1, TOKEN_RSQUARE );

			if( *i1 == '^' )
				return token( it, ++i1, TOKEN_OPERATOR );

		}
	} else {
		//limit search to the lower-half of the ASCII table
		//start by checking for whitespaces/end of line char
		if( *i1 <= ' ' ) {
			if( *i1 == '\n' ) {
				return token( it, ++i1, TOKEN_EOL);
			} else {

				while( i1 != i2 && *i1 <= ' ' && *i1 != '\n' )
					++i1;

				return token( it, i1, TOKEN_WHITESPACE );
			}
		//try to further limit number of characters that we need to check:
		} else if ( *i1 >= '0' ){
			//current character is between '0' and '@'
			if( *i1 <= '9' ) {
				//we parse integer or decimal number
				++i1;
				bool dot = false;

				while( i1 != i2 ) {
					if( *i1 >= '0' && *i1 <= '9' ) {
						//do nothing
					} else {
						//look for '.' in case of decimal number
						if( *i1 == '.' ) {
							//allow only one dot in such expression
							if( !dot )
								dot = true;
							else
								raise_exception(it, i2, "Multiple dots near decimal expression");
						} else
							break;
					}
					++i1;
				}

				if( dot )
					return token( it, i1, TOKEN_DECIMAL );
				else
					return token( it, i1, TOKEN_INTEGER );

			} else {
				//current character is between ':' and '@'
				//possible tokens at this point that we are interested in:
				// ; < = > <= >=

				if( *i1 == ';' ) {
					return token( it, ++i1, TOKEN_SEMICOLON);
				} else if( *i1 == '=' ) {
					return token( it, ++i1, TOKEN_OPERATOR);
				} else if( *i1 == '<' ) {
					++i1;
					if( i1 != i2 ) {
						if( *i1 == '=' )
							return token( it, ++i1, TOKEN_OPERATOR);
						else
							return token( it, i1, TOKEN_OPERATOR);
					} else
						return token( it, i1, TOKEN_OPERATOR);
				} else if( *i1 == '>' ) {
					++i1;
					if( i1 != i2 ) {
						if( *i1 == '=' )
							return token( it, ++i1, TOKEN_OPERATOR);
						else
							return token( it, i1, TOKEN_OPERATOR);
					} else
						return token( it, i1, TOKEN_OPERATOR);
				}
			}
		//current character is between '!' and '/'
		} else if ( *i1 == ',' ) {
			return token( it, ++i1, TOKEN_COMMA);

		} else if ( *i1 == '.' ) {
			++i1;

			if( i1 != i2 ) {
				if( *i1 == '+' || *i1 == '-' || *i1 == '*' || *i1 == '/')
					return token( it, ++i1, TOKEN_OPERATOR );
				else
					return token( it, i1, TOKEN_OPERATOR );
			} else {
				return token( it, i1, TOKEN_OPERATOR);
			}

		} else if ( *i1 == '(' ) {
			return token( it, ++i1, TOKEN_LPARENS);

		} else if ( *i1 == ')' ) {
			return token( it, ++i1, TOKEN_RPARENS);

		} else if ( *i1 == '\'' ) {
			++i1;
			while( i1 != i2 && *i1 != '\'' )
				++i1;

			if( i1 != i2 ) {
				return token( it, ++i1, TOKEN_STRING_LITERAL );
			} else {
				raise_exception(it, i2, "Missing closing ' for formula string");
			}

		} else if ( *i1 == '#' ) {
			++i1;
			while( i1 != i2 && *i1 != '#' )
				++i1;

			if( i1 != i2 ) {
				return token( it, ++i1, TOKEN_COMMENT );
			} else {
				raise_exception(it, i2, "Missing closing # for formula comment");
			}

		} else if ( *i1 == '+' ) {
			return token( it, ++i1, TOKEN_OPERATOR);

		} else if ( *i1 == '-' ) {
			++i1;

			if( i1 != i2 ) {
				if( *i1 == '>' )
					return token( it, ++i1, TOKEN_POINTER );
				else
					return token( it, i1, TOKEN_OPERATOR );
			} else {
				return token( it, i1, TOKEN_OPERATOR);
			}

		} else if ( *i1 == '*' ) {
			return token( it, ++i1, TOKEN_OPERATOR);

		} else if ( *i1 == '/' ) {
			return token( it, ++i1, TOKEN_OPERATOR);

		} else if ( *i1 == '%' ) {
			return token( it, ++i1, TOKEN_OPERATOR);

		} else if ( *i1 == '!' ) {
			++i1;
			if( *i1 == '=' )
				return token( it, ++i1, TOKEN_OPERATOR);
			else
				raise_exception(it, i2, std::string() );
		}
	}
	raise_exception(it, i2, std::string() );
	return token();
}

}

#ifdef UNIT_TEST_TOKENIZER

int main()
{
	using namespace formula_tokenizer;
	std::string test = "(abc + 4 * (5+3))^2";
	std::string::const_iterator i1 = test.begin();
	std::string::const_iterator i2 = test.end();
	TOKEN_TYPE types[] = {TOKEN_LPARENS, TOKEN_IDENTIFIER,
	                      TOKEN_WHITESPACE, TOKEN_OPERATOR,
						  TOKEN_WHITESPACE, TOKEN_INTEGER,
						  TOKEN_WHITESPACE, TOKEN_OPERATOR,
						  TOKEN_WHITESPACE, TOKEN_LPARENS,
						  TOKEN_INTEGER, TOKEN_OPERATOR,
						  TOKEN_INTEGER, TOKEN_RPARENS,
						  TOKEN_RPARENS, TOKEN_KEYWORD,
	                      TOKEN_OPERATOR, TOKEN_INTEGER};
	std::string tokens[] = {"(", "abc", " ", "+", " ", "4", " ",
	                        "*", " ", "(", "5", "+", "3", ")", ")", "functions"};
	for(int n = 0; n != sizeof(types)/sizeof(*types); ++n) {
		token t = get_token(i1,i2);
		assert(std::string(t.begin,t.end) == tokens[n]);
		assert(t.type == types[n]);

	}
	return 0;
}

#endif
