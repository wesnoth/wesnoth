/*
   Copyright (C) 2008 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_EDITOR_DISPLAY_HPP_INCLUDED
#define EDITOR_EDITOR_DISPLAY_HPP_INCLUDED

#include "map/editor_map.hpp"
#include "../display.hpp"

namespace editor {

const display_context * get_dummy_display_context();

class editor_display : public display
{
public:
	editor_display(const display_context * dc, CVideo& video,
			reports & reports_object,
			const config& theme_cfg, const config& level);

	bool in_editor() const { return true; }

	void add_brush_loc(const map_location& hex);
	void set_brush_locs(const std::set<map_location>& hexes);
	void clear_brush_locs();
	void remove_brush_loc(const map_location& hex);
	const editor_map& map() const { return static_cast<const editor_map&>(get_map()); }
	void rebuild_terrain(const map_location &loc);

	void set_palette_report(const config& palette_report) {palette_report_ = palette_report;}

protected:
	void pre_draw();
	/**
	* The editor uses different rules for terrain highlighting (e.g. selections)
	*/
	image::TYPE get_image_type(const map_location& loc);

	void draw_hex(const map_location& loc);

	const SDL_Rect& get_clip_rect();
	void draw_sidebar();

	std::set<map_location> brush_locations_;

private:
	config palette_report_;
};

} //end namespace editor
#endif
