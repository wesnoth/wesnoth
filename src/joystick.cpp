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

double joystick_manager::get_scroll_xaxis() {
	int scroll_joystick = preferences::scroll_joystick_xaxis();
	int scroll_axis = preferences::scroll_xaxis();
	if(!SDL_JoystickOpened(scroll_joystick))
		return 0.0;
	if(SDL_JoystickNumAxes(joysticks_[scroll_joystick]) < scroll_axis)
		return 0.0;
	return ((double)SDL_JoystickGetAxis(joysticks_[scroll_joystick], scroll_axis)) / 32768.0;
}

double joystick_manager::get_scroll_yaxis() {
	int scroll_joystick = preferences::scroll_joystick_yaxis();
	int scroll_yaxis = preferences::scroll_yaxis();
	if(!SDL_JoystickOpened(scroll_joystick))
		return 0.0;
	if(SDL_JoystickNumAxes(joysticks_[scroll_joystick]) < scroll_yaxis)
		return 0.0;
	return ((double)SDL_JoystickGetAxis(joysticks_[scroll_joystick], scroll_yaxis)) / 32768.0;
}

bool joystick_manager::next_highlighted_hex(map_location& highlighted_hex) {
	if(!SDL_JoystickOpened(0))
		return false;

	int x_axis = 0, y_axis =0;

	int cursor_joystick_xaxis = preferences::cursor_joystick_xaxis();
	int cursor_xaxis = preferences::cursor_xaxis();
	if(SDL_JoystickOpened(cursor_joystick_xaxis))
		if(SDL_JoystickNumAxes(joysticks_[cursor_joystick_xaxis]) > cursor_xaxis)
			x_axis = SDL_JoystickGetAxis(joysticks_[cursor_joystick_xaxis], cursor_xaxis);

	int cursor_joystick_yaxis = preferences::cursor_joystick_yaxis();
	int cursor_yaxis = preferences::cursor_yaxis();
	if(SDL_JoystickOpened(cursor_joystick_yaxis))
		if(SDL_JoystickNumAxes(joysticks_[cursor_joystick_yaxis]) > cursor_yaxis)
			y_axis = SDL_JoystickGetAxis(joysticks_[cursor_joystick_yaxis], cursor_yaxis);

	//int scroll_speed = preferences::scroll_speed();

	const int threshold = 1600;
	const int threshold2 = 10*threshold;
	const int max = 100000;

	const bool greater_threshold = ((x_axis < -threshold) || (x_axis > threshold)) ||
			((y_axis < -threshold) || (y_axis > threshold));

	const bool greater_threshold2 = ((x_axis < -threshold2) || (x_axis > threshold2)) ||
			((y_axis < -threshold2) || (y_axis > threshold2));

	const int radius = sqrt(pow(x_axis, 2) + pow(y_axis, 2));

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

const map_location joystick_manager::get_next_hex(int x_axis, int y_axis, map_location loc)  {

	map_location new_loc = map_location::null_location;
	double winkel = (atan2((double)y_axis, (double)x_axis)) * 180.0 / PI;

	if (winkel < -112.5 && winkel > -157.5)
		new_loc = loc.get_direction(map_location::NORTH_WEST);

	if (winkel < -67.5 && winkel > -112.5)
		new_loc = loc.get_direction(map_location::NORTH);

	if (winkel < -22.5 && winkel > -67.5)
		new_loc = loc.get_direction(map_location::NORTH_EAST);

	if (winkel < 22.5 && winkel > -22.5 )
		new_loc = loc.get_direction(map_location::EAST);

	if (winkel > 22.5 && winkel < 67.5 )
		new_loc = loc.get_direction(map_location::SOUTH_EAST);

	if (winkel > 67.5 && winkel < 113.5)
		new_loc = loc.get_direction(map_location::SOUTH);

	if (winkel > 113.5 && winkel < 158.5)
		new_loc = loc.get_direction(map_location::SOUTH_WEST);

	if (winkel > 158.5 || winkel < -157.5)
		new_loc = loc.get_direction(map_location::WEST);

	return new_loc;
}

