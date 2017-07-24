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

#pragma once

#include "global.hpp"

#include <cstddef>
#include <map>

class gamemap;
class surface;
class team;
class texture;
struct map_location;
class gamemap;

namespace image
{
/// function to create the minimap for a given map
/// the surface returned must be freed by the user
DEPRECATED("") surface getMinimap(int w,
		int h,
		const gamemap& map_,
		const team* vm = nullptr,
		const std::map<map_location, unsigned int>* reach_map = nullptr,
		bool ignore_terrain_disabled = false);

/**
 * Renders the minimap to the given texture.
 */
void render_minimap(texture& tex,
		const gamemap& map,
		const team* vw = nullptr,
		const std::map<map_location, unsigned int>* reach_map = nullptr,
        bool ignore_terrain_disabled = false);
}
