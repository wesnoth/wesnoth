/* $Id$ */
/*
   Copyright (C) 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED

#include <exception>
#include <string>

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
 * Base class for all the exceptions for changing the control flow.
 * Its message only carries a description of the exception.
 * It also handles sticky exceptions that are automatically rethrown if
 * lost; such exceptions cannot have any embedded payload, since it would
 * still be lost.
 */
struct exception : std::exception
{
	const char *message;

	/**
	 * Rethrows the current sticky exception, if any.
	 */
	static void rethrow();

	/**
	 * Marks an exception of name @a sticky as a rethrow candidate.
	 * @note The value should be set to NULL in order to discard the
	 *       sticky exception once it has been handled.
	 */
	static const char *sticky;

	exception(const char *msg, const char *stick = NULL) : message(msg)
	{
		sticky = stick;
	}

	~exception() throw() {}

	const char *what() const throw()
	{
		return message;
	}
};

}

#endif
