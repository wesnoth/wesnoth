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

/** @file */

#pragma once

#include "units/map.hpp"

namespace unit_helper {

/**
 * Encapsulates the logic for deciding whether an iterator @a u points to
 * a unit that can advance.
 * @return true if the unit exists, has available advances, and can_advance().
 */
bool will_certainly_advance(const unit_map::iterator &u);

/**
 * Determines the total number of available advancements (of any kind) for
 * a given unit. This includes normal advances and modifiers.
 * @return the total number of possible advancements.
 */
int number_of_possible_advances(const unit &unit);

/**
 * @return the name of the color encoding the weight of the unit's
 * resistance value for presenting it to the player.
 */
std::string resistance_color(const int resistance);

}
