/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 * Support functions for dealing with units.
 */

#include "units/unit.hpp"
#include "units/helper.hpp"

namespace unit_helper {

int number_of_possible_advances(const unit &u)
{
	return u.advances_to().size() + u.get_modification_advances().size();
}

bool will_certainly_advance(const unit_map::iterator &u)
{
	return u.valid() && u->advances() && number_of_possible_advances(*u) > 0;
}

std::string resistance_color(const int resistance)
{
	if (resistance < 0)
		return std::string("#FF0000");

	if (resistance <= 20)
		return std::string("#FFFF00");

	if (resistance <= 40)
		return std::string("#FFFFFF");

	return std::string("#00FF00");
}

}
