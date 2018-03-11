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

#include "fake_unit_manager.hpp"

#include "display.hpp"
#include "fake_unit_ptr.hpp"
#include "log.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

/** Temporarily register a unit to be drawn on the map (moving: can overlap others).
 *  The temp unit is added at the end of the temporary unit dequeue,
 *  and therefore gets drawn last, over other units and temp units.
 *  Adding the same unit twice isn't allowed.
 */
void fake_unit_manager::place_temporary_unit(internal_ptr_type u)
{
	if(std::find(fake_units_.begin(),fake_units_.end(), u) != fake_units_.end()) {
		ERR_NG << "In fake_unit_manager::place_temporary_unit: attempt to add duplicate fake unit." << std::endl;
	} else {
		fake_units_.push_back(u);
		my_display_.invalidate(u->get_location());
	}
}

/** Removes any instances of this unit from the temporary unit database. */
int fake_unit_manager::remove_temporary_unit(internal_ptr_type u)
{
	int removed = 0;
	if (fake_units_.empty())
		return removed;
	std::deque<internal_ptr_type>::iterator it =
			std::remove(fake_units_.begin(), fake_units_.end(), u);
	if (it != fake_units_.end()) {
		removed = std::distance(it, fake_units_.end());
		//std::remove doesn't remove anything without using erase afterwards.
		fake_units_.erase(it, fake_units_.end());
		my_display_.invalidate(u->get_location());
		// Redraw with no location to get rid of haloes
		u->anim_comp().clear_haloes();
	}
	if (removed > 1) {
		ERR_NG << "Error: duplicate temp unit found in fake_unit_manager::remove_temporary_unit" << std::endl;
	}
	return removed;
}
