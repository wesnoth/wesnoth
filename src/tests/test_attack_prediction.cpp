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
 * Tests biased/reduced RNG mode, default RNG mode is not added yet.
 *
 * Holds the Reduced RNG attack prediction against the dice it is predicting. Every case below whose
 * name ends in _matches_simulation runs the same shape: the prediction fights the matchup out, then
 * the same fight is played strike by strike on the primitives perform_hit() spends, and the two are
 * compared on mean hit points, the chance of dying and the chance of coming through untouched.
 *
 * What separates the cases is the path through attack_prediction.cpp the matchup takes. A solver_
 * case is one can_solve() accepts, answered in closed form; a replay_ case is one it declines, left
 * to the course listing. Firststrike, poison and petrify are outside what the simulator covers, so
 * no case uses them.
 *
 *   biased_rng_is_active                            the fixture really is in Reduced RNG mode
 *   biased_solver_matches_simulation                plain damage, kills reachable; last is a knife edge
 *   biased_solver_no_kill_matches_simulation        neither side can kill, so the credit fixes the count
 *   biased_solver_levelup_matches_simulation        the advancement heal, off the kill and off surviving
 *   biased_solver_berserk_matches_simulation        one credit over many rounds, the asymmetry it rests on
 *   biased_solver_swarm_matches_simulation          blow count sliced up before the solver ever sees it
 *   biased_replay_slow_matches_simulation           slow moves a later strike's damage, so solving is out
 *   biased_replay_drain_matches_simulation          drain does the same, and can undo a killing blow
 *   biased_replay_levelup_matches_simulation        that heal through the matrix, on both of its rules
 *   biased_solver_berserk_swarm_matches_simulation  both cost multipliers at once, priced and still taken
 *   biased_replay_all_specials_matches_simulation   drain, slow, swarm, rounds and advancement together
 *   biased_solve_declines_past_the_budget           plain damage, but too many strikes to walk
 *
 * Every one of them also holds the rng call count still across the prediction, since both exact
 * paths answer without drawing and the sampler they can hand over to does not.
 */

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include "attack_prediction.hpp"
#include "attack_prediction_biased.hpp"

#include "actions/attack.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "mt_rng.hpp"
#include "random.hpp"
#include "random_deterministic.hpp"
#include "resources.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t TEST_SEED = 0x5eedf00d;
constexpr unsigned int ITERATIONS = 20000;
// Used to get higher accuracy if the first, cheaper test round fails.
constexpr unsigned int ITERATIONS_RECHECK = 200000;

// 2 percentage points on a probability, and 1/4th of a hit point on a mean rather
// than a share of the unit's maximum, since the sampling error follows the damage spread and not the
// health bar.
//
// One standard deviation at ITERATIONS draws is about 0.004 and 0.07 respectively, so the
// bands sit only a few of those out; that is close enough that a marginal miss is worth resampling at
// ITERATIONS_RECHECK, which shrinks both figures threefold. The seed is fixed as well, which means a
// failure is the prediction moving rather than the dice.
constexpr double HP_TOLERANCE = 0.25;
constexpr double PROB_TOLERANCE = 0.02;

/** Puts the prediction code in Reduced RNG mode over a fixed random stream. */
struct biased_fixture
{
	biased_fixture()
		: seed_(TEST_SEED)
		, scoped_rng_(seed_)
		, saved_classification_(resources::classification)
	{
		classification_.random_mode = "biased";
		resources::classification = &classification_;
	}

	~biased_fixture()
	{
		resources::classification = saved_classification_;
	}

	randomness::mt_rng seed_;
	randomness::set_random_determinstic scoped_rng_;
	game_classification classification_;
	game_classification* saved_classification_;
};

battle_context_unit_stats make_stats(int damage,
		int blows,
		int hp,
		int max_hp,
		int chance_to_hit,
		bool drains = false,
		bool slows = false,
		unsigned int rounds = 1,
		bool swarm = false)
{
	battle_context_unit_stats stats(
			damage, blows, hp, max_hp, chance_to_hit, drains, slows, false, false, false, swarm);

	// The stand-alone constructor leaves this one member unset, and solve_fight() reads it.
	stats.can_advance = false;

	// Passing its berserk flag would fix the fight at thirty rounds, too long a walk to sample.
	stats.rounds = rounds;

	return stats;
}

