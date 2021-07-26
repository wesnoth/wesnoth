/*
	Copyright (C) 2015 - 2021
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

#include <exception>
#include <cerrno>
#include <cstring>
#include <string>

/**
 * Exception type used to propagate C runtime errors across functions.
 */
class libc_error : public std::exception
{
public:
	libc_error()
		: e_(errno)
		, desc_(strerror(e_))
		, msg_("C library error: " + desc_)
	{
	}

	virtual ~libc_error() noexcept
	{
	}

	/** Returns the value of @a errno at the time the exception was thrown. */
	int num() const
	{
		return e_;
	}

	/** Returns an explanatory string describing the runtime error alone. */
	const std::string& desc() const
	{
		return desc_;
	}

	/** Returns an explanatory string describing the exception. */
	const char* what() const noexcept
	{
		return msg_.c_str();
	}

private:
	int e_;
	std::string desc_;
	std::string msg_;
};
