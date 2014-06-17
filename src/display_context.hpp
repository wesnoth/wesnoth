/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *
 * This class is an abstract base class designed to simplify the use
 * of the display object.
 *
 **/

#ifndef DISPLAY_CONTEXT_HPP_INCLUDED
#define DISPLAY_CONTEXT_HPP_INCLUDED

#include<vector>

class team;
class gamemap;
class unit_map;

class unit;
struct map_location;

class display_context {
public:
	virtual const std::vector<team> & teams() const = 0;
	virtual const gamemap & map() const = 0;
	virtual const unit_map & units() const = 0;

	// Needed for reports

	const unit * get_visible_unit(const map_location &loc, const team &current_team, bool see_all = false) const;

	// From actions:: namespace

	bool unit_can_move(const unit & u) const;

	// Dtor

	virtual ~display_context() {}
};

#endif
