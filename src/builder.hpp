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
#include "animated.hpp"
#include "SDL.h"

#include <string>
#include <map>

/**
 * The class terrain_builder is constructed from a config object, and a gamemap
 * object. On construction, it parses the configuration and extracts the list
 * of [terrain_graphics] rules. Each terrain_graphics rule attachs one or more
 * images to a specific terrain pattern. 
 * It then applies the rules loaded from the configuration to the current map,
 * and calculates the list of images that must be associated to each hex of the
 * map.
 *
 * The get_terrain_at method can then be used to obtain the list of images
 * necessary to draw the terrain on a given tile.
 */
class terrain_builder
{
public:
	enum ADJACENT_TERRAIN_TYPE { ADJACENT_BACKGROUND, ADJACENT_FOREGROUND };
	typedef std::vector<animated<image::locator> > imagelist;

	terrain_builder(const config& cfg, const config &level, const gamemap& gmap);

	//returns a vector of string representing the images to load & blit together to get the
	//built content for this tile.
	//Returns NULL if there is no built content for this tile.
	const imagelist *get_terrain_at(const gamemap::location &loc,
			const std::string &tod, ADJACENT_TERRAIN_TYPE terrain_type) const;

	//updates the animation at a given tile. returns true if something has
	//changed, and must be redrawn.
	bool terrain_builder::update_animation(const gamemap::location &loc);

	// regenerate the generated content at the given
	// location. Currently: set the image at that location to the
	// default image for the terrain.
	void rebuild_terrain(const gamemap::location &loc);
	void rebuild_all();
	

	/**
	 * Each image may have several variants, which are used depending to
	 * the current time of day. The rule_image_variant structure represents
	 * an image variant.
	 */
	struct rule_image_variant {
		rule_image_variant(const std::string &image_string, const std::string &tod) :
			image_string(image_string), tod(tod) {};

		std::string image_string;

		animated<image::locator> image;
		std::string tod;
	};

	/**
	 * A map associating a rule_image_variant to a string representing the
	 * time of day.
	 */
	typedef std::map<std::string, rule_image_variant> rule_image_variantlist;

	/**
	 * Each terrain_graphics rule is associated a set of images, which are
	 * applied on the terrain if the rule matches. An image is more than
	 * graphics: it is graphics (with several possible tod-alternatives,)
	 * and a position for these graphics.
	 * The rule_image structure represents one such image.
	 */
	struct rule_image {
		rule_image(int layer, bool global_image=false);
		rule_image(int x, int y, bool global_image=false);

		// There are 2 kinds of images:
		// * Flat, horizontal, stacked images, who have a layer
		// * Vertical images who have an x/y position
		enum { HORIZONTAL, VERTICAL } position;

		int layer; 		// If the image is horizontal
		int basex, basey; 	// If the image is vertical

		// The tile width used when using basex and basey. This is not,
		// necessarily, the tile width in pixels, this is totally
		// arbitrary. However, it will be set to 72 for convenience.
		static const int TILEWIDTH;
		// The position of unit graphics in a tile. Graphics whose y
		// position is below this value are considered background for
		// this tile; graphics whose y position is above this value are
		// considered foreground.
		static const int UNITPOS;

		bool global_image;
		rule_image_variantlist variants;
	};

	typedef std::vector<rule_image> rule_imagelist;

	/**
	 * Each terrain_graphics rule consists in a set of constraints
	 */
	struct terrain_constraint
	{
		terrain_constraint() : loc() {};
		
		terrain_constraint(gamemap::location loc) : loc(loc) {};
		
		gamemap::location loc;
		std::string terrain_types;
		std::vector<std::string> set_flag;
		std::vector<std::string> no_flag;
		std::vector<std::string> has_flag;

		rule_imagelist images;
	};


	struct tile
	{
		typedef std::multimap<int, const rule_image*> ordered_ri_list;

		tile(); 
		void add_image_to_cache(const std::string &tod, ordered_ri_list::const_iterator itor) const;
		void rebuild_cache(const std::string &tod) const;
		void clear();

		std::set<std::string> flags;

		ordered_ri_list horizontal_images;
		ordered_ri_list vertical_images;

		mutable imagelist images_foreground;	//this is a mutable cache
		mutable imagelist images_background;	//this is a mutable cache

		mutable std::string last_tod;

		gamemap::TERRAIN adjacents[7];
		
	};

private:
	typedef std::map<gamemap::location, terrain_constraint> constraint_set;

	struct building_rule
	{
		constraint_set constraints;
		gamemap::location location_constraints;

		int probability;
		int precedence;
		
		// rule_imagelist images;
	};
	
	class tilemap
	{
	public:
		tilemap(int x, int y) : map_((x+2)*(y+2)), x_(x), y_(y) {}

		tile &operator[](const gamemap::location &loc);
		const tile &operator[] (const gamemap::location &loc) const;
		
		bool on_map(const gamemap::location &loc) const;
		
		void reset();
	private:
		std::vector<tile> map_;
		int x_;
		int y_;
	};

	typedef std::multimap<int, building_rule> building_ruleset;

	
	bool rule_valid(const building_rule &rule);
	bool start_animation(building_rule &rule);
	terrain_constraint rotate(const terrain_constraint &constraint, int angle);
	void replace_token(std::string &, const std::string &token, const std::string& replacement);
	void replace_token(rule_image_variant &, const std::string &token, const std::string& replacement);
	void replace_token(rule_image &, const std::string &token, const std::string& replacement);
	void replace_token(rule_imagelist &, const std::string &token, const std::string& replacement);
	void replace_token(building_rule &s, const std::string &token, const std::string& replacement);

	building_rule rotate_rule(const building_rule &rule, int angle, const std::vector<std::string>& angle_name);

	void add_rule(building_ruleset& rules, building_rule &rule);
	void add_rotated_rules(building_ruleset& rules, building_rule& tpl, const std::string &rotations);
	void add_constraint_item(std::vector<std::string> &list, const config& cfg, const std::string &item);

	void add_images_from_config(rule_imagelist &images, const config &cfg, bool global,
			int dx=0, int dy=0);

	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints,
			const gamemap::location &loc, const std::string& type,
			const config& global_images);
	void add_constraints(std::map<gamemap::location, terrain_constraint>& constraints,
			const gamemap::location &loc, const config &cfg,
			const config& global_images);
	
	typedef std::multimap<int, gamemap::location> anchormap;
	void parse_mapstring(const std::string &mapstring, struct building_rule &br,
			     anchormap& anchors, const config& global_images);
	void parse_config(const config &cfg);
	bool terrain_matches(gamemap::TERRAIN letter, const std::string &terrains);
	bool rule_matches(const building_rule &rule, const gamemap::location &loc, int rule_index, bool check_loc);
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
