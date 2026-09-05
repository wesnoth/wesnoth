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

/**
 * @file
 * Combat prediction for the Reduced RNG (random_mode "biased").
 *
 * A fight under that RNG opens with a single real roll per side - the credit - and every strike
 * after it is a guaranteed hit or a guaranteed miss, hits = (chance_to_hit * strikes + credit) / 100.
 * The independent-strike matrices in attack_prediction.cpp describe none of that, so where the hit
 * points follow from the hit counts alone this file answers the fight outright instead, and where
 * they do not it lists the courses the fight can take and weighs them.
 */

#include "attack_prediction_biased.hpp"

#include "actions/attack.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "random.hpp"
#include "resources.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <vector>

namespace biased_rng
{
namespace
{
/**
 * Swings wide enough that listing their patterns is hopeless anyway. Bounding this keeps the bit a
 * strike_patterns() position needs inside the 32 a pattern has.
 */
constexpr unsigned int MAX_PATTERN_STRIKES = 24;

// Saturating, because the callers only ever compare the result against a budget.
constexpr std::uint64_t COST_CAP = std::numeric_limits<std::uint64_t>::max();

std::uint64_t saturating_mul(std::uint64_t a, std::uint64_t b)
{
	return (a != 0 && b > COST_CAP / a) ? COST_CAP : a * b;
}

std::uint64_t saturating_add(std::uint64_t a, std::uint64_t b)
{
	return b > COST_CAP - a ? COST_CAP : a + b;
}

/** A run of credit values the fight behaves identically for, and the chance of rolling into it. */
struct credit_region
{
	unsigned int credit;
	double probability;
};

// Hits owed by the end of `round`, given the credit the fight opened with.
// perform_hit carries one running credit across every round, adding chance_to_hit per strike and
// subtracting 100 per landed hit, while the strikes-left count restarts each round. Those two
// cancel, leaving the running total a function of the opening credit and the round alone.
int hits_owed(unsigned int strikes_per_round, unsigned int chance_to_hit, unsigned int credit, unsigned int round)
{
	return static_cast<int>(chance_to_hit * strikes_per_round * (round + 1) + credit) / 100;
}

// The chance this strike hits: hits still due, divided by strikes left.
// The due count covers every strike so far, including berserk's earlier rounds.
double chance_this_strike_hits(unsigned int strikes_per_round,
		unsigned int chance_to_hit,
		unsigned int credit,
		unsigned int round,
		unsigned int strike_index,
		unsigned int hits_landed)
{
	const int strikes_left = static_cast<int>(strikes_per_round - strike_index);
	const int due = hits_owed(strikes_per_round, chance_to_hit, credit, round) - static_cast<int>(hits_landed);

	return std::clamp(due, 0, strikes_left) / static_cast<double>(strikes_left);
}

/** How many equally likely courses a round of `strikes` swings landing `hits` of them can take. */
std::uint64_t choose(unsigned int strikes, unsigned int hits)
{
	if(hits > strikes) {
		return 0;
	}

	hits = std::min(hits, strikes - hits);

	std::uint64_t out = 1;
	for(unsigned int i = 0; i < hits; ++i) {
		out = saturating_mul(out, strikes - i) / (i + 1);
	}

	return out;
}

// Past this can_solve() declines and the caller simulates instead. The budget is about the work
// the default matrix does at MONTE_CARLO_SIMULATION_THRESHOLD, so neither path costs more than
// the other one would.
constexpr std::uint64_t MAX_SOLVE_STEPS = 300000;

/**
 * "However the strikes fall, this unit lives." Every starting hit point value that is safe shares
 * this one threshold, and so shares a single solve.
 */
constexpr std::size_t KILL_NEVER = std::numeric_limits<std::size_t>::max();

/**
 * How many landed hits it takes to kill, which is the *only* thing solve_hit_counts() learns about
 * a unit's hit points. Two fights that agree on both thresholds have the same distribution of hit
 * counts however far apart their hit point totals are, which is what lets one solve serve a whole
 * band of them.
 */
std::size_t hits_to_kill(int damage, std::size_t hp, std::size_t strikes_total)
{
	if(damage <= 0) {
		return KILL_NEVER;
	}

	const auto per_hit = static_cast<std::size_t>(damage);
	const std::size_t needed = (hp + per_hit - 1) / per_hit;
	return needed > strikes_total ? KILL_NEVER : needed;
}

/** One starting hit point value out of an incoming summary, with the chance of holding it. */
struct start_hp
{
	std::size_t hp;
	double prob;
};

/** A run of starting hit point values sharing a kill threshold, and so sharing one solve. */
struct kill_band
{
	std::size_t threshold;
	std::size_t begin;
	std::size_t end;
};

/**
 * Splits starting hit points into bands of equal kill threshold. `damage` and `strikes_total`
 * belong to the *opponent*: the threshold asked about is what it takes to kill these hit points.
 * collect() emits hit points in ascending order and the threshold rises with them, so the bands
 * are contiguous runs and one pass finds them.
 */
std::vector<kill_band> kill_bands(const std::vector<start_hp>& start, int damage, std::size_t strikes_total)
{
	std::vector<kill_band> out;

	for(std::size_t i = 0; i < start.size(); ++i) {
		const std::size_t threshold = hits_to_kill(damage, start[i].hp, strikes_total);
		if(out.empty() || out.back().threshold != threshold) {
			out.push_back({threshold, i, i + 1});
		} else {
			out.back().end = i + 1;
		}
	}

	return out;
}

/** One side's share of a solve. */
struct side_params
{
	unsigned int strikes_per_round;
	unsigned int chance_to_hit;
	unsigned int credit;
};

/**
 * The joint distribution of landed hit counts, mass[ai * b_states + bi], where side A has landed
 * a_base + ai hits and side B b_base + bi. The top count on each side is absorbing: it is the
 * killing hit, past which nothing is tracked.
 */
struct hit_counts
{
	std::size_t a_base = 0;
	std::size_t b_base = 0;
	std::size_t a_states = 0;
	std::size_t b_states = 0;
	std::vector<double> mass;
};

/**
 * Runs one fight, in hit counts rather than hit points. `a_kill` is what A needs to land to kill B
 * and `b_kill` what B needs to kill A; KILL_NEVER means it cannot be done.
 */
hit_counts solve_hit_counts(
		const side_params& a, const side_params& b, unsigned int rounds, std::size_t a_kill, std::size_t b_kill)
{
	const std::size_t a_total = static_cast<std::size_t>(a.strikes_per_round) * rounds;
	const std::size_t b_total = static_cast<std::size_t>(b.strikes_per_round) * rounds;

	hit_counts out;

	// Nothing can cut either side's swings short, so neither hit count is in any doubt: the credit
	// alone fixes it. Skipping the walk matters most for berserk, where it is the difference
	// between one state and a couple of hundred.
	if(a_kill == KILL_NEVER && b_kill == KILL_NEVER) {
		const auto owed = [rounds](const side_params& p, std::size_t total) {
			return std::min<std::size_t>(hits_owed(p.strikes_per_round, p.chance_to_hit, p.credit, rounds - 1), total);
		};

		out.a_base = owed(a, a_total);
		out.b_base = owed(b, b_total);
		out.a_states = 1;
		out.b_states = 1;
		out.mass.assign(1, 1.0);
		return out;
	}

	out.a_states = (a_kill == KILL_NEVER ? a_total : std::min(a_kill, a_total)) + 1;
	out.b_states = (b_kill == KILL_NEVER ? b_total : std::min(b_kill, b_total)) + 1;
	out.mass.assign(out.a_states * out.b_states, 0.0);
	out.mass[0] = 1.0;

	std::vector<double> next;

	const auto over = [&](std::size_t a_hits, std::size_t b_hits) {
		return (a_kill != KILL_NEVER && a_hits >= a_kill) || (b_kill != KILL_NEVER && b_hits >= b_kill);
	};

	// One strike; `mine` selects which side swings and which hit count advances.
	const auto strike = [&](bool mine, const side_params& p, unsigned int round, unsigned int strike_index) {
		next.assign(out.mass.size(), 0.0);

		for(std::size_t a_hits = 0; a_hits < out.a_states; ++a_hits) {
			for(std::size_t b_hits = 0; b_hits < out.b_states; ++b_hits) {
				const std::size_t here = a_hits * out.b_states + b_hits;
				const double mass = out.mass[here];
				if(mass <= 0.0) {
					continue;
				}

				const std::size_t landed = mine ? a_hits : b_hits;
				// A dead unit does not swing, and the killing hit is the last one tracked.
				const double hit_prob = over(a_hits, b_hits) || landed + 1 >= (mine ? out.a_states : out.b_states)
					? 0.0
					: chance_this_strike_hits(p.strikes_per_round, p.chance_to_hit, p.credit, round, strike_index,
						  static_cast<unsigned int>(landed));

				if(hit_prob <= 0.0) {
					next[here] += mass;
					continue;
				}

				next[here] += mass * (1.0 - hit_prob);
				next[mine ? here + out.b_states : here + 1] += mass * hit_prob;
			}
		}

		out.mass.swap(next);
	};

	for(unsigned int round = 0; round < rounds; ++round) {
		for(unsigned int i = 0; i < std::max(a.strikes_per_round, b.strikes_per_round); ++i) {
			if(i < a.strikes_per_round) {
				strike(true, a, round, i);
			}

			if(i < b.strikes_per_round) {
				strike(false, b, round, i);
			}
		}
	}

	return out;
}

/**
 * Starting hit points and their probabilities, ascending. An empty summary means the unit has not
 * fought yet and sits on its current hit points for certain.
 */
void collect(const std::vector<double>& in, unsigned int hp, unsigned int max_hp, std::vector<start_hp>& out)
{
	if(in.empty()) {
		out.push_back({hp, 1.0});
		return;
	}

	for(std::size_t i = 0; i < in.size() && i <= max_hp; ++i) {
		if(in[i] > 0.0) {
			out.push_back({i, in[i]});
		}
	}
}

/**
 * The credit values that make a difference, one region per distinct rounding point of the running
 * hits-owed total. Every credit inside a region yields the same hits in every round, so one of them
 * stands for all of them.
 *
 * Example: 55% over 5 strikes earns 2.75 hits, so rolls 0-24 land 2 and rolls 25-99 land 3, giving
 * {{0, 0.25}, {25, 0.75}}.
 */
std::vector<credit_region> credit_regions(
		unsigned int strikes_per_round, unsigned int chance_to_hit, unsigned int rounds)
{
	std::vector<int> cuts;
	for(unsigned int round = 0; round < rounds; ++round) {
		if(const int over = static_cast<int>(chance_to_hit * strikes_per_round * (round + 1)) % 100; over != 0) {
			cuts.push_back(100 - over);
		}
	}

	std::sort(cuts.begin(), cuts.end());
	cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());

