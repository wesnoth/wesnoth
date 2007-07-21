/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TERRAIN_FILTER_H_INCLUDED
#define TERRAIN_FILTER_H_INCLUDED

class config;
class gamestatus;
class unit;
class vconfig;
class unit_map;

#include "map.hpp"
#include "terrain.hpp"
#include "variable.hpp"

// These functions usd to be methods of the map class, but they don't 
// really fit in the "everything-is-a-method" paradigm because they
// merge data from several peer classes. 

//the terrain filter, also known as "standard location filter" or SLF
bool terrain_matches_filter(const gamemap& map,
		const gamemap::location& loc, const vconfig& cfg, 
		const gamestatus& game_status, const unit_map& units,
		const bool flat_tod=false, const size_t max_loop=MAX_MAP_AREA);

//gets all locations that match a given terrain filter
void get_locations(const gamemap& map,
		std::set<gamemap::location>& locs, const vconfig& filter, 
		const gamestatus& game_status, const unit_map& units,
		const bool flat_tod=false, const size_t max_loop=MAX_MAP_AREA);

#endif

