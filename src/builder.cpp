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

#include "terrain.hpp"
#include "display.hpp"

terrain_builder::terrain_builder(const config& cfg, const gamemap& gmap) :
	 map_(gmap), cfg_(cfg.get_children("built_terrain"))
{
	build_terrains();
}

const std::vector<std::string>* terrain_builder::get_terrain_at(const gamemap::location &loc,
			       ADJACENT_TERRAIN_TYPE terrain_type) const
{
	const buildings_map& buildings = (terrain_type == ADJACENT_BACKGROUND) ? buildings_background : buildings_foreground;
	const buildings_map::const_iterator itor = buildings.find(loc);
	if(itor != buildings.end()) {
		return &(itor->second);
	} else {
		return NULL;
	}
}

void terrain_builder::rebuild_terrain(const gamemap::location &loc) {
    const std::vector<std::string> &vback = build_terrain_at(loc, ADJACENT_BACKGROUND);
	if (vback.empty()) {
		buildings_background.erase(loc);
	}
	else {
		buildings_background[loc] = vback;
	}
    const std::vector<std::string> &vfore = build_terrain_at(loc, ADJACENT_FOREGROUND);
	if (vfore.empty()) {
		buildings_foreground.erase(loc);
	}
	else {
		buildings_foreground[loc] = vfore;
	}
}


void terrain_builder::build_terrains()
{
	for(int x = -1; x <= map_.x(); ++x) {
		for(int y = -1; y <= map_.y(); ++y) {
			const gamemap::location loc(x, y);

			const std::vector<std::string> &vback = build_terrain_at(loc, ADJACENT_BACKGROUND);		
			if(!vback.empty())
				buildings_background[loc] = vback;
			
			const std::vector<std::string> &vfore = build_terrain_at(loc, ADJACENT_FOREGROUND);
			if(!vfore.empty())
				buildings_foreground[loc] = vfore;
		}
	}
}

std::vector<std::string> terrain_builder::build_terrain_at(const gamemap::location &loc,
							   ADJACENT_TERRAIN_TYPE terrain_type)
{
	std::vector<std::string> res;

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);

	static const int corner_order[] = {0, 5, 1, 4, 2, 3};
	
	// Calculate each corner's type.
	for(int index = 0; index != 6; ++index) {
		int i = corner_order[index];
		
		const bool angle_northern = angle_is_northern(i);
		
		if(terrain_type == ADJACENT_FOREGROUND && angle_northern)
			continue;
		
		int j = i+1;
		if(j == 6)
			j = 0;

		for(config::child_list::const_iterator item = cfg_.begin(); item != cfg_.end(); ++item) {
			
			const std::string &layer = (**item)["layer"];
			
			if(terrain_type == ADJACENT_FOREGROUND && layer == "background" || 
			   terrain_type == ADJACENT_BACKGROUND && !angle_northern && layer == "foreground" )
				continue; 
			
			std::vector<std::string> terrain_types = config::split((**item)["terrains"]);
			// Ignore invalid terrain types definitions
			if(terrain_types.size() != 3)
				continue;
			
			// If (loc, adjacent[i], adjacent[j]), or any of their rotations, do match the
			// current terrains, the corresponding map will be blitted into the current surface.
			gamemap::TERRAIN to = map_.get_terrain(loc);
			gamemap::TERRAIN ti = map_.get_terrain(adjacent[i]);
			gamemap::TERRAIN tj = map_.get_terrain(adjacent[j]);
			
			for(int r = 0; r != 3; ++r) {
				if(map_.get_terrain_info(to).matches(terrain_types[0]) &&
				   map_.get_terrain_info(ti).matches(terrain_types[1]) &&
				   map_.get_terrain_info(tj).matches(terrain_types[2])) {

					// We found a match. Add the current rotation.					
					const std::string &image = (**item)["image"];
					if(image.size() != 0)
					{
						std::stringstream imagename;
						imagename << image;
						int rotation = (2*r + i) % 6;
						imagename << get_angle_direction(rotation);
						imagename << get_angle_direction(i);
						
						res.push_back(imagename.str());						
					}

					break; // We will not try other rotations if this one matches
				}

				// rotate terrains
				gamemap::TERRAIN tmp;
				tmp = to;
				to = ti;
				ti = tj;
				tj = tmp;
			}
		}
	}
	
	return res;
}


