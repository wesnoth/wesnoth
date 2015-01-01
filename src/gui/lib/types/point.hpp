/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_LIB_TYPES_POINT_HPP_INCLUDED
#define GUI_LIB_TYPES_POINT_HPP_INCLUDED

#include <iosfwd>

namespace gui2
{

/** Holds a 2D point. */
struct tpoint
{
	tpoint(const int x_, const int y_) : x(x_), y(y_)
	{
	}

	/** x coordinate. */
	int x;

	/** y coordinate. */
	int y;

	bool operator==(const tpoint& point) const
	{
		return x == point.x && y == point.y;
	}
	bool operator!=(const tpoint& point) const
	{
		return x != point.x || y != point.y;
	}
	bool operator<(const tpoint& point) const
	{
		return x < point.x || (x == point.x && y < point.y);
	}

	bool operator<=(const tpoint& point) const
	{
		return x < point.x || (x == point.x && y <= point.y);
	}

	tpoint operator+(const tpoint& point) const
	{
		return tpoint(x + point.x, y + point.y);
	}

	tpoint& operator+=(const tpoint& point);

	tpoint operator-(const tpoint& point) const
	{
		return tpoint(x - point.x, y - point.y);
	}

	tpoint& operator-=(const tpoint& point);
};

std::ostream& operator<<(std::ostream& stream, const tpoint& point);


} // namespace gui2

#endif
