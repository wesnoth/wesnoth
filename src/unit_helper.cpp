/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Support functions for dealing with units.
 */

#include "resources.hpp"
#include "unit.hpp"
#include "unit_helper.hpp"

namespace unit_helper {

int number_of_possible_advances(const unit &u)
{
	return u.advances_to().size() + u.get_modification_advances().size();
}

bool will_certainly_advance(const unit_map::iterator &u)
{
	return u.valid() && u->advances() && number_of_possible_advances(*u) > 0;
}

}
