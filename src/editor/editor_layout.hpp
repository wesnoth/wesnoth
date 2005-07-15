/*
  Copyright (C) 2003 by David White <davidnwhite@verizon.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "../display.hpp"

#ifndef EDITOR_LAYOUT_H_INCLUDED
#define EDITOR_LAYOUT_H_INCLUDED

namespace map_editor {

/// Size specifications for the map editor.
struct size_specs {
	/// Initialize the values to dummies that will avoid floating point
	/// errors if calculations are made before the sizes are adjuusted
	/// through adjust_sizes().
	size_specs();
	size_t terrain_size;
	size_t terrain_padding;
	size_t terrain_space;
	size_t terrain_width;
	size_t palette_x;
	size_t palette_y;
	size_t palette_h;
	size_t palette_w;
	size_t brush_x;
	size_t brush_y;
	size_t brush_padding;
};

/// Adjust the internal size specifications to fit the display.
void adjust_sizes(const display &disp, size_specs &sizes);


}

#endif // EDITOR_LAYOUT_H_INCLUDED
