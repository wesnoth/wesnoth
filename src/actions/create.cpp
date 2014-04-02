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
 * Recruiting, recalling.
 */

#include "create.hpp"

#include "move.hpp"
#include "undo.hpp"
#include "vision.hpp"

#include "../config.hpp"
#include "../config_assign.hpp"
#include "../game_display.hpp"
#include "../game_events/pump.hpp"
#include "../game_preferences.hpp"
#include "../gamestatus.hpp"
#include "../gettext.hpp"
#include "../log.hpp"
#include "../map.hpp"
#include "../pathfind/pathfind.hpp"
#include "../random.hpp"
#include "../replay.hpp"
#include "../replay_helper.hpp"
#include "../resources.hpp"
#include "../statistics.hpp"
#include "../synced_checkup.hpp"
#include "../synced_context.hpp"
#include "../team.hpp"
#include "../unit_display.hpp"
#include "../variable.hpp"
#include "../whiteboard/manager.hpp"

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)


namespace {
	struct castle_cost_calculator : pathfind::cost_calculator
	{
		castle_cost_calculator(const gamemap& map, const team & view_team) :
			map_(map),
			viewer_(view_team),
			use_shroud_(view_team.uses_shroud())
		{}

		virtual double cost(const map_location& loc, const double) const
		{
			if(!map_.is_castle(loc))
				return 10000;

			if ( use_shroud_ && viewer_.shrouded(loc) )
				return 10000;

			return 1;
		}

	private:
		const gamemap& map_;
		const team& viewer_;
		const bool use_shroud_; // Allows faster checks when shroud is disabled.
	};
}//anonymous namespace


unit_creator::unit_creator(team &tm, const map_location &start_pos)
  : add_to_recall_(false),discover_(false),get_village_(false),invalidate_(false), rename_side_(false), show_(false), start_pos_(start_pos), team_(tm)
{
}


unit_creator& unit_creator::allow_show(bool b)
{
	show_=b;
	return *this;
}


unit_creator& unit_creator::allow_get_village(bool b)
{
	get_village_=b;
	return *this;
}


unit_creator& unit_creator::allow_rename_side(bool b)
{
	rename_side_=b;
	return *this;
}

unit_creator& unit_creator::allow_invalidate(bool b)
{
	invalidate_=b;
	return *this;
}


unit_creator& unit_creator::allow_discover(bool b)
{
	discover_=b;
	return *this;
}


unit_creator& unit_creator::allow_add_to_recall(bool b)
{
	add_to_recall_=b;
	return *this;
}


map_location unit_creator::find_location(const config &cfg, const unit* pass_check)
{

	DBG_NG << "finding location for unit with id=["<<cfg["id"]<<"] placement=["<<cfg["placement"]<<"] x=["<<cfg["x"]<<"] y=["<<cfg["y"]<<"] for side " << team_.side() << "\n";

	std::vector< std::string > placements = utils::split(cfg["placement"]);

	placements.push_back("map");
	placements.push_back("recall");

	BOOST_FOREACH(const std::string& place, placements) {
		map_location loc;

		if ( place == "recall" ) {
			return map_location::null_location;
		}

		else if ( place == "leader"  ||  place == "leader_passable" ) {
			unit_map::const_iterator leader = resources::units->find_leader(team_.side());
			//todo: take 'leader in recall list' possibility into account
			if (leader.valid()) {
				loc = leader->get_location();
			} else {
				loc = start_pos_;
			}
		}

		// "map", "map_passable", and "map_overwrite".
		else if ( place == "map"  ||  place.compare(0, 4, "map_") == 0 ) {
			loc = map_location(cfg,resources::gamedata);
		}

		if(loc.valid() && resources::game_map->on_board(loc)) {
			const bool pass((place == "leader_passable") || (place == "map_passable"));
			if ( place != "map_overwrite" ) {
				loc = find_vacant_tile(loc, pathfind::VACANT_ANY,
				                       pass ? pass_check : NULL);
			}
			if(loc.valid() && resources::game_map->on_board(loc)) {
				return loc;
			}
		}
	}

	return map_location::null_location;

}


