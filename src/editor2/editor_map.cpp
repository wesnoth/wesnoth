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

#include "editor_map.hpp"

namespace editor2 {

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
: gamemap(terrain_cfg, data)
{
}

editor_map editor_map::new_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler)
{
	const t_translation::t_list column(height, filler);
	const t_translation::t_map map(width, column);
	return editor_map(terrain_cfg, gamemap::default_map_header + t_translation::write_game_map(map));
}
	


} //end namespace editor2
