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

#include "map.hpp"
#include <set>
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

enum ORIENTATION { NORMAL, HREVERSE, VREVERSE, HVREVERSE };

///function to add a haloing effect using 'image'
///centered on (x,y)
///returns the handle to the halo object
///0 is the invalid handle
//
// if the halo is attached to an item it needs to be hidden if the shroud is
// active. (Note it will be shown with the fog active.) If it's not attached
// to an iten the location should be set to -1, -1
int add(int x, int y, const std::string& image, const gamemap::location& loc,
		ORIENTATION orientation=NORMAL, bool infinite=true);

///function to set the position of an existing haloing
///effect, according to its handle
void set_location(int handle, int x, int y);

///function to remove the halo with the given handle
void remove(int handle);

struct remover
{
	void operator()(int handle) const { remove(handle); }
};

///functions to render and unrender haloes. Which haloes are rendered 
// is determined by invalidated_locations and the internal state in
// the control sets (in halo.cpp)
void unrender(std::set<gamemap::location> invalidated_locations);
void render();

}

#endif
