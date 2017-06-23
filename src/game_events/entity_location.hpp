/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Define locations as used by the game's events mechanism.
 */

#pragma once

#include "map/location.hpp"
#include "units/map.hpp"

class unit;
class vconfig;


namespace game_events
{
	struct entity_location : public map_location {
		entity_location(const map_location &loc, size_t id = 0);
		entity_location(const map_location &loc, size_t id,
		                const map_location &filter_loc);
		explicit entity_location(const unit &);
		entity_location(const unit &u, const map_location &filter_loc);

		const map_location& filter_loc() const { return filter_loc_;  }
		bool matches_unit(const unit_map::const_iterator & un_it) const;
		bool matches_unit_filter(const unit_map::const_iterator & un_it,
		                         const vconfig & filter) const;
		unit_const_ptr get_unit() const;

		static const entity_location null_entity;

	private:
		/// The underlying ID of the unit associated with this.
		/// Set to 0 if there is no associated unit.
		size_t id_;

		/// This map_location allows a unit to be filtered as if it were
		/// somewhere other than where it is. (Use the parent struct if
		/// you want to locate the unit.)
		map_location filter_loc_;
	};
}
