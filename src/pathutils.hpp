/* $Id$ */
/*
Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2
or at your option any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

/** @file pathutils.hpp */

#ifndef PATHUTILS_H_INCLUDED
#define PATHUTILS_H_INCLUDED

#include "map_location.hpp"

/** Function which tells if two locations are adjacent. */
bool tiles_adjacent(const map_location& a, const map_location& b);

/**
 * Function which, given a location, will place all adjacent locations in res.
 * res must point to an array of 6 location objects.
 */
void get_adjacent_tiles(const map_location& a, map_location* res);

/**
 * Function which, given a location, will place all locations in a ring of
 * distance r in res. res must be a std::vector of location
 */
void get_tile_ring(const map_location& a, const int r, std::vector<map_location>& res);

/**
 * Function which, given a location, will place all locations in the radius of r in res
 * res must be a std::vector of location
 */
void get_tiles_in_radius(const map_location& a, const int r, std::vector<map_location>& res);

/**
 * Function which gives the number of hexes between two tiles
 * (i.e. the minimum number of hexes that have to be traversed
 * to get from one hex to the other).
 */
size_t distance_between(const map_location& a, const map_location& b);

#endif

