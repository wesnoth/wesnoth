/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

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
	const std::vector<std::string> *terrain_builder::get_terrain_at(const gamemap::location &loc,
								  ADJACENT_TERRAIN_TYPE terrain_type) const;
       
private:
	//pre-calculates the list of generated content for all tiles (will slow the game
	//too much otherwise)
	void build_terrains();
	//returns a vector of strings representing the images for a given tile. Same as 
	//get_terrain_at, except that the content is actually generated there.
	std::vector<std::string> build_terrain_at(const gamemap::location &loc, 
						  ADJACENT_TERRAIN_TYPE terrain_type);
		
	const config::child_list cfg_;
	const gamemap& map_;

	typedef std::map<gamemap::location, std::vector<std::string> > buildings_map;
	buildings_map buildings_background, buildings_foreground;
};

#endif