void unit_creator::add_unit(const config &cfg, const vconfig* vcfg)
{
	config temp_cfg(cfg);
	temp_cfg["side"] = team_.side();
	temp_cfg.remove_attribute("player_id");
	temp_cfg.remove_attribute("faction_from_recruit");

	const std::string& id =(cfg)["id"];
	bool animate = temp_cfg["animate"].to_bool();
	temp_cfg.remove_attribute("animate");

	std::vector<unit> &recall_list = team_.recall_list();
	std::vector<unit>::iterator recall_list_element = find_if_matches_id(recall_list, id);

	if ( recall_list_element == recall_list.end() ) {
		//make a temporary unit
		boost::scoped_ptr<unit> temp_unit(new unit(temp_cfg, true, vcfg));
		map_location loc = find_location(temp_cfg, temp_unit.get());
		if ( loc.valid() ) {
			unit *new_unit = temp_unit.get();
			//add temporary unit to map
			resources::units->replace(loc, *new_unit);
			LOG_NG << "inserting unit for side " << new_unit->side() << "\n";
			post_create(loc,*(resources::units->find(loc)),animate);
		}
		else if ( add_to_recall_ ) {
			//add to recall list
			unit *new_unit = temp_unit.get();
			recall_list.push_back(*new_unit);
			DBG_NG << "inserting unit with id=["<<id<<"] on recall list for side " << new_unit->side() << "\n";
			preferences::encountered_units().insert(new_unit->type_id());
		}
	} else {
		//get unit from recall list
		map_location loc = find_location(temp_cfg, &(*recall_list_element));
		if ( loc.valid() ) {
			resources::units->replace(loc, *recall_list_element);
			LOG_NG << "inserting unit from recall list for side " << recall_list_element->side()<< " with id="<< id << "\n";
			post_create(loc,*(resources::units->find(loc)),animate);
			//if id is not empty, delete units with this ID from recall list
			erase_if_matches_id(recall_list, id);
		}
		else if ( add_to_recall_ ) {
			LOG_NG << "wanted to insert unit on recall list, but recall list for side " << (cfg)["side"] << "already contains id=" <<id<<"\n";
			return;
		}
	}
}


void unit_creator::post_create(const map_location &loc, const unit &new_unit, bool anim)
{

	if (discover_) {
		preferences::encountered_units().insert(new_unit.type_id());
	}

	bool show = show_ && (resources::screen !=NULL) && !resources::screen->fogged(loc);
	bool animate = show && anim;

	if (get_village_) {
		if (resources::game_map->is_village(loc)) {
			actions::get_village(loc, new_unit.side());
		}
	}

	if (resources::screen!=NULL) {

		if (invalidate_ ) {
			resources::screen->invalidate(loc);
		}

		if (animate) {
			unit_display::unit_recruited(loc);
		} else if (show) {
			resources::screen->draw();
		}
	}
}


/**
 * Checks to see if a leader at @a leader_loc could recruit somewhere.
 * This takes into account terrain, shroud (for side @a side), and the presence
 * of visible units.
 * The behavior for an invalid @a side is subject to change for future needs.
 */
bool can_recruit_from(const map_location& leader_loc, int side)
{
	const gamemap& map = *resources::game_map;

	if( !map.is_keep(leader_loc) )
		return false;

	if ( side < 1  ||  resources::teams == NULL  ||
	     resources::teams->size() < static_cast<size_t>(side) ) {
		// Invalid side specified.
		// Currently this cannot happen, but it could conceivably be used in
		// the future to request that shroud and visibility be ignored. Until
		// that comes to pass, just return.
 		return false;
	}

	return pathfind::find_vacant_tile(leader_loc, pathfind::VACANT_CASTLE, NULL,
	                                  &(*resources::teams)[side-1])
	       != map_location::null_location;
}

/**
 * Checks to see if a leader at @a leader_loc could recruit on @a recruit_loc.
 * This takes into account terrain, shroud (for side @a side), and whether or
 * not there is already a visible unit at recruit_loc.
 * The behavior for an invalid @a side is subject to change for future needs.
 */
