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

#ifndef EDITOR2_MAP_FRAGMENT_HPP_INCLUDED
#define EDITOR2_MAP_FRAGMENT_HPP_INCLUDED

#include "editor_map.hpp"

#include "../config.hpp"

#include <set>

namespace editor2 {

struct tile_info
{
	tile_info(const gamemap::location& offset, t_translation::t_terrain terrain)
	: offset(offset), terrain(terrain)
	{
	}
	gamemap::location offset;
	t_translation::t_terrain terrain;
};
	
class map_fragment
{
	public:
		map_fragment();
		map_fragment(const gamemap& map, const std::set<gamemap::location>& area);
		void add_tile(const gamemap& map, const gamemap::location& loc);
		const std::vector<tile_info>& get_items() const { return items_; }
		std::set<gamemap::location> get_area() const;
		void paste_into(gamemap& map, const gamemap::location& loc) const;
	private:
		std::vector<tile_info> items_;
};

} //end namespace editor2

#endif
