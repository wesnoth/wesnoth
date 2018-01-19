/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "units/ptr.hpp"

class fake_unit_manager;

/** Holds a temporary unit that can be drawn on the map without
	being placed in the unit_map.
	Temporary units can overlap units.
	They are drawn after the normal units, and so draw over them.
	Adding the same unit twice isn't allowed.
	The fake_unit privately holds a referenced counted point to its underlying unit,
	when it goes out of scope it removes the entry from the fake_units list.
	The intent is to provide exception safety when the code
	creating the temp unit is unexpectedly forced out of scope.
 */
class fake_unit_ptr {
public:
	typedef unit_ptr internal_ptr;
	typedef unit_const_ptr internal_const_ptr;

	fake_unit_ptr();
	explicit fake_unit_ptr(const internal_ptr & u); //!< Construct a fake unit pointer wrapping a normal unit pointer, marking it as a fake unit.
	fake_unit_ptr(const internal_ptr & u, fake_unit_manager * mgr); //!< Construct a fake unit pointer, and simultaenously register with a manager.
	fake_unit_ptr(const fake_unit_ptr & ptr); //!< Copy construct a fake unit pointer. Does not reallocate the underlying unit.

	void swap (fake_unit_ptr & o); //!< Pointer swap.

	fake_unit_ptr & operator=(fake_unit_ptr other); //!< Copy assignment operator using copy-and-swap idiom

	void reset(); //!< Reset the internal unit pointer, and deregister from the manager. This fake_unit_ptr is now dissassociated from the manager.
	void reset(const internal_ptr & ptr); //!< Reset the internal unit pointer and point to a new unit. The old unit is deregistered, and the new unit is registered with the old manager, if there was one.

	internal_ptr operator->() { return unit_; } //!< Dereference the internal unit pointer.
	internal_const_ptr operator->() const { return unit_; } //!< Dereference the internal unit pointer.

	internal_ptr get_unit_ptr() { return unit_; } //!< Get a copy of the internal unit pointer.
	internal_const_ptr get_unit_ptr() const { return unit_; } //!< Get a copy of the internal unit pointer.

	unit & operator*() { return *unit_; } //!< Derference the internal unit pointer.
	unit * get() { return unit_.get(); } //!< Get a raw pointer to the underlying unit.

	/// Removes @a this from the fake_units_ list if necessary.
	~fake_unit_ptr();

	/// Place @a this on @a manager's fake_units_ dequeue.
	void place_on_fake_unit_manager(fake_unit_manager * d);
	/// Removes @a this from whatever fake_units_ list it is on (if any).
	int remove_from_fake_unit_manager();

private :
	internal_ptr unit_; //!< Internal unit pointer.
	fake_unit_manager * my_manager_; //!< Raw pointer to the manager.

public:

	explicit operator bool() const
	{ return unit_.get() != nullptr; }
};
