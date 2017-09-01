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
 * Recruiting, recalling.
 */

#include "actions/create.hpp"

#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "actions/vision.hpp"

#include "config.hpp"
#include "filter_context.hpp"
#include "game_display.hpp"
#include "game_events/pump.hpp"
#include "game_state.hpp"
#include "preferences/game.hpp"
#include "game_data.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "pathfind/pathfind.hpp"
#include "recall_list_manager.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "synced_checkup.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/udisplay.hpp"
#include "units/filter.hpp"
#include "variable.hpp"
#include "whiteboard/manager.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace actions {

const std::set<std::string> get_recruits(int side, const map_location &recruit_loc)
{
	const team & current_team = resources::gameboard->get_team(side);

	LOG_NG << "getting recruit list for side " << side << " at location " << recruit_loc << "\n";

	std::set<std::string> local_result;
	std::set<std::string> global_result;
	unit_map::const_iterator u = resources::gameboard->units().begin(),
			u_end = resources::gameboard->units().end();

	bool leader_in_place = false;
	bool allow_local = resources::gameboard->map().is_castle(recruit_loc);


	// Check for a leader at recruit_loc (means we are recruiting from there,
	// rather than to there).
	unit_map::const_iterator find_it = resources::gameboard->units().find(recruit_loc);
	if ( find_it != u_end ) {
		if ( find_it->can_recruit()  &&  find_it->side() == side  &&
		     resources::gameboard->map().is_keep(recruit_loc) )
		{
			// We have been requested to get the recruit list for this
			// particular leader.
			leader_in_place = true;
			local_result.insert(find_it->recruits().begin(),
			                    find_it->recruits().end());
		}
		else if ( find_it->is_visible_to_team(current_team, *resources::gameboard, false) )
		{
			// This hex is visibly occupied, so we cannot recruit here.
			allow_local = false;
		}
	}

	if ( !leader_in_place ) {
		// Check all leaders for their ability to recruit here.
		for( ; u != u_end; ++u ) {
			// Only consider leaders on this side.
			if ( !(u->can_recruit() && u->side() == side) )
				continue;

			// Check if the leader is on a connected keep.
			if (allow_local && dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*u, recruit_loc)) {
				leader_in_place= true;
				local_result.insert(u->recruits().begin(), u->recruits().end());
			}
			else if ( !leader_in_place )
				global_result.insert(u->recruits().begin(), u->recruits().end());
		}
	}

	// Determine which result set to use.
	std::set<std::string> & result = leader_in_place ? local_result : global_result;

	// Add the team-wide recruit list.
	const std::set<std::string>& recruit_list = current_team.recruits();
	result.insert(recruit_list.begin(), recruit_list.end());

	return result;
}


namespace { // Helpers for get_recalls()
	/**
	 * Adds to @a result those units that @a leader (assumed a leader) can recall.
	 * If @a already_added is supplied, it contains the underlying IDs of units
	 * that can be skipped (because they are already in @a result), and the
	 * underlying ID of units added to @a result will be added to @a already_added.
	 */
	void add_leader_filtered_recalls(const unit_const_ptr leader,
	                                 std::vector< unit_const_ptr > & result,
	                                 std::set<size_t> * already_added = nullptr)
	{
		const team& leader_team = resources::gameboard->get_team(leader->side());
		const std::string& save_id = leader_team.save_id();

		const unit_filter ufilt(vconfig(leader->recall_filter()), resources::filter_con);

		for (const unit_const_ptr & recall_unit_ptr : leader_team.recall_list())
		{
			const unit & recall_unit = *recall_unit_ptr;
			// Do not add a unit twice.
			size_t underlying_id = recall_unit.underlying_id();
			if ( !already_added  ||  already_added->count(underlying_id) == 0 )
			{
				// Only units that match the leader's recall filter are valid.
				scoped_recall_unit this_unit("this_unit", save_id, leader_team.recall_list().find_index(recall_unit.id()));

				if ( ufilt(recall_unit, map_location::null_location()) )
				{
					result.push_back(recall_unit_ptr);
					if ( already_added != nullptr )
						already_added->insert(underlying_id);
				}
			}
		}
	}
}// anonymous namespace

