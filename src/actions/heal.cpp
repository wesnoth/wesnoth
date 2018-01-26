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
 * Healing (at start of side turn).
 */

#include "actions/heal.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
#include "units/udisplay.hpp"
#include "units/map.hpp"

#include <list>
#include <vector>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)


namespace {

	// Contains the data needed to display healing.
	struct heal_unit {
		heal_unit(unit &patient, const std::vector<unit *> &treaters, int healing,
		          bool poison) :
			healed(patient),
			healers(treaters),
			amount(healing),
			cure_poison(poison)
		{}

		unit & healed;
		std::vector<unit *> healers;
		int amount;
		bool cure_poison;
	};

	// Keep these ordered from weakest cure to strongest cure.
	enum POISON_STATUS { POISON_NORMAL, POISON_SLOW , POISON_CURE };


	/**
	 * Converts a string into its corresponding POISON_STATUS.
	 */
	POISON_STATUS poison_status(const std::string & status)
	{
		if ( status == "cured" )
			return POISON_CURE;

		if ( status == "slowed" )
			return POISON_SLOW;

		// No other states recognized.
		return POISON_NORMAL;
	}


	/**
	 * Determines if @a patient is affected by anything that impacts poison.
	 * If cured by a unit, that unit is added to @a healers.
	 */
	POISON_STATUS poison_progress(int side, const unit & patient,
	                              std::vector<unit *> & healers)
	{
		const std::vector<team> &teams = resources::gameboard->teams();
		unit_map &units = resources::gameboard->units();

		POISON_STATUS curing = POISON_NORMAL;


		if ( patient.side() == side )
		{
			// Village healing?
			if ( resources::gameboard->map().gives_healing(patient.get_location()) )
				return POISON_CURE;

			// Regeneration?
			for (const unit_ability & regen : patient.get_abilities("regenerate"))
			{
				curing = std::max(curing, poison_status((*regen.first)["poison"]));
				if ( curing == POISON_CURE )
					// This is as good as it gets.
					return POISON_CURE;
			}
		}

		// Look through the healers to find a curer.
		unit_map::iterator curer = units.end();
		// Assumed: curing is not POISON_CURE at the start of any iteration.
		for (const unit_ability & heal : patient.get_abilities("heals"))
		{
			POISON_STATUS this_cure = poison_status((*heal.first)["poison"]);
			if ( this_cure <= curing )
				// We already recorded this level of curing.
				continue;

			// NOTE: At this point, this_cure will be *_SLOW or *_CURE.

			unit_map::iterator cure_it = units.find(heal.second);
			assert(cure_it != units.end());
			const int cure_side = cure_it->side();

			// Healers on the current side can cure poison (for any side).
			// Allies of the current side can slow poison (for the current side).
			// Enemies of the current side can do nothing.
			if ( teams[cure_side-1].is_enemy(side) )
				continue;

			// Allied healers can only slow poison, not cure it.
			if ( cure_side != side )
				this_cure = POISON_SLOW;
				// This is where the loop assumption comes into play,
				// as we do not bother comparing POISON_SLOW to curing.

			if ( this_cure == POISON_CURE ) {
				// Return what we found.
				healers.push_back(&*cure_it);
				return POISON_CURE;
			}

			// Record this potential treatment.
			curer = cure_it;
			curing = this_cure;
		}

		// Return the best curing we found.
		if ( curer != units.end() )
			healers.push_back(&*curer);
		return curing;
	}


	/**
	 * Updates the current healing and harming amounts based on a new value.
	 * This is a helper function for heal_amount().
	 * @returns true if an amount was updated.
	 */
	inline bool update_healing(int & healing, int & harming, int value)
	{
		// If the new value magnifies the healing, update and return true.
		if ( value > healing ) {
			healing = value;
			return true;
		}

		// If the new value magnifies the harming, update and return true.
		if ( value < harming ) {
			harming = value;
			return true;
		}

		// Keeping the same values as before.
		return false;
	}
	/**
	 * Calculate how much @patient heals this turn.
	 * If healed by units, those units are added to @a healers.
	 */
	int heal_amount(int side, const unit & patient, std::vector<unit *> & healers)
	{
		unit_map &units = resources::gameboard->units();

		int healing = 0;
		int harming = 0;


		if ( patient.side() == side )
		{
			// Village healing?
			update_healing(healing, harming,
			               resources::gameboard->map().gives_healing(patient.get_location()));

			// Regeneration?
			unit_ability_list regen_list = patient.get_abilities("regenerate");
			unit_abilities::effect regen_effect(regen_list, 0, false);
			update_healing(healing, harming, regen_effect.get_composite_value());
		}

		// Check healing from other units.
		unit_ability_list heal_list = patient.get_abilities("heals");
		// Remove all healers not on this side (since they do not heal now).
		unit_ability_list::iterator heal_it = heal_list.begin();
		while ( heal_it != heal_list.end() ) {
			unit_map::iterator healer = units.find(heal_it->second);
			assert(healer != units.end());

			if ( healer->side() != side )
				heal_it = heal_list.erase(heal_it);
			else
				++heal_it;
		}

		// Now we can get the aggregate healing amount.
		unit_abilities::effect heal_effect(heal_list, 0, false);
		if ( update_healing(healing, harming, heal_effect.get_composite_value()) )
		{
			// Collect the healers involved.
			for (const unit_abilities::individual_effect & heal : heal_effect)
				healers.push_back(&*units.find(heal.loc));

			if ( !healers.empty() ) {
				DBG_NG << "Unit has " << healers.size() << " healers.\n";
			}
		}

		return healing + harming;
	}


