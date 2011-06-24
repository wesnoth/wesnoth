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
#include "log.hpp"

#define PI 3.14159265

static lg::log_domain log_joystick("joystick");
#define ERR_JOY LOG_STREAM(err, log_joystick)
#define LOG_JOY LOG_STREAM(info, log_joystick)
#define DBG_JOY LOG_STREAM(debug, log_joystick)

joystick_manager::joystick_manager()
	: joysticks_()
	, joystick_area_(0)
	, counter_(0)
{
	init();
}

joystick_manager::~joystick_manager() {
	close();
}

bool joystick_manager::close() {

	int joysticks = joysticks_.size();
	bool all_closed = true;

	for (int i = 0; i<joysticks; i++)  {
		if (SDL_JoystickOpened(i)) {
			SDL_JoystickClose(joysticks_[i]);
			LOG_JOY << "Closed Joystick" << i;
		    LOG_JOY << "Name: " << SDL_JoystickName(i);
		} else {
			ERR_JOY << "Joystick" << i << " closing failed.";
			all_closed = false;
		}
	}

	joysticks_.clear();
	return all_closed;
}

bool joystick_manager::init() {
	LOG_JOY << "Initializing joysticks...\n";
	if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
			return false;

	joysticks_.clear();

	int joysticks = SDL_NumJoysticks();
	if (joysticks == 0) return false;

	SDL_JoystickEventState(SDL_ENABLE);

	bool joystick_found = false;
	for (int i = 0; i<joysticks; i++) {
		joysticks_.resize(i+1);
		joysticks_[i] = SDL_JoystickOpen(i);

		if (joysticks_[i] && SDL_JoystickOpened(i)) {

			joystick_found = true;

		    LOG_JOY << "Opened Joystick" << i;
		    LOG_JOY << "Name: " << SDL_JoystickName(i);
		    LOG_JOY << "Number of Axes: " << SDL_JoystickNumAxes(joysticks_[i]);
		    LOG_JOY << "Number of Buttons: " << SDL_JoystickNumButtons(joysticks_[i]);
		    LOG_JOY << "Number of Balls: " << SDL_JoystickNumBalls(joysticks_[i]);
		    LOG_JOY << "Number of Hats: ", SDL_JoystickNumHats(joysticks_[i]);
		} else {
			ERR_JOY << "Couldn't open Joystick" << i;
		}
	}
	return joystick_found;
}

std::pair<double, double> joystick_manager::get_mouse_axis_pair() {

	const int mouse_joystick_x = preferences::joystick_num_mouse_xaxis();
	const int mouse_xaxis = preferences::joystick_mouse_xaxis_num();

	const int mouse_joystick_y = preferences::joystick_num_mouse_yaxis();
	const int mouse_yaxis = preferences::joystick_mouse_yaxis_num();

	std::pair<int, int> values;
	double thrust;
	{
		values = get_axis_pair(mouse_joystick_x, mouse_xaxis, mouse_joystick_y, mouse_yaxis);
		thrust = get_thrust_xaxis();
	}

	const int radius = round_double(sqrt(pow(values.first, 2.0f) + pow(values.second, 2.0f)));
	const int deadzone = preferences::joystick_mouse_deadzone();
	const double multiplier = 1.0 + thrust;

	if (deadzone > radius)
		return std::make_pair(0.0, 0.0);

	// TODO do some math to normalize over the value - deadzone.
	//const double relation = abs( (double)values.first / (double)values.second );
	//const int range_x = values.first - round_double(relation * deadzone);
	//const int range_y = values.second - ((1.0 - relation) * deadzone);
	//double x_value = ((double)(values.first - deadzone) / (double)(32768 - deadzone)) *

	return std::make_pair( (((double)values.first) / 32768.0) * multiplier, (((double)values.second) / 32768.0) * multiplier );

}

std::pair<double, double> joystick_manager::get_scroll_axis_pair() {

	const int scroll_joystick_x = preferences::joystick_num_scroll_xaxis();
	const int scroll_axis = preferences::joystick_scroll_xaxis_num();

	const int scroll_joystick_y = preferences::joystick_num_scroll_yaxis();
	const int scroll_yaxis = preferences::joystick_scroll_yaxis_num();

	std::pair<int, int> values;
	double thrust;
	{
		values = get_axis_pair(scroll_joystick_x, scroll_axis, scroll_joystick_y, scroll_yaxis);
		thrust = get_thrust_xaxis();
	}

	const int radius = round_double(sqrt(pow(values.first, 2.0f) + pow(values.second, 2.0f)));
	const int deadzone = preferences::joystick_scroll_deadzone();
	const double multiplier = 1.0 + thrust;

	if (deadzone > radius)
		return std::make_pair(0.0, 0.0);

	return std::make_pair( (((double)values.first) / 32768.0) * multiplier, (((double)values.second) / 32768.0) * multiplier );
}