std::vector<unit_const_ptr > get_recalls(int side, const map_location &recall_loc)
{
	LOG_NG << "getting recall list for side " << side << " at location " << recall_loc << "\n";

	std::vector<unit_const_ptr > result;

	/*
	 * We have three use cases:
	 * 1. An empty castle tile is highlighted; we return only the units recallable there.
	 * 2. A leader on a keep is highlighted; we return only the units recallable by that leader.
	 * 3. Otherwise, we return all units in the recall list that can be recalled by any leader on the map.
	 */

	bool leader_in_place = false;
	bool allow_local = resources::gameboard->map().is_castle(recall_loc);


	// Check for a leader at recall_loc (means we are recalling from there,
	// rather than to there).
	const unit_map::const_iterator find_it = resources::gameboard->units().find(recall_loc);
	if ( find_it != resources::gameboard->units().end() ) {
		if ( find_it->can_recruit()  &&  find_it->side() == side  &&
		     resources::gameboard->map().is_keep(recall_loc) )
		{
			// We have been requested to get the recalls for this
			// particular leader.
			add_leader_filtered_recalls(find_it.get_shared_ptr(), result);
			return result;
		}
		else if ( find_it->is_visible_to_team(resources::gameboard->get_team(side), *resources::gameboard, false) )
		{
			// This hex is visibly occupied, so we cannot recall here.
			allow_local = false;
		}
	}

	if ( allow_local )
	{
		unit_map::const_iterator u = resources::gameboard->units().begin(),
				u_end = resources::gameboard->units().end();
		std::set<size_t> valid_local_recalls;

		for(; u != u_end; ++u) {
			//We only consider leaders on our side.
			if (!(u->can_recruit() && u->side() == side))
				continue;

			// Check if the leader is on a connected keep.
			if (!dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*u, recall_loc))
				continue;
			leader_in_place= true;

			add_leader_filtered_recalls(u.get_shared_ptr(), result, &valid_local_recalls);
		}
	}

	if ( !leader_in_place )
	{
		std::set<size_t> valid_local_recalls;

		for(auto u = resources::gameboard->units().begin(); u != resources::gameboard->units().end(); ++u) {
			//We only consider leaders on our side.
			if(!u->can_recruit() || u->side() != side) {
				continue;
			}

			add_leader_filtered_recalls(u.get_shared_ptr(), result, &valid_local_recalls);
		}
	}

	return result;
}

namespace { // Helpers for check_recall_location()
	/**
	 * Checks if @a recaller can recall @a recall_unit at @a preferred.
	 * If recalling can occur but not at the preferred location, then a
	 * permissible location is stored in @a alternative.
	 * @returns the reason why recalling is not allowed (or RECRUIT_OK).
	 */
	RECRUIT_CHECK check_unit_recall_location(
		const unit & recaller, const unit & recall_unit,
		const map_location & preferred, map_location & alternative)
	{
		// Make sure the unit can actually recall.
		if ( !recaller.can_recruit() )
			return RECRUIT_NO_LEADER;

		// Make sure the recalling unit can recall this specific unit.
		team& recall_team = (*resources::gameboard).get_team(recaller.side());
		scoped_recall_unit this_unit("this_unit", recall_team.save_id(),
						recall_team.recall_list().find_index(recall_unit.id()));

		const unit_filter ufilt(vconfig(recaller.recall_filter()), resources::filter_con);
		if ( !ufilt(recall_unit, map_location::null_location()) )
			return RECRUIT_NO_ABLE_LEADER;

		// Make sure the unit is on a keep.
		if ( !resources::gameboard->map().is_keep(recaller.get_location()) )
			return RECRUIT_NO_KEEP_LEADER;

		// Make sure there is a permissible location to which to recruit.
		map_location permissible = pathfind::find_vacant_castle(recaller);
		if ( !permissible.valid() )
			return RECRUIT_NO_VACANCY;

		// See if the preferred location cannot be used.
		if (!dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(recaller, preferred)) {
			alternative = permissible;
			return RECRUIT_ALTERNATE_LOCATION;
		}

		// All tests passed.
		return RECRUIT_OK;
	}
}//anonymous namespace

