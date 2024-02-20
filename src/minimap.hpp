/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <cstddef>
#include <map>

class gamemap;
class surface;
class team;
class unit_map;
struct map_location;

namespace image {
	/**
	 * function to create the minimap for a given map
	 * the surface returned must be freed by the user
	 */
	surface getMinimap(int w, int h, const gamemap &map_, const team *vm = nullptr, const std::map<map_location,unsigned int> *reach_map = nullptr, bool ignore_terrain_disabled = false);

/**
 * Renders the minimap to the screen.
 */
void render_minimap(unsigned dst_w,
	unsigned dst_h,
	const gamemap& map,
	const team* vw = nullptr,
	const unit_map* units = nullptr,
	const std::map<map_location, unsigned int>* reach_map = nullptr,
	bool ignore_terrain_disabled = false);
}