/** Leaves the unit needing @a max_experience to advance, on no experience of its own. */
battle_context_unit_stats advancing(battle_context_unit_stats stats, unsigned int max_experience)
{
	stats.can_advance = true;
	stats.max_experience = max_experience;

	return stats;
}

struct levelup_rule
{
	bool on_survival = false;
	bool on_kill = false;
};

levelup_rule levelup_for(const battle_context_unit_stats& s, const battle_context_unit_stats& opp)
{
	levelup_rule out;
	if(!s.can_advance) {
		return out;
	}

	out.on_survival = s.experience + game_config::combat_xp(opp.level) >= s.max_experience;
	out.on_kill = !out.on_survival && s.experience + game_config::kill_xp(opp.level) >= s.max_experience;

	return out;
}

struct outcome
{
	double a_mean_hp = 0.0;
	double b_mean_hp = 0.0;
	double a_dead = 0.0;
	double b_dead = 0.0;
	double a_untouched = 0.0;
	double b_untouched = 0.0;
};

/** What a unit carries into a fight, and out of it into the next one. */
struct fighter_state
{
	unsigned int hp = 0;
	bool slowed = false;
	bool was_hit = false;
};

/**
 * Plays one fight out strike by strike on the same primitives perform_hit spends, mirroring
 * monte_carlo_combat_matrix::simulate(). Firststrike, poison and petrify are left out; no case
 * below uses them. Swarm arrives as the blow count calc_blows() gives for the hit points brought
 * in, which is what split_summary() slices the prediction by.
 */
void play_fight(const battle_context_unit_stats& a,
		const battle_context_unit_stats& b,
		fighter_state& as,
		fighter_state& bs,
		randomness::rng& rng)
{
	const unsigned int rounds = std::max({1u, a.rounds, b.rounds});
	const unsigned int a_strikes = a.calc_blows(as.hp);
	const unsigned int b_strikes = b.calc_blows(bs.hp);

	const levelup_rule a_levels = levelup_for(a, b);
	const levelup_rule b_levels = levelup_for(b, a);

	int a_credit = biased_rng::roll_credit(rng);
	int b_credit = biased_rng::roll_credit(rng);

	for(unsigned int r = 0; r < rounds && as.hp > 0 && bs.hp > 0; ++r) {
		for(unsigned int k = 0; k < std::max(a_strikes, b_strikes); ++k) {
			if(k < a_strikes && biased_rng::roll_hit(rng, a_strikes - k, a.chance_to_hit, a_credit)) {
				const unsigned int damage
					= std::min(static_cast<unsigned int>(as.slowed ? a.slow_damage : a.damage), bs.hp);
				bs.was_hit = true;
				bs.slowed |= a.slows;

				const int drain = a.drain_percent * static_cast<int>(damage) / 100 + a.drain_constant;
				as.hp = static_cast<unsigned int>(std::clamp<int>(
						static_cast<int>(as.hp) + drain, 1, static_cast<int>(a.max_hp)));

				bs.hp -= damage;
				if(bs.hp == 0) {
					break;
				}
			}

			if(k < b_strikes && biased_rng::roll_hit(rng, b_strikes - k, b.chance_to_hit, b_credit)) {
				const unsigned int damage
					= std::min(static_cast<unsigned int>(bs.slowed ? b.slow_damage : b.damage), as.hp);
				as.was_hit = true;
				as.slowed |= b.slows;

				const int drain = b.drain_percent * static_cast<int>(damage) / 100 + b.drain_constant;
				bs.hp = static_cast<unsigned int>(std::clamp<int>(
						static_cast<int>(bs.hp) + drain, 1, static_cast<int>(b.max_hp)));

				as.hp -= damage;
				if(as.hp == 0) {
					break;
				}
			}
		}
	}

	// An advancing unit full heals. Both ends are read before either is healed, so "the opponent
	// died" is still true of the unit that just levelled off the kill.
	const unsigned int a_end = as.hp;
	const unsigned int b_end = bs.hp;

	if(a_end > 0 && (a_levels.on_survival || (a_levels.on_kill && b_end == 0))) {
		as.hp = a.max_hp;
	}

	if(b_end > 0 && (b_levels.on_survival || (b_levels.on_kill && a_end == 0))) {
		bs.hp = b.max_hp;
	}
}

