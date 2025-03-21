/*
	Copyright (C) 2003 - 2025
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

#include <functional>
#include <map>

class gamemap;
class team;
class unit_map;
struct map_location;
struct rect;

namespace image {
/**
 * Prepares the minimap texture and returns a function which will render it to the current rendering target when invoked.
 */
std::function<rect(rect)> prep_minimap_for_rendering(
	const gamemap& map,
	const team* vw = nullptr,
	const unit_map* units = nullptr,
	const std::map<map_location, unsigned int>* reach_map = nullptr,
	bool ignore_terrain_disabled = false);
}