double joystick_manager::get_thrust_xaxis() {
	const int thrust_joystick_x = preferences::joystick_num_thrust_xaxis();
	const int thrust_axis_x = preferences::joystick_thrust_xaxis_num();
//	const int thrust_axis_x_deadzone = preferences::joystick_thrust_deadzone();

	const int value = get_axis(thrust_joystick_x, thrust_axis_x) + 32768;
	return (double)value / 65536;
}

std::pair<int, int> joystick_manager::get_axis_pair(int joystick_xaxis, int xaxis, int joystick_yaxis, int yaxis) {

	int x_axis = 0, y_axis = 0;
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

int joystick_manager::get_axis(int joystick_axis, int axis) {

	if(SDL_JoystickOpened(joystick_axis))
		if(SDL_JoystickNumAxes(joysticks_[joystick_axis]) > axis)
			return SDL_JoystickGetAxis(joysticks_[joystick_axis], axis);
	return 0;
}

/*
bool joystick_manager::update_highlighted_hex(map_location& highlighted_hex, const map_location& selected_hex) {

	const int cursor_joystick_xaxis = preferences::joystick_num_cursor_xaxis();
	const int cursor_xaxis = preferences::joystick_cursor_xaxis_num();

	const int cursor_joystick_yaxis = preferences::joystick_num_cursor_yaxis();
	const int cursor_yaxis = preferences::joystick_cursor_yaxis_num();

	const std::pair<int, int> values = get_axis_pair(cursor_joystick_xaxis, cursor_xaxis, cursor_joystick_yaxis, cursor_yaxis);

	const int x_axis = values.first;
	const int y_axis = values.second;

	const int radius = round_double(sqrt(pow(x_axis, 2.0f) + pow(y_axis, 2.0f)));

	const int deadzone = preferences::joystick_cursor_deadzone();
	//const int threshold2 = 10*threshold;
	//const int max = 100000;

	const bool greater_deadzone = radius > deadzone;
	//const bool greater_threshold2 = radius > threshold2;

	int x = selected_hex.x + round_double(x_axis / 3200);
	int y = selected_hex.y + round_double(y_axis / 3200);
	highlighted_hex = map_location(x,y);

	if (!greater_threshold) {
		counter_ = 0;
		joystick_area_ = 0;
		return false;
	}

	return true;
}
*/

bool joystick_manager::update_highlighted_hex(map_location& highlighted_hex) {

	const int cursor_joystick_xaxis = preferences::joystick_num_cursor_xaxis();
	const int cursor_xaxis = preferences::joystick_cursor_xaxis_num();

	const int cursor_joystick_yaxis = preferences::joystick_num_cursor_yaxis();
	const int cursor_yaxis = preferences::joystick_cursor_yaxis_num();

	const std::pair<int, int> values = get_axis_pair(cursor_joystick_xaxis, cursor_xaxis, cursor_joystick_yaxis, cursor_yaxis);

	const int x_axis = values.first;
	const int y_axis = values.second;

	const int radius = round_double(sqrt(pow(x_axis, 2.0f) + pow(y_axis, 2.0f)));

	const int deadzone = preferences::joystick_cursor_deadzone();
	const int threshold = deadzone + preferences::joystick_cursor_threshold();
	//TODO fendrin take max from preferences as well
	const int max = 100000;

	const bool greater_deadzone = radius > deadzone;
	const bool greater_threshold2 = radius > threshold;

	if (!greater_deadzone) {
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

	if (x_axis == 0) return (y_axis > 0) ? get_direction(loc, SOUTH) : get_direction(loc, NORTH);
	if (y_axis == 0) return (x_axis > 0) ? get_direction(loc, EAST) : get_direction(loc, WEST);
	const double angle = (atan2((double)y_axis, (double)x_axis)) * 180.0 / PI;

	if (angle < -112.5 && angle > -157.5)
		new_loc = get_direction(loc, NORTH_WEST);

	if (angle < -67.5 && angle > -112.5)
		new_loc = get_direction(loc, NORTH);

	if (angle < -22.5 && angle > -67.5)
		new_loc = get_direction(loc, NORTH_EAST);

	if (angle < 22.5 && angle > -22.5 )
		new_loc = get_direction(loc, EAST);

	if (angle > 22.5 && angle < 67.5 )
		new_loc = get_direction(loc, SOUTH_EAST);

	if (angle > 67.5 && angle < 113.5)
		new_loc = get_direction(loc, SOUTH);

	if (angle > 113.5 && angle < 158.5)
		new_loc = get_direction(loc, SOUTH_WEST);

	if (angle > 158.5 || angle < -157.5)
		new_loc = get_direction(loc, WEST);

	return new_loc;
}