	/**
	 * Handles the actual healing.
	 */
	void do_heal(unit &patient, int amount, bool cure_poison)
	{
		if ( cure_poison )
			patient.set_state(unit::STATE_POISONED, false);
		if ( amount > 0)
			patient.heal(amount);
		else if ( amount < 0 )
			patient.take_hit(-amount);
		game_display::get_singleton()->invalidate_unit();
	}


	/**
	 * Animates the healings in the provided list.
	 * (The list will be empty when this returns.)
	 */
	void animate_heals(std::list<heal_unit> &unit_list)
	{
		// Use a nearest-first algorithm.
		map_location last_loc(0,-1);
		while ( !unit_list.empty() )
		{
			std::list<heal_unit>::iterator nearest;
			int min_dist = INT_MAX;

			// Next unit to be healed is the entry in list nearest to last_loc.
			for ( std::list<heal_unit>::iterator check_it = unit_list.begin();
			      check_it != unit_list.end(); ++check_it )
			{
				int distance = distance_between(last_loc, check_it->healed.get_location());
				if ( distance < min_dist ) {
					min_dist = distance;
					nearest = check_it;
					// Allow an early exit if we cannot get closer.
					if ( distance == 1 )
						break;
				}
			}

			std::string cure_text = "";
			if ( nearest->cure_poison )
				cure_text = nearest->healed.gender() == unit_race::FEMALE ?
					            _("female^cured") : _("cured");

			// The heal (animated, then actual):
			unit_display::unit_healing(nearest->healed, nearest->healers,
			                           nearest->amount, cure_text);
			do_heal(nearest->healed, nearest->amount, nearest->cure_poison);

			// Update the loop variables.
			last_loc = nearest->healed.get_location();
			unit_list.erase(nearest);
		}
	}

}//anonymous namespace


// Simple algorithm: no maximum number of patients per healer.
void calculate_healing(int side, bool update_display)
{
	DBG_NG << "beginning of healing calculations\n";

	std::list<heal_unit> unit_list;

	// We look for all allied units, then we see if our healer is near them.
	for (unit &patient : resources::gameboard->units()) {

		if ( patient.get_state("unhealable") || patient.incapacitated() ) {
			if ( patient.side() == side )
				patient.set_resting(true);
			continue;
		}

		DBG_NG << "found healable unit at (" << patient.get_location() << ")\n";

		POISON_STATUS curing = POISON_NORMAL;
		int healing = 0;
		std::vector<unit *> healers;


		// Rest healing.
		if ( patient.side() == side ) {
			if ( patient.resting() || patient.is_healthy() )
				healing += game_config::rest_heal_amount;
			patient.set_resting(true);
		}

		// Main healing.
		if ( !patient.get_state(unit::STATE_POISONED) ) {
			healing += heal_amount(side, patient, healers);
		}
		else {
			curing = poison_progress(side, patient, healers);
			// Poison can be cured at any time, but damage is only
			// taken on the patient's turn.
			if ( curing == POISON_NORMAL  &&  patient.side() == side )
				healing -= game_config::poison_amount;
		}

		// Cap the healing.
		int max_heal = std::max(0, patient.max_hitpoints() - patient.hitpoints());
		int min_heal = std::min(0, 1 - patient.hitpoints());
		if ( healing < min_heal )
			healing = min_heal;
		else if ( healing > max_heal )
			healing = max_heal;

		// Is there nothing to do?
		if ( curing != POISON_CURE  &&  healing == 0 )
			continue;

		if (!healers.empty()) {
			DBG_NG << "Just before healing animations, unit has " << healers.size() << " potential healers.\n";
		}

		const team & viewing_team =
			resources::gameboard->teams()[game_display::get_singleton()->viewing_team()];
		if (!resources::controller->is_skipping_replay() && update_display &&
		    patient.is_visible_to_team(viewing_team, *resources::gameboard, false) )
		{
			unit_list.emplace_front(patient, healers, healing, curing == POISON_CURE);
		}
		else
		{
			// Do the healing now since it will not be animated.
			do_heal(patient, healing, curing == POISON_CURE);
		}
	}

	animate_heals(unit_list);

	DBG_NG << "end of healing calculations\n";
}

