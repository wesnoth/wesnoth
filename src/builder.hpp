/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef BUILDER_H_INCLUDED
#define BUILDER_H_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "image.hpp"
#include "SDL.h"

#include <string>
#include <map>

//builder: dynamically builds castle and other dynamically-generated tiles. 
class terrain_builder
{
public:
	enum ADJACENT_TERRAIN_TYPE { ADJACENT_BACKGROUND, ADJACENT_FOREGROUND };

	terrain_builder(const config& cfg, const gamemap& gmap);

	//returns a vector of string representing the images to load & blit together to get the
	//built content for this tile.
	//Returns NULL if there is no built content for this tile.
	const std::vector<image::locator> *get_terrain_at(const gamemap::location &loc,
						       ADJACENT_TERRAIN_TYPE terrain_type) const;
	// regenerate the generated content at the given location.
	void rebuild_terrain(const gamemap::location &loc);

	struct terrain_constraint
	{
		terrain_constraint() : loc() {};
		
		terrain_constraint(gamemap::location loc) : loc(loc) {};
		
		gamemap::location loc;
		std::string terrain_types;
		std::vector<std::string> set_flag;
		std::vector<std::string> no_flag;
		std::vector<std::string> has_flag;
	};

private:

	struct building_rule
	{
		typedef std::map<gamemap::location, terrain_constraint> constraint_set;
		constraint_set constraints;
		gamemap::location location_constraints;

		int probability;
		std::string image_foreground;
		std::string image_background;
	};
	
	struct tile
	{
		std::set<std::string> flags;
		std::vector<image::locator> images_foreground;
		std::vector<image::locator> images_background;
	};

	struct tilemap
	{
		tilemap(int x, int y) : x_(x), y_(y), map_((x+2)*(y+2)) {}

		tile &operator[](const gamemap::location &loc) { return map_[(loc.x+1) + (loc.y+1)*(x_+2)]; }
		const tile &operator[] (const gamemap::location &loc) const { return map_[(loc.x+1) + (loc.y+1)*(x_+2)]; }
		void clear() { map_.clear(); }
		
		std::vector<tile> map_;
		int x_;
		int y_;
	};

	terrain_constraint rotate(const terrain_constraint &constraint, int angle);
	void replace_rotation(std::string &s, const std::string &replacement);

	building_rule rotate_rule(const building_rule &rule, int angle, const std::string &angle_name);

	void add_constraint_item(std::vector<std::string> &list, const std::string &item);
	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints,
			     const gamemap::location &loc, const std::string& type);
	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints, const gamemap::location &loc,
			     const config &cfg);

	typedef std::multimap<int, gamemap::location> anchormap;
	void parse_mapstring(const std::string &mapstring, struct building_rule &br,
			     anchormap& anchors);
	
	void parse_config(const config &cfg);
	bool rule_matches(const building_rule &rule, const gamemap::location &loc, int rule_index);
	void apply_rule(const building_rule &rule, const gamemap::location &loc);
	void build_terrains();
	
	const gamemap& map_;
	tilemap tile_map_;

	typedef std::map<unsigned char, std::vector<gamemap::location> > terrain_by_type_map;
	terrain_by_type_map terrain_by_type_;
	
	typedef std::vector<building_rule> building_ruleset;
	building_ruleset building_rules_;

};

#endif
