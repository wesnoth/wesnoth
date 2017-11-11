/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "sdl/point.hpp"

#include <iostream>

point::operator SDL_Point()
{
	return {x, y};
}

point& point::operator+=(const point& point)
{
	x += point.x;
	y += point.y;
	return *this;
}

point& point::operator-=(const point& point)
{
	x -= point.x;
	y -= point.y;
	return *this;
}

std::ostream& operator<<(std::ostream& stream, const point& point)
{
	stream << point.x << ',' << point.y;
	return stream;
}
