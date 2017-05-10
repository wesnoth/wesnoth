/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/map/map_fragment.hpp"

namespace editor {

map_fragment::map_fragment()
	: items_()
	, area_()
{
}

map_fragment::map_fragment(const gamemap& map, const std::set<map_location>& area)
	: items_()
	, area_()
{
	add_tiles(map, area);
}

void map_fragment::add_tile(const gamemap& map, const map_location& loc)
{
	if (area_.find(loc) == area_.end()) {
		items_.emplace_back(map, loc);
		area_.insert(loc);
	}
}

void map_fragment::add_tiles(const gamemap& map, const std::set<map_location>& locs)
{
	for (const map_location& loc : locs) {
		add_tile(map, loc);
	}
}

std::set<map_location> map_fragment::get_area() const
{
	return area_;
}

std::set<map_location> map_fragment::get_offset_area(const map_location& loc) const
{
	std::set<map_location> result;
	for (const tile_info& i : items_) {
		result.insert(i.offset.vector_sum(loc));
	}
	return result;
}

void map_fragment::paste_into(gamemap& map, const map_location& loc) const
{
	for (const tile_info& i : items_) {
		map.set_terrain(i.offset.vector_sum(loc), i.terrain);
	}
}

void map_fragment::shift(const map_location& offset)
{
	for (tile_info& ti : items_) {
		ti.offset.vector_sum_assign(offset);
	}
}

map_location map_fragment::center_of_mass() const
{
	map_location sum(0, 0);
	for (const tile_info& ti : items_) {
		sum.vector_sum_assign(ti.offset);
	}
	if (items_.size() > 0) {
		sum.x /= static_cast<int>(items_.size());
		sum.y /= static_cast<int>(items_.size());
	}
	return sum;
}

void map_fragment::center_by_mass()
{
	shift(center_of_mass().vector_negation());
	area_.clear();
	for (tile_info& ti : items_) {
		area_.insert(ti.offset);
	}
}

void map_fragment::rotate_60_cw()
{
	area_.clear();
	for (tile_info& ti : items_) {
		map_location l = map_location::ZERO();
		int x = ti.offset.x;
		int y = ti.offset.y;
		// rotate the X-Y axes to SOUTH/SOUTH_EAST - SOUTH_WEST axes
		// but if x is odd, simply using x/2 + x/2 will lack a step
		l = l.get_direction(map_location::SOUTH, (x+is_odd(x))/2);
		l = l.get_direction(map_location::SOUTH_EAST, (x-is_odd(x))/2 );
		l = l.get_direction(map_location::SOUTH_WEST, y);
		ti.offset = l;
		area_.insert(l);
	}
	if (get_area().size() != items_.size()) {
		throw editor_exception("Map fragment rotation resulted in duplicate entries");
	}
}

void map_fragment::rotate_60_ccw()
{
	area_.clear();
	for (tile_info& ti : items_) {
		map_location l = map_location::ZERO();
		int x = ti.offset.x;
		int y = ti.offset.y;
		// rotate the X-Y axes to NORTH/NORTH_EAST - SOUTH_EAST axes'
		// reverse of what the cw rotation does
		l = l.get_direction(map_location::NORTH, (x-is_odd(x))/2);
		l = l.get_direction(map_location::NORTH_EAST, (x+is_odd(x))/2 );
		l = l.get_direction(map_location::SOUTH_EAST, y);
		ti.offset = l;
		area_.insert(l);
	}
	if (get_area().size() != items_.size()) {
		throw editor_exception("Map fragment rotation resulted in duplicate entries");
	}
}

void map_fragment::flip_horizontal()
{
	for (tile_info& ti : items_) {
		ti.offset.x = -ti.offset.x;
	}
	center_by_mass();
}

void map_fragment::flip_vertical()
{
	for (tile_info& ti : items_) {
		ti.offset.y = -ti.offset.y;
		if (ti.offset.x % 2) {
			ti.offset.y--;
		}
	}
	center_by_mass();
}


bool map_fragment::empty() const
{
	return items_.empty();
}

std::string map_fragment::dump() const
{
	std::stringstream ss;
	ss << "MF: ";
	for (const tile_info& ti : items_) {
		ss << "(" << ti.offset << ")";
	}
	ss << " -- ";
	for (const map_location& loc : area_) {
		ss << "(" << loc << ")";
	}
	return ss.str();
}

} //end namespace editor
