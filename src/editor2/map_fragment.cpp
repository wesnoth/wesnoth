/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "map_fragment.hpp"

#include "../foreach.hpp"

#include <vector>

namespace editor2 {

map_fragment::map_fragment()
{
}

map_fragment::map_fragment(const gamemap& map, const std::set<gamemap::location>& area)
{
	foreach (const gamemap::location& loc, area) {
		add_tile(map, loc);
	}
}

void map_fragment::add_tile(const gamemap& map, const gamemap::location& loc)
{
	items_.push_back(tile_info(loc, map.get_terrain(loc)));
}

std::set<gamemap::location> map_fragment::get_area() const
{
	std::set<gamemap::location> result;
	foreach (const tile_info& i, items_) {
		result.insert(i.offset);
	}
	return result;
}

void map_fragment::paste_into(gamemap& map, const gamemap::location& loc) const
{
	foreach (const tile_info& i, items_) {
		map.set_terrain(i.offset.vector_sum(loc), i.terrain);
	}
}

} //end namespace editor2
