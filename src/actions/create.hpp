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
 * Various functions related to the creation of units (recruits, recalls,
 * and placed units).
 */

#pragma once

class team;
class unit_type;

#include "unit_creator.hpp"

#include "map/location.hpp"
#include "units/ptr.hpp"

#include <tuple>

namespace actions {

/// The possible results of finding a location for recruiting (or recalling).
enum RECRUIT_CHECK {
	RECRUIT_NO_LEADER,         	/// No leaders exist
	RECRUIT_NO_ABLE_LEADER,    	/// No leaders able to recall/recruit the given unit/type.
	RECRUIT_NO_KEEP_LEADER,    	/// No able leaders are on a keep.
	RECRUIT_NO_VACANCY,        	/// No vacant castle tiles around a leader on a keep.
	RECRUIT_ALTERNATE_LOCATION,	/// Recruitment OK, but not at the specified location.
	RECRUIT_OK                 	/// Recruitment OK.
};

/**
 * Checks if there is a location on which to place a recruited unit.
 * A leader of the @a side must be on a keep connected by castle to a
 * legal recruiting location to get an "OK" or "ALTERNATE_LOCATION" result.
 *
 * If "OK" is returned, then the location provided in @a recruit_location
 * is legal. If "ALTERNATE_LOCATION" is returned, the provided location was
 * illegal, so its value was replaced by a location where recruitment can
 * occur.
 *
 * The location of the recruiting leader is stored in @a recruited_from.
 * The incoming value of this parameter is used as a hint for finding a
 * legal recruiter, but this hint is given lower priority than finding a
 * leader who can recruit at recruit_location.
 *
 * The @a unit_type is needed in case this is a leader-specific recruit.
 */
RECRUIT_CHECK check_recruit_location(const int side, map_location &recruit_location,
                                     map_location& recruited_from,
                                     const std::string& unit_type);

/**
 * Finds a location on which to place a unit.
 * A leader of the @a side must be on a keep
 * connected by castle to a legal recruiting location. Otherwise, an error
 * message explaining this is returned.
 *
 * If no errors are encountered, the location where a unit can be recruited
 * is stored in @a recruit_location. Its value is considered first, if it is a
 * legal option.
 * Also, the location of the recruiting leader is stored in @a recruited_from.
 * The incoming value of recruited_from is used as a hint for finding a
 * legal recruiter, but this hint is given lower priority than finding a
 * leader who can recruit at recruit_location.
 *
 * The @a unit_type is needed in case this is a leader-specific recruit.
 *
 * @return an empty string on success. Otherwise a human-readable message
 *         describing the failure is returned.
 */
std::string find_recruit_location(const int side, map_location &recruit_location, map_location& recruited_from, const std::string& unit_type);

/**
 * Checks if there is a location on which to recall @a unit_recall.
 * A leader of the @a side must be on a keep connected by castle to a legal
 * recalling location to get an "OK" or "ALTERNATE_LOCATION" result.
 *
 * If "OK" is returned, then the location provided in @a recall_location
 * is legal. If "ALTERNATE_LOCATION" is returned, the provided location was
 * illegal, so its value was replaced by a location where recalling can
 * occur.
 *
 * The location of the recalling leader is stored in @a recall_from.
 * The incoming value of this parameter is used as a hint for finding a
 * legal recaller, but this hint is given lower priority than finding a
 * leader who can recall at recall_location.
 */
RECRUIT_CHECK check_recall_location(const int side, map_location& recall_location,
                                    map_location& recall_from,
                                    const unit &unit_recall);

/**
 * Finds a location on which to recall @a unit_recall.
 * A leader of the @a side must be on a keep
 * connected by castle to a legal recalling location. Otherwise, an error
 * message explaining this is returned.
 *
 * If no errors are encountered, the location where a unit can be recalled
 * is stored in @a recall_location. Its value is considered first, if it is a
 * legal option.
 * Also, the location of the recalling leader is stored in @a recall_from.
 * The incoming value of this parameter is used as a hint for finding a
 * legal recaller, but this hint is given lower priority than finding a
 * leader who can recall at recall_location.
 *
 * @return an empty string on success. Otherwise a human-readable message
 *         describing the failure is returned.
 */
std::string find_recall_location(const int side, map_location& recall_location, map_location& recall_from, const unit &unit_recall);

/**
 * Gets the recruitable units from a side's leaders' personal recruit lists who can recruit on or from a specific hex field.
 * @param side of the leaders to search for their personal recruit lists.
 * @param recruit_location the hex field being part of the castle the player wants to recruit on or from.
 * @return a set of units that can be recruited either by the leader on @a recruit_location or by leaders on keeps connected by castle tiles to @a recruit_location.
 */
const std::set<std::string> get_recruits(int side, const map_location &recruit_location);

/**
 * Gets the recallable units for a side, restricted by that side's leaders' personal abilities to recall on or from a specific hex field.
 * If no leader is able to recall on or from the given location, the full recall list of the side is returned.
 * @param side of the leaders to search for their personal recall filters.
 * @param recall_loc the hex field being part of the castle the player wants to recruit on or from.
 * @return a set of units that can be recalled by @a side on (or from) @a recall_loc or the full recall list of @a side.
 */
std::vector<unit_const_ptr > get_recalls(int side, const map_location &recall_loc);

typedef std::tuple<bool /*event modified*/, int /*previous village owner side*/, bool /*capture bonus time*/> place_recruit_result;

/**
 * Place a unit into the game.
 * The unit will be placed on @a recruit_location, which should be retrieved
 * through a call to recruit_location().
 * @param facing the desired facing for the unit, map_location::NDIRECTIONS to determine facing automatically.
 * @returns true if an event (or fog clearing) has mutated the game state.
 */
place_recruit_result place_recruit(unit_ptr u, const map_location &recruit_location, const map_location& recruited_from,
	int cost, bool is_recall, map_location::DIRECTION facing = map_location::NDIRECTIONS, bool show = false, bool fire_event = true, bool full_movement = false, bool wml_triggered = false);

/**
 * Recruits a unit of the given type for the given side.
 * This is the point at which the code merges for recruits originating from players,
 * the AI, and replays. It starts just after the recruit location is successfully
 * found, and it handles creating the unit, paying gold, firing events, tracking
 * statistics, and (unless @a is_ai) updating the undo stack.
 */
void recruit_unit(const unit_type & u_type, int side_num, const map_location & loc,
                  const map_location & from, bool show=true, bool use_undo=true);

/**
 * Recalls the unit with the indicated ID for the provided team.
 * The ID can be a reference to data in the recall list.
 * This is the point at which the code merges for recalls originating from players,
 * the AI, and replays. It starts just after the recall location is successfully
 * found, and it handles moving the unit to the board, paying gold, firing events,
 * tracking statistics, updating the undo stack (unless @a use_undo is false), and
 * recording the recall (unless @a use_recorder is false).
 * @param facing the desired facing for the unit, map_location::NDIRECTIONS to determine facing automatically.
 * @returns false if the recall could not be found in the team's recall list.
 */
bool recall_unit(const std::string & id, team & current_team,
                 const map_location & loc, const map_location & from,
                 map_location::DIRECTION facing = map_location::NDIRECTIONS,
                 bool show=true, bool use_undo=true);
}//namespace actions
