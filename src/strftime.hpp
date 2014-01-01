/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef STRFTIME_HPP_INCLUDED
#define STRFTIME_HPP_INCLUDED

#include <ctime>

namespace util {

/*
 * std::strftime wrapper to support date translations and
 * add missing am/pm designations.
 */
size_t strftime(char* str, size_t count, const char* format,
	const std::tm* time);

}

#endif
