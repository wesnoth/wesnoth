/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_TOKENIZER_HPP_INCLUDED
#define FORMULA_TOKENIZER_HPP_INCLUDED

#include <string>

namespace formula_tokenizer
{

typedef std::string::const_iterator iterator;

enum TOKEN_TYPE { TOKEN_OPERATOR, TOKEN_STRING_LITERAL,
		          TOKEN_IDENTIFIER, TOKEN_INTEGER,
                  TOKEN_LPARENS, TOKEN_RPARENS,
				  TOKEN_LSQUARE, TOKEN_RSQUARE, TOKEN_COMMA,
				  TOKEN_WHITESPACE, TOKEN_KEYWORD };

struct token {
	TOKEN_TYPE type;
	iterator begin, end;
};

token get_token(iterator& i1, iterator i2);

struct token_error {};

}

#endif
