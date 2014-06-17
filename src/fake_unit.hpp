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

#ifndef INCL_FAKE_UNIT_HPP_
#define INCL_FAKE_UNIT_HPP_

#include "unit.hpp"

class fake_unit_manager;

/** A temporary unit that can be placed on the map.
	Temporary units can overlap units.
	Adding the same unit twice isn't allowed.
	The fake_unit owns its underlying unit and when
	it goes out of scope it removes itself from the fake_units list.
	The intent is to provide exception safety when the code
	creating the temp unit is unexpectedly forced out of scope.
 */
class fake_unit : public unit {
public:
	explicit fake_unit(unit const & u) : unit(u), my_manager_(NULL) {ref_count_ = ref_count_ + 1; } //prevent UnitPtr from deleting this
	fake_unit(fake_unit const & u) : unit(u), my_manager_(NULL) {ref_count_ = ref_count_ + 1; }
	fake_unit(const unit_type& t, int side, unit_race::GENDER gender = unit_race::NUM_GENDERS)
		: unit(t, side, false, gender)
		, my_manager_(NULL)
	{ref_count_ = ref_count_ + 1; }
	/// Assignment operator, taking a fake_unit.
	/// If already in the queue, @a this will be moved to the end of the
	/// queue (drawn last). The queue (if any) of the parameter is ignored.
	fake_unit & operator=(fake_unit const & u)
	{ return operator=(static_cast<unit const &>(u)); }
	/// Assignment operator, taking a unit.
	virtual fake_unit & operator=(unit const & u);
	/// Removes @a this from the fake_units_ list if necessary.
	~fake_unit();

	/// Place @a this on @a manager's fake_units_ dequeue.
	void place_on_fake_unit_manager(fake_unit_manager * d);
	/// Removes @a this from whatever fake_units_ list it is on (if any).
	int remove_from_fake_unit_manager();

	private :
	fake_unit_manager * my_manager_;
};

#endif