	std::vector<credit_region> out;
	int begin = 0;
	for(const int cut : cuts) {
		out.push_back({static_cast<unsigned int>(begin), (cut - begin) / 100.0});
		begin = cut;
	}

	out.push_back({static_cast<unsigned int>(begin), (100 - begin) / 100.0});
	return out;
}

/**
 * How many hits land in each round, given the credit the fight opened with. This is not a
 * distribution: the credit fixes the count outright, leaving only *which* swings land.
 */
std::vector<unsigned int> round_hit_counts(
		unsigned int strikes_per_round, unsigned int chance_to_hit, unsigned int credit, unsigned int rounds)
{
	std::vector<unsigned int> out;
	out.reserve(rounds);

	int landed = 0;
	for(unsigned int round = 0; round < rounds; ++round) {
		const int due = hits_owed(strikes_per_round, chance_to_hit, credit, round) - landed;
		const int hits = std::clamp(due, 0, static_cast<int>(strikes_per_round));
		out.push_back(static_cast<unsigned int>(hits));
		landed += hits;
	}

	return out;
}

/** The size of one side's pattern space across the whole fight. */
std::uint64_t pattern_count(unsigned int strikes, const std::vector<unsigned int>& hits_per_round)
{
	std::uint64_t out = 1;
	for(const unsigned int hits : hits_per_round) {
		out = saturating_mul(out, choose(strikes, hits));
	}

	return out;
}

/**
 * Every way `hits` of `strikes` swings can land, one bit per swing. The Reduced RNG draws without
 * replacement - hits still due over strikes left - so each of these is equally likely, and together
 * they are the whole of the randomness a round has left once the credit is known.
 */
std::vector<std::uint32_t> strike_patterns(unsigned int strikes, unsigned int hits)
{
	std::vector<std::uint32_t> out;

	if(hits > strikes) {
		return out;
	}

	if(hits == 0) {
		out.push_back(0);
		return out;
	}

	out.reserve(static_cast<std::size_t>(choose(strikes, hits)));

	const std::uint32_t limit = 1u << strikes;
	for(std::uint32_t mask = (1u << hits) - 1; mask < limit;) {
		out.push_back(mask);

		// Gosper's hack: the next value up carrying the same number of set bits.
		const std::uint32_t lowest = mask & (~mask + 1u);
		const std::uint32_t carried = mask + lowest;
		mask = carried | (((mask ^ carried) >> 2) / lowest);
	}

	return out;
}

} // end anon namespace

