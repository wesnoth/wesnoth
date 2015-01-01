/*
   Copyright (C) 2004 - 2015 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EXPLODER_UTILS_HPP_INCLUDED
#define EXPLODER_UTILS_HPP_INCLUDED

#include "../sdl/utils.hpp"
#include <string>

struct exploder_failure
{
	exploder_failure(const std::string& message) :
	       message(message) {}

	std::string message;
};

struct exploder_point
{
	exploder_point() : x(0), y(0) {}
	exploder_point(int x, int y) : x(x), y(y) {}
	exploder_point(const std::string &s);

	int x;
	int y;
};

struct exploder_rect
{
	exploder_rect() : x(0), y(0), w(0), h(0) {}
	exploder_rect(int x,int y, int w, int h) : x(x), y(y), w(w), h(h) {}
	exploder_rect(const std::string &s);

	int x;
	int y;
	int w;
	int h;
};

std::string get_mask_dir();
std::string get_exploder_dir();

void masked_overwrite_surface(surface dest, surface src, surface mask, int x, int y);
bool image_empty(surface surf);
void save_image(surface surf, const std::string &filename);

#endif
