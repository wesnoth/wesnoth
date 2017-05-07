/*
   Copyright (C) 2007 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef ATTACK_PREDICTION_H_INCLUDED
#define ATTACK_PREDICTION_H_INCLUDED

#include <vector>
#include <array>
#include <cstring>

struct battle_context_unit_stats;

// This encapsulates all we need to know for this combat.
/** All combat-related info. */
struct combatant
{
	/** Construct a combatant. */
	combatant(const battle_context_unit_stats &u, const combatant *prev = nullptr);

	/** Copy constructor */
	combatant(const combatant &that, const battle_context_unit_stats &u);

	combatant(const combatant &that) = delete;
	combatant& operator=(const combatant &) = delete;

	/** Simulate a fight!  Can be called multiple times for cumulative calculations. */
	void fight(combatant &opponent, bool levelup_considered=true);

	/** Resulting probability distribution (might be not as large as max_hp) */
	std::vector<double> hp_dist;

	/** Resulting chance we were not hit by this opponent (important if it poisons) */
	double untouched;

	/** Resulting chance we are poisoned. */
	double poisoned;

	/** Resulting chance we are slowed. */
	double slowed;

	/** What's the average hp (weighted average of hp_dist). */
	double average_hp(unsigned int healing = 0) const;

#if defined(BENCHMARK) || defined(CHECK)
	// Functions used in the stand-alone version of attack_prediction.cpp
	void print(const char label[], unsigned int battle, unsigned int fighter) const;
	void reset();
#endif

private:
	static const unsigned int MONTE_CARLO_SIMULATION_THRESHOLD = 50000u;

	const battle_context_unit_stats &u_;

	/** Summary of matrix used to calculate last battle (unslowed & slowed).
	 *  Invariant: summary[1].size() == summary[0].size() or summary[1].empty() */
	std::array<std::vector<double>, 2> summary;
};

#endif
