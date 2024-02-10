/*
	Copyright (C) 2008 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
	editor_display(editor_controller& controller, reports& reports_object);

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

	/**
	 * TLD layout() override. Replaces old refresh_reports(). Be sure to
	 * call the base class method as well.
	 *
	 * This updates some reports that may need to be refreshed every frame.
	 */
	virtual void layout() override;

	/** Sets texture to be drawn in hex under the mouse's location. */
	void set_mouseover_hex_overlay(const texture& image)
	{
		mouseover_hex_overlay_ = image;
	}

	void clear_mouseover_hex_overlay()
	{
		mouseover_hex_overlay_.reset();
	}

	/**
	 * Sets and shows the tooltip-like text at the top or bottom of the map area.
	 *
	 * @param str                 The text to display.
	 */
	void set_help_string(const std::string& str);

	/** Removes the help string. */
	void clear_help_string();

protected:
	void draw_hex(const map_location& loc) override;

	/** Inherited from display. */
	virtual overlay_map& get_overlays() override;

	rect get_clip_rect() const override;

	std::set<map_location> brush_locations_;

	/* The controller that owns this display. */
	editor_controller& controller_;

	texture mouseover_hex_overlay_;

private:
	/** ID of the floating label that's controlled by set_help_string() / clear_help_string(). */
	int help_handle_ = 0;

	/**
	 * Ignored when help_handle_ == 0. Othewise, true if the help label obscures the
	 * northern hexes in the map area, false if it's over the southern hexes instead.
	 */
	bool help_string_at_top_ = false;
};

} //end namespace editor
