/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef AI_ATTACK_INCLUDED
#define AI_ATTACK_INCLUDED

#include "actions.hpp"
#include "ai.hpp"
#include "gamestatus.hpp"
#include "map.hpp"
#include "unit_types.hpp"

#include <map>
#include <vector>

namespace ai {

struct attack_analysis
{
	void analyze(const gamemap& map, std::map<location,unit>& units,
	             const gamestatus& status, const game_data& info, int sims);

	double rating(double aggression) const;

	gamemap::location target;
	std::vector<std::pair<gamemap::location,gamemap::location> > movements;
	std::vector<int> weapons;

	//the value of the unit being targeted
	double target_value;

	//the value on average, of units lost in the combat
	double avg_losses;

	//estimated % chance to kill the unit
	double chance_to_kill;

	//the average hitpoints damage inflicted
	double avg_damage_inflicted;

	int target_starting_damage;

	//the average hitpoints damage taken
	double avg_damage_taken;

	//the sum of the values of units used in the attack
	double resources_used;

	//the weighted average of the % chance to hit each attacking unit
	double terrain_quality;

	//the ratio of the attacks the unit being attacked will get to
	//the strength of its most powerful attack
	double counter_strength_ratio;
};

std::vector<attack_analysis> analyze_targets(
             const gamemap& map,
             const std::multimap<location,location>& srcdst,
			 const std::multimap<location,location>& dstsrc,
			 std::map<location,unit>& units,
			 const team& current_team, int team_num,
			 const gamestatus& status, const game_data& data
            );

}

#endif