void tally(outcome& out, const fighter_state& as, const fighter_state& bs)
{
	out.a_mean_hp += as.hp;
	out.b_mean_hp += bs.hp;
	out.a_dead += as.hp == 0 ? 1.0 : 0.0;
	out.b_dead += bs.hp == 0 ? 1.0 : 0.0;
	out.a_untouched += as.was_hit ? 0.0 : 1.0;
	out.b_untouched += bs.was_hit ? 0.0 : 1.0;
}

outcome simulate(const battle_context_unit_stats& a, const battle_context_unit_stats& b, unsigned int iterations)
{
	randomness::rng& rng = *randomness::generator;

	outcome out;

	for(unsigned int i = 0; i < iterations; ++i) {
		fighter_state as{a.hp, a.is_slowed, false};
		fighter_state bs{b.hp, b.is_slowed, false};

		play_fight(a, b, as, bs, rng);
		tally(out, as, bs);
	}

	const double n = iterations;
	out.a_mean_hp /= n;
	out.b_mean_hp /= n;
	out.a_dead /= n;
	out.b_dead /= n;
	out.a_untouched /= n;
	out.b_untouched /= n;

	return out;
}

/**
 * A attacks @a first, then carries what it has left into @a second. The statistics reported are A's
 * over the pair of fights and @a second's over its own, which is what the two combatants hold.
 */
outcome simulate_chain(const battle_context_unit_stats& a,
		const battle_context_unit_stats& first,
		const battle_context_unit_stats& second,
		unsigned int iterations)
{
	randomness::rng& rng = *randomness::generator;

	outcome out;

	for(unsigned int i = 0; i < iterations; ++i) {
		fighter_state as{a.hp, a.is_slowed, false};
		fighter_state firsts{first.hp, first.is_slowed, false};
		fighter_state seconds{second.hp, second.is_slowed, false};

		play_fight(a, first, as, firsts, rng);

		// A unit that died in the first fight does not swing in the second, and the prediction leaves
		// that mass sitting at zero hit points.
		if(as.hp > 0) {
			play_fight(a, second, as, seconds, rng);
		}

		tally(out, as, seconds);
	}

	const double n = iterations;
	out.a_mean_hp /= n;
	out.b_mean_hp /= n;
	out.a_dead /= n;
	out.b_dead /= n;
	out.a_untouched /= n;
	out.b_untouched /= n;

	return out;
}

struct deltas
{
	double a_hp;
	double b_hp;
	double a_dead;
	double b_dead;
	double a_untouched;
	double b_untouched;

	bool within() const
	{
		return std::abs(a_hp) < HP_TOLERANCE && std::abs(b_hp) < HP_TOLERANCE
			&& std::abs(a_dead) < PROB_TOLERANCE && std::abs(b_dead) < PROB_TOLERANCE
			&& std::abs(a_untouched) < PROB_TOLERANCE && std::abs(b_untouched) < PROB_TOLERANCE;
	}
};

deltas compare(const combatant& att, const combatant& def, const outcome& mc)
{
	return {att.average_hp() - mc.a_mean_hp, def.average_hp() - mc.b_mean_hp, att.hp_dist[0] - mc.a_dead,
			def.hp_dist[0] - mc.b_dead, att.untouched - mc.a_untouched, def.untouched - mc.b_untouched};
}

