/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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
class fake_unit_ptr;

///Manages a list of fake units for the display object.
class fake_unit_manager {
public:
	///Construct a fake unit manager from a display which owns it.
	fake_unit_manager(display & disp) : fake_units_(), my_display_(disp) {}

	//Anticipate making place_temporary_unit and remove_temporary_unit private to force exception safety
	friend class fake_unit_ptr;

	//Typedef internal_ptr_type is the object held internally. It should point to a const unit, since const units are drawable.
	typedef unit const * internal_ptr_type;

	//Typedefs and iterator methods which make this object "boost_foreachable"
	typedef std::deque<internal_ptr_type>::const_iterator iterator;
	typedef std::deque<internal_ptr_type>::const_iterator const_iterator;

	iterator begin() { return fake_units_.begin(); }
	iterator end() { return fake_units_.end(); }

	const_iterator begin() const { return fake_units_.begin(); }
	const_iterator end() const { return fake_units_.end(); }
	bool empty() const { return fake_units_.empty(); }
private:
	/** Register a unit with this manager. private, should only be called by fake_unit_ptr. */
	void place_temporary_unit(internal_ptr_type);

	/** Deregister a unit from this manager. private, should only be called by fake_unit_ptr.
	 *  @return the number of temp units deleted (0 or 1, any other number indicates an error).
	 */
	int remove_temporary_unit(internal_ptr_type);

	/// collection of units destined to be drawn but not put into the unit map
	std::deque<internal_ptr_type> fake_units_;
	display & my_display_; //!< Reference to my display
};

#endif
