/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "../util.hpp"

#include "editor_layout.hpp"


namespace {
	const size_t default_terrain_size = 35;
}
namespace map_editor {

void adjust_sizes(const display &disp, size_specs &sizes,
				  const unsigned int num_terrains) {
	const size_t button_height = 24;
	const size_t button_palette_padding = 8;
	sizes.terrain_size = default_terrain_size;
	sizes.terrain_padding = 2;
	sizes.terrain_space = sizes.terrain_size + sizes.terrain_padding;
	sizes.palette_x = 40;
	sizes.button_x = sizes.palette_x + sizes.terrain_space - 12;
	sizes.brush_x = 25;
	sizes.brush_y = 190;
	sizes.top_button_y = sizes.brush_y + 40;
	sizes.palette_y = sizes.top_button_y + button_height +
		button_palette_padding;
	const size_t max_bot_button_y = disp.y() - 60 - button_height;
	size_t space_for_terrains = max_bot_button_y - button_palette_padding -
		sizes.palette_y;
	space_for_terrains = space_for_terrains / sizes.terrain_space % 2 == 0 ? 
		space_for_terrains : space_for_terrains - sizes.terrain_space;
	sizes.nterrains = minimum((space_for_terrains / sizes.terrain_space) * 2,
							  num_terrains);
	sizes.bot_button_y = sizes.palette_y +
		(sizes.nterrains / 2) * sizes.terrain_space + button_palette_padding;
}

}
