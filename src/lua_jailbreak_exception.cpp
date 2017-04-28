/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lua_jailbreak_exception.hpp"

#include <cassert>

lua_jailbreak_exception *lua_jailbreak_exception::jailbreak_exception = nullptr;

void lua_jailbreak_exception::store() const NOEXCEPT
{
	/*
	 * It should not be  possible to call this function with an exception still
	 * pending. It could happen if the code doesn't call
	 * lua_jailbreak_exception::rethrow() or a logic error in the code.
	 */
	assert(!jailbreak_exception);

	jailbreak_exception = this->clone();
}

void lua_jailbreak_exception::rethrow()
{
	if(!jailbreak_exception) {
		return;
	}

	/*
	 * We need to call lua_jailbreak_exception::clear() after the exception
	 * is thrown. The most straightforward approach would be a small helper
	 * class calling clear in its destructor, but alas g++ then complains about
	 * an unused variable. Since we're sure there will be something thrown use
	 * that fact to make sure the the function is called.
	 */
	try {
		jailbreak_exception->execute();
	} catch(...) {
		clear();
		throw;
	}

	/* We never should reach this point. */
	assert(false);
}

void lua_jailbreak_exception::clear() NOEXCEPT
{
	delete jailbreak_exception;
	jailbreak_exception = nullptr;
}

