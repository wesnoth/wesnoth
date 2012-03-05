/* $Id$ */
/*
  Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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

/**
 * @file
 * Set various sizes for the screen-layout of the map editor.
 */

#include "display.hpp"
#include "editor_layout.hpp"


namespace {
	const size_t default_terrain_size = 36;
	const size_t default_terrain_width = 4;
	const size_t default_brush_width = 4;
}

namespace editor {

size_specs::size_specs()
	: terrain_size(default_terrain_size)
	, terrain_padding(0)
	, terrain_space(0)
	, terrain_width(default_terrain_width)
	, palette_x(0)
	, palette_y(0)
	, palette_h(20)
	, palette_w(10)
	, brush_x(0)
	, brush_y(0)
	, brush_padding(1)
	, brush_width(default_brush_width)
{
}

void adjust_sizes(const display &disp, size_specs &sizes) {
	/** @todo Hardcoded coordinates for brush selection, make them themeable. */
	sizes.brush_x = disp.map_outside_area().w + 12;
	sizes.brush_y = (disp.h() >= 768) ? 340 : 270;
	/** @todo Hardcoded coordinates for terrain palette, make them themeable. */
	sizes.palette_x = disp.map_outside_area().w + 12;
	sizes.palette_y = sizes.brush_y + 33;
	sizes.palette_w = sizes.terrain_space * sizes.terrain_width;
	sizes.palette_h = disp.h() - sizes.palette_y;
	sizes.terrain_padding = (disp.h() >= 768) ? 3 : 1;
	sizes.terrain_space = sizes.terrain_size + sizes.terrain_padding;
}

} // end namespace editor

