/*
   Copyright (C) 2011 - 2017 by Fabian Mueller
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

#include <vector>
#include "map/location.hpp"

#include <SDL.h>

class joystick_manager {

public:

	joystick_manager();

	~joystick_manager();

	bool init();
	bool close();

	/**
	 * Used for absolute movement of the cursor.
	 * @param highlighted_hex will change if the cursor moved.
	 * @return true if the highlighted hex changed.
	 */
	bool update_highlighted_hex(map_location& highlighted_hex);

	/**
	 * Used for relative movement of the cursor.
	 * @param highlighted_hex will change if the cursor moved.
	 * @return true if the highlighted hex changed.
	 */
	bool update_highlighted_hex(map_location& highlighted_hex, const map_location& selected_hex);

	/**
	 * @return a value in range [-1,+1] representing the gauges of the scroll axes.
	 */
	std::pair<double, double> get_scroll_axis_pair();

	/**
	 * TODO fendrin
	 */
	std::pair<double, double> get_cursor_polar_coordinates();

	/**
	 * TODO fendrin
	 */
	std::pair<double, double> get_mouse_axis_pair();

	/**
	 * TODO fendrin
	 */
	double get_thrusta_axis();

	/**
	 * TODO fendrin
	 */
	double get_thrustb_axis();

	/**
	 * TODO fendrin
	 */
	double get_angle();

	/**
	 * TODO fendrin
	 */
	std::pair<double, double> get_polar_coordinates(int joystick_xaxis, int xaxis, int joystick_yaxis, int yaxis);

private:

	enum DIRECTION { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH,
		 SOUTH_WEST, NORTH_WEST, NDIRECTIONS, WEST, EAST };


	std::pair<int, int> get_axis_pair(int joystick_xaxis, int xaxis, int joystick_yaxis, int yaxis);
	int get_axis(int joystick_axis, int axis);

	const map_location get_next_hex(int x_axis, int y_axis, map_location old_hex);

	const map_location get_direction(const map_location& loc, joystick_manager::DIRECTION direction);

	std::vector<SDL_Joystick*> joysticks_;
	int joystick_area_;
	int counter_;
};
