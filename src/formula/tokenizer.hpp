/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <string>

namespace wfl
{
namespace tokenizer
{

typedef std::string::const_iterator iterator;

enum class token_type {
	operator_token, // Cannot simply be named 'operator' since that's a reserved C++ keyword
	string_literal,
	identifier,
	integer,
	decimal,
	lparens,
	rparens,
	lsquare,
	rsquare,
	comma,
	semicolon,
	whitespace,
	eol,
	keyword,
	comment,
	pointer
};

struct token {

	token() :
		type(token_type::comment),
		begin(),
		end(),
		line_number(1),
		filename()
	{
	}

	token(iterator& i1, iterator i2, token_type type) :
		type(type),
		begin(i1),
		end(i2),
		line_number(1),
		filename()
	{
	}

	token_type type;
	iterator begin, end;
	int line_number;
	const std::string* filename;
};

token get_token(iterator& i1, iterator i2);

struct token_error
{
	token_error() : description_(), formula_() {}
	token_error(const std::string& dsc, const std::string& formula) : description_(dsc), formula_(formula) {}
	std::string description_;
	std::string formula_;
};

}

}
