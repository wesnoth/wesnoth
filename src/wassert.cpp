/*
   Copyright (C) 2005 by Isaac Clerencia <isaac@sindominio.net>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef STANDARD_ASSERT_DOES_NOT_WORK

#include "log.hpp"

#include <iostream>

void wassert(bool expression)
{
	// crash if expression is false
	if(! expression) {
		LOG_STREAM(err, general) << "Assertion failure" << "\n";
		*reinterpret_cast<int*>(0) = 5;
	}
}

#endif
