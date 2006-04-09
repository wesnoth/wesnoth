/* $Id$ */

#ifndef ATTACK_PREDICTION_H_INCLUDED
#define ATTACK_PREDICTION_H_INCLUDED

#include "global.hpp"

#include <vector>
#include "actions.hpp"

// This encapsulates all we need to know for this combat.
struct combatant
{
	// Construct a combatant.
	combatant(const battle_context::unit_stats &u);

	// Simulate a fight!  Can be called multiple times for cumulative calculations.
	void fight(combatant &opponent);

	// Resulting probability distribution (may NOT be as large as max_hp)
	std::vector<double> hp_dist;

	// Resulting chance we were not hit by this opponent (important if it poisons)
	double untouched;

private:
	// We must adjust for swarm after every combat.
	void adjust_hitchance();

	const battle_context::unit_stats &u_;

	// Usually uniform, but if we have swarm, then can be different.
	std::vector<double> hit_chances_;

	// Summary of matrix used to calculate last battle (unslowed & slowed).
	std::vector<double> summary[2];
};

#endif
