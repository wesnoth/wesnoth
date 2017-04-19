/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "default_map_generator.hpp"
#include "map/location.hpp"
#include "serialization/string_utils.hpp"
#include "terrain/translation.hpp"
#include "utils/name_generator.hpp"

#include <boost/random.hpp>
#include <cstdint>
#include <map>
#include <memory>

class default_map_generator_job
{
public:
	default_map_generator_job();
	default_map_generator_job(uint32_t seed);

	/** Generate the map. */
	std::string default_generate_map(generator_data data, std::map<map_location,std::string>* labels, const config& cfg);

private:
	typedef std::vector<std::vector<int> > height_map;
	typedef t_translation::ter_map terrain_map;

	bool generate_river_internal(const height_map& heights,
			terrain_map& terrain, int x, int y, std::vector<map_location>& river,
			std::set<map_location>& seen_locations, int river_uphill);

	std::vector<map_location> generate_river(const height_map& heights, terrain_map& terrain, int x, int y, int river_uphill);

	height_map generate_height_map(size_t width, size_t height,
			size_t iterations, size_t hill_size,
			size_t island_size, size_t island_off_center);

	bool generate_lake(t_translation::ter_map& terrain, int x, int y, int lake_fall_off, std::set<map_location>& locs_touched);
	map_location random_point_at_side(size_t width, size_t height);

	std::mt19937 rng_;
	const config& game_config_;

};
#endif
