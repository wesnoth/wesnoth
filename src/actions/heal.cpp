/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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
 * Healing (at start of side turn).
 */

#include "heal.hpp"

#include "../game_display.hpp"
#include "../log.hpp"
#include "../map.hpp"
#include "../replay.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../unit.hpp"
#include "../unit_abilities.hpp"
#include "../unit_display.hpp"
#include "../unit_map.hpp"

#include <boost/foreach.hpp>
#include <list>
#include <vector>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)


void reset_resting(unit_map& units, int side)
{
	BOOST_FOREACH(unit &u, units) {
		if (u.side() == side)
			u.set_resting(true);
	}
}

/* Contains all the data used to display healing */
struct unit_healing_struct {
	unit *healed;
	std::vector<unit *> healers;
	int healing;
};

// Simple algorithm: no maximum number of patients per healer.
void calculate_healing(int side, bool update_display)
{
	DBG_NG << "beginning of healing calculations\n";
	unit_map &units = *resources::units;

	std::list<unit_healing_struct> l;

	// We look for all allied units, then we see if our healer is near them.
	BOOST_FOREACH(unit &u, units) {

		if (u.get_state("unhealable") || u.incapacitated())
			continue;

		DBG_NG << "found healable unit at (" << u.get_location() << ")\n";

		unit_map::iterator curer = units.end();
		std::vector<unit *> healers;

		int healing = 0;
		int rest_healing = 0;

		std::string curing;

		unit_ability_list heal = u.get_abilities("heals");

		const bool is_poisoned = u.get_state(unit::STATE_POISONED);
		if(is_poisoned) {
			// Remove the enemies' healers to determine if poison is slowed or cured
			for ( unit_ability_list::iterator h_it = heal.begin();
			      h_it != heal.end(); ) {

				unit_map::iterator potential_healer = units.find(h_it->second);

				assert(potential_healer != units.end());
				if ((*resources::teams)[potential_healer->side() - 1].is_enemy(side)) {
					h_it = heal.erase(h_it);
				} else {
					++h_it;
				}
			}
			BOOST_FOREACH (const unit_ability & ability, heal) {

				if((*ability.first)["poison"] == "cured") {
					curer = units.find(ability.second);
					// Full curing only occurs on the healer turn (may be changed)
					if(curer->side() == side) {
						curing = "cured";
					} else if(curing != "cured") {
						curing = "slowed";
					}
				} else if(curing != "cured" && (*ability.first)["poison"] == "slowed") {
					curer = units.find(ability.second);
					curing = "slowed";
				}
			}
		}

		// For heal amounts, only consider healers on side which is starting now.
		// Remove all healers not on this side.
		for ( unit_ability_list::iterator h_it = heal.begin(); h_it != heal.end(); ) {

			unit_map::iterator potential_healer = units.find(h_it->second);
			assert(potential_healer != units.end());
			if(potential_healer->side() != side) {
				h_it = heal.erase(h_it);
			} else {
				++h_it;
			}
		}

		unit_abilities::effect heal_effect(heal,0,false);
		healing = heal_effect.get_composite_value();

		for(std::vector<unit_abilities::individual_effect>::const_iterator heal_loc = heal_effect.begin(); heal_loc != heal_effect.end(); ++heal_loc) {
			healers.push_back(&*units.find(heal_loc->loc));
		}

		if (!healers.empty()) {
			DBG_NG << "Unit has " << healers.size() << " potential healers\n";
		}

		if (u.side() == side) {
			unit_ability_list regen = u.get_abilities("regenerate");
			unit_abilities::effect regen_effect(regen,0,false);
			if(regen_effect.get_composite_value() > healing) {
				healing = regen_effect.get_composite_value();
				healers.clear();
			}
			if ( !regen.empty() ) {
				BOOST_FOREACH (const unit_ability & ability, regen) {
					if((*ability.first)["poison"] == "cured") {
						curer = units.end();
						curing = "cured";
					} else if(curing != "cured" && (*ability.first)["poison"] == "slowed") {
						curer = units.end();
						curing = "slowed";
					}
				}
			}
			if (int h = resources::game_map->gives_healing(u.get_location())) {
				if (h > healing) {
					healing = h;
					healers.clear();
				}
				/** @todo FIXME */
				curing = "cured";
				curer = units.end();
			}
			if (u.resting() || u.is_healthy()) {
				rest_healing = game_config::rest_heal_amount;
				healing += rest_healing;
			}
		}
		if(is_poisoned) {
			if(curing == "cured") {
				u.set_state(unit::STATE_POISONED, false);
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(&*curer);
			} else if(curing == "slowed") {
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(&*curer);
			} else {
				healers.clear();
				healing = rest_healing;
				if (u.side() == side) {
					healing -= game_config::poison_amount;
				}
			}
		}

		if (curing == "" && healing==0) {
			continue;
		}

		int pos_max = u.max_hitpoints() - u.hitpoints();
		int neg_max = -(u.hitpoints() - 1);
		if(healing > 0 && pos_max <= 0) {
			// Do not try to "heal" if HP >= max HP
			continue;
		}
		if(healing > pos_max) {
			healing = pos_max;
		} else if(healing < neg_max) {
			healing = neg_max;
		}

		if (!healers.empty()) {
			DBG_NG << "Just before healing animations, unit has " << healers.size() << " potential healers\n";
		}


		if (!recorder.is_skipping() && update_display &&
		    !(u.invisible(u.get_location()) &&
		      (*resources::teams)[resources::screen->viewing_team()].is_enemy(side)))
		{
			unit_healing_struct uhs = { &u, healers, healing };
			l.push_front(uhs);
		}
		if (healing > 0)
			u.heal(healing);
		else if (healing < 0)
			u.take_hit(-healing);
		resources::screen->invalidate_unit();
	}

	// Display healing with nearest first algorithm.
	if (!l.empty()) {

		// The first unit to be healed is chosen arbitrarily.
		unit_healing_struct uhs = l.front();
		l.pop_front();

		unit_display::unit_healing(*uhs.healed, uhs.healed->get_location(),
			uhs.healers, uhs.healing);

		/* next unit to be healed is nearest from uhs left in list l */
		while (!l.empty()) {

			std::list<unit_healing_struct>::iterator nearest;
			int min_d = INT_MAX;

			/* for each unit in l, remember nearest */
			for (std::list<unit_healing_struct>::iterator i =
			     l.begin(), i_end = l.end(); i != i_end; ++i)
			{
				int d = distance_between(uhs.healed->get_location(), i->healed->get_location());
				if (d < min_d) {
					min_d = d;
					nearest = i;
				}
			}

			uhs = *nearest;
			l.erase(nearest);

			unit_display::unit_healing(*uhs.healed, uhs.healed->get_location(),
				uhs.healers, uhs.healing);
		}
	}

	DBG_NG << "end of healing calculations\n";
}

