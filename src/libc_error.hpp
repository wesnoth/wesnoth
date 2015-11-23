/*
   By Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   The contents of this file are placed in the public domain.
 */

#include <cerrno>
#include <cstring>
#include <string>

#ifndef LIBC_ERROR_HPP_INCLUDED
#define LIBC_ERROR_HPP_INCLUDED

/**
 * Exception type used to propagate C runtime errors across functions.
 */
class libc_error
{
public:
	libc_error(): e_(errno), msg_(strerror(e_))
	{
	}

	/** Returns the value of @a errno at the time the exception was thrown. */
	int num() const
	{
		return e_;
	}

	/** Returns an explanatory string describing the runtime error. */
	const std::string& desc() const
	{
		return msg_;
	}

private:
	int e_;
	std::string msg_;
};

#endif