bool enabled()
{
	return resources::classification != nullptr
		&& resources::classification->random_mode == "biased"
		&& randomness::generator != nullptr
		&& !randomness::generator->is_networked();
}

int roll_credit(randomness::rng& rng)
{
	return rng.get_random_int(0, 99);
}

bool roll_hit(randomness::rng& rng, unsigned int strikes_left, unsigned int chance_to_hit, int& credit)
{
	if(chance_to_hit == 0 || chance_to_hit == 100) {
		// perform_hit never/always hits here, and leaves the credit untouched.
		return chance_to_hit == 100;
	}

	const int cth = static_cast<int>(chance_to_hit);
	const int remaining = static_cast<int>(strikes_left);
	const int expected_hits = (cth * remaining + credit) / 100;
	const bool does_hit = rng.get_random_int(0, remaining - 1) < expected_hits;
	credit += cth - 100 * static_cast<int>(does_hit);
	return does_hit;
}

bool can_solve(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		const summary_t& summary,
		const summary_t& opp_summary)
{
	if(!enabled()) {
		return false;
	}

	// Hit points have to follow from hit counts, so anything that moves the damage of a strike or
	// adds hit points mid-fight is out. Swarm is not: by the time solve_fight() runs, the caller
	// has already sliced the fight into runs of a fixed strike count.
	if(stats.slows || opp_stats.slows || stats.drains || opp_stats.drains || stats.petrifies
		|| opp_stats.petrifies) {
		return false;
	}

	if(stats.drain_percent != 0 || opp_stats.drain_percent != 0 || stats.drain_constant != 0
		|| opp_stats.drain_constant != 0) {
		return false;
	}

	if(!summary[1].empty() || !opp_summary[1].empty()) {
		return false;
	}

	// What solve_fight() below is about to cost, counted the same way it spends: a walk of the hit
	// count grid per strike for every (credit, credit, band, band), then one pass over that grid for
	// each pair of starting hit points the two bands hold.
	const unsigned int rounds = std::max({1u, stats.rounds, opp_stats.rounds});
	const std::size_t a_strikes = static_cast<std::size_t>(stats.num_blows) * rounds;
	const std::size_t b_strikes = static_cast<std::size_t>(opp_stats.num_blows) * rounds;

	std::vector<start_hp> a_start, b_start;
	collect(summary[0], stats.hp, stats.max_hp, a_start);
	collect(opp_summary[0], opp_stats.hp, opp_stats.max_hp, b_start);

	const std::vector<kill_band> a_bands = kill_bands(a_start, opp_stats.damage, b_strikes);
	const std::vector<kill_band> b_bands = kill_bands(b_start, stats.damage, a_strikes);

	// The credit splits the roll into one region per distinct rounding point, and those repeat as
	// the running total wraps - so there are only ever a handful, however long a berserk fight runs.
	const std::uint64_t credits = static_cast<std::uint64_t>(credit_regions(stats.num_blows, stats.chance_to_hit, rounds).size())
		* credit_regions(opp_stats.num_blows, opp_stats.chance_to_hit, rounds).size();

	// Swarm buys a whole solve per pair of slices, since the caller runs one fight per strike count.
	const auto slices = [](const battle_context_unit_stats& s) -> std::uint64_t {
		return s.swarm ? std::max<std::uint64_t>(1, s.swarm_max) : 1;
	};

	// Mirrors solve_hit_counts(): the grid stops at the killing hit, so a fight that ends in a few
	// hits stays cheap however many swings it takes to get there - which is what makes berserk
	// affordable. Both sides unkillable is the shortcut that skips the walk entirely.
	const auto states = [](std::size_t kill, std::size_t total) -> std::uint64_t {
		return (kill == KILL_NEVER ? total : std::min(kill, total)) + 1;
	};

	std::uint64_t steps = 0;
	for(const kill_band& a_band : a_bands) {
		for(const kill_band& b_band : b_bands) {
			const std::uint64_t pairs = (a_band.end - a_band.begin) * (b_band.end - b_band.begin);

			if(a_band.threshold == KILL_NEVER && b_band.threshold == KILL_NEVER) {
				steps = saturating_add(steps, pairs);
				continue;
			}

			// b_band holds what A must land to kill, a_band what B must land.
			const std::uint64_t grid
				= saturating_mul(states(b_band.threshold, a_strikes), states(a_band.threshold, b_strikes));

			steps = saturating_add(steps, saturating_mul(grid, std::max(a_strikes, b_strikes) + pairs));
		}
	}

	steps = saturating_mul(steps, saturating_mul(credits, slices(stats) * slices(opp_stats)));

	return steps <= MAX_SOLVE_STEPS;
}

