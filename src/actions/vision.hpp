/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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
 * Various functions implementing vision (through fog of war and shroud).
 */

#ifndef ACTIONS_VISION_H_INCLUDED
#define ACTIONS_VISION_H_INCLUDED

struct map_location;
class  team;
class  unit;

#include <cstring>
#include <map>
#include <set>


/// Clears shroud (and fog) around the provided location for @a view_team as
/// if a unit with @a viewer's sight range was standing there.
bool clear_shroud_unit(const map_location &view_loc, const unit &viewer,
                       team &view_team, const std::map<map_location, int>& jamming_map,
                       const std::set<map_location>* known_units = NULL,
                       std::set<map_location>* seen_units = NULL,
                       std::set<map_location>* petrified_units = NULL);

/// Wrapper for the invalidations that should occur after fog or
/// shroud is cleared.
void invalidate_after_clearing_shroud();

void calculate_jamming(int side, std::map<map_location, int>& jamming_map);

/// Function that recalculates the fog of war.
void recalculate_fog(int side);

/// Function that will clear shroud (and fog) based on current unit positions.
bool clear_shroud(int side, bool reset_fog=false);

#endif
