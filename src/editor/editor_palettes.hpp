/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#ifndef EDITOR_PALETTES_H_INCLUDED
#define EDITOR_PALETTES_H_INCLUDED

#include "SDL.h"

#include "../sdl_utils.hpp"
#include "../display.hpp"
#include "../map.hpp"
#include "editor_layout.hpp"

#include <vector>

namespace map_editor {

/// A palette where the terrain to be drawn can be selected.
class terrain_palette {
public:
	terrain_palette(display &gui, const size_specs &sizes,
					const gamemap &map);

	/// Scroll the terrain palette up one step if possible.
	void scroll_up();

	/// Scroll the terrain palette down one step if possible.
	void scroll_down();

	/// Return the currently selected terrain.
	gamemap::TERRAIN selected_terrain() const;
	
	/// Select a terrain.
	void select_terrain(gamemap::TERRAIN);

	/// To be called when a mouse click occurs. Check if the coordinates
	/// is a terrain that may be chosen, select the terrain if that is
	/// the case.
	void left_mouse_click(const int mousex, const int mousey);

	// Draw the palette. If force is true everything will be redrawn
	// even though it is not invalidated.
	void draw(bool force=false);

	/// Return the number of terrains in the palette.
	size_t num_terrains() const;


private:
	/// Return the number of the tile that is at coordinates (x, y) in the
	/// panel.
	int tile_selected(const int x, const int y) const;
					  
	const size_specs &size_specs_;
	scoped_sdl_surface surf_;
	display &gui_;
	unsigned int tstart_;
	std::vector<gamemap::TERRAIN> terrains_;
	gamemap::TERRAIN selected_terrain_;
	const gamemap &map_;
	// Set invalid_ to true if an operation that requires that the
	// palette is redrawn takes place.
	bool invalid_;
};

/// A bar where the brush is drawin
class brush_bar
{
public:
	brush_bar(display &gui, const size_specs &sizes);

	/// Return the size of currently selected brush.
 	unsigned int selected_brush_size();

	/// To be called when a mouse click occurs. Check if the coordinates
	/// is a terrain that may be chosen, select the terrain if that is
	/// the case.
 	void left_mouse_click(const int mousex, const int mousey);

	// Draw the palette. If force is true everything will be redrawn
	// even though it is not invalidated.
	void draw(bool force=false);

private:
	/// Return the index of the brush that is at coordinates (x, y) in the
	/// panel.
	int selected_index(const int x, const int y) const;
					  
	const size_specs &size_specs_;
	scoped_sdl_surface surf_;
	display &gui_;
	unsigned int selected_;
	const int total_brush_;
	const size_t size_;
	// Set invalid_ to true if an operation that requires that the
	// bar is redrawn takes place.
	bool invalid_;
};


}
#endif // EDITOR_PALETTES_H_INCLUDED