/// Checks if there is a location on which to recall @a unit_recall.
RECRUIT_CHECK check_recall_location(const int side, map_location& recall_location,
                                    map_location& recall_from,
                                    const unit &unit_recall)
{
	const unit_map & units = resources::gameboard->units();
	const unit_map::const_iterator u_end = units.end();

	map_location check_location = recall_location;
	map_location alternative;	// Set by check_unit_recall_location().

	// If the specified location is occupied, proceed as if no location was specified.
	if ( resources::gameboard->units().count(recall_location) != 0 )
		check_location = map_location::null_location();

	// If the check location is not valid, we will never get an "OK" result.
	RECRUIT_CHECK const goal_result = check_location.valid() ? RECRUIT_OK :
	                                                           RECRUIT_ALTERNATE_LOCATION;
	RECRUIT_CHECK best_result = RECRUIT_NO_LEADER;

	// Test the specified recaller (if there is one).
	unit_map::const_iterator u = units.find(recall_from);
	if ( u != u_end  &&  u->side() == side ) {
		best_result =
			check_unit_recall_location(*u, unit_recall, check_location, alternative);
	}

	// Loop through all units on the specified side.
	for ( u = units.begin(); best_result < goal_result  &&  u != u_end; ++u ) {
		if ( u->side() != side )
			continue;

		// Check this unit's viability as a recaller.
		RECRUIT_CHECK current_result =
			check_unit_recall_location(*u, unit_recall, check_location, alternative);

		// If this is not an improvement, proceed to the next unit.
		if ( current_result <= best_result )
			continue;
		best_result = current_result;

		// If we have a viable recaller, record its location.
		if ( current_result >= RECRUIT_ALTERNATE_LOCATION )
			recall_from = u->get_location();
	}

	if ( best_result == RECRUIT_ALTERNATE_LOCATION )
		// Report the alternate location to the caller.
		recall_location = alternative;

	return best_result;
}

std::string find_recall_location(const int side, map_location& recall_location, map_location& recall_from, const unit &unit_recall)
{
	LOG_NG << "finding recall location for side " << side << " and unit " << unit_recall.id() << "\n";

	// This function basically translates check_recall_location() to a
	// human-readable string.
	switch ( check_recall_location(side, recall_location, recall_from, unit_recall) )
	{
	case RECRUIT_NO_LEADER:
		LOG_NG << "No leaders on side " << side << " when recalling " << unit_recall.id() << ".\n";
		return _("You do not have a leader to recall with.");

	case RECRUIT_NO_ABLE_LEADER:
		LOG_NG << "No leader is able to recall " << unit_recall.id() << " on side " << side << ".\n";
		return _("None of your leaders are able to recall that unit.");

	case RECRUIT_NO_KEEP_LEADER:
		LOG_NG << "No leader able to recall " << unit_recall.id() << " is on a keep.\n";
		return _("You must have a leader on a keep who is able to recall that unit.");

	case RECRUIT_NO_VACANCY:
		LOG_NG << "No vacant castle tiles around a keep are available for recalling " << unit_recall.id() << "; requested location is " << recall_location << ".\n";
		return _("There are no vacant castle tiles in which to recall the unit.");

	case RECRUIT_ALTERNATE_LOCATION:
	case RECRUIT_OK:
		return std::string();
	}

	// We should never get down to here. But just in case someone decides to
	// mess with the enum without updating this function:
	ERR_NG << "Unrecognized enum in find_recall_location()" << std::endl;
	return _("An unrecognized error has occurred.");
}