void solve_fight(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		unsigned int strikes,
		unsigned int opp_strikes,
		std::vector<double>& hp_dist,
		std::vector<double>& opp_hp_dist,
		double& self_not_hit,
		double& opp_not_hit,
		bool levelup_considered)
{
	// An advancing unit full heals - on survival if combat XP alone levels it, else on the kill.
	const bool a_advances = levelup_considered && stats.can_advance;
	const bool b_advances = levelup_considered && opp_stats.can_advance;
	const bool a_levels_on_survival
		= a_advances && stats.experience + game_config::combat_xp(opp_stats.level) >= stats.max_experience;
	const bool b_levels_on_survival
		= b_advances && opp_stats.experience + game_config::combat_xp(stats.level) >= opp_stats.max_experience;
	const bool a_levels_on_kill = a_advances && !a_levels_on_survival
		&& stats.experience + game_config::kill_xp(opp_stats.level) >= stats.max_experience;
	const bool b_levels_on_kill = b_advances && !b_levels_on_survival
		&& opp_stats.experience + game_config::kill_xp(stats.level) >= opp_stats.max_experience;

	const unsigned int rounds = std::max({1u, stats.rounds, opp_stats.rounds});
	const std::size_t a_strikes_total = static_cast<std::size_t>(strikes) * rounds;
	const std::size_t b_strikes_total = static_cast<std::size_t>(opp_strikes) * rounds;

	std::vector<start_hp> a_start, b_start;
	collect(hp_dist, stats.hp, stats.max_hp, a_start);
	collect(opp_hp_dist, opp_stats.hp, opp_stats.max_hp, b_start);

	// Band each side's starting hit points by the kill threshold they present to the other. A band
	// is one solve however many hit point values sit in it, which is what keeps a chain of attacks
	// against a single target - the shape the AI asks for most - from paying per value.
	const std::vector<kill_band> a_bands = kill_bands(a_start, opp_stats.damage, b_strikes_total);
	const std::vector<kill_band> b_bands = kill_bands(b_start, stats.damage, a_strikes_total);

	const std::vector<credit_region> a_credits = credit_regions(strikes, stats.chance_to_hit, rounds);
	const std::vector<credit_region> b_credits = credit_regions(opp_strikes, opp_stats.chance_to_hit, rounds);

	std::vector<double> a_out(stats.max_hp + 1, 0.0);
	std::vector<double> b_out(opp_stats.max_hp + 1, 0.0);
	double a_never_hit = 0.0;
	double b_never_hit = 0.0;

	for(const credit_region& a_credit : a_credits) {
		for(const credit_region& b_credit : b_credits) {
			const double credit_mass = a_credit.probability * b_credit.probability;

			for(const kill_band& a_band : a_bands) {
				for(const kill_band& b_band : b_bands) {
					// b_band holds what A must land to kill, a_band what B must land.
					const hit_counts hits = solve_hit_counts(
							{strikes, stats.chance_to_hit, a_credit.credit},
							{opp_strikes, opp_stats.chance_to_hit, b_credit.credit},
							rounds, b_band.threshold, a_band.threshold);

					for(std::size_t a_index = a_band.begin; a_index < a_band.end; ++a_index) {
						for(std::size_t b_index = b_band.begin; b_index < b_band.end; ++b_index) {
							const std::size_t a_hp = a_start[a_index].hp;
							const std::size_t b_hp = b_start[b_index].hp;
							const double start_mass
								= credit_mass * a_start[a_index].prob * b_start[b_index].prob;

							for(std::size_t ai = 0; ai < hits.a_states; ++ai) {
								for(std::size_t bi = 0; bi < hits.b_states; ++bi) {
									const double mass = hits.mass[ai * hits.b_states + bi] * start_mass;
									if(mass <= 0.0) {
										continue;
									}

									const std::size_t a_hits = hits.a_base + ai;
									const std::size_t b_hits = hits.b_base + bi;

									// Damage per strike is fixed here, so hit points follow from hits taken.
									// Resolve both ends before advancing, so "opponent died" survives the heal.
									const std::size_t a_end = a_hp
										- std::min(a_hp, b_hits * static_cast<std::size_t>(opp_stats.damage));
									const std::size_t b_end = b_hp
										- std::min(b_hp, a_hits * static_cast<std::size_t>(stats.damage));

									std::size_t a_res = a_end;
									std::size_t b_res = b_end;
									if(a_end > 0 && (a_levels_on_survival || (a_levels_on_kill && b_end == 0))) {
										a_res = stats.max_hp;
									}

									if(b_end > 0 && (b_levels_on_survival || (b_levels_on_kill && a_end == 0))) {
										b_res = opp_stats.max_hp;
									}

									a_out[a_res] += mass;
									b_out[b_res] += mass;
									// A zero hit count is exactly the "never hit" case the callers report.
									if(b_hits == 0) {
										a_never_hit += mass;
									}

									if(a_hits == 0) {
										b_never_hit += mass;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	hp_dist.swap(a_out);
	opp_hp_dist.swap(b_out);
	self_not_hit *= std::min(a_never_hit, 1.0);
	opp_not_hit *= std::min(b_never_hit, 1.0);
}

/*
 * The fallback for the fights can_solve() turns away. Once the credit is known the number of hits
 * each round is known too, so all that is left to chance is which swings land - a choice of k out of
 * n, every choice as likely as every other. That is a short list, and each entry on it decides the
 * fight outright, so walking it is exact as well. Nothing inside one course is random, which is why
 * slow, drain and berserk need no handling of their own here: the replay just plays them out.
 */

namespace
{
/**
 * A's hit points against B's, over the four planes the two slowed states make - the same shape as
 * the matrices in attack_prediction.cpp. This one only ever holds finished courses, so it needs
 * none of their machinery for stepping a fight forwards: somewhere to add a course up, and the
 * corrections that apply once the fight is over.
 */
class replay_matrix
{
public:
	replay_matrix(unsigned int a_max_hp, unsigned int b_max_hp, bool need_a_slowed, bool need_b_slowed)
		: rows_(a_max_hp + 1)
		, cols_(b_max_hp + 1)
	{
		allocate(NEITHER_SLOWED);

		if(need_a_slowed) {
			allocate(A_SLOWED);
		}

		if(need_b_slowed) {
			allocate(B_SLOWED);
		}

		if(need_a_slowed && need_b_slowed) {
			allocate(BOTH_SLOWED);
		}
	}

	/** Record one course the fight can take, carrying the weight it is taken with. */
	void record(unsigned int a_hp, unsigned int b_hp, bool a_slowed, bool b_slowed, double weight)
	{
		assert(a_hp < rows_ && b_hp < cols_);
		at((a_slowed ? 1u : 0u) + (b_slowed ? 2u : 0u), a_hp, b_hp) += weight;
	}

	// We lied: actually did less damage, adjust matrix.
	void remove_petrify_distortion_a(unsigned int damage, unsigned int slow_damage, unsigned int b_hp)
	{
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			// A is slow in planes 1 and 3.
			const unsigned int actual_damage = (p & 1) ? slow_damage : damage;
			if(used(p) && b_hp > actual_damage) {
				// B was actually petrified, not killed.
				move_column(p, b_hp - actual_damage, 0);
			}
		}
	}

	void remove_petrify_distortion_b(unsigned int damage, unsigned int slow_damage, unsigned int a_hp)
	{
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			// B is slow in planes 2 and 3.
			const unsigned int actual_damage = (p & 2) ? slow_damage : damage;
			if(used(p) && a_hp > actual_damage) {
				// A was actually petrified, not killed.
				move_row(p, a_hp - actual_damage, 0);
			}
		}
	}

	void forced_levelup_a()
	{
		/* Move all the values (except 0hp) of all the planes to the "fully healed"
		row of the planes unslowed for A. */
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			if(used(p)) {
				merge_cols(p & ~1u, p, rows_ - 1);
			}
		}
	}

	void forced_levelup_b()
	{
		/* Move all the values (except 0hp) of all the planes to the "fully healed"
		column of planes unslowed for B. */
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			if(used(p)) {
				merge_rows(p & ~2u, p, cols_ - 1);
			}
		}
	}

	void conditional_levelup_a()
	{
		/* Move the values of the first column (except 0hp) of all the
		planes to the "fully healed" row of the planes unslowed for A. */
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			if(used(p)) {
				merge_col(p & ~1u, p, 0, rows_ - 1);
			}
		}
	}

	void conditional_levelup_b()
	{
		/* Move the values of the first row (except 0hp) of all the
		planes to the last column of the planes unslowed for B. */
		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			if(used(p)) {
				merge_row(p & ~2u, p, 0, cols_ - 1);
			}
		}
	}

	/**
	 * Its over, and here's the bill. The courses were weighted as they were recorded, so unlike the
	 * sampler's version there is nothing left to divide by.
	 */
	void extract_results(summary_t& summary_a, summary_t& summary_b) const
	{
		// Reset the summaries.
		summary_a[0] = std::vector<double>(rows_);
		summary_b[0] = std::vector<double>(cols_);

		if(used(A_SLOWED)) {
			summary_a[1] = std::vector<double>(rows_);
		}

		if(used(B_SLOWED)) {
			summary_b[1] = std::vector<double>(cols_);
		}

		for(unsigned int p = 0; p < NUM_PLANES; ++p) {
			if(!used(p)) {
				continue;
			}

			// A is slow in planes 1 and 3.
			const unsigned int dst_a = (p & 1) ? 1u : 0u;
			// B is slow in planes 2 and 3.
			const unsigned int dst_b = (p & 2) ? 1u : 0u;

			for(unsigned int row = 0; row < rows_; ++row) {
				for(unsigned int col = 0; col < cols_; ++col) {
					const double prob = plane_[p][static_cast<std::size_t>(row) * cols_ + col];
					summary_a[dst_a][row] += prob;
					summary_b[dst_b][col] += prob;
				}
			}
		}
	}

private:
	// We need four matrices, or "planes", reflecting the possible
	// "slowed" states (neither slowed, A slowed, B slowed, both slowed).
	enum { NEITHER_SLOWED, A_SLOWED, B_SLOWED, BOTH_SLOWED, NUM_PLANES };

	bool used(unsigned int plane) const
	{
		return !plane_[plane].empty();
	}

	void allocate(unsigned int plane)
	{
		plane_[plane].assign(static_cast<std::size_t>(rows_) * cols_, 0.0);
	}

	double& at(unsigned int plane, unsigned int row, unsigned int col)
	{
		if(!used(plane)) {
			allocate(plane);
		}

		return plane_[plane][static_cast<std::size_t>(row) * cols_ + col];
	}

	/** Adds one value to another and empties the source, which is how every move below shifts mass. */
	void xfer(unsigned int d_plane,
			unsigned int s_plane,
			unsigned int d_row,
			unsigned int d_col,
			unsigned int s_row,
			unsigned int s_col)
	{
		if(!used(s_plane) || (d_plane == s_plane && d_row == s_row && d_col == s_col)) {
			return;
		}

		double& src = at(s_plane, s_row, s_col);
		if(src != 0.0) {
			at(d_plane, d_row, d_col) += src;
			src = 0.0;
		}
	}

	/** Move a column (adding it to the destination). */
	void move_column(unsigned int plane, unsigned int d_col, unsigned int s_col)
	{
		for(unsigned int row = 0; row < rows_; ++row) {
			xfer(plane, plane, row, d_col, row, s_col);
		}
	}

	/** Move a row (adding it to the destination). */
	void move_row(unsigned int plane, unsigned int d_row, unsigned int s_row)
	{
		for(unsigned int col = 0; col < cols_; ++col) {
			xfer(plane, plane, d_row, col, s_row, col);
		}
	}

	/** Move values in one column -- excluding row zero -- to @a d_row of that column. */
	void merge_col(unsigned int d_plane, unsigned int s_plane, unsigned int col, unsigned int d_row)
	{
		for(unsigned int row = 1; row < rows_; ++row) {
			xfer(d_plane, s_plane, d_row, col, row, col);
		}
	}

	void merge_cols(unsigned int d_plane, unsigned int s_plane, unsigned int d_row)
	{
		for(unsigned int col = 0; col < cols_; ++col) {
			merge_col(d_plane, s_plane, col, d_row);
		}
	}

	/** Move values in one row -- excluding column zero -- to @a d_col of that row. */
	void merge_row(unsigned int d_plane, unsigned int s_plane, unsigned int row, unsigned int d_col)
	{
		for(unsigned int col = 1; col < cols_; ++col) {
			xfer(d_plane, s_plane, row, d_col, row, col);
		}
	}

	void merge_rows(unsigned int d_plane, unsigned int s_plane, unsigned int d_col)
	{
		for(unsigned int row = 0; row < rows_; ++row) {
			merge_row(d_plane, s_plane, row, d_col);
		}
	}

	const unsigned int rows_, cols_;
	std::array<std::vector<double>, NUM_PLANES> plane_;
};

/**
 * Turns one plane of an incoming summary into a distribution over starting hit points, normalised
 * so that it stands on its own once the chance of arriving in this plane is accounted for
 * separately. An empty source means the unit has not fought yet and sits on `singular_hp`.
 */
void scale_initial_probabilities(
		const std::vector<double>& source, std::vector<double>& target, double divisor, unsigned int singular_hp)
{
	if(divisor == 0.0) {
		// The target distribution isn't used, so there is nothing to scale.
		return;
	}

	if(source.empty()) {
		target.resize(singular_hp + 1u, 0.0);
		target[singular_hp] = 1.0;
	} else {
		std::transform(
				source.begin(), source.end(), std::back_inserter(target), [=](double prob) { return prob / divisor; });
	}

	assert(std::abs(std::accumulate(target.begin(), target.end(), 0.0) - 1.0) < 0.001);
}

/** One group of a side's openings sharing a strike count, and how many courses that count allows. */
struct opening_group
{
	unsigned int strikes;
	std::uint64_t count;
	/** Courses this side's swings can take, one entry per credit region. */
	std::vector<std::uint64_t> courses;
};

/**
 * Groups one side's openings - one per hit point value it can start on, per slow state it can start
 * in - by the number of strikes each gets, minus the hit point values that are already zero: nobody
 * swings at a corpse, so those openings cost nothing. The course count follows from the strike count
 * alone, so grouping keeps it worked out once per distinct count rather than once per opening.
 */
std::vector<opening_group> group_openings(const summary_t& summary,
		const battle_context_unit_stats& stats,
		double slowed_chance,
		unsigned int rounds)
{
	std::map<unsigned int, std::uint64_t> counts;

	for(unsigned int plane = 0; plane < 2; ++plane) {
		if((plane ? slowed_chance : 1.0 - slowed_chance) <= 0.0) {
			continue;
		}

		if(summary[plane].empty()) {
			// The unit has not fought yet, so it opens on its current hit points for certain.
			if(stats.hp > 0) {
				++counts[stats.calc_blows(stats.hp)];
			}
		} else {
			for(unsigned int hp = 1; hp < summary[plane].size(); ++hp) {
				if(summary[plane][hp] > 0.0) {
					++counts[stats.calc_blows(hp)];
				}
			}
		}
	}

	std::vector<opening_group> out;

	for(const auto& [strikes, count] : counts) {
		opening_group group{strikes, count, {}};

		for(const auto& credit : credit_regions(strikes, stats.chance_to_hit, rounds)) {
			group.courses.push_back(
					pattern_count(strikes, round_hit_counts(strikes, stats.chance_to_hit, credit.credit, rounds)));
		}

		out.push_back(std::move(group));
	}

	return out;
}

/**
 * Lists every course the fight can take and weighs each into the matrix.
 *
 * Petrify is the one thing not played out: as with the sampler, replay_fight() runs the fight with
 * damage that kills and moves the mass back afterwards.
 */
class course_listing
{
public:
	course_listing(const battle_context_unit_stats& stats,
			const battle_context_unit_stats& opp_stats,
			const summary_t& summary,
			const summary_t& opp_summary,
			unsigned int a_damage,
			unsigned int b_damage,
			unsigned int a_slow_damage,
			unsigned int b_slow_damage,
			unsigned int rounds,
			double a_initially_slowed_chance,
			double b_initially_slowed_chance);

	void enumerate();

	replay_matrix& matrix()
	{
		return matrix_;
	}

	double a_hit_probability() const
	{
		return weight_a_hit_;
	}

	double b_hit_probability() const
	{
		return weight_b_hit_;
	}

private:
	replay_matrix matrix_;
	const battle_context_unit_stats& a_stats_;
	const battle_context_unit_stats& b_stats_;
	unsigned int a_damage_, b_damage_;
	unsigned int a_slow_damage_, b_slow_damage_;
	unsigned int rounds_;
	std::vector<double> a_initial_, b_initial_;
	std::vector<double> a_initial_slowed_, b_initial_slowed_;
	double a_initially_slowed_chance_, b_initially_slowed_chance_;
	double weight_a_hit_ = 0.0;
	double weight_b_hit_ = 0.0;

	/**
	 * Every starting position the fight can open from - both slow states, both sides' hit points -
	 * with the chance of opening there. can_replay() counts the same set to price this walk.
	 */
	template<typename F>
	void for_each_opening(F&& f) const
	{
		for(unsigned int a_case = 0; a_case < 2; ++a_case) {
			const double a_case_weight = a_case ? a_initially_slowed_chance_ : 1.0 - a_initially_slowed_chance_;
			if(a_case_weight <= 0.0) {
				continue;
			}

			const std::vector<double>& a_initial = a_case ? a_initial_slowed_ : a_initial_;

			for(unsigned int b_case = 0; b_case < 2; ++b_case) {
				const double b_case_weight = b_case ? b_initially_slowed_chance_ : 1.0 - b_initially_slowed_chance_;
				if(b_case_weight <= 0.0) {
					continue;
				}

				const std::vector<double>& b_initial = b_case ? b_initial_slowed_ : b_initial_;

				for(unsigned int a_hp = 0; a_hp < a_initial.size(); ++a_hp) {
					if(a_initial[a_hp] <= 0.0) {
						continue;
					}

					for(unsigned int b_hp = 0; b_hp < b_initial.size(); ++b_hp) {
						if(b_initial[b_hp] <= 0.0) {
							continue;
						}

						f(a_case != 0, b_case != 0, a_hp, b_hp, a_stats_.calc_blows(a_hp), b_stats_.calc_blows(b_hp),
								a_case_weight * b_case_weight * a_initial[a_hp] * b_initial[b_hp]);
					}
				}
			}
		}
	}

	void replay(unsigned int a_hp,
			unsigned int b_hp,
			bool a_slowed,
			bool b_slowed,
			unsigned int a_strikes,
			unsigned int b_strikes,
			const std::vector<std::uint32_t>& a_pattern,
			const std::vector<std::uint32_t>& b_pattern,
			double weight);
};

course_listing::course_listing(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		const summary_t& summary,
		const summary_t& opp_summary,
		unsigned int a_damage,
		unsigned int b_damage,
		unsigned int a_slow_damage,
		unsigned int b_slow_damage,
		unsigned int rounds,
		double a_initially_slowed_chance,
		double b_initially_slowed_chance)
	// A ends up slowed if B slows, and vice versa; either can also arrive slowed.
	: matrix_(stats.max_hp,
			opp_stats.max_hp,
			opp_stats.slows || !summary[1].empty(),
			stats.slows || !opp_summary[1].empty())
	, a_stats_(stats)
	, b_stats_(opp_stats)
	, a_damage_(a_damage)
	, b_damage_(b_damage)
	, a_slow_damage_(a_slow_damage)
	, b_slow_damage_(b_slow_damage)
	, rounds_(rounds)
	, a_initially_slowed_chance_(a_initially_slowed_chance)
	, b_initially_slowed_chance_(b_initially_slowed_chance)
{
	scale_initial_probabilities(summary[0], a_initial_, 1.0 - a_initially_slowed_chance, stats.hp);
	scale_initial_probabilities(summary[1], a_initial_slowed_, a_initially_slowed_chance, stats.hp);
	scale_initial_probabilities(opp_summary[0], b_initial_, 1.0 - b_initially_slowed_chance, opp_stats.hp);
	scale_initial_probabilities(opp_summary[1], b_initial_slowed_, b_initially_slowed_chance, opp_stats.hp);
}

void course_listing::replay(unsigned int a_hp,
		unsigned int b_hp,
		bool a_slowed,
		bool b_slowed,
		unsigned int a_strikes,
		unsigned int b_strikes,
		const std::vector<std::uint32_t>& a_pattern,
		const std::vector<std::uint32_t>& b_pattern,
		double weight)
{
	bool a_was_hit = false;
	bool b_was_hit = false;

	for(unsigned int j = 0u; j < rounds_ && a_hp > 0u && b_hp > 0u; ++j) {
		for(unsigned int k = 0u; k < std::max(a_strikes, b_strikes); ++k) {
			if(k < a_strikes && ((a_pattern[j] >> k) & 1u)) {
				// A hits B
				unsigned int damage = a_slowed ? a_slow_damage_ : a_damage_;
				damage = std::min(damage, b_hp);
				b_was_hit = true;
				b_slowed |= a_stats_.slows;

				int drain_amount
						= (a_stats_.drain_percent * static_cast<signed>(damage) / 100 + a_stats_.drain_constant);
				a_hp = std::clamp(a_hp + drain_amount, 1u, a_stats_.max_hp);

				b_hp -= damage;

				if(b_hp == 0u) {
					// A killed B
					break;
				}
			}

			if(k < b_strikes && ((b_pattern[j] >> k) & 1u)) {
				// B hits A
				unsigned int damage = b_slowed ? b_slow_damage_ : b_damage_;
				damage = std::min(damage, a_hp);
				a_was_hit = true;
				a_slowed |= b_stats_.slows;

				int drain_amount
						= (b_stats_.drain_percent * static_cast<signed>(damage) / 100 + b_stats_.drain_constant);
				b_hp = std::clamp(b_hp + drain_amount, 1u, b_stats_.max_hp);

				a_hp -= damage;

				if(a_hp == 0u) {
					// B killed A
					break;
				}
			}
		}
	}

	if(a_was_hit) {
		weight_a_hit_ += weight;
	}

	if(b_was_hit) {
		weight_b_hit_ += weight;
	}

	matrix_.record(a_hp, b_hp, a_slowed, b_slowed, weight);
}

void course_listing::enumerate()
{
	// tables[strikes][hits], each list built the first time that pair turns up. A list for a pair in
	// range is never empty, so emptiness doubles as "not built yet".
	std::vector<std::vector<std::vector<std::uint32_t>>> tables(MAX_PATTERN_STRIKES + 1);

	const auto masks_for = [&tables](unsigned int strikes, unsigned int hits) -> const std::vector<std::uint32_t>& {
		// can_replay() turns away anything wider, which is what keeps this in range.
		assert(strikes <= MAX_PATTERN_STRIKES);

		std::vector<std::vector<std::uint32_t>>& by_hits = tables[strikes];
		if(by_hits.empty()) {
			by_hits.resize(strikes + 1);
		}

		if(by_hits[hits].empty()) {
			by_hits[hits] = strike_patterns(strikes, hits);
		}

		return by_hits[hits];
	};

	// Steps one side's per-round choice of mask on by one, odometer style.
	const auto advance = [](std::vector<std::size_t>& index,
							 const std::vector<const std::vector<std::uint32_t>*>& masks) {
		for(std::size_t r = index.size(); r-- > 0;) {
			if(++index[r] < masks[r]->size()) {
				return true;
			}

			index[r] = 0;
		}

		return false;
	};

	for_each_opening([&](bool a_slowed, bool b_slowed, unsigned int a_hp, unsigned int b_hp, unsigned int a_strikes,
							 unsigned int b_strikes, double opening_weight) {
		// Nobody swings at a corpse, so the opening is already the outcome.
		if(a_hp == 0 || b_hp == 0) {
			matrix_.record(a_hp, b_hp, a_slowed, b_slowed, opening_weight);
			return;
		}

		std::vector<std::size_t> a_index(rounds_, 0), b_index(rounds_, 0);
		std::vector<std::uint32_t> a_pattern(rounds_), b_pattern(rounds_);
		std::vector<const std::vector<std::uint32_t>*> a_masks(rounds_), b_masks(rounds_);

		for(const auto& a_credit : credit_regions(a_strikes, a_stats_.chance_to_hit, rounds_)) {
			const std::vector<unsigned int> a_hits
					= round_hit_counts(a_strikes, a_stats_.chance_to_hit, a_credit.credit, rounds_);

			for(unsigned int r = 0; r < rounds_; ++r) {
				a_masks[r] = &masks_for(a_strikes, a_hits[r]);
			}

			for(const auto& b_credit : credit_regions(b_strikes, b_stats_.chance_to_hit, rounds_)) {
				const std::vector<unsigned int> b_hits
						= round_hit_counts(b_strikes, b_stats_.chance_to_hit, b_credit.credit, rounds_);

				for(unsigned int r = 0; r < rounds_; ++r) {
					b_masks[r] = &masks_for(b_strikes, b_hits[r]);
				}

				// Every course under this pair of credits is as likely as every other.
				const double course_weight = opening_weight * a_credit.probability * b_credit.probability
					/ static_cast<double>(pattern_count(a_strikes, a_hits))
					/ static_cast<double>(pattern_count(b_strikes, b_hits));

				std::fill(a_index.begin(), a_index.end(), 0);
				do {
					for(unsigned int r = 0; r < rounds_; ++r) {
						a_pattern[r] = (*a_masks[r])[a_index[r]];
					}

					std::fill(b_index.begin(), b_index.end(), 0);
					do {
						for(unsigned int r = 0; r < rounds_; ++r) {
							b_pattern[r] = (*b_masks[r])[b_index[r]];
						}

						replay(a_hp, b_hp, a_slowed, b_slowed, a_strikes, b_strikes, a_pattern, b_pattern,
								course_weight);
					} while(advance(b_index, b_masks));
				} while(advance(a_index, a_masks));
			}
		}
	});
}

} // end anon namespace

bool can_replay(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		const summary_t& summary,
		const summary_t& opp_summary,
		double slowed_chance,
		double opp_slowed_chance)
{
	if(!enabled()) {
		return false;
	}

	// Whatever replay_fight() will run, so that this prices the fight it will actually list.
	const unsigned int rounds = std::max<unsigned int>(stats.rounds, opp_stats.rounds);

	const std::vector<opening_group> a = group_openings(summary, stats, slowed_chance, rounds);
	const std::vector<opening_group> b = group_openings(opp_summary, opp_stats, opp_slowed_chance, rounds);

	// The sampler plays out a fixed number of fights whatever the fight is, so all that varies is
	// how long one of them runs. This is monte_carlo_combat_matrix's iteration count; the two only
	// have to agree closely enough to price one path against the other.
	constexpr std::uint64_t sampling_iterations = 5000;

	unsigned int widest = 0;
	for(const opening_group& group : a) {
		widest = std::max(widest, group.strikes);
	}

	for(const opening_group& group : b) {
		widest = std::max(widest, group.strikes);
	}

	// A pattern carries one bit per swing, and a fight this wide is hopeless to list anyway.
	if(widest > MAX_PATTERN_STRIKES) {
		return false;
	}

	const std::uint64_t budget = sampling_iterations * rounds * widest;

	std::uint64_t total = 0;
	for(const opening_group& a_group : a) {
		for(const opening_group& b_group : b) {
			const std::uint64_t replays = saturating_mul(
					a_group.count * b_group.count, rounds * std::max(a_group.strikes, b_group.strikes));

			for(const std::uint64_t a_courses : a_group.courses) {
				for(const std::uint64_t b_courses : b_group.courses) {
					total = saturating_add(total, saturating_mul(saturating_mul(a_courses, b_courses), replays));

					if(total > budget) {
						return false;
					}
				}
			}
		}
	}

	return true;
}

void replay_fight(const battle_context_unit_stats& stats,
		const battle_context_unit_stats& opp_stats,
		summary_t& summary,
		summary_t& opp_summary,
		double& self_not_hit,
		double& opp_not_hit,
		bool levelup_considered,
		double slowed_chance,
		double opp_slowed_chance)
{
	const unsigned int rounds = std::max<unsigned int>(stats.rounds, opp_stats.rounds);

	unsigned int a_damage = stats.damage, a_slow_damage = stats.slow_damage;
	unsigned int b_damage = opp_stats.damage, b_slow_damage = opp_stats.slow_damage;

	// To simulate stoning, we set to amount which kills, and re-adjust after.
	if(stats.petrifies) {
		a_damage = a_slow_damage = opp_stats.max_hp;
	}

	if(opp_stats.petrifies) {
		b_damage = b_slow_damage = stats.max_hp;
	}

	course_listing listing(stats, opp_stats, summary, opp_summary, a_damage, b_damage, a_slow_damage, b_slow_damage,
			rounds, slowed_chance, opp_slowed_chance);

	listing.enumerate();

	self_not_hit = 1.0 - listing.a_hit_probability();
	opp_not_hit = 1.0 - listing.b_hit_probability();

	replay_matrix& matrix = listing.matrix();

	if(stats.petrifies) {
		matrix.remove_petrify_distortion_a(stats.damage, stats.slow_damage, opp_stats.hp);
	}

	if(opp_stats.petrifies) {
		matrix.remove_petrify_distortion_b(opp_stats.damage, opp_stats.slow_damage, stats.hp);
	}

	if(levelup_considered && stats.can_advance) { // we assume that the unit full-heals if it advances
		if(stats.experience + game_config::combat_xp(opp_stats.level) >= stats.max_experience) {
			matrix.forced_levelup_a();
		} else if(stats.experience + game_config::kill_xp(opp_stats.level) >= stats.max_experience) {
			matrix.conditional_levelup_a();
		}

		if(opp_stats.experience + game_config::combat_xp(stats.level) >= opp_stats.max_experience) {
			matrix.forced_levelup_b();
		} else if(opp_stats.experience + game_config::kill_xp(stats.level) >= opp_stats.max_experience) {
			matrix.conditional_levelup_b();
		}
	}

	matrix.extract_results(summary, opp_summary);
}

} // namespace biased_rng
