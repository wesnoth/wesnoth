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

	bool next_highlighted_hex(map_location& highlighted_hex);

	double get_scroll_xaxis();

	double get_scroll_yaxis();

private:

	const map_location get_next_hex(int x_axis, int y_axis, map_location old_hex);

	/**
	 * TODO fendrin
	 */
	bool handle_cursor(int x_offset, int y_offset);

	std::vector<SDL_Joystick*> joysticks_;
	int joystick_area_;
	int counter_;
};

#endif /* JOYSTICK_HPP_ */