namespace { // Helpers for check_recruit_location()
	/**
	 * Checks if @a recruiter can recruit at @a preferred.
	 * If @a unit_type is not empty, it must be in the unit-specific recruit list.
	 * If recruitment can occur but not at the preferred location, then a
	 * permissible location is stored in @a alternative.
	 * @returns the reason why recruitment is not allowed (or RECRUIT_OK).
	 */
	RECRUIT_CHECK check_unit_recruit_location(
		const unit & recruiter, const std::string & unit_type,
		const map_location & preferred, map_location & alternative)
	{
		// Make sure the unit can actually recruit.
		if ( !recruiter.can_recruit() )
			return RECRUIT_NO_LEADER;

		if ( !unit_type.empty() ) {
			// Make sure the specified type is in the unit's recruit list.
			if ( !utils::contains(recruiter.recruits(), unit_type) )
				return RECRUIT_NO_ABLE_LEADER;
		}

		// Make sure the unit is on a keep.
		if ( !resources::gameboard->map().is_keep(recruiter.get_location()) )
			return RECRUIT_NO_KEEP_LEADER;

		// Make sure there is a permissible location to which to recruit.
		map_location permissible = pathfind::find_vacant_castle(recruiter);
		if ( !permissible.valid() )
			return RECRUIT_NO_VACANCY;

		// See if the preferred location cannot be used.
		if (!dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(recruiter, preferred)) {
			alternative = permissible;
			return RECRUIT_ALTERNATE_LOCATION;
		}

		// All tests passed.
		return RECRUIT_OK;
	}
}//anonymous namespace

/// Checks if there is a location on which to place a recruited unit.
RECRUIT_CHECK check_recruit_location(const int side, map_location &recruit_location,
                                     map_location& recruited_from,
                                     const std::string& unit_type)
{
	const unit_map & units = resources::gameboard->units();
	const unit_map::const_iterator u_end = units.end();

	map_location check_location = recruit_location;
	std::string check_type = unit_type;
	map_location alternative;	// Set by check_unit_recruit_location().

	// If the specified location is occupied, proceed as if no location was specified.
	if ( resources::gameboard->units().count(recruit_location) != 0 )
		check_location = map_location::null_location();

	// If the specified unit type is in the team's recruit list, there is no
	// need to check each leader's list.
	if ( utils::contains(resources::gameboard->get_team(side).recruits(), unit_type) )
		check_type.clear();

	// If the check location is not valid, we will never get an "OK" result.
	RECRUIT_CHECK const goal_result = check_location.valid() ? RECRUIT_OK :
	                                                           RECRUIT_ALTERNATE_LOCATION;
	RECRUIT_CHECK best_result = RECRUIT_NO_LEADER;

	// Test the specified recruiter (if there is one).
	unit_map::const_iterator u = units.find(recruited_from);
	if ( u != u_end  &&  u->side() == side ) {
		best_result =
			check_unit_recruit_location(*u, check_type, check_location, alternative);
	}

	// Loop through all units on the specified side.
	for ( u = units.begin(); best_result < goal_result  &&  u != u_end; ++u ) {
		if ( u->side() != side )
			continue;

		// Check this unit's viability as a recruiter.
		RECRUIT_CHECK current_result =
			check_unit_recruit_location(*u, check_type, check_location, alternative);

		// If this is not an improvement, proceed to the next unit.
		if ( current_result <= best_result )
			continue;
		best_result = current_result;

		// If we have a viable recruiter, record its location.
		if ( current_result >= RECRUIT_ALTERNATE_LOCATION )
			recruited_from = u->get_location();
	}

	if ( best_result == RECRUIT_ALTERNATE_LOCATION )
		// Report the alternate location to the caller.
		recruit_location = alternative;

	return best_result;
}

