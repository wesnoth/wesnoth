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

#include "joystick.hpp"
#include "preferences.hpp"

#define PI 3.14159265

joystick_manager::joystick_manager() : joysticks_() {}

joystick_manager::~joystick_manager() {
	  // Close if opened
//if(SDL_JoystickOpened(0))
//  SDL_JoystickClose(joy);
}

bool joystick_manager::init() {
	//LOG_AUDIO << "Initializing audio...\n";
	if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
			return false;

	joysticks_.clear();

	int joysticks = SDL_NumJoysticks();
	if (joysticks == 0) return false;

	SDL_JoystickEventState(SDL_ENABLE);

	bool joystick_found = false;
	for (int i = 0; i<joysticks; i++)  {
		joysticks_.resize(i+1);
		joysticks_[i] = SDL_JoystickOpen(i);

		if (joysticks_[i]) {

			joystick_found = true;

		    printf("Opened Joystick 0\n");
		    printf("Name: %s\n", SDL_JoystickName(i));
		    printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joysticks_[i]));
		    printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joysticks_[i]));
		    printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joysticks_[i]));
		} else {
			printf("Couldn't open Joystick 0\n");
		}
	}
	return joystick_found;
}

std::pair<double, double> joystick_manager::get_scroll_axis_pair() {

	int scroll_joystick_x = preferences::scroll_joystick_xaxis();
	int scroll_axis = preferences::scroll_xaxis();

	int scroll_joystick_y = preferences::scroll_joystick_yaxis();
	int scroll_yaxis = preferences::scroll_yaxis();

	std::pair<int, int> values = get_axis_pair(scroll_joystick_x, scroll_axis, scroll_joystick_y, scroll_yaxis);

	const int radius = sqrt(pow(values.first, 2.0f) + pow(values.second, 2.0f));

	const int threshold = preferences::scroll_threshold();

	if (threshold > radius)
		return std::make_pair(0.0, 0.0);

	return std::make_pair(((double)values.first) / 32768.0, ((double)values.second) / 32768.0);
//	return ((double)SDL_JoystickGetAxis(joysticks_[scroll_joystick], scroll_axis)) / 32768.0;
}

//double joystick_manager::get_scroll_yaxis() {
//
//
//
//
//
//	if(!SDL_JoystickOpened(scroll_joystick))
//		return 0.0;
//	if(SDL_JoystickNumAxes(joysticks_[scroll_joystick]) < scroll_yaxis)
//		return 0.0;
//	return ((double)SDL_JoystickGetAxis(joysticks_[scroll_joystick], scroll_yaxis)) / 32768.0;
//}

std::pair<int, int> joystick_manager::get_axis_pair(int joystick_xaxis, int xaxis, int joystick_yaxis, int yaxis) {

	int x_axis = 0, y_axis =0;
	bool get_xaxis = false, get_yaxis = false;

	if(SDL_JoystickOpened(joystick_xaxis))
		if(SDL_JoystickNumAxes(joysticks_[joystick_xaxis]) > xaxis)
			get_xaxis = true;

	if(SDL_JoystickOpened(joystick_yaxis))
		if(SDL_JoystickNumAxes(joysticks_[joystick_yaxis]) > yaxis)
			get_yaxis = true;

	//TODO Does the block prevent the commands from being interrupted?
	//We want the readings to be from a similar time slice.
	{
			if (get_xaxis) x_axis = SDL_JoystickGetAxis(joysticks_[joystick_xaxis], xaxis);
			if (get_yaxis) y_axis = SDL_JoystickGetAxis(joysticks_[joystick_yaxis], yaxis);
	}
	return std::make_pair(x_axis, y_axis);
}


bool joystick_manager::next_highlighted_hex(map_location& highlighted_hex) {


	const int cursor_joystick_xaxis = preferences::cursor_joystick_xaxis();
	const int cursor_xaxis = preferences::cursor_xaxis();

	const int cursor_joystick_yaxis = preferences::cursor_joystick_yaxis();
	const int cursor_yaxis = preferences::cursor_yaxis();

	const std::pair<int, int> values = get_axis_pair(cursor_joystick_xaxis, cursor_xaxis, cursor_joystick_yaxis, cursor_yaxis);

	const int x_axis = values.first;
	const int y_axis = values.second;

	const int radius = sqrt(pow(x_axis, 2.0f) + pow(y_axis, 2.0f));

	const int threshold = 1600;
	const int threshold2 = 10*threshold;
	const int max = 100000;

	const bool greater_threshold = radius > threshold;

	const bool greater_threshold2 = radius > threshold2;

	const int radius = sqrt(pow(x_axis, 2.0f) + pow(y_axis, 2.0f));

	if (!greater_threshold) {
		counter_ = 0;
		joystick_area_ = 0;
		return false;
	} else {
		if (joystick_area_ == 0) {
			highlighted_hex = get_next_hex(x_axis, y_axis, highlighted_hex);
		}
		if (!greater_threshold2) {
			joystick_area_ = 1;
		} else {
			joystick_area_ = 2;
			counter_ += radius;
			if (counter_ > max) {
				counter_ -= max;
				highlighted_hex = get_next_hex(x_axis, y_axis, highlighted_hex);
				return true;
			} else return false;
		}
	}

	return true;
}

const map_location joystick_manager::get_direction(const map_location& loc, joystick_manager::DIRECTION direction) {

	int x = loc.x;
	int y = loc.y;

	switch(direction) {
		case NORTH:      return map_location(x, y - 1);
		case SOUTH:      return map_location(x, y + 1);
		case SOUTH_EAST: return map_location(x + 1, y + (1+is_odd(x))/2 );
		case SOUTH_WEST: return map_location(x - 1, y + (1+is_odd(x))/2 );
		case NORTH_EAST: return map_location(x + 1, y - (1+is_even(x))/2 );
		case NORTH_WEST: return map_location(x - 1, y - (1+is_even(x))/2 );
		case WEST:       return map_location(x - 1, y);
		case EAST:       return map_location(x + 1, y);
		default:
			assert(false);
			return map_location();
	}
}

const map_location joystick_manager::get_next_hex(int x_axis, int y_axis, map_location loc)  {

	map_location new_loc = map_location::null_location;
	const double winkel = (atan2((double)y_axis, (double)x_axis)) * 180.0 / PI;

	if (winkel < -112.5 && winkel > -157.5)
		new_loc = get_direction(loc, NORTH_WEST);

	if (winkel < -67.5 && winkel > -112.5)
		new_loc = get_direction(loc, NORTH);

	if (winkel < -22.5 && winkel > -67.5)
		new_loc = get_direction(loc, NORTH_EAST);

	if (winkel < 22.5 && winkel > -22.5 )
		new_loc = get_direction(loc, EAST);

	if (winkel > 22.5 && winkel < 67.5 )
		new_loc = get_direction(loc, SOUTH_EAST);

	if (winkel > 67.5 && winkel < 113.5)
		new_loc = get_direction(loc, SOUTH);

	if (winkel > 113.5 && winkel < 158.5)
		new_loc = get_direction(loc, SOUTH_WEST);

	if (winkel > 158.5 || winkel < -157.5)
		new_loc = get_direction(loc, WEST);

	return new_loc;
}

