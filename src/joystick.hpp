/* $Id$ */
/*
   Copyright (C) 2011 - 2011 by Fabian Mueller
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <vector>
#include "sdl_utils.hpp"
#include "map.hpp"

#ifndef JOYSTICK_HPP_
#define JOYSTICK_HPP_

class joystick_manager {

public:

	joystick_manager();

	~joystick_manager();

	bool init();

	/**
	 * @param highlighted_hex will change if the cursor moved
	 * @return true if the highlighted hex changed.
	 */
	bool next_highlighted_hex(map_location& highlighted_hex);


	/**
	 * @returns a value in range [-1,+1] representing the gauge of the scroll xaxis
	 */
	std::pair<double,double> get_scroll_axis_pair();

	/**
	 * @returns a value in range [-1,+1] representing the gauge of the scroll yaxis
	 */
//	double get_scroll_yaxis();

private:

	enum DIRECTION { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH,
		 SOUTH_WEST, NORTH_WEST, NDIRECTIONS, WEST, EAST };


	std::pair<int, int> get_axis_pair(int joystick_xaxis, int xaxis, int joystick_yaxis, int yaxis);

	const map_location get_next_hex(int x_axis, int y_axis, map_location old_hex);

	const map_location get_direction(const map_location& loc, joystick_manager::DIRECTION direction);

	std::vector<SDL_Joystick*> joysticks_;
	int joystick_area_;
	int counter_;
};

#endif /* JOYSTICK_HPP_ */
