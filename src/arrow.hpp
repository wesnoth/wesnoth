/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file arrow.hpp
 * Arrows destined to be drawn on the map. Created for the whiteboard project.
 */

#ifndef _ARROW_H
#define _ARROW_H

#include "display.hpp"

#include <vector>
#include <list>
#include <map>

typedef std::vector<map_location> arrow_path_t;

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

	typedef std::map<map_location, image::locator> arrow_symbols_map_t;

public:
	//operations

	arrow();
	virtual ~arrow();

	virtual void set_path(const arrow_path_t &path);

	virtual void reset();

	/**
	 * The string color parameter is in the same format expected by the
	 * image::locator modifiers parameter. Examples: red is "red" or "FF0000" or "255,0,0".
	 * Feel free to add another method that accepts an Uint32 as a parameter instead.
	 */
	virtual void set_color(const std::string& color);

	/**
	 * The style is simply the name of a subdirectory under images/arrows,
	 * that holds an alternate copy of the arrow graphics.
	 * If it doesn't exist or has missing images, you'll get "under construction"
	 * symbols instead of arrow graphics.
	 */
	void set_style(const std::string& style);

	void set_layer(const display::tdrawing_layer & layer);

	/// Sets transparency to the specified alpha value
	/// 0.5 is 50% transparent, anything above 1.0 brightens the arrow images
	void set_alpha(double alpha);

	/// If set to false (default), the end symbol is drawn in the second-to-last hex
	void set_draw_last_hex(bool draw_last_hex) { draw_last_hex_ = draw_last_hex; }

	const arrow_path_t & get_path() const;

	const arrow_path_t & get_previous_path() const;

	bool path_contains(const map_location & hex) const;

	virtual void draw_hex(const map_location & hex);

	/// Checks that the path is not of length 0 or 1
	virtual bool valid_path(arrow_path_t path) const;

	virtual void notify_arrow_changed();

protected:
	//operations

	/**
	 * @param old_path : the path to erase and replace with the new symbols
	 */
	virtual void update_symbols(arrow_path_t old_path);

	virtual void invalidate_arrow_path(arrow_path_t path);

protected:
	//properties

	display::tdrawing_layer layer_;

	std::string color_;
	/// represents the subdirectory that holds images for this arrow style
	std::string style_;
	float alpha_;
	bool draw_last_hex_;

	arrow_path_t path_;
	arrow_path_t previous_path_;

	arrow_symbols_map_t symbols_map_;
};
#endif
