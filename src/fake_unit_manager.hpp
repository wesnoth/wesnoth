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

#ifndef INCL_FAKE_UNIT_MGR_HPP_
#define INCL_FAKE_UNIT_MGR_HPP_

#include <deque>

class display;
class unit;
class fake_unit;

class fake_unit_manager {
public:
	fake_unit_manager(display & disp) : fake_units_(), my_display_(disp) {}

	//Anticipate making place_temporary_unit and remove_temporary_unit private to force exception safety
	friend class fake_unit;

	const std::deque<unit*> & get_fake_unit_list_for_invalidation() {return fake_units_; }

private:
	/** Temporarily place a unit on map (moving: can overlap others).
	 *  The temp unit is added at the end of the temporary unit dequeue,
	 *  and therefore gets drawn last, over other units and temp units.
	 *  Adding the same unit twice isn't allowed.
	 */
	void place_temporary_unit(unit *u);

	/** Removes any instances of this temporary unit from the temporary unit vector.
	 *  Returns the number of temp units deleted (0 or 1, any other number indicates an error).
	 */
	int remove_temporary_unit(unit *u);

	/// collection of units destined to be drawn but not put into the unit map
	std::deque<unit*> fake_units_;
	display & my_display_;
};

#endif
