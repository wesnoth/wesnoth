/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "brush.hpp"
#include "editor_common.hpp"

#include "../foreach.hpp"
#include "../pathutils.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

namespace editor {

brush::brush()
	: relative_tiles_()
	, name_()
	, image_()
{
}

brush::brush(const config& cfg)
	: relative_tiles_()
 	, name_(cfg["name"])
	, image_(cfg["image"])
{
	int radius = lexical_cast_default<int>(cfg["radius"], 0);
	if (radius > 0) {
		std::vector<map_location> in_radius;
		get_tiles_in_radius(map_location(0, 0), radius, in_radius);
		BOOST_FOREACH (map_location& loc, in_radius) {
			add_relative_location(loc.x, loc.y);
		}
	}
	BOOST_FOREACH (const config &relative, cfg.child_range("relative"))
	{
		int x = lexical_cast_default<int>(relative["x"], 0);
		int y = lexical_cast_default<int>(relative["y"], 0);
		add_relative_location(x, y);
	}
	if (relative_tiles_.empty()) {
		WRN_ED << "Empty brush definition, name=" << name_ << "\n";
	}
}

void brush::add_relative_location(int relative_x, int relative_y)
{
	relative_tiles_.insert(map_location(relative_x, relative_y));
}

std::set<map_location> brush::project(const map_location& hotspot) const
{
	std::set<map_location> result;
	BOOST_FOREACH (const map_location& relative, relative_tiles_) {
		result.insert(relative.vector_sum(hotspot));
	}
	return result;
}


} //end namespace editor
