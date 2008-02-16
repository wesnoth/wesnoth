/* $Id$ */
/*
   Copyright (C) 2007 - 2008
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file attack_prediction.hpp
//!

#ifndef ATTACK_PREDICTION_H_INCLUDED
#define ATTACK_PREDICTION_H_INCLUDED

#include "global.hpp"

#include <vector>
#include "actions.hpp"

// This encapsulates all we need to know for this combat.
//! All combat-related infos.
struct combatant
{
	//! Construct a combatant.
	combatant(const battle_context::unit_stats &u, const combatant *prev = NULL);

	//! Copy constructor
	combatant(const combatant &that, const battle_context::unit_stats &u);

	//! Simulate a fight!  Can be called multiple times for cumulative calculations.
	void fight(combatant &opponent);

	//! Resulting probability distribution (may NOT be as large as max_hp)
	std::vector<double> hp_dist;

	//! Resulting chance we were not hit by this opponent (important if it poisons)
	double untouched;

	//! Resulting chance we are poisoned.
	double poisoned;

	//! Resulting chance we are slowed.
	double slowed;

	//! What's the average hp (weighted average of hp_dist).
	double average_hp(unsigned int healing = 0) const;

private:
	combatant(const combatant &that);
	combatant& operator=(const combatant &);

	//! Minimum hp we could possibly have.
	unsigned min_hp() const;

	//! HP distribution we could end up with.
	static unsigned hp_dist_size(const battle_context::unit_stats &u, const combatant *prev);

	//! Combat without chance of death, berserk, slow or drain is simple.
	void no_death_fight(combatant &opponent);

	//! Combat with <= 1 strike each is simple, too.
	void one_strike_fight(combatant &opponent);

	//! All other cases.
	void complex_fight(combatant &opponent, unsigned int rounds);

	//! We must adjust for swarm after every combat.
	void adjust_hitchance();

	const battle_context::unit_stats &u_;

	//! Usually uniform, but if we have swarm, then can be different.
	std::vector<double> hit_chances_;

	//! Summary of matrix used to calculate last battle (unslowed & slowed).
	std::vector<double> summary[2];
};

#endif
