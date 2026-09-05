/*
	Copyright (C) 2025
	by Durzi/mentos987
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

/**
 * @file
 * Combat prediction for the Reduced RNG (random_mode "biased").
 * A single real RNG roll per side - the credit - and the rest are guaranteed hits or misses.
 * hits = (chance_to_hit * strikes + credit) / 100
 *
 * This is all of the Reduced RNG that attack_prediction.cpp asks about: whether a fight can be
 * answered, and the answer. How either is arrived at is private to attack_prediction_biased.cpp.
 */

#include <array>
#include <vector>

struct battle_context_unit_stats;

namespace randomness
{
class rng;
}

namespace biased_rng
{
/** Hit point distribution of one combatant, unslowed in [0] and slowed in [1]. */
using summary_t = std::array<std::vector<double>, 2>;

/** Whether this combat uses the Reduced RNG at all. */
bool enabled();

/**
 * Whether solve_fight() can answer for this pairing, false whenever the Reduced RNG is not in use.
 * It needs damage per strike fixed, so that hit points follow from hit counts: slow, drain and
 * petrify decline, as do an incoming slowed summary and a state space past the budget. Swarm does
 * not - the caller slices those out beforehand.
 */
bool can_solve(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		const summary_t& summary,
		const summary_t& opp_summary);

/**
 * Writes both sides' exact hit point distributions and folds in their chances of taking no hit at
 * all. Only legal where can_solve() agrees, which the caller settles once per fight.
 */
void solve_fight(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		unsigned int strikes,
		unsigned int opp_strikes,
		std::vector<double>& hp_dist,
		std::vector<double>& opp_hp_dist,
		double& self_not_hit,
		double& opp_not_hit,
		bool levelup_considered);

/**
 * Whether listing this fight's courses is worth it. The listing answers the fights can_solve()
 * turns away, and is exact too, but it grows with the number of ways the landed hits can be
 * arranged. Past the point where it would outwork the sampler the caller is better off leaving the
 * fight to the default RNG's own routing, so this is asked before replay_fight() is reached for.
 */
bool can_replay(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		const summary_t& summary,
		const summary_t& opp_summary,
		double slowed_chance,
		double opp_slowed_chance);

/**
 * Lists the fight's courses and writes the summaries their weighted outcomes add up to, along with
 * each side's chance of taking no hit. Only legal where can_replay() agrees.
 */
void replay_fight(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		summary_t& summary,
		summary_t& opp_summary,
		double& self_not_hit,
		double& opp_not_hit,
		bool levelup_considered,
		double slowed_chance,
		double opp_slowed_chance);

/*
 * perform_hit's rule, restated as something callable so that the predictions above can be checked
 * against the combat they claim to describe rather than against a second reading of it.
 */

/** The once-per-side credit a fight opens with. */
int roll_credit(randomness::rng& rng);

/** One strike. strikes_left counts this one, and chance_to_hit is the percentage perform_hit uses. */
bool roll_hit(randomness::rng& rng, unsigned int strikes_left, unsigned int chance_to_hit, int& credit);

} // namespace biased_rng
