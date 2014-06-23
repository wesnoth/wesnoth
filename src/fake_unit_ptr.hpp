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

#include "unit_ptr.hpp"
#include "unit_types.hpp"

class fake_unit_manager;

/** A temporary unit that can be placed on the map.
	Temporary units can overlap units.
	Adding the same unit twice isn't allowed.
	The fake_unit owns its underlying unit and when
	it goes out of scope it removes itself from the fake_units list.
	The intent is to provide exception safety when the code
	creating the temp unit is unexpectedly forced out of scope.
 */
class fake_unit_ptr {
public:
	typedef UnitPtr internal_ptr;
	typedef UnitConstPtr internal_const_ptr;

	fake_unit_ptr();
	explicit fake_unit_ptr(const internal_ptr & u);
	fake_unit_ptr(const internal_ptr & u, fake_unit_manager * mgr);
	fake_unit_ptr(const fake_unit_ptr & ptr);

	void swap (fake_unit_ptr & o);

	fake_unit_ptr & operator=(fake_unit_ptr other);

	void reset();
	void reset(const internal_ptr & ptr);

	internal_ptr operator->() { return unit_; }
	internal_const_ptr operator->() const { return unit_; }

	internal_ptr get_unit_ptr() { return unit_; }
	internal_const_ptr get_unit_ptr() const { return unit_; }

	unit & operator*() { return *unit_; }
	unit * get() { return unit_.get(); }

	/// Removes @a this from the fake_units_ list if necessary.
	~fake_unit_ptr();

	/// Place @a this on @a manager's fake_units_ dequeue.
	void place_on_fake_unit_manager(fake_unit_manager * d);
	/// Removes @a this from whatever fake_units_ list it is on (if any).
	int remove_from_fake_unit_manager();

private :
	internal_ptr unit_;
	fake_unit_manager * my_manager_;

#ifndef HAVE_CXX11
	struct safe_bool_impl { void nonnull() {} };
	/**
	 * Used as t he return type of the conversion operator for boolean contexts.
	 * Needed, since the compiler would otherwise consider the following
	 * conversion (C legacy): cfg["abc"] -> "abc"[bool(cfg)] -> 'b'
	 */
	typedef void (safe_bool_impl::*safe_bool)();
#endif

public:
#ifdef HAVE_CXX11
	explicit operator bool() const
	{ return unit_; }
#else
	operator safe_bool() const
	{ return unit_ ? &safe_bool_impl::nonnull : NULL; }
#endif
};

#endif