bool can_recruit_on(const map_location& leader_loc, const map_location& recruit_loc, int side)
{
	const gamemap& map = *resources::game_map;

	if( !map.is_castle(recruit_loc) )
		return false;

	if( !map.is_keep(leader_loc) )
		return false;

	if ( side < 1  ||  resources::teams == NULL  ||
	     resources::teams->size() < static_cast<size_t>(side) ) {
		// Invalid side specified.
		// Currently this cannot happen, but it could conceivably be used in
		// the future to request that shroud and visibility be ignored. Until
		// that comes to pass, just return.
 		return false;
	}
	const team & view_team = (*resources::teams)[side-1];

	if ( view_team.shrouded(recruit_loc) )
		return false;

	if ( get_visible_unit(recruit_loc, view_team) != NULL )
		return false;

	castle_cost_calculator calc(map, view_team);
	// The limit computed in the third argument is more than enough for
	// any convex castle on the map. Strictly speaking it could be
	// reduced to sqrt(map.w()**2 + map.h()**2).
	pathfind::plain_route rt =
		pathfind::a_star_search(leader_loc, recruit_loc, map.w()+map.h(), &calc,
		                        map.w(), map.h());
	return !rt.steps.empty();
}


namespace actions {


const std::set<std::string> get_recruits(int side, const map_location &recruit_loc)
{
	const team & current_team = (*resources::teams)[side -1];

	LOG_NG << "getting recruit list for side " << side << " at location " << recruit_loc << "\n";

	std::set<std::string> local_result;
	std::set<std::string> global_result;
	unit_map::const_iterator u = resources::units->begin(),
			u_end = resources::units->end();

	bool leader_in_place = false;
	bool allow_local = resources::game_map->is_castle(recruit_loc);


	// Check for a leader at recruit_loc (means we are recruiting from there,
	// rather than to there).
	unit_map::const_iterator find_it = resources::units->find(recruit_loc);
	if ( find_it != u_end ) {
		if ( find_it->can_recruit()  &&  find_it->side() == side  &&
		     resources::game_map->is_keep(recruit_loc) )
		{
			// We have been requested to get the recruit list for this
			// particular leader.
			leader_in_place = true;
			local_result.insert(find_it->recruits().begin(),
			                    find_it->recruits().end());
		}
		else if ( find_it->is_visible_to_team(current_team, false) )
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
			if ( allow_local && can_recruit_on(*u, recruit_loc) ) {
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
	void add_leader_filtered_recalls(const unit & leader,
	                                 std::vector<const unit*> & result,
	                                 std::set<size_t> * already_added = NULL)
	{
		const team& leader_team = (*resources::teams)[leader.side()-1];
		const std::vector<unit>& recall_list = leader_team.recall_list();
		const std::string& save_id = leader_team.save_id();

		BOOST_FOREACH(const unit& recall_unit, recall_list)
		{
			// Do not add a unit twice.
			size_t underlying_id = recall_unit.underlying_id();
			if ( !already_added  ||  already_added->count(underlying_id) == 0 )
			{
				// Only units that match the leader's recall filter are valid.
				scoped_recall_unit this_unit("this_unit", save_id, &recall_unit - &recall_list[0]);

				if ( recall_unit.matches_filter(vconfig(leader.recall_filter()), map_location::null_location) )
				{
					result.push_back(&recall_unit);
					if ( already_added != NULL )
						already_added->insert(underlying_id);
				}
			}
		}
	}
}// anonymous namespace

const std::vector<const unit*> get_recalls(int side, const map_location &recall_loc)
{
	LOG_NG << "getting recall list for side " << side << " at location " << recall_loc << "\n";

	std::vector<const unit*> result;

	/*
	 * We have three use cases:
	 * 1. An empty castle tile is highlighted; we return only the units recallable there.
	 * 2. A leader on a keep is highlighted; we return only the units recallable by that leader.
	 * 3. Otherwise, we return all units in the recall list.
	 */

	bool leader_in_place = false;
	bool allow_local = resources::game_map->is_castle(recall_loc);


	// Check for a leader at recall_loc (means we are recalling from there,
	// rather than to there).
	unit_map::const_iterator find_it = resources::units->find(recall_loc);
	if ( find_it != resources::units->end() ) {
		if ( find_it->can_recruit()  &&  find_it->side() == side  &&
		     resources::game_map->is_keep(recall_loc) )
		{
			// We have been requested to get the recalls for this
			// particular leader.
			add_leader_filtered_recalls(*find_it, result);
			return result;
		}
		else if ( find_it->is_visible_to_team((*resources::teams)[side-1], false) )
		{
			// This hex is visibly occupied, so we cannot recall here.
			allow_local = false;
		}
	}

	if ( allow_local )
	{
		unit_map::const_iterator u = resources::units->begin(),
				u_end = resources::units->end();
		std::set<size_t> valid_local_recalls;

		for(; u != u_end; ++u) {
			//We only consider leaders on our side.
			if (!(u->can_recruit() && u->side() == side))
				continue;

			// Check if the leader is on a connected keep.
			if ( !can_recruit_on(*u, recall_loc) )
				continue;
			leader_in_place= true;

			add_leader_filtered_recalls(*u, result, &valid_local_recalls);
		}
	}

	if ( !leader_in_place )
	{
		// Return the full recall list.
		const std::vector<unit>& recall_list = (*resources::teams)[side-1].recall_list();
		BOOST_FOREACH(const unit &recall, recall_list)
		{
			result.push_back(&recall);
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
		const team& recall_team = (*resources::teams)[recaller.side()-1];
		scoped_recall_unit this_unit("this_unit", recall_team.save_id(),
		                             &recall_unit - &recall_team.recall_list()[0]);
		if ( !recall_unit.matches_filter(vconfig(recaller.recall_filter()),
		                                 map_location::null_location) )
			return RECRUIT_NO_ABLE_LEADER;

		// Make sure the unit is on a keep.
		if ( !resources::game_map->is_keep(recaller.get_location()) )
			return RECRUIT_NO_KEEP_LEADER;

		// Make sure there is a permissible location to which to recruit.
		map_location permissible = pathfind::find_vacant_castle(recaller);
		if ( !permissible.valid() )
			return RECRUIT_NO_VACANCY;

		// See if the preferred location cannot be used.
		if ( !can_recruit_on(recaller, preferred) ) {
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
	const unit_map & units = *resources::units;
	const unit_map::const_iterator u_end = units.end();

	map_location check_location = recall_location;
	map_location alternative;	// Set by check_unit_recall_location().

	// If the specified location is occupied, proceed as if no location was specified.
	if ( resources::units->count(recall_location) != 0 )
		check_location = map_location::null_location;

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
		return _("You don’t have a leader to recall with.");

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
	ERR_NG << "Unrecognized enum in find_recall_location()\n";
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
			if ( !util::contains(recruiter.recruits(), unit_type) )
				return RECRUIT_NO_ABLE_LEADER;
		}

		// Make sure the unit is on a keep.
		if ( !resources::game_map->is_keep(recruiter.get_location()) )
			return RECRUIT_NO_KEEP_LEADER;

		// Make sure there is a permissible location to which to recruit.
		map_location permissible = pathfind::find_vacant_castle(recruiter);
		if ( !permissible.valid() )
			return RECRUIT_NO_VACANCY;

		// See if the preferred location cannot be used.
		if ( !can_recruit_on(recruiter, preferred) ) {
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
	const unit_map & units = *resources::units;
	const unit_map::const_iterator u_end = units.end();

	map_location check_location = recruit_location;
	std::string check_type = unit_type;
	map_location alternative;	// Set by check_unit_recruit_location().

	// If the specified location is occupied, proceed as if no location was specified.
	if ( resources::units->count(recruit_location) != 0 )
		check_location = map_location::null_location;

	// If the specified unit type is in the team's recruit list, there is no
	// need to check each leader's list.
	if ( util::contains((*resources::teams)[side-1].recruits(), unit_type) )
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
		return _("You don’t have a leader to recruit with.");

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
	ERR_NG << "Unrecognized enum in find_recruit_location()\n";
	return _("An unrecognized error has occurred.");
}


namespace { // Helpers for place_recruit()
	/**
	 * Performs a checksum check on a newly recruited/recalled unit.
	 */
	void recruit_checksums(const unit &new_unit, bool /*wml_triggered*/)
	{
		const std::string checksum = get_checksum(new_unit);
		config original_checksum_config;

		bool checksum_equals = checkup_instance->local_checkup(config_of("checksum", checksum),original_checksum_config);
		if(!checksum_equals)
		{
			const std::string old_checksum = original_checksum_config["checksum"];
			std::stringstream error_msg;
			error_msg << "SYNC: In recruit " << new_unit.type_id() <<
				": has checksum " << checksum <<
				" while datasource has checksum " << old_checksum << "\n";
			ERR_NG << error_msg.str();
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
		const unit_map & units = *resources::units;

		// See if the preferred location is an option.
		unit_map::const_iterator leader = units.find(recruited_from);
		if ( leader != units.end()  &&  leader->can_recruit()  &&
			 leader->side() == side &&  can_recruit_on(*leader, recruit_location) )
			return leader->get_location();

		// Check all units.
		for ( leader = units.begin(); leader != units.end(); ++leader )
			if ( leader->can_recruit()  &&  leader->side() == side  &&
				 can_recruit_on(*leader, recruit_location) )
				return leader->get_location();

		// No usable leader found.
		return map_location::null_location;
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
			un_it = resources::units->find(current_loc);
			if ( un_it == resources::units->end() )
				// The unit is gone.
				return false;
		}
		current_loc = un_it->get_location();
		return true;
	}
}// anonymous namespace

bool place_recruit(const unit &u, const map_location &recruit_location, const map_location& recruited_from,
    int cost, bool is_recall, bool show, bool fire_event, bool full_movement,
    bool wml_triggered)
{
	LOG_NG << "placing new unit on location " << recruit_location << "\n";

	bool mutated = false;
	unit new_unit = u;
	if (full_movement) {
		new_unit.set_movement(new_unit.total_movement(), true);
	} else {
		new_unit.set_movement(0, true);
		new_unit.set_attacks(0);
	}
	new_unit.heal_all();
	new_unit.set_hidden(true);

	// Get the leader location before adding the unit to the board.
	const map_location leader_loc = !show ? map_location::null_location :
			find_recruit_leader(new_unit.side(), recruit_location, recruited_from);

	// Add the unit to the board.
	std::pair<unit_map::iterator, bool> add_result =
			resources::units->add(recruit_location, new_unit);
	assert(add_result.second);
	unit_map::iterator & new_unit_itor = add_result.first;
	map_location current_loc = recruit_location;

	// Do some bookkeeping.
	recruit_checksums(new_unit, wml_triggered);
	resources::whiteboard->on_gamestate_change();

	if ( fire_event ) {
		const std::string event_name = is_recall ? "prerecall" : "prerecruit";
		LOG_NG << "firing " << event_name << " event\n";
		{
			mutated |= game_events::fire(event_name, current_loc, recruited_from);
		}
		if ( !validate_recruit_iterator(new_unit_itor, current_loc) )
			return true;
		new_unit_itor->set_hidden(true);
	}
	preferences::encountered_units().insert(new_unit_itor->type_id());
	(*resources::teams)[new_unit.side()-1].spend_gold(cost);

	if ( show ) {
		unit_display::unit_recruited(current_loc, leader_loc);
	}
	// Make sure the unit appears (if either !show or the animation is suppressed).
	new_unit_itor->set_hidden(false);
	if ( resources::screen != NULL ) {
		resources::screen->invalidate(current_loc);
		resources::screen->redraw_minimap();
	}

	// Village capturing.
	if ( resources::game_map->is_village(current_loc) ) {
		mutated |= actions::get_village(current_loc, new_unit_itor->side());
		if ( !validate_recruit_iterator(new_unit_itor, current_loc) )
			return true;
	}

	// Fog clearing.
	actions::shroud_clearer clearer;
	if ( !wml_triggered ) // To preserve current WML behavior.
		mutated |= clearer.clear_unit(current_loc, *new_unit_itor);

	if ( fire_event ) {
		const std::string event_name = is_recall ? "recall" : "recruit";
		LOG_NG << "firing " << event_name << " event\n";
		{
			mutated |= game_events::fire(event_name, current_loc, recruited_from);
		}
	}

	// "sighted" event(s).
	mutated |= clearer.fire_events();
	if ( new_unit_itor.valid() )
		mutated |= actions::actor_sighted(*new_unit_itor);

	return mutated;
}


/**
 * Recruits a unit of the given type for the given side.
 */
void recruit_unit(const unit_type & u_type, int side_num, const map_location & loc,
                  const map_location & from, bool show, bool use_undo)
{
	const unit new_unit(u_type, side_num, true);


	// Place the recruit.
	/*bool mutated = */place_recruit(new_unit, loc, from, u_type.cost(), false, show);
	statistics::recruit_unit(new_unit);

	// To speed things a bit, don't bother with the undo stack during
	// an AI turn. The AI will not undo nor delay shroud updates.
	// (Undo stack processing is also suppressed when redoing a recruit.)
	if ( use_undo ) {
		resources::undo_stack->add_recruit(new_unit, loc, from);
		// Check for information uncovered or randomness used.
		
		// undoing unit recruits currently ends in oos in most cases
		// because the random generator is called during unit creation for names.
		// This is not impossible to fix, ofc we could just enable it for nameless units 
		// but i'm not 100% sure wether units with no names call the random generator for random names or not.
		// also names normaly don't have any influence on the game (same for genders maybe?)
		// so there is no reason not to be able to undo it from a players point of view.
		// we also cannot store the random state in undo_stack->add_recruit, because the unit creation happens before that.
		
		//if ( mutated  ||  u_type.genders().size() > 1  ||
		//     new_unit.type().has_random_traits() ) {
		if ( true ) {
			resources::undo_stack->clear();
		}
	}

	// Update the screen.
	if ( resources::screen != NULL )
		resources::screen->invalidate_game_status();
		// Other updates were done by place_recruit().
}


/**
 * Recalls the unit with the indicated ID for the provided team.
 */
bool recall_unit(const std::string & id, team & current_team,
                 const map_location & loc, const map_location & from,
                 bool show, bool use_undo)
{
	std::vector<unit> & recall_list = current_team.recall_list();

	// Find the unit to recall.
	std::vector<unit>::iterator recall_it = find_if_matches_id(recall_list, id);
	if ( recall_it == recall_list.end() )
		return false;
		

	// Make a copy of the unit before erasing it from the list.
	unit recall(*recall_it);
	recall_list.erase(recall_it);
	// ** IMPORTANT: id might become invalid at this point!
	// (Use recall.id() instead, if needed.)

	// Place the recall.
	// We also check to see if a custom unit level recall has been set if not,
	// we use the team's recall cost otherwise the unit's.
	bool mutated;
	if (recall.recall_cost() < 0) {
		mutated = place_recruit(recall, loc, from, current_team.recall_cost(),
	                             true, show);
	}
	else {
		mutated = place_recruit(recall, loc, from, recall.recall_cost(),
	                             true, show);
	}
	statistics::recall_unit(recall);

	// To speed things a bit, don't bother with the undo stack during
	// an AI turn. The AI will not undo nor delay shroud updates.
	// (Undo stack processing is also suppressed when redoing a recall.)
	if ( use_undo ) {
		resources::undo_stack->add_recall(recall, loc, from);
		if ( mutated ) {
			resources::undo_stack->clear();
		}
	}

	// Update the screen.
	if ( resources::screen != NULL )
		resources::screen->invalidate_game_status();
		// Other updates were done by place_recruit().

	// Record a checksum so the replay can be verified.
	checkup_instance->unit_checksum(loc);
	return true;
}


}//namespace actions
