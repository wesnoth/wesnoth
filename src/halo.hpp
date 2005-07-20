/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef HALO_HPP_INCLUDED
#define HALO_HPP_INCLUDED

class display;
#include "animated.hpp"

#include <string>

namespace halo
{

struct manager
{
	manager(display& disp);
	~manager();

private:
	display* const old;
};

struct halo_hider
{
	halo_hider();
	~halo_hider();
private:
	bool old;
};

enum ORIENTATION { NORMAL, REVERSE };

///function to add a haloing effect using 'image'
///centered on (x,y)
///returns the handle to the halo object
int add(int x, int y, const std::string& image, ORIENTATION orientation=NORMAL, int lifetime_cycles=-1);

///function to set the position of an existing haloing
///effect, according to its handle
void set_location(int handle, int x, int y);

///function to remove the halo with the given handle
void remove(int handle);

struct remover
{
	void operator()(int handle) const { remove(handle); }
};

///functions to render and unrender all haloes. Should
///be called immediately before/after flipping the screen
void render();
void unrender();

}

#endif
