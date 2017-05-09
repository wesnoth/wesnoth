/*
   Copyright (C) 2015 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "serialization/string_utils.hpp"

#include <stdexcept>
#include <vector>

namespace campaignd
{

/**
 * Represents a server control line written to a communication socket.
 *
 * Control lines are plain text command lines using the ASCII space character
 * (0x20) as command separator. This type is really only used to keep the code
 * pretty.
 */
class control_line
{
public:
	/**
	 * Parses a control line string.
	 */
	control_line(const std::string& str) : args_(utils::split(str, ' '))
	{
		if(args_.empty()) {
			args_.emplace_back();
		}
	}

	/**
	 * Whether the control line is empty.
	 */
	bool empty() const
	{
		// Because of how utils::split() works, this can only happen if there
		// are no other arguments.
		return args_[0].empty();
	}

	/**
	 * Returns the control command.
	 *
	 * Equivalent to calling arg(0).
	 */
	operator const std::string&() const
	{
		return cmd();
	}

	/**
	 * Returns the control command.
	 *
	 * Equivalent to calling arg(0).
	 */
	const std::string& cmd() const
	{
		return args_[0];
	}

	/**
	 * Returns the total number of arguments, not including the command itself.
	 */
	size_t args_count() const
	{
		return args_.size() - 1;
	}

	/**
	 * Returns the nth argument.
	 *
	 * @throws std::out_of_range @a n exceeds args_count().
	 */
	const std::string& operator[](size_t n) const
	{
		return arg(n);
	}

	/**
	 * Returns the nth argument.
	 *
	 * @throws std::out_of_range @a n exceeds args_count().
	 */
	const std::string& arg(size_t n) const
	{
		if(n > args_count()) {
			throw std::out_of_range("control line argument range exceeded");
		}

		return args_[n];
	}

	/**
	 * Return the full command line string.
	 */
	std::string full() const
	{
		return utils::join(args_, " ");
	}

private:
	std::vector<std::string> args_;
};

}  // end namespace campaignd
