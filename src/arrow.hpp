/*
 Copyright (C) 2010 - 2017 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 * Arrows destined to be drawn on the map. Created for the whiteboard project.
 */

#pragma once

#include "display.hpp"

typedef std::vector<map_location> arrow_path_t;

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

public:

	arrow(const arrow&) = delete;
	arrow& operator=(const arrow&) = delete;

	arrow(bool hidden = false);
	virtual ~arrow();

	///Sets the arrow's visibility
	void hide();
	void show();

	virtual void set_path(arrow_path_t const& path);

	///invalidates and clears the present path, forgets the previous path, clears the symbols map
	virtual void reset();

	/**
	 * The string color parameter is in the same format expected by the
	 * image::locator modifiers parameter. Examples: red is "red" or "FF0000" or "255,0,0".
	 * Feel free to add another method that accepts an Uint32 as a parameter instead.
	 */
	virtual void set_color(const std::string& color);

	virtual std::string get_color() const { return color_; }

	/**
	 * The style is simply the name of a subdirectory under images/arrows,
	 * that holds an alternate copy of the arrow graphics.
	 * If it doesn't exist or has missing images, you'll get "under construction"
	 * symbols instead of arrow graphics.
	 */
	std::string get_style() const {return style_;}
	void set_style(const std::string& style);
	///If you add more styles, you should look at move::update_arrow_style()
	static const std::string STYLE_STANDARD;
	static const std::string STYLE_HIGHLIGHTED;
	static const std::string STYLE_FOCUS;
	static const std::string STYLE_FOCUS_INVALID;

	arrow_path_t const& get_path() const;
	arrow_path_t const& get_previous_path() const;

	bool path_contains(map_location const& hex) const;

	virtual void draw_hex(map_location const& hex);

	/// Checks that the path is not of length 0 or 1
	static bool valid_path(arrow_path_t const& path);
	/// Invalidates every hex along the given path
	static void invalidate_arrow_path(arrow_path_t const& path);

	virtual void notify_arrow_changed();

protected:

	/**
	 * Calculate the symbols to place along the arrow path.
	 * Invalidates every hex along the path.
	 */
	virtual void update_symbols();

	display::drawing_layer layer_;

	std::string color_;
	/// represents the subdirectory that holds images for this arrow style
	std::string style_;

	arrow_path_t path_;
	arrow_path_t previous_path_;

	typedef std::map<map_location, image::locator> arrow_symbols_map_t;
	arrow_symbols_map_t symbols_map_;

	bool hidden_;
};
