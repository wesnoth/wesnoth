/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef __GUI_WIDGETS_HELPER_HPP_INCLUDED__
#define __GUI_WIDGETS_HELPER_HPP_INCLUDED__

#include "SDL.h"

#include <string>

namespace gui2 {

// init needs a cfg object later to init the subsystem
bool init();

struct tpoint
{
	tpoint(const int x_, const int y_) : 
		x(x_),
		y(y_) 
		{}

	int x;
	int y;

	bool operator==(const tpoint& point) const { return x == point.x && y == point.y; }
	bool operator!=(const tpoint& point) const { return x != point.x || y != point.y; }
	bool operator<(const tpoint& point) const 
		{ return x < point.x || (x == point.x && y < point.y); }

	bool operator<=(const tpoint& point) const 
		{ return x < point.x || (x == point.x && y <= point.y); }
		
	tpoint operator+(const tpoint& point) const 
		{ return tpoint(x + point.x, y + point.y); }

	tpoint& operator+=(const tpoint& point);

};


std::ostream &operator<<(std::ostream &stream, const tpoint& point);

SDL_Rect create_rect(const tpoint& origin, const tpoint& size);

struct terror 
{
	terror(const std::string& msg) : message(msg) {}

	const std::string message;
};

Uint32 decode_colour(const std::string& colour);

int decode_font_style(const std::string& style);

} // namespace gui2

#endif
