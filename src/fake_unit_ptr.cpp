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

#include "fake_unit_ptr.hpp"

#include "fake_unit_manager.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"

#include <boost/swap.hpp>

fake_unit_ptr::fake_unit_ptr() : unit_(), my_manager_(nullptr) {}
fake_unit_ptr::fake_unit_ptr(const internal_ptr & u) : unit_(u), my_manager_(nullptr) {}
fake_unit_ptr::fake_unit_ptr(const internal_ptr & u, fake_unit_manager * mgr) : unit_(u), my_manager_(nullptr)
{
	place_on_fake_unit_manager(mgr);
}
fake_unit_ptr::fake_unit_ptr(const fake_unit_ptr & ptr) : unit_(ptr.unit_), my_manager_(nullptr) {}

void fake_unit_ptr::swap (fake_unit_ptr & o) {
	boost::swap(unit_, o.unit_);
	std::swap(my_manager_, o.my_manager_);
}

fake_unit_ptr & fake_unit_ptr::operator=(fake_unit_ptr other) {
	swap(other);
	return *this;
}


/**
 * Assignment operator, taking a unit.
 * If already in the queue, @a this will be moved to the end of the
 * queue (drawn last).
 *
 * This function is unsuitable for derived classes and MUST be overridden.
 * Furthermore, derived classes must not explicitly call this version.
 *
 * The overriding function can be almost the same, except "new (this)" should
 * be followed by the derived class instead of "fake_unit(a)".
 */
/*fake_unit & fake_unit::operator=(const unit& a)
{
	if ( this != &a ) {
		fake_unit_manager * mgr = my_manager_;

		// Use the copy constructor to make sure we are coherent.
		// (Methodology copied from unit::operator=)
		this->~fake_unit();
		new (this) fake_unit(a);
		// Restore our old manager.
		if ( mgr != nullptr )
			place_on_fake_unit_manager(mgr);
	}
	return *this;
}*/

/**
 * Removes the unit from the fake manager, and resets the internal unit pointer.
 * After this, both pointers are null.
 */
void fake_unit_ptr::reset()
{
	remove_from_fake_unit_manager();
	unit_.reset();
}

/**
 * Resets the internal unit pointer to match the given pointer.
 * The value of my_manager_ is preserved -- the old unit is deregistered,
 * and the new unit is registered with the same manager.
 */
void fake_unit_ptr::reset(const internal_ptr & ptr)
{
	if (unit_.get() != ptr.get()) {
		fake_unit_manager * mgr = my_manager_;

		remove_from_fake_unit_manager();
		unit_ = ptr;
		if (mgr)
			place_on_fake_unit_manager(mgr);
	}
}

/**
 * Removes @a this from the fake_units_ list if necessary.
 */
fake_unit_ptr::~fake_unit_ptr()
{
	try {
	// The fake_unit class exists for this one line, which removes the
	// fake_unit from the managers's fake_units_ dequeue in the event of an
	// exception.
	if(my_manager_){remove_from_fake_unit_manager();}

	} catch (...) {}
}

/**
 * Place @a this on @a manager's fake_units_ dequeue.
 * This will be added at the end (drawn last, over all other units).
 * Duplicate additions are not allowed.
 */
void fake_unit_ptr::place_on_fake_unit_manager(fake_unit_manager * manager){
	assert(my_manager_ == nullptr); //Can only be placed on 1 fake_unit_manager
	my_manager_=manager;
	my_manager_->place_temporary_unit(unit_.get());
}

/**
 * Removes @a this from whatever fake_units_ list it is on (if any).
 * @returns the number of fake_units deleted, which should be 0 or 1
 *          (any other number indicates an error).
 */
int fake_unit_ptr::remove_from_fake_unit_manager(){
	int ret(0);
	if(my_manager_ != nullptr){
		ret = my_manager_->remove_temporary_unit(unit_.get());
		my_manager_=nullptr;
	}
	return ret;
}
