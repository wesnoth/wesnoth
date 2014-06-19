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

#include <boost/range/iterator_range.hpp>

class display;
class unit;
class fake_unit_ptr;

class fake_unit_manager {
public:
	fake_unit_manager(display & disp) : fake_units_(), my_display_(disp) {}

	//Anticipate making place_temporary_unit and remove_temporary_unit private to force exception safety
	friend class fake_unit_ptr;

	typedef unit const * internal_ptr_type;
	typedef boost::iterator_range<std::deque<internal_ptr_type>::const_iterator> range;

	range get_unit_range() const { return boost::make_iterator_range(fake_units_.begin(), fake_units_.end()); }

private:
	/** Temporarily place a unit on map (moving: can overlap others).
	 *  The temp unit is added at the end of the temporary unit dequeue,
	 *  and therefore gets drawn last, over other units and temp units.
	 *  Adding the same unit twice isn't allowed.
	 */
	void place_temporary_unit(internal_ptr_type);

	/** Removes any instances of this temporary unit from the temporary unit vector.
	 *  Returns the number of temp units deleted (0 or 1, any other number indicates an error).
	 */
	int remove_temporary_unit(internal_ptr_type);

	/// collection of units destined to be drawn but not put into the unit map
	std::deque<internal_ptr_type> fake_units_;
	display & my_display_;
};

#endif
