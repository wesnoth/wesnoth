/*
	Copyright (C) 2008 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/toolkit/brush.hpp"
#include "editor/editor_common.hpp"

#include "pathutils.hpp"

namespace editor {

brush::brush()
	: relative_tiles_()
	, name_()
	, id_()
{
}

brush::brush(const config& cfg)
	: relative_tiles_()
	, name_(cfg["name"])
	, id_(cfg["id"])
{
	int radius = cfg["radius"].to_int();
	if (radius > 0) {
		std::vector<map_location> in_radius;
		get_tiles_in_radius(map_location(0, 0), radius, in_radius);
		for (map_location& loc : in_radius) {
			add_relative_location(loc.x, loc.y);
		}
	}
	for (const config &relative : cfg.child_range("relative"))
	{
		int x = relative["x"].to_int();
		int y = relative["y"].to_int();
		add_relative_location(x, y);
	}
	if (relative_tiles_.empty()) {
		WRN_ED << "Empty brush definition, name=" << name_;
	}
}

void brush::add_relative_location(int relative_x, int relative_y)
{
	relative_tiles_.insert(map_location(relative_x, relative_y));
}

std::set<map_location> brush::project(const map_location& hotspot) const
{
	std::set<map_location> result;
	for (const map_location& relative : relative_tiles_) {
		result.insert(relative.vector_sum(hotspot));
	}
	return result;
}


} //end namespace editor
