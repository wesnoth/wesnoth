/* $Id$ */

#ifndef ATTACK_PREDICTION_H_INCLUDED
#define ATTACK_PREDICTION_H_INCLUDED

#include <vector>

// This encapsulates all we need to know for this combat.
struct combatant
{
	// Construct a combatant.
	// As an optimization, if it has no weapons which could drain,
	// we can simply never calculate hitpoints > current hp.
	combatant(unsigned hp, unsigned max_hp, bool slowed,
			  bool could_ever_drain = true);

	// Select a weapon.
	void set_weapon(unsigned num_attacks, bool drains, bool berserk,
					bool swarm, bool firststrike);

	// Set effect against this particular opponent.
	void set_effectiveness(unsigned damage, double hit_chance, bool slows);

	// Fight!
	void fight(combatant &opponent);

	// Only used in benchmarking.
	void reset();
	void print(const char label[], unsigned int battle) const;

	// Resulting probability distribution (may NOT be as large as max_hp)
	std::vector<double> hp_dist;

	// Resulting chance we were not hit by this opponent (important if
	// it poisons)
	double untouched;

private:
	// How many attacks?  (Matters for swarm).
	unsigned num_attacks(unsigned int hp) const;

	// Usually uniform, but if we have swarm, then can be different.
	std::vector<double> hit_chances_;
	double base_hit_chance_;

	// Starting hitpoints, max hp.
	unsigned hp_, max_hp_;

	// Are we slowed already?  (Halves damage, can't be slowed again).
	bool slowed_;

	// Weapon stats.
	unsigned base_num_attacks_, damage_;
	bool drains_, slows_, berserk_, swarm_, firststrike_;

	// Summary of matrix used to calculate last battle (unslowed & slowed).
	std::vector<double> summary[2];
};

#endif
