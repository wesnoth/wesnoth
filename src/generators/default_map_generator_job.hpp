/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#ifndef DEFAULT_MAP_GENERATOR_JOB_HPP_INCLUDED
#define DEFAULT_MAP_GENERATOR_JOB_HPP_INCLUDED

class config;

#include <map>

#include "map_location.hpp"

/** Generate the map. */
std::string default_generate_map(size_t width, size_t height, size_t island_size, size_t island_off_center,
                                 size_t iterations, size_t hill_size,
								 size_t max_lakes, size_t nvillages, size_t castle_size, size_t nplayers,
								 bool roads_between_castles, std::map<map_location,std::string>* labels,
						         const config& cfg);

#endif
