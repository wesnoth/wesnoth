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

void adjust_sizes(const display &disp, size_specs &sizes) {
	sizes.terrain_size = default_terrain_size;
	sizes.terrain_padding = 2;
	sizes.terrain_space = sizes.terrain_size + sizes.terrain_padding;
	sizes.brush_x = disp.mapx() + 33;
	sizes.brush_y = 190;
	sizes.palette_x = disp.mapx() + 40;
	sizes.palette_y = sizes.brush_y + 30 + 10;
	sizes.palette_w = sizes.terrain_space * 2;
	sizes.palette_h = disp.y() - sizes.palette_y - 60;
}

}
