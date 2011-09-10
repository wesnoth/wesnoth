/* $Id$ */
/*
   Copyright (C) 2010 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED

#include <exception>
#include <string>
#include "token.hpp"

namespace game {

/**
 * Base class for all the errors encountered by the engine.
 * It provides a field for storing custom messages related to the actual
 * error.
 */
struct error : std::exception
{
	std::string message;

	error() : message() {}
	error(const std::string &msg) : message(msg) {}
	~error() throw() {}

	const char *what() const throw()
	{
		return message.c_str();
	}
};

/**
///WML syntax error exception
When parsing wml from a vector of tokens when the location of the error is known an exact 
error message can be thrown.
*/
class wml_syntax_error : public std::exception {
	typedef n_token::t_token t_token;
	typedef std::vector<t_token> t_tokens;
	std::string output_; ///the parsed what string
 public:
	~wml_syntax_error() throw() {}
	///Construct a wml syntax error with the vectors of parsed tokens, the position of the error and a reason for the error.
	wml_syntax_error(t_tokens const & tokens, size_t const & pos, std::string const & reason = "unknown reason");
	///Construct a wml syntax error with the unparsed string the position of the error and a reason for the error.
	wml_syntax_error(std::string const & tokens, size_t const & pos, std::string const & reason = "unknown reason");
	
	wml_syntax_error(std::string const & reason = "unknown reason");
	///Tries to parse the error message so that an arror <-- points to the location of the error
	virtual const char * what() const throw() ;
};


}

#endif