/** Holds the fought-out pair against @a sample, which takes the iteration count to spend. */
template<typename Sampler>
void settle(const combatant& att, const combatant& def, const Sampler& sample)
{
	BOOST_CHECK_CLOSE(std::accumulate(att.hp_dist.begin(), att.hp_dist.end(), 0.0), 1.0, 0.01);
	BOOST_CHECK_CLOSE(std::accumulate(def.hp_dist.begin(), def.hp_dist.end(), 0.0), 1.0, 0.01);

	deltas d = compare(att, def, sample(ITERATIONS));

	// A miss at the cheap count can still be the sampler rather than the prediction, so spend a sharper
	// sample before reporting one. The checks below run against whichever sample settled it.
	if(!d.within()) {
		d = compare(att, def, sample(ITERATIONS_RECHECK));
	}

	BOOST_CHECK_SMALL(d.a_hp, HP_TOLERANCE);
	BOOST_CHECK_SMALL(d.b_hp, HP_TOLERANCE);

	BOOST_CHECK_SMALL(d.a_dead, PROB_TOLERANCE);
	BOOST_CHECK_SMALL(d.b_dead, PROB_TOLERANCE);

	BOOST_CHECK_SMALL(d.a_untouched, PROB_TOLERANCE);
	BOOST_CHECK_SMALL(d.b_untouched, PROB_TOLERANCE);
}

/**
 * Both exact paths answer without drawing, while the sampler the replay hands over to draws once a
 * strike. Holding this count still is what separates "the listing agreed with the dice" from "two
 * samples of the same dice agreed with each other".
 */
unsigned int draws_so_far()
{
	return randomness::generator->get_random_calls();
}

void check_matchup(const battle_context_unit_stats& a, const battle_context_unit_stats& b)
{
	combatant att(a);
	combatant def(b);

	const unsigned int before = draws_so_far();
	att.fight(def);
	BOOST_CHECK_EQUAL(draws_so_far(), before);

	settle(att, def, [&](unsigned int iterations) { return simulate(a, b, iterations); });
}

/**
 * A fights @a first and carries what it has left into @a second, which is the only way a swarming
 * unit reaches its second fight on a distribution of blow counts rather than one of them.
 */
void check_chain(const battle_context_unit_stats& a,
		const battle_context_unit_stats& first,
		const battle_context_unit_stats& second)
{
	combatant att(a);
	combatant def_first(first);
	combatant def_second(second);

	const unsigned int before = draws_so_far();
	att.fight(def_first);

	// Nothing is under test unless the opening fight left A straddling a blow count boundary.
	unsigned int fewest = a.swarm_max;
	unsigned int most = 0;
	for(unsigned int hp = 1; hp < att.hp_dist.size(); ++hp) {
		if(att.hp_dist[hp] > 0.0) {
			fewest = std::min(fewest, a.calc_blows(hp));
			most = std::max(most, a.calc_blows(hp));
		}
	}

	BOOST_REQUIRE(fewest != most);

	// The second fight is the one under test, and it is the one whose summary can_solve() cannot be
	// asked about from out here; that it drew nothing is what says an exact path took it.
	att.fight(def_second);
	BOOST_CHECK_EQUAL(draws_so_far(), before);

	settle(att, def_second, [&](unsigned int iterations) { return simulate_chain(a, first, second, iterations); });
}

} // namespace

BOOST_AUTO_TEST_SUITE(attack_prediction)

BOOST_FIXTURE_TEST_CASE(biased_rng_is_active, biased_fixture)
{
	BOOST_CHECK(biased_rng::enabled());
}

