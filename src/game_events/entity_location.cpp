/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * @file
 * The structure that tracks WML event locations.
 */

#include "global.hpp"
#include "entity_location.hpp"

#include "../unit.hpp"
#include "../variable.hpp"


// This file is in the game_events namespace.
namespace game_events {

const entity_location entity_location::null_entity(map_location::null_location());

/**
 * Constructor for when an event has a location but not necessarily a unit.
 * Can also be used if the event has a unit and the caller already has the
 * unit's location and underlying ID.
 */
entity_location::entity_location(const map_location &loc, size_t id)
	: map_location(loc), id_(id), filter_loc_(loc)
{}

/**
 * Constructor for when an event has a unit that needs to be filtered as if
 * it was in a different location.
 */
entity_location::entity_location(const map_location &loc, size_t id,
                                 const map_location & filter_loc)
	: map_location(loc), id_(id), filter_loc_(filter_loc)
{}

/**
 * Convenience constructor for when an event has a unit, saving the caller
 * the need to explicitly get the location and underlying ID.
 */
entity_location::entity_location(const unit &u)
	: map_location(u.get_location())
	, id_(u.underlying_id())
	, filter_loc_(*this)
{}

/**
 * Convenience constructor for when an event has a unit that needs to be
 * filtered as if it was in a different location, and the caller does not
 * want to explicitly get the unit's location and underlying ID.
 */
entity_location::entity_location(const unit &u, const map_location & filter_loc)
	: map_location(u.get_location())
	, id_(u.underlying_id())
	, filter_loc_(filter_loc)
{}


/**
 * Determines if @a un_it matches (using underlying ID) the unit that was
 * supplied when this was constructed.
 * If no unit was supplied, then all units (including non-existent units)
 * match.
 */
bool entity_location::matches_unit(const unit_map::const_iterator & un_it) const
{
	return id_ == 0  ||  ( un_it.valid() && id_ == un_it->underlying_id() );
}


/**
 * Determines if @a un_it matches @a filter. If the filter is not empty,
 * the unit is required to additionally match the unit that was supplied
 * when this was constructed.
 */
bool entity_location::matches_unit_filter(const unit_map::const_iterator & un_it,
                                          const vconfig & filter) const
{
	if ( !un_it.valid() )
		return false;

	if ( filter.empty() )
		// Skip the check for un_it matching *this.
		return true;

	// Filter the unit at the filter location (should be the unit's
	// location if no special filter location was specified).
	return un_it->matches_filter(filter, filter_loc_)  &&
	       matches_unit(un_it);
}

} // end namespace game_events