std::string find_recruit_location(const int side, map_location& recruit_location, map_location& recruited_from, const std::string& unit_type)
{
	LOG_NG << "finding recruit location for side " << side << "\n";

	// This function basically translates check_recruit_location() to a
	// human-readable string.
	switch ( check_recruit_location(side, recruit_location, recruited_from, unit_type) )
	{
	case RECRUIT_NO_LEADER:
		LOG_NG << "No leaders on side " << side << " when recruiting '" << unit_type << "'.\n";
		return _("You do not have a leader to recruit with.");

	case RECRUIT_NO_ABLE_LEADER:
		LOG_NG << "No leader is able to recruit '" << unit_type << "' on side " << side << ".\n";
		return _("None of your leaders are able to recruit that unit.");

	case RECRUIT_NO_KEEP_LEADER:
		LOG_NG << "No leader able to recruit '" << unit_type << "' is on a keep.\n";
		return _("You must have a leader on a keep who is able to recruit the unit.");

	case RECRUIT_NO_VACANCY:
		LOG_NG << "No vacant castle tiles around a keep are available for recruiting '" << unit_type << "'; requested location is " << recruit_location  << ".\n";
		return _("There are no vacant castle tiles in which to recruit the unit.");

	case RECRUIT_ALTERNATE_LOCATION:
	case RECRUIT_OK:
		return std::string();
	}

	// We should never get down to here. But just in case someone decides to
	// mess with the enum without updating this function:
	ERR_NG << "Unrecognized enum in find_recruit_location()" << std::endl;
	return _("An unrecognized error has occurred.");
}


namespace { // Helpers for place_recruit()
	/**
	 * Performs a checksum check on a newly recruited/recalled unit.
	 */
	void recruit_checksums(const unit &new_unit, bool wml_triggered)
	{
		if(wml_triggered)
		{
			return;
		}
		const std::string checksum = get_checksum(new_unit);
		config original_checksum_config;

		bool checksum_equals = checkup_instance->local_checkup(config {"checksum", checksum},original_checksum_config);
		if(!checksum_equals)
		{
			const std::string old_checksum = original_checksum_config["checksum"];
			std::stringstream error_msg;
			error_msg << "SYNC: In recruit " << new_unit.type_id() <<
				": has checksum " << checksum <<
				" while datasource has checksum " << old_checksum << "\n";
			if(old_checksum.empty())
			{
				error_msg << "Original result is \n" << original_checksum_config << "\n";
			}
			config cfg_unit1;
			new_unit.write(cfg_unit1);
			DBG_NG << cfg_unit1;
			replay::process_error(error_msg.str());
		}
	}

	/**
	 * Locates a leader on side @a side who can recruit at @a recruit_location.
	 * A leader at @a recruited_from is chosen in preference to others.
	 */
	const map_location & find_recruit_leader(int side,
		const map_location &recruit_location, const map_location &recruited_from)
	{
		const unit_map & units = resources::gameboard->units();

		// See if the preferred location is an option.
		unit_map::const_iterator leader = units.find(recruited_from);
		if (leader != units.end()  &&  leader->can_recruit()  &&
			leader->side() == side && dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*leader, recruit_location))
			return leader->get_location();