/**
 * Damage per strike is fixed, so solve_fight() answers and the matrix never runs. A kill has to be
 * reachable on the hits the credit can actually buy - floor((chance_to_hit * strikes + 99) / 100) of
 * them, not every strike - so the thresholds below are set against that count rather than against
 * the raw damage. The last pairing puts both kill thresholds a hit apart, where which side falls
 * turns on where in the round the hits land.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_matches_simulation, biased_fixture)
{
	const std::vector<std::pair<battle_context_unit_stats, battle_context_unit_stats>> matchups{
		{make_stats(6, 3, 24, 24, 60), make_stats(5, 2, 12, 12, 50)},
		{make_stats(3, 5, 30, 30, 30), make_stats(8, 2, 6, 6, 80)},
		{make_stats(6, 3, 14, 14, 60), make_stats(7, 2, 12, 12, 70)},
	};

	for(const auto& [a, b] : matchups) {
		BOOST_REQUIRE(biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
		check_matchup(a, b);
	}
}

/**
 * Neither side can land enough damage to kill in the strikes it has. Nothing can cut the swings
 * short, so solve_hit_counts() takes the credit as the hit count outright and skips the walk.
 *
 * A side lands floor((chance_to_hit * strikes + credit) / 100) hits, so coming through untouched
 * needs chance_to_hit * strikes under 100 - which almost no matchup manages, since the whole point
 * of the Reduced RNG is that a hit is normally certain. The last two pairings are the ones that do,
 * and being no-kill fights they hold the exact figure: 0.20 against the single 80% strike, then 0.60
 * and 0.40 against the pair below it.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_no_kill_matches_simulation, biased_fixture)
{
	const std::vector<std::pair<battle_context_unit_stats, battle_context_unit_stats>> matchups{
		{make_stats(6, 3, 40, 40, 60), make_stats(5, 2, 36, 36, 50)},
		{make_stats(9, 2, 25, 25, 70), make_stats(4, 4, 30, 30, 40)},
		{make_stats(3, 5, 50, 50, 30), make_stats(8, 1, 28, 28, 80)},
		{make_stats(4, 2, 30, 30, 30), make_stats(3, 1, 26, 26, 40)},
	};

	for(const auto& [a, b] : matchups) {
		BOOST_REQUIRE(biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
		check_matchup(a, b);
	}
}

/**
 * Advancing full heals, so the survivor's hit points turn on how the fight went rather than on the
 * damage alone. solve_fight() carries that correlation cell by cell rather than scaling the
 * distribution. The first unit levels off the kill, the second off surviving the fight at all.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_levelup_matches_simulation, biased_fixture)
{
	const std::vector<std::pair<battle_context_unit_stats, battle_context_unit_stats>> matchups{
		{advancing(make_stats(6, 3, 14, 14, 60), 8), make_stats(7, 2, 12, 12, 70)},
		{advancing(make_stats(5, 3, 10, 30, 50), 1), make_stats(6, 2, 14, 14, 60)},
	};

	for(const auto& [a, b] : matchups) {
		BOOST_REQUIRE(biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
		check_matchup(a, b);
	}
}

/**
 * One credit is rolled per fight and carried, while the strike count starts over every round; that
 * asymmetry is what the closed form is built on, and only a multi-round fight shows it. Both sides
 * can reach the kill here, in round three or in round four depending on the credit.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_berserk_matches_simulation, biased_fixture)
{
	const battle_context_unit_stats a = make_stats(5, 2, 10, 10, 60, false, false, 4);
	const battle_context_unit_stats b = make_stats(4, 3, 18, 18, 30, false, false, 4);

	BOOST_REQUIRE(biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
	check_matchup(a, b);
}

/**
 * Swarm never reaches solve_fight() as swarm: split_summary() cuts the fight into runs of a fixed
 * blow count first, and the solver answers each run on its own. That only happens once the unit's
 * hit points are a distribution, so the fight under test is the second one.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_swarm_matches_simulation, biased_fixture)
{
	const battle_context_unit_stats a = make_stats(3, 4, 20, 20, 60, false, false, 1, true);
	const battle_context_unit_stats first = make_stats(4, 3, 30, 30, 60);
	const battle_context_unit_stats second = make_stats(5, 2, 6, 6, 50);

	BOOST_REQUIRE(biased_rng::can_solve(a, first, biased_rng::summary_t{}, biased_rng::summary_t{}));
	check_chain(a, first, second);
}

/** Slow moves the damage of a later strike, so this goes to the course listing instead. */
BOOST_FIXTURE_TEST_CASE(biased_replay_slow_matches_simulation, biased_fixture)
{
	const battle_context_unit_stats a = make_stats(5, 3, 40, 40, 60, false, true);
	const battle_context_unit_stats b = make_stats(6, 2, 36, 36, 50);

	BOOST_REQUIRE(!biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
	check_matchup(a, b);
}

/**
 * Drain moves the damage of a later strike the same way, and reaches the course listing for it. The
 * second pairing starts the drainer below the damage coming its way, so what it takes back over the
 * round decides whether it lives.
 */
BOOST_FIXTURE_TEST_CASE(biased_replay_drain_matches_simulation, biased_fixture)
{
	const std::vector<std::pair<battle_context_unit_stats, battle_context_unit_stats>> matchups{
		{make_stats(6, 3, 30, 40, 60, true, false), make_stats(5, 2, 36, 36, 50)},
		{make_stats(7, 3, 8, 40, 60, true, false), make_stats(6, 2, 14, 14, 70)},
	};

	for(const auto& [a, b] : matchups) {
		BOOST_REQUIRE(!biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
		check_matchup(a, b);
	}
}

/**
 * The advancement heal on the course listing, which reaches it through the shared matrix instead of
 * through solve_fight(), and so has to be checked on both of the rules that matrix keeps apart.
 *
 * The first pairing levels off the kill: both sides start a hit from it, and A's slow is what can
 * take B's second strike below the damage it needs, so the heal and the death it hangs on are
 * decided by the same strikes. The second levels off surviving the fight at all, which A cannot
 * reach by killing - it has nowhere near the damage - so the heal turns purely on whether the hit
 * points it drains back stay ahead of what B is landing.
 */
BOOST_FIXTURE_TEST_CASE(biased_replay_levelup_matches_simulation, biased_fixture)
{
	const std::vector<std::pair<battle_context_unit_stats, battle_context_unit_stats>> matchups{
		{advancing(make_stats(6, 3, 14, 14, 60, false, true), 8), make_stats(7, 2, 12, 12, 70)},
		{advancing(make_stats(5, 3, 10, 30, 50, true, false), 1), make_stats(9, 2, 30, 30, 60)},
	};

	for(const auto& [a, b] : matchups) {
		BOOST_REQUIRE(!biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
		check_matchup(a, b);
	}
}

/**
 * Swarm and berserk are the two multipliers on what a solve costs: one whole solve per pair of
 * slices, each over a strike count the rounds multiply. Both at once is what can_solve() has to
 * price, and it still takes the fight.
 */
BOOST_FIXTURE_TEST_CASE(biased_solver_berserk_swarm_matches_simulation, biased_fixture)
{
	// Three strikes a round at two in five leaves the second round's last hit on the credit, which is
	// what puts A on either side of a blow count boundary; an even hit count would fix its blows.
	const battle_context_unit_stats a = make_stats(3, 4, 20, 20, 60, false, false, 2, true);
	const battle_context_unit_stats first = make_stats(4, 3, 30, 30, 40);
	const battle_context_unit_stats second = make_stats(5, 2, 6, 6, 50);

	BOOST_REQUIRE(biased_rng::can_solve(a, first, biased_rng::summary_t{}, biased_rng::summary_t{}));
	check_chain(a, first, second);
}

/**
 * Everything the simulator covers on one unit at once: it drains, slows, swarms, fights two rounds
 * and levels off the kill, and it carries the lot into a second fight.
 */
BOOST_FIXTURE_TEST_CASE(biased_replay_all_specials_matches_simulation, biased_fixture)
{
	const battle_context_unit_stats a = advancing(make_stats(6, 4, 20, 30, 60, true, true, 2, true), 8);
	const battle_context_unit_stats first = make_stats(5, 3, 18, 18, 50);
	const battle_context_unit_stats second = make_stats(7, 2, 10, 10, 60);

	BOOST_REQUIRE(!biased_rng::can_solve(a, first, biased_rng::summary_t{}, biased_rng::summary_t{}));
	check_chain(a, first, second);
}

/**
 * Plain damage on both sides, but too many strikes to walk: the solver declines on the step budget
 * alone and the caller simulates instead.
 */
BOOST_FIXTURE_TEST_CASE(biased_solve_declines_past_the_budget, biased_fixture)
{
	const battle_context_unit_stats a = make_stats(1, 8, 200, 200, 40, false, false, 30);
	const battle_context_unit_stats b = make_stats(1, 8, 200, 200, 40, false, false, 30);

	BOOST_CHECK(!biased_rng::can_solve(a, b, biased_rng::summary_t{}, biased_rng::summary_t{}));
}

BOOST_AUTO_TEST_SUITE_END()
