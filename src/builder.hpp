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


	// regenerate the generated content at the given
	// location. Currently: set the image at that location to the
	// default image for the terrain.
	void rebuild_terrain(const gamemap::location &loc);
	void rebuild_all();
	

	typedef std::multimap<int, std::string> imagelist;

	struct terrain_constraint
	{
		terrain_constraint() : loc() {};
		
		terrain_constraint(gamemap::location loc) : loc(loc) {};
		
		gamemap::location loc;
		std::string terrain_types;
		std::vector<std::string> set_flag;
		std::vector<std::string> no_flag;
		std::vector<std::string> has_flag;

		imagelist images;
	};
	
	struct tile
	{
		tile(); 
		std::set<std::string> flags;
		std::vector<image::locator> images_foreground;
		std::vector<image::locator> images_background;

		gamemap::TERRAIN adjacents[7];
		
		void clear();
	};

private:
	typedef std::map<gamemap::location, terrain_constraint> constraint_set;

	struct building_rule
	{
		constraint_set constraints;
		gamemap::location location_constraints;

		int probability;
		int precedence;
		
		imagelist images;
	};
	
	struct tilemap
	{
		tilemap(int x, int y) : x_(x), y_(y), map_((x+2)*(y+2)) {}

		tile &operator[](const gamemap::location &loc);
		const tile &operator[] (const gamemap::location &loc) const;
		
		bool on_map(const gamemap::location &loc) const;
		
		void reset();
		
		std::vector<tile> map_;
		int x_;
		int y_;
	};

	typedef std::multimap<int, building_rule> building_ruleset;

	terrain_constraint rotate(const terrain_constraint &constraint, int angle);
	void replace_token(std::string &, const std::string &token, const std::string& replacement);
	void replace_token(imagelist &, const std::string &token, const std::string& replacement);
	void replace_token(building_rule &s, const std::string &token, const std::string& replacement);

	building_rule rotate_rule(const building_rule &rule, int angle, const std::vector<std::string>& angle_name);
	void add_rotated_rules(building_ruleset& rules, const building_rule& tpl, const std::string &rotations);
	void add_constraint_item(std::vector<std::string> &list, const config& cfg, const std::string &item);

	void terrain_builder::add_images_from_config(imagelist &images, const config &cfg);

	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints,
			     const gamemap::location &loc, const std::string& type);
	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints, const gamemap::location &loc,
			     const config &cfg);
	
	typedef std::multimap<int, gamemap::location> anchormap;
	void parse_mapstring(const std::string &mapstring, struct building_rule &br,
			     anchormap& anchors);
	terrain_builder::building_rule rule_from_terrain_template(const terrain_builder::building_rule &tpl, const gamemap::TERRAIN terrain);
	void parse_config(const config &cfg);
	bool terrain_matches(gamemap::TERRAIN letter, const std::string &terrains);
	bool rule_matches(const building_rule &rule, const gamemap::location &loc, int rule_index);
	void apply_rule(const building_rule &rule, const gamemap::location &loc);

	int get_constraint_adjacents(const building_rule& rule, const gamemap::location& loc);
	int get_constraint_size(const building_rule& rule, const terrain_constraint& constraint, bool& border);
	void build_terrains();
	
	const gamemap& map_;
	tilemap tile_map_;

	typedef std::map<unsigned char, std::vector<gamemap::location> > terrain_by_type_map;
	terrain_by_type_map terrain_by_type_;
	terrain_by_type_map terrain_by_type_border_;

	building_ruleset building_rules_;

};

#endif
