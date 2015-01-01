/*
   Copyright (C) 2011 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Contains the exceptions throw by the @ref gui2::iterator::titerator classes.
 */

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_EXCEPTION_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_EXCEPTION_HPP_INCLUDED

#include "lua_jailbreak_exception.hpp"

#include <stdexcept>
#include <string>

namespace gui2
{

namespace iterator
{

/**
 * Thrown when deferring an invalid iterator.
 *
 * Invalid means the initial state at_end() == true.
 */
class tlogic_error : public std::logic_error, public tlua_jailbreak_exception
{
public:
	explicit tlogic_error(const std::string& message)
		: std::logic_error("GUI2 ITERATOR: " + message)
		, tlua_jailbreak_exception()
	{
	}

private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(tlogic_error)
};

/**
 * Thrown when moving an invalid iterator.
 *
 * Invalid means the initial state at_end() == true.
 */
class trange_error : public std::range_error, public tlua_jailbreak_exception
{
public:
	explicit trange_error(const std::string& message)
		: std::range_error("GUI2 ITERATOR: " + message)
		, tlua_jailbreak_exception()
	{
	}

private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(trange_error)
};

} // namespace iterator

} // namespace gui2

#endif
