/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 * Various functions implementing vision (through fog of war and shroud).
 */

#pragma once

#include "movetype.hpp"
#include "game_events/fwd.hpp"

struct map_location;
class  team;
class  unit;

#include <cstring>
#include <map>
#include <set>
#include <vector>


namespace actions {
	class  move_unit_spectator;

/// Class that stores the part of a unit's data that is needed for fog clearing.
/// (Used by the undo stack as that cannot rely on a unit sticking around, and
/// we do not really need to copy the entire unit.)
struct clearer_info {
	size_t underlying_id;
	int sight_range;
	bool slowed;
	movetype::terrain_costs costs;

	clearer_info(const unit & viewer);
	clearer_info(const config & cfg);

	void write(config & cfg) const;
};

/// Class to encapsulate fog/shroud clearing and the resultant sighted events.
/// Note: This class uses teams as parameters (instead of sides) since a
/// function using this should first check to see if fog/shroud is in use (to
/// save processing when it is not), which implies the team is readily available.
class shroud_clearer {
public:
	shroud_clearer(const shroud_clearer&) = delete;
	shroud_clearer& operator=(const shroud_clearer&) = delete;

	shroud_clearer();
	~shroud_clearer();

	/// Function to be called if units have moved or otherwise changed.
	/// It can also be called if it is desirable to calculate the cache
	/// in advance of fog clearing.
	/// @param[in] new_team  The team whose vision will be used. If left as
	///                      nullptr, the cache will be just be cleared (to be
	///                      recalculated later as needed).
	void cache_units(const team * new_team=nullptr) { calculate_jamming(new_team); }
	// cache_units() is currently a near-synonym for calculate_jamming(). The
	// reason for the two names is so the private function says what it does,
	// while the public one says why it might be invoked.

	/// Clears shroud (and fog) around the provided location for @a view_team
	/// based on @a sight_range, @a costs, and @a slowed.
	bool clear_unit(const map_location &view_loc, team &view_team,
	                size_t viewer_id, int sight_range, bool slowed,
	                const movetype::terrain_costs & costs,
	                const map_location & real_loc,
					const std::set<map_location>* known_units = nullptr,
	                size_t * enemy_count = nullptr, size_t * friend_count = nullptr,
	                move_unit_spectator * spectator = nullptr, bool instant = true);
	/// Clears shroud (and fog) around the provided location for @a view_team
	/// as if @a viewer was standing there.
	bool clear_unit(const map_location &view_loc,
	                const unit &viewer, team &view_team,
	                const std::set<map_location>* known_units = nullptr,
	                size_t * enemy_count = nullptr, size_t * friend_count = nullptr,
	                move_unit_spectator * spectator = nullptr, bool instant = true);
	/// Clears shroud (and fog) around the provided location for @a view_team
	/// as if @a viewer was standing there. Setting @a instant to false
	/// allows some drawing delays that are used to make movement look better.
	bool clear_unit(const map_location &view_loc, const unit &viewer,
	                team &view_team, bool instant)
	{ return clear_unit(view_loc, viewer, view_team, nullptr, nullptr, nullptr, nullptr, instant); }
	/// Clears shroud (and fog) around the provided location for @a view_team
	/// as if @a viewer was standing there.
	bool clear_unit(const map_location &view_loc, team &view_team,
	                const clearer_info &viewer, bool instant);
	/// Clears shroud (and fog) around the provided location as if @a viewer
	/// was standing there.
	bool clear_unit(const map_location &view_loc, const unit &viewer,
	                bool can_delay = false, bool invalidate = true,
	                bool instant = true);

	/// Clears shroud (and fog) at the provided location and its immediate neighbors.
	bool clear_dest(const map_location &dest, const unit &viewer);

	/// Erases the record of sighted events from earlier fog/shroud clearing.
	void drop_events();

	/// Fires the sighted events that were earlier recorded by fog/shroud clearing.
	game_events::pump_result_t fire_events();

	/// The invalidations that should occur after invoking clear_unit().
	void invalidate_after_clear();

private:
	/// A record of a sighting event.
	struct sight_data;

	/// Causes this object's "jamming" map to be recalculated.
	void calculate_jamming(const team * new_team);

	/// Clears shroud from a single location.
	bool clear_loc(team &tm, const map_location &loc, const map_location &view_loc,
	               const map_location &event_non_loc, size_t viewer_id,
	               bool check_units, size_t &enemy_count, size_t &friend_count,
	               move_unit_spectator * spectator = nullptr);

	/// Convenience wrapper for adding sighting data to the sightings_ vector.
	inline void record_sighting(const unit & seen, const map_location & seen_loc,
	                            size_t sighter_id, const map_location & sighter_loc);

private: // data
	std::map<map_location, int> jamming_;
	std::vector<sight_data> sightings_;
	/// Keeps track of the team associated with jamming_.
	const team * view_team_;
};


/// Returns the sides that cannot currently see @a target.
std::vector<int> get_sides_not_seeing(const unit & target);
/// Fires sighted events for the sides that can see @a target.
game_events::pump_result_t actor_sighted(const unit & target, const std::vector<int> * cache =  nullptr);


/// Function that recalculates the fog of war.
void recalculate_fog(int side);

/// Function that will clear shroud (and fog) based on current unit positions.
bool clear_shroud(int side, bool reset_fog = false, bool fire_events = true);


}//namespace actions