		// Check all units.
		for (leader = units.begin(); leader != units.end(); ++leader)
			if (leader->can_recruit() && leader->side() == side &&
				dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*leader, recruit_location))
				return leader->get_location();

		// No usable leader found.
		return map_location::null_location();
	}

	/**
	 * Tries to make @a un_it valid, and updates @a current_loc.
	 * Used by place_recruit() after WML might have changed something.
	 * @returns true if the iterator was made valid.
	 */
	bool validate_recruit_iterator(unit_map::iterator & un_it,
		                           map_location & current_loc)
	{
		if ( !un_it.valid() ) {
			// Maybe WML provided a replacement?
			un_it = resources::gameboard->units().find(current_loc);
			if ( un_it == resources::gameboard->units().end() )
				// The unit is gone.
				return false;
		}
		current_loc = un_it->get_location();
		return true;
	}

	void set_recruit_facing(unit_map::iterator &new_unit_itor, const unit &new_unit,
		const map_location &recruit_loc, const map_location &leader_loc)
	{
		// Find closest enemy and turn towards it (level 2s count more than level 1s, etc.)
		const gamemap *map = & resources::gameboard->map();
		const unit_map & units = resources::gameboard->units();
		unit_map::const_iterator unit_itor;
		map_location min_loc;
		int min_dist = INT_MAX;

		for ( unit_itor = units.begin(); unit_itor != units.end(); ++unit_itor ) {
			if (resources::gameboard->get_team(unit_itor->side()).is_enemy(new_unit.side()) &&
				unit_itor->is_visible_to_team(resources::gameboard->get_team(new_unit.side()), *resources::gameboard, false)) {
				int dist = distance_between(unit_itor->get_location(),recruit_loc) - unit_itor->level();
				if (dist < min_dist) {
					min_dist = dist;
					min_loc = unit_itor->get_location();
				}
			}
		}
		if (min_dist < INT_MAX) {
			// Face towards closest enemy
			new_unit_itor->set_facing(recruit_loc.get_relative_dir(min_loc));
		} else if (leader_loc != map_location::null_location()) {
			// Face away from leader
			new_unit_itor->set_facing(map_location::get_opposite_dir(recruit_loc.get_relative_dir(leader_loc)));
		} else {
			// Face towards center of map
			const map_location center(map->w()/2, map->h()/2);
			new_unit_itor->set_facing(recruit_loc.get_relative_dir(center));
		}
	}
}// anonymous namespace
//Used by recalls and recruits
place_recruit_result place_recruit(unit_ptr u, const map_location &recruit_location, const map_location& recruited_from,
    int cost, bool is_recall, map_location::DIRECTION facing, bool show, bool fire_event, bool full_movement,
    bool wml_triggered)
{
	place_recruit_result res(false, 0, false);
	LOG_NG << "placing new unit on location " << recruit_location << "\n";
	if (full_movement) {
		u->set_movement(u->total_movement(), true);
	} else {
		u->set_movement(0, true);
		u->set_attacks(0);
	}
	u->heal_fully();
	u->set_hidden(true);

	// Get the leader location before adding the unit to the board.
	const map_location leader_loc = !show ? map_location::null_location() :
			find_recruit_leader(u->side(), recruit_location, recruited_from);
	u->set_location(recruit_location);
	// Add the unit to the board.
	std::pair<unit_map::iterator, bool> add_result = resources::gameboard->units().insert(u);
	assert(add_result.second);
	unit_map::iterator & new_unit_itor = add_result.first;
	map_location current_loc = recruit_location;

	if (facing == map_location::NDIRECTIONS) {
		set_recruit_facing(new_unit_itor, *u, recruit_location, leader_loc);
	} else {
		new_unit_itor->set_facing(facing);
	}

	// Do some bookkeeping.
	recruit_checksums(*u, wml_triggered);
	resources::whiteboard->on_gamestate_change();

	resources::game_events->pump().fire("unit_placed", current_loc);

	if ( fire_event ) {
		const std::string event_name = is_recall ? "prerecall" : "prerecruit";
		LOG_NG << "firing " << event_name << " event\n";
		{
			std::get<0>(res) |= std::get<0>(resources::game_events->pump().fire(event_name, current_loc, recruited_from));
		}
		if ( !validate_recruit_iterator(new_unit_itor, current_loc) )
			return std::make_tuple(true, 0, false);
		new_unit_itor->set_hidden(true);
	}
	preferences::encountered_units().insert(new_unit_itor->type_id());
	resources::gameboard->get_team(u->side()).spend_gold(cost);

	if ( show ) {
		unit_display::unit_recruited(current_loc, leader_loc);
	}
	// Make sure the unit appears (if either !show or the animation is suppressed).
	new_unit_itor->set_hidden(false);
	if ( resources::screen != nullptr ) {
		resources::screen->invalidate(current_loc);
		resources::screen->redraw_minimap();
	}

	// Village capturing.
	if ( resources::gameboard->map().is_village(current_loc) ) {
		std::get<1>(res) = resources::gameboard->village_owner(current_loc) + 1;
		std::get<0>(res) |= std::get<0>(actions::get_village(current_loc, new_unit_itor->side(), &std::get<2>(res)));
		if ( !validate_recruit_iterator(new_unit_itor, current_loc) )
			return std::make_tuple(true, 0, false);
	}

	// Fog clearing.
	actions::shroud_clearer clearer;
	if ( !wml_triggered ) // To preserve current WML behavior.
		std::get<0>(res) |= clearer.clear_unit(current_loc, *new_unit_itor);

	if ( fire_event ) {
		const std::string event_name = is_recall ? "recall" : "recruit";
		LOG_NG << "firing " << event_name << " event\n";
		{
			std::get<0>(res) |= std::get<0>(resources::game_events->pump().fire(event_name, current_loc, recruited_from));
		}
	}

	// "sighted" event(s).
	std::get<0>(res) |= std::get<0>(clearer.fire_events());
	if ( new_unit_itor.valid() )
		std::get<0>(res) |= std::get<0>(actions::actor_sighted(*new_unit_itor));

	return res;
}


