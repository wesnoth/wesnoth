/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

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
	size_t nterrains;
	size_t terrain_size;
	size_t terrain_padding;
	size_t terrain_space;
	size_t palette_x;
	size_t button_x;
	size_t brush_x;
	size_t brush_y;
	size_t top_button_y;
	size_t palette_y;
	size_t bot_button_y;
};

/// Adjust the internal size specifications to fit the display.
void adjust_sizes(const display &disp, size_specs &sizes,
				  const unsigned int num_terrains);
	

}

#endif // EDITOR_LAYOUT_H_INCLUDED
