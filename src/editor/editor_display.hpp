/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "map/editor_map.hpp"
#include "display.hpp"

namespace editor {

class editor_display : public display
{
public:
	editor_display(editor_controller& controller, reports& reports_object, const config& theme_cfg);

	bool in_editor() const override { return true; }

	void add_brush_loc(const map_location& hex);
	void set_brush_locs(const std::set<map_location>& hexes);
	void clear_brush_locs();
	void remove_brush_loc(const map_location& hex);
	const editor_map& map() const { return static_cast<const editor_map&>(get_map()); }
	void rebuild_terrain(const map_location &loc);

	/** Inherited from display. */
	virtual const time_of_day& get_time_of_day(const map_location& loc = map_location::null_location()) const override;

	editor_controller& get_controller()
	{
		return controller_;
	}

protected:
	void pre_draw() override;
	/**
	* The editor uses different rules for terrain highlighting (e.g. selections)
	*/
	image::TYPE get_image_type(const map_location& loc) override;

	void draw_hex(const map_location& loc) override;

	const SDL_Rect& get_clip_rect() override;
	void draw_sidebar() override;

	std::set<map_location> brush_locations_;

	/* The controller that owns this display. */
	editor_controller& controller_;
};

} //end namespace editor
