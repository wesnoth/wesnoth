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

/** @file serialization/tokenizer.cpp */

#include "global.hpp"

#include "wesconfig.h"
#include "serialization/tokenizer.hpp"


tokenizer::tokenizer(std::istream& in) :
	current_(EOF),
	lineno_(1),
	startlineno_(0),
	textdomain_(PACKAGE),
	file_(),
	token_(),
	in_(in)
{
	in_.exceptions(std::ios_base::badbit);
	next_char_fast();
}

tokenizer::~tokenizer()
{
	in_.clear(std::ios_base::goodbit);
	in_.exceptions(std::ios_base::goodbit);
}

#ifdef DEBUG
const token& tokenizer::previous_token() const
{
	return previous_token_;
}
#endif

