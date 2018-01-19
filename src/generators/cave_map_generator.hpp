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

#include "config.hpp"
#include "generators/map_generator.hpp"
#include "terrain/translation.hpp"

#include <set>
#include <boost/optional.hpp>
#include <random>

class cave_map_generator : public map_generator
{
public:
	cave_map_generator(const config &game_config);

	std::string name() const { return "cave"; }

	std::string config_name() const;

	std::string create_map(boost::optional<uint32_t> randomseed = boost::none);
	config create_scenario(boost::optional<uint32_t> randomseed = boost::none);

private:
	struct cave_map_generator_job
	{
		cave_map_generator_job(const cave_map_generator& params, boost::optional<uint32_t> randomseed = boost::none);

		struct chamber {
			chamber()
				: center()
				, locs()
				, items(0)
			{
			}

			map_location center;
			std::set<map_location> locs;
			const config *items;
		};

		struct passage {
			passage(map_location s, map_location d, const config& c)
				: src(s), dst(d), cfg(c)
			{}
			map_location src, dst;
			config cfg;
		};

		void generate_chambers();
		void build_chamber(map_location loc, std::set<map_location>& locs, size_t size, size_t jagged);

		void place_chamber(const chamber& c);

		void place_passage(const passage& p);

		void set_terrain(map_location loc, const t_translation::terrain_code & t);
		void place_castle(int starting_position, const map_location &loc);

		size_t translate_x(size_t x) const;
		size_t translate_y(size_t y) const;


		const cave_map_generator& params;
		bool flipx_, flipy_;

		t_translation::ter_map map_;
		t_translation::starting_positions starting_positions_;
		std::map<std::string,size_t> chamber_ids_;
		std::vector<chamber> chambers_;
		std::vector<passage> passages_;
		config res_;
		std::mt19937 rng_;
	};

	bool on_board(const map_location& loc) const
	{
		return loc.x >= 0 && loc.y >= 0 && loc.x < width_ && loc.y < height_;
	}

	t_translation::terrain_code wall_, clear_, village_, castle_, keep_;

	config cfg_;
	int width_, height_, village_density_;

	// The scenario may have a chance to flip all x values or y values
	// to make the scenario appear all random. This is kept track of here.
	int flipx_chance_, flipy_chance_;
};
