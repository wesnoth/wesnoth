/* $Id$ */
/*
Copyright (C) 2003 by David White <davidnwhite@verizon.net>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#ifndef PATHUTILS_H_INCLUDED
#define PATHUTILS_H_INCLUDED

#include "map.hpp"

//function which tells if two locations are adjacent.
bool tiles_adjacent(const gamemap::location& a, const gamemap::location& b);

//function which, given a location, will place all adjacent locations in
//res. res must point to an array of 6 location objects.
void get_adjacent_tiles(const gamemap::location& a, gamemap::location* res);

//function which returns the direction from 'from' to 'to'. If 'from' and 'to' are not adjacent, then
//the function will return 'NDIRECTIONS'.
gamemap::location::DIRECTION get_adjacent_direction(const gamemap::location& from, const gamemap::location& to);

//function which gives the number of hexes between two tiles (i.e. the minimum
//number of hexes that have to be traversed to get from one hex to the other)
size_t distance_between(const gamemap::location& a, const gamemap::location& b);

#endif