/**
 * Recruits a unit of the given type for the given side.
 */
void recruit_unit(const unit_type & u_type, int side_num, const map_location & loc,
                  const map_location & from, bool show, bool use_undo)
{
	const unit_ptr new_unit = unit_ptr( new unit(u_type, side_num, true));


	// Place the recruit.
	place_recruit_result res = place_recruit(new_unit, loc, from, u_type.cost(), false, map_location::NDIRECTIONS, show);
	statistics::recruit_unit(*new_unit);

	// To speed things a bit, don't bother with the undo stack during
	// an AI turn. The AI will not undo nor delay shroud updates.
	// (Undo stack processing is also suppressed when redoing a recruit.)
	if ( use_undo ) {
		resources::undo_stack->add_recruit(new_unit, loc, from, std::get<1>(res), std::get<2>(res));
		// Check for information uncovered or randomness used.

		if ( std::get<0>(res) || !synced_context::can_undo()) {
			resources::undo_stack->clear();
		}
	}

	// Update the screen.
	if ( resources::screen != nullptr )
		resources::screen->invalidate_game_status();
		// Other updates were done by place_recruit().
}


/**
 * Recalls the unit with the indicated ID for the provided team.
 */
bool recall_unit(const std::string & id, team & current_team,
                 const map_location & loc, const map_location & from,
                 map_location::DIRECTION facing, bool show, bool use_undo)
{
	unit_ptr recall = current_team.recall_list().extract_if_matches_id(id);

	if ( !recall )
		return false;


	// ** IMPORTANT: id might become invalid at this point!
	// (Use recall.id() instead, if needed.)

	// Place the recall.
	// We also check to see if a custom unit level recall has been set if not,
	// we use the team's recall cost otherwise the unit's.
	place_recruit_result res;
	if (recall->recall_cost() < 0) {
		res = place_recruit(recall, loc, from, current_team.recall_cost(),
	                             true, facing, show);
	}
	else {
		res = place_recruit(recall, loc, from, recall->recall_cost(),
	                             true, facing, show);
	}
	statistics::recall_unit(*recall);

	// To speed things a bit, don't bother with the undo stack during
	// an AI turn. The AI will not undo nor delay shroud updates.
	// (Undo stack processing is also suppressed when redoing a recall.)
	if ( use_undo ) {
		resources::undo_stack->add_recall(recall, loc, from, std::get<1>(res), std::get<2>(res));
		if ( std::get<0>(res) || !synced_context::can_undo()) {
			resources::undo_stack->clear();
		}
	}

	// Update the screen.
	if ( resources::screen != nullptr )
		resources::screen->invalidate_game_status();
		// Other updates were done by place_recruit().

	return true;
}


}//namespace actions
