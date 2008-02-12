/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file cavegen.hpp 
//!

#ifndef CAVEGEN_HPP_INCLUDED
#define CAVEGEN_HPP_INCLUDED

#include "config.hpp"
#include "mapgen.hpp"

#include <set>

class cave_map_generator : public map_generator
{
public:
	cave_map_generator(const config* game_config);

	bool allow_user_config() const { return true; }
	// This is a pure virtual function in the base class, so must be here
	void user_config(display& /* disp*/) { return; }

	std::string name() const { return "cave"; }

	std::string create_map(const std::vector<std::string>& args);
	config create_scenario(const std::vector<std::string>& args);

private:

	struct chamber {
		gamemap::location center;
		std::set<gamemap::location> locs;
		config* items;
	};

	struct passage {
		passage(gamemap::location s, gamemap::location d, const config& c)
			: src(s), dst(d), cfg(c)
		{}
		gamemap::location src, dst;
		config cfg;
	};

	void generate_chambers();
	void build_chamber(gamemap::location loc, std::set<gamemap::location>& locs, size_t size, size_t jagged);

	void place_chamber(const chamber& c);
	void place_items(const chamber& c, config::all_children_iterator i1, config::all_children_iterator i2);

	void place_passage(const passage& p);

	bool on_board(const gamemap::location& loc) const
		{ return loc.x >= 0 && loc.y >= 0 &&
            loc.x < static_cast<long>(width_) &&
            loc.y < static_cast<long>(height_); }

	void set_terrain(gamemap::location loc, t_translation::t_terrain t);
	void place_castle(const std::string& side, gamemap::location loc);

	t_translation::t_terrain wall_, clear_, village_, castle_, keep_;
	t_translation::t_map map_;
	std::map<int, t_translation::coordinate> starting_positions_;

	std::map<std::string,size_t> chamber_ids_;
	std::vector<chamber> chambers_;
	std::vector<passage> passages_;

	config res_;
	const config* cfg_;
	size_t width_, height_, village_density_;

	// The scenario may have a chance to flip all x values or y values
	// to make the scenario appear all random. This is kept track of here.
	bool flipx_, flipy_;

	size_t translate_x(size_t x) const;
	size_t translate_y(size_t y) const;
};

#endif
