/* $Id$ */
/*
  Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

//! @file editor/editor_layout.cpp 
//! Set various sizes for the screen-layout of the map editor.

#include "editor_layout.hpp"
#include "util.hpp"


namespace {
	const size_t default_terrain_size = 36;
	const size_t default_palette_width = 3;
}

namespace map_editor {

size_specs::size_specs() {
	terrain_size = default_terrain_size;
	terrain_padding = 2;
	terrain_space = terrain_size + terrain_padding;
	terrain_width = default_palette_width;
	brush_x = 0;
	brush_y = 0;
	brush_padding = 5;
	palette_x = 0;
	palette_y = 0;
	palette_w = 10;
	palette_h = 20;
}

void adjust_sizes(const display &disp, size_specs &sizes) {
	//! @todo Hardcoded coordinates for brush selection, make it themeable
	sizes.brush_x = disp.map_outside_area().w + 22;
	sizes.brush_y = 165;
	//! @todo Hardcoded coordinates for terrain palette, make it themeable
	sizes.palette_x = disp.map_outside_area().w + 16;
	sizes.palette_y = sizes.brush_y + 132 + 10;
	sizes.palette_w = sizes.terrain_space * default_palette_width;
	//the '-5' make sure that the scroll down button can not be outside of
	//the window, this is probably a bug in placement anyway...
	sizes.palette_h = disp.h() - sizes.palette_y - 5;
}

} // end namespace map_editor

