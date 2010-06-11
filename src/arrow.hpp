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

typedef std::map<map_location, image::locator> arrow_symbols_map_t;

typedef std::vector<map_location> arrow_path_t;

class arrow_observer;

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

public:
	//operations

	arrow(display* screen);

	 /** Notifies it's arrow_observer list of the arrow's destruction */
	virtual ~arrow() {
		notify_arrow_deleted();
	}

	virtual void set_path(const arrow_path_t &path);

	/**
	 * The string color parameter is in the same format expected by the
	 * image::locator modifiers parameter. Examples: red is "FF0000" or "255,0,0".
	 * Feel free to add another method that accepts an Uint32 as a parameter instead.
	 */
	void set_color(const std::string& color);

	void set_layer(const display::tdrawing_layer & layer);

	const arrow_path_t & get_path() const;

	const arrow_path_t & get_previous_path() const;

	void draw_hex(const map_location & hex);

	void add_observer(arrow_observer & observer);

	void remove_observer(arrow_observer & observer);

protected:
	//operations

	/**
	 * @param old_path : the path to erase and replace with the new symbols
	 */
	void update_symbols(arrow_path_t old_path);

private:
	//operations

	void invalidate_arrow_path(arrow_path_t path);

	void notify_arrow_changed();

	void notify_arrow_deleted();

private:
	//properties

	display* screen_;

	display::tdrawing_layer layer_;

	std::string color_;

	arrow_path_t path_;
	arrow_path_t previous_path_;

	arrow_symbols_map_t symbols_map_;

    std::list<arrow_observer*> observers_;

};
#endif
