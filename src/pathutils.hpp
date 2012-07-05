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

/** @file */

#ifndef PATHUTILS_H_INCLUDED
#define PATHUTILS_H_INCLUDED

#include "map_location.hpp"

class gamemap;

class xy_pred : public std::unary_function<map_location const&, bool>
{
public:
	virtual bool operator()(map_location const&) const = 0;
protected:
	virtual ~xy_pred() {}
};

/**
 * Function which will add to @a result all locations in a ring of distance
 * @a radius from @a center. @a result must be a std::vector of locations.
 */
void get_tile_ring(const map_location& center, const int radius,
                   std::vector<map_location>& result);

/**
 * Function which will add to @a result all locations within a distance of
 * @a radius from @a center (excluding @a center itself). @a result must be
 * a std::vector of locations.
 */
void get_tiles_in_radius(const map_location& center, const int radius,
                         std::vector<map_location>& result);

/**
 * Function which will add to @a result all locations within a distance of
 * @a radius from @a center (including @a center itself). @a result must be
 * a std::set of locations.
 */
void get_tiles_radius(const map_location& center, size_t radius,
                      std::set<map_location>& result);

/**
 * Function which will add to @a result all elements of @a locs, plus all
 * on-board locations matching @a pred that are connected to elements of
 * locs by a chain of at most @a radius tiles, each of which matches @a pred.
 * @a result must be a std::set of locations.
 */
void get_tiles_radius(const gamemap& map, const std::vector<map_location>& locs,
                      size_t radius, std::set<map_location>& result,
                      bool with_border=false, const xy_pred *pred=NULL);

#endif

