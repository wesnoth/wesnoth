/*
	Copyright (C) 2010 - 2024
	by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

	explicit arrow(bool hidden = false);
	~arrow();

	/** Sets the arrow's visibility */
	void hide();
	void show();

	void set_path(const arrow_path_t& path);

	/** invalidates and clears the present path, forgets the previous path, clears the symbols map */
	void reset();

	/**
	 * The string color parameter is in the same format expected by the
	 * image::locator modifiers parameter. Examples: red is "red" or "FF0000" or "255,0,0".
	 * Feel free to add another method that accepts an uint32_t as a parameter instead.
	 */
	void set_color(const std::string& color);

	std::string get_color() const { return color_; }

	/**
	 * The style is simply the name of a subdirectory under images/arrows,
	 * that holds an alternate copy of the arrow graphics.
	 * If it doesn't exist or has missing images, you'll get "under construction"
	 * symbols instead of arrow graphics.
	 */
	std::string get_style() const {return style_;}
	void set_style(const std::string& style);
	/** If you add more styles, you should look at move::update_arrow_style() */
	static const std::string STYLE_STANDARD;
	static const std::string STYLE_HIGHLIGHTED;
	static const std::string STYLE_FOCUS;
	static const std::string STYLE_FOCUS_INVALID;

	const arrow_path_t& get_path() const;
	const arrow_path_t& get_previous_path() const;

	bool path_contains(const map_location& hex) const;

	image::locator get_image_for_loc(const map_location& hex) const;

	/** Checks that the path is not of length 0 or 1 */
	static bool valid_path(const arrow_path_t& path);
	/** Invalidates every hex along the given path */
	static void invalidate_arrow_path(const arrow_path_t& path);

	void notify_arrow_changed();

private:

	/**
	 * Calculate the symbols to place along the arrow path.
	 * Invalidates every hex along the path.
	 */
	void update_symbols();

	std::string color_;
	/** represents the subdirectory that holds images for this arrow style */
	std::string style_;

	arrow_path_t path_;
	arrow_path_t previous_path_;

	typedef std::map<map_location, image::locator> arrow_symbols_map_t;
	arrow_symbols_map_t symbols_map_;

	bool hidden_;
};
