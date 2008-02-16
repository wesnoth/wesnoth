/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.

   Full algorithm by Yogin.  Typing and optimization by Rusty.

   This code has lots of debugging.  It is there for a reason:
   this code is kinda tricky.  Do not remove it.
*/

//! @file attack_prediction.cpp
//! Simulate combat to calculate attacks. Standalone program, benchmark.

#include "attack_prediction.hpp"

#include <cassert>
#include <cstring> // For memset
#include <vector>


// Compile with -O3 -DBENCHMARK for speed testing,
// -DCHECK for testing correctness
// (run tools/wesnoth-attack-sim.c --check on output)
#if !defined(BENCHMARK) && !defined(CHECK)
#include "util.hpp"
#else
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#define maximum(a,b) ((a) > (b) ? (a) : (b))
#define minimum(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef ATTACK_PREDICTION_DEBUG
#define debug(x) printf x
#else
#define debug(x)
#endif

namespace
{
//! A matrix of A's hitpoints vs B's hitpoints.
struct prob_matrix
{
	// Simple matrix, both known HP.
	prob_matrix(unsigned int a_max_hp, unsigned int b_max_hp,
				bool a_slows, bool b_slows,
				unsigned int a_hp, unsigned int b_hp,
				const std::vector<double> a_summary[2],
				const std::vector<double> b_summary[2]);

	~prob_matrix();

	// A hits B.
	void receive_blow_b(unsigned damage, unsigned slow_damage, double hit_chance,
						bool a_slows, bool a_drains);

	// B hits A.  Why can't they just get along?
	void receive_blow_a(unsigned damage, unsigned slow_damage, double hit_chance,
						bool b_slows, bool b_drains);

	// We lied: actually did less damage, adjust matrix.
	void remove_stone_distortion_a(unsigned damage, unsigned slow_damage, unsigned b_hp);
	void remove_stone_distortion_b(unsigned damage, unsigned slow_damage, unsigned a_hp);

	// Its over, and here's the bill.
	void extract_results(std::vector<double> summary_a[2],
						 std::vector<double> summary_b[2]);

	// What's the chance one is dead?
	double dead_prob() const;

	void dump() const;

	// We need four matrices, or "planes", reflecting the possible
	// "slowed" states (neither slowed, A slowed, B slowed, both slowed).
	enum {
		NEITHER_SLOWED,
		A_SLOWED,
		B_SLOWED,
		BOTH_SLOWED
	};

private:
	// This gives me 10% speed improvement over std::vector<> (g++4.0.3 x86)
	double *new_arr(unsigned int size);

	double &val(unsigned plane, unsigned row, unsigned col);
	const double &val(unsigned plane, unsigned row, unsigned col) const;

	// Move this much from src to dst.  Returns true if anything transferred.
	void xfer(unsigned dst_plane, unsigned src_plane,
			  unsigned row_dst, unsigned col_dst,
			  unsigned row_src, unsigned col_src,
			  double prob);

	// Shift columns on this plane (b taking damage).  Returns min col.
	void shift_cols(unsigned dst, unsigned src,
					unsigned damage, double prob, bool drain);

	// Shift rows on this plane (a taking damage).  Returns new min row.
	void shift_rows(unsigned dst, unsigned src,
					unsigned damage, double prob, bool drain);

	//! @todo FIXME: rename using _ at end.
	unsigned int rows, cols;
	double *plane[4];

	// For optimization, we keep track of the lower row/col we need to consider
	unsigned int min_row[4], min_col[4];
};

prob_matrix::prob_matrix(unsigned int a_max_hp, unsigned int b_max_hp,
						 bool a_slows, bool b_slows,
						 unsigned int a_hp, unsigned int b_hp,
						 const std::vector<double> a_summary[2],
						 const std::vector<double> b_summary[2])
	: rows(a_max_hp+1), cols(b_max_hp+1)
{
	if (!a_summary[0].empty()) {
		// A has fought before.  Do we need a slow plane for it?
		if (!a_summary[1].empty())
			b_slows = true;
		// Don't handle both being reused.
		assert(b_summary[0].empty());
	}
	if (!b_summary[0].empty()) {
		// B has fought before.  Do we need a slow plane for it?
		if (!b_summary[1].empty())
			a_slows = true;
	}

	plane[NEITHER_SLOWED] = new_arr(rows*cols);
	if (b_slows)
		plane[A_SLOWED] = new_arr(rows*cols);
	else
		plane[A_SLOWED] = NULL;
	if (a_slows)
		plane[B_SLOWED] = new_arr(rows*cols);
	else
		plane[B_SLOWED] = NULL;
	if (a_slows && b_slows)
		plane[BOTH_SLOWED] = new_arr(rows*cols);
	else
		plane[BOTH_SLOWED] = NULL;

	min_row[NEITHER_SLOWED] = a_hp - 1;
	min_col[NEITHER_SLOWED] = b_hp - 1;
	min_row[A_SLOWED] = min_row[B_SLOWED] = min_row[BOTH_SLOWED] = rows;
	min_col[A_SLOWED] = min_col[B_SLOWED] = min_col[BOTH_SLOWED] = cols;

	// Transfer HP distribution from A?
	if (!a_summary[0].empty()) {
		// @todo FIXME: Can optimize here.
		min_row[NEITHER_SLOWED] = 0;
		min_row[A_SLOWED] = 0;
		min_col[A_SLOWED] = b_hp - 1;
		for (unsigned int row = 0; row < a_summary[0].size(); row++)
			val(NEITHER_SLOWED, row, b_hp) = a_summary[0][row];
		if (!a_summary[1].empty()) {
			for (unsigned int row = 0; row < a_summary[1].size(); row++)
				val(A_SLOWED, row, b_hp) = a_summary[1][row];
		}
		debug(("A has fought before\n"));
		dump();
	} else if (!b_summary[0].empty()) {
		min_col[NEITHER_SLOWED] = 0;
		min_col[B_SLOWED] = 0;
		min_row[B_SLOWED] = a_hp - 1;
		for (unsigned int col = 0; col < b_summary[0].size(); col++)
			val(NEITHER_SLOWED, a_hp, col) = b_summary[0][col];
		if (!b_summary[1].empty()) {
			for (unsigned int col = 0; col < b_summary[1].size(); col++)
				val(B_SLOWED, a_hp, col) = b_summary[1][col];
		}
		debug(("B has fought before\n"));
		dump();
	} else {
		// If a unit has drain it might end with more HP than before.
		// Make sure we don't access the matrix in invalid positions.
		a_hp = minimum<unsigned int>(a_hp, rows - 1);
		b_hp = minimum<unsigned int>(b_hp, cols - 1);
		val(NEITHER_SLOWED, a_hp, b_hp) = 1.0;
	}
}

prob_matrix::~prob_matrix()
{
	delete[] plane[NEITHER_SLOWED];
	delete[] plane[A_SLOWED];
	delete[] plane[B_SLOWED];
	delete[] plane[BOTH_SLOWED];
}

// Allocate a new probability array, initialized to 0.
double *prob_matrix::new_arr(unsigned int size)
{
	double *arr = new double[size];
	memset(arr, 0, sizeof(double) * size);
	return arr;
}

double &prob_matrix::val(unsigned p, unsigned row, unsigned col)
{
	assert(row < rows);
	assert(col < cols);
	return plane[p][row * cols + col];
}

const double &prob_matrix::val(unsigned p, unsigned row, unsigned col) const
{
	assert(row < rows);
	assert(col < cols);
	return plane[p][row * cols + col];
}

#ifdef CHECK
void prob_matrix::dump() const
{
	unsigned int row, col, m;
	const char *names[]
		= { "NEITHER_SLOWED", "A_SLOWED", "B_SLOWED", "BOTH_SLOWED" };

	for (m = 0; m < 4; m++) {
		if (!plane[m])
			continue;
		debug(("%s:\n", names[m]));
		for (row = 0; row < rows; row++) {
			debug(("  "));
			for (col = 0; col < cols; col++)
				debug(("%4.3g ", val(m, row, col)*100));
			debug(("\n"));
		}
	}
}
#else
void prob_matrix::dump() const
{
}
#endif

// xfer, shift_cols and shift_rows use up most of our time.  Careful!
void prob_matrix::xfer(unsigned dst_plane, unsigned src_plane,
					   unsigned row_dst, unsigned col_dst,
					   unsigned row_src, unsigned col_src,
					   double prob)
{
	double &src = val(src_plane, row_src, col_src);
	if (src != 0.0) {
		double diff = src * prob;
		src -= diff;

		// This is here for drain.
		if (col_dst >= cols)
			col_dst = cols - 1;
		if (row_dst >= rows)
			row_dst = rows - 1;

		val(dst_plane, row_dst, col_dst) += diff;

		debug(("Shifted %4.3g from %s(%u,%u) to %s(%u,%u)\n",
			   diff, src_plane == NEITHER_SLOWED ? ""
			   : src_plane == A_SLOWED ? "[A_SLOWED]"
			   : src_plane == B_SLOWED ? "[B_SLOWED]"
			   : src_plane == BOTH_SLOWED ? "[BOTH_SLOWED]" : "INVALID",
			   row_src, col_src,
			   dst_plane == NEITHER_SLOWED ? ""
			   : dst_plane == A_SLOWED ? "[A_SLOWED]"
			   : dst_plane == B_SLOWED ? "[B_SLOWED]"
			   : dst_plane == BOTH_SLOWED ? "[BOTH_SLOWED]" : "INVALID",
			   row_dst, col_dst));
	}
}

void prob_matrix::shift_cols(unsigned dst, unsigned src,
							 unsigned damage, double prob, bool drain)
{
	unsigned int row, col;
	unsigned int shift = drain ? 1 : 31; // Avoids a branch.

	if (damage >= cols)
		damage = cols - 1;

	// Loop backwards so we write drain behind us, for when src == dst.
	for (row = rows - 1; row > min_row[src]; row--) {
		// These are all going to die (move to col 0).
		for (col = 1; col <= damage; col++)
			xfer(dst, src, row+(col>>shift), 0, row, col, prob);
		for (col = damage+1; col < cols; col++)
			xfer(dst, src, row+(damage>>shift), col - damage, row, col, prob);
	}
}

void prob_matrix::shift_rows(unsigned dst, unsigned src,
							 unsigned damage, double prob, bool drain)
{
	unsigned int row, col;
	unsigned int shift = drain ? 1 : 31; // Avoids a branch.

	if (damage >= rows)
		damage = rows - 1;

	// Loop downwards so if we drain, we write behind us.
	for (col = cols - 1; col > min_col[src]; col--) {
		// These are all going to die (move to row 0).
		for (row = 1; row <= damage; row++)
			xfer(dst, src, 0, col+(row>>shift), row, col, prob);
		for (row = damage+1; row < rows; row++)
			xfer(dst, src, row - damage, col+(damage>>shift), row, col, prob);
	}
}

// Shift prob_matrix to reflect probability 'hit_chance'
// that damage (up to) 'damage' is done to 'b'.
void prob_matrix::receive_blow_b(unsigned damage, unsigned slow_damage, double hit_chance,
								 bool a_slows, bool a_drains)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned int actual_damage;

		if (!plane[src])
			continue;

		// If A slows us, we go from 0=>2, 1=>3, 2=>2 3=>3.
		if (a_slows)
			dst = (src|2);
		else
			dst = src;

		// A is slow in planes 1 and 3.
		if (src & 1)
			actual_damage = slow_damage;
		else
			actual_damage = damage;

		shift_cols(dst, src, actual_damage, hit_chance, a_drains);
		if (min_col[src] < damage)
			min_col[dst] = 0;
		else if (min_col[src] - damage < min_col[dst])
			min_col[dst] = min_col[src] - damage;
		if (min_row[src] < min_row[dst])
			min_row[dst] = min_row[src];
	}
}

// We lied: actually did less damage, adjust matrix.
void prob_matrix::remove_stone_distortion_a(unsigned damage, unsigned slow_damage,
											unsigned b_hp)
{
	for (int p = 0; p < 4; p++) {
		if (!plane[p])
			continue;

		// A is slow in planes 1 and 3.
		if (p & 1) {
			if (b_hp > slow_damage)
				for (unsigned int row = 0; row < rows; row++)
					xfer(p, p, row, b_hp - slow_damage, row, 0, 1.0);
		} else {
			if (b_hp > damage)
				for (unsigned int row = 0; row < rows; row++)
					xfer(p, p, row, b_hp - damage, row, 0, 1.0);
		}
	}
}

void prob_matrix::remove_stone_distortion_b(unsigned damage, unsigned slow_damage,
											unsigned a_hp)
{
	for (int p = 0; p < 4; p++) {
		if (!plane[p])
			continue;

		// B is slow in planes 2 and 3.
		if (p & 2) {
			if (a_hp > slow_damage)
				for (unsigned int col = 0; col < cols; col++)
					xfer(p, p, a_hp - slow_damage, col, 0, col, 1.0);
		} else {
			if (a_hp > damage)
				for (unsigned int col = 0; col < cols; col++)
					xfer(p, p, a_hp - damage, col, 0, col, 1.0);
		}
	}
}

void prob_matrix::extract_results(std::vector<double> summary_a[2],
								  std::vector<double> summary_b[2])
{
	unsigned int p, row, col;

	summary_a[0] = std::vector<double>(rows);
	summary_b[0] = std::vector<double>(cols);

	if (plane[A_SLOWED])
		summary_a[1] = std::vector<double>(rows);
	if (plane[B_SLOWED])
		summary_b[1] = std::vector<double>(cols);

	for (p = 0; p < 4; p++) {
		int dst_a, dst_b;
		if (!plane[p])
			continue;

		// A is slow in planes 1 and 3.
		dst_a = (p & 1);
		// B is slow in planes 2 and 3.
		dst_b = !!(p & 2);
		for (row = 0; row < rows; row++) {
			for (col = 0; col < cols; col++) {
				summary_a[dst_a][row] += val(p, row, col);
				summary_b[dst_b][col] += val(p, row, col);
			}
		}
	}
}

// What's the chance one is dead?
double prob_matrix::dead_prob() const
{
	unsigned int p, row, col;
	double prob = 0.0;

	for (p = 0; p < 4; p++) {
		if (!plane[p])
			continue;
		// We might count 0,0 twice, but that is always 0 anyway.
		for (row = min_row[p]; row < rows; row++)
			prob += val(p, row, 0);
		for (col = min_col[p]; col < cols; col++)
			prob += val(p, 0, col);
	}
	return prob;
}

// Shift matrix to reflect probability 'hit_chance'
// that damage (up to) 'damage' is done to 'a'.
void prob_matrix::receive_blow_a(unsigned damage, unsigned slow_damage, double hit_chance,
								 bool b_slows, bool b_drains)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned actual_damage;

		if (!plane[src])
			continue;

		// If B slows us, we go from 0=>1, 1=>1, 2=>3 3=>3.
		if (b_slows)
			dst = (src|1);
		else
			dst = src;

		// B is slow in planes 2 and 3.
		if (src & 2)
			actual_damage = slow_damage;
		else
			actual_damage = damage;

		shift_rows(dst, src, actual_damage, hit_chance, b_drains);
		if (min_row[src] < damage)
			min_row[dst] = 0;
		else if (min_row[src] - damage < min_row[dst])
			min_row[dst] = min_row[src] - damage;
		if (min_col[src] < min_col[dst])
			min_col[dst] = min_col[src];
	}
}

} // end anon namespace

unsigned combatant::hp_dist_size(const battle_context::unit_stats &u, const combatant *prev)
{
	// Our summary must be as big as previous one.
	if (prev) {
		return prev->hp_dist.size();
	}

	// If this unit drains, HP can increase, so alloc full array.
	if (u.drains) {
		return u.max_hp + 1;
	}
	return u.hp+1;
}

combatant::combatant(const battle_context::unit_stats &u, const combatant *prev)
	: hp_dist(hp_dist_size(u, prev)),
	  u_(u),
	  hit_chances_(u.num_blows, u.chance_to_hit / 100.0)
{
	// We inherit current state from previous combatant.
	if (prev) {
		summary[0] = prev->summary[0];
		summary[1] = prev->summary[1];
		poisoned = prev->poisoned;
		untouched = prev->untouched;
		slowed = prev->slowed;
	} else {
		untouched = 1.0;
		poisoned = u.is_poisoned ? 1.0 : 0.0;
		slowed = u.is_slowed ? 1.0 : 0.0;
	}
}

// Copy constructor (except use this copy of unit_stats)
combatant::combatant(const combatant &that, const battle_context::unit_stats &u)
	: hp_dist(that.hp_dist), untouched(that.untouched), poisoned(that.poisoned), slowed(that.slowed), u_(u), hit_chances_(that.hit_chances_)
{
		summary[0] = that.summary[0];
		summary[1] = that.summary[1];
}



// For swarm, whether we get an attack depends on HP distribution
// from previous combat.  So we roll this into our P(hitting),
// since no attack is equivalent to missing.
void combatant::adjust_hitchance()
{
	if (summary[0].empty() || u_.swarm_min == u_.swarm_max)
		return;

	hit_chances_ = std::vector<double>(u_.swarm_max);
	double alive_prob;

	if (summary[1].empty())
		alive_prob = 1 - summary[0][0];
	else
		alive_prob = 1 - summary[0][0] - summary[1][0];

	unsigned int i;
	for (i = 1; i <= u_.max_hp; i++) {
		double prob = 0.0;
		if(i < summary[0].size()) {
			prob = summary[0][i];
		}
		if (!summary[1].empty())
			prob += summary[1][i];
		for (unsigned int j = 0; j < u_.swarm_min + (u_.swarm_max -
                static_cast<double>(u_.swarm_min)) * u_.hp / u_.max_hp; j++)

			hit_chances_[j] += prob * u_.chance_to_hit / 100.0 / alive_prob;
	}

	debug(("\nhit_chances_ (base %u%%):", u_.chance_to_hit));
	for (i = 0; i < u_.swarm_max; i++)
		debug((" %.2f", hit_chances_[i] * 100.0 + 0.5));
	debug(("\n"));
}

// Minimum HP we could possibly have.
unsigned combatant::min_hp() const
{
	if (summary[0].empty())
		return u_.hp;

	// We don't handle this (yet).
	assert(summary[1].empty());

	unsigned int i;
	for (i = 0; summary[0][i] == 0; i++);
	return i;
}

// Combat without chance of death, berserk, slow or drain is simple.
void combatant::no_death_fight(combatant &opp)
{
	if (summary[0].empty()) {
		// Starts with a known HP, so Pascal's triangle.
		summary[0] = std::vector<double>(u_.hp+1);
		summary[0][u_.hp] = 1.0;
		for (unsigned int i = 0; i < opp.hit_chances_.size(); i++) {
			for (int j = i; j >= 0; j--) {
				double move = summary[0][u_.hp - j * opp.u_.damage] * opp.hit_chances_[i];
				summary[0][u_.hp - j * opp.u_.damage] -= move;
				summary[0][u_.hp - (j+1) * opp.u_.damage] += move;
			}
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < opp.hit_chances_.size(); i++) {
			for (unsigned int j = opp.u_.damage; j <= u_.hp; j++) {
				double move = summary[0][j] * opp.hit_chances_[i];
				summary[0][j] -= move;
				summary[0][j - opp.u_.damage] += move;
			}
		}
	}

	if (opp.summary[0].empty()) {
		// Starts with a known HP, so Pascal's triangle.
		opp.summary[0] = std::vector<double>(opp.u_.hp+1);
		opp.summary[0][opp.u_.hp] = 1.0;
		for (unsigned int i = 0; i < hit_chances_.size(); i++) {
			for (int j = i; j >= 0; j--) {
				double move = opp.summary[0][opp.u_.hp - j * u_.damage] * hit_chances_[i];
				opp.summary[0][opp.u_.hp - j * u_.damage] -= move;
				opp.summary[0][opp.u_.hp - (j+1) * u_.damage] += move;
			}
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < hit_chances_.size(); i++) {
			for (unsigned int j = u_.damage; j <= opp.u_.hp; j++) {
				double move = opp.summary[0][j] * hit_chances_[i];
				opp.summary[0][j] -= move;
				opp.summary[0][j - u_.damage] += move;
			}
		}
	}
}

// Combat with <= 1 strike each is simple, too.
void combatant::one_strike_fight(combatant &opp)
{
	if (opp.summary[0].empty()) {
		opp.summary[0] = std::vector<double>(opp.u_.hp+1);
		if (hit_chances_.size() == 1) {
			opp.summary[0][opp.u_.hp] = 1.0 - hit_chances_[0];
			opp.summary[0][maximum<int>(opp.u_.hp - u_.damage, 0)] = hit_chances_[0];
		} else {
			assert(hit_chances_.size() == 0);
			opp.summary[0][opp.u_.hp] = 1.0;
		}
	} else {
		if (hit_chances_.size() == 1) {
			for (unsigned int i = 1; i < opp.summary[0].size(); i++) {
				double move = opp.summary[0][i] * hit_chances_[0];
				opp.summary[0][i] -= move;
				opp.summary[0][maximum<int>(i - u_.damage, 0)] += move;
			}
		}
	}

	// If we killed opponent, it won't attack us.
	double opp_alive_prob = 1.0 - opp.summary[0][0];
	if (summary[0].empty()) {
		summary[0] = std::vector<double>(u_.hp+1);
		if (opp.hit_chances_.size() == 1) {
			summary[0][u_.hp] = 1.0 - opp.hit_chances_[0] * opp_alive_prob;
			summary[0][maximum<int>(u_.hp - opp.u_.damage, 0)] = opp.hit_chances_[0] * opp_alive_prob;
		} else {
			assert(opp.hit_chances_.size() == 0);
			summary[0][u_.hp] = 1.0;
		}
	} else {
		if (opp.hit_chances_.size() == 1) {
			for (unsigned int i = 1; i < summary[0].size(); i++) {
				double move = summary[0][i] * opp.hit_chances_[0] * opp_alive_prob;
				summary[0][i] -= move;
				summary[0][maximum<int>(i - opp.u_.damage, 0)] += move;
			}
		}
	}
}

void combatant::complex_fight(combatant &opp, unsigned int rounds)
{
	prob_matrix m(hp_dist.size()-1, opp.hp_dist.size()-1,
				  u_.slows && !opp.u_.is_slowed, opp.u_.slows && !u_.is_slowed,
				  u_.hp, opp.u_.hp, summary, opp.summary);

	unsigned max_attacks = maximum(hit_chances_.size(), opp.hit_chances_.size());

	debug(("A gets %u attacks, B %u\n", hit_chances_.size(), opp.hit_chances_.size()));

	unsigned int a_damage = u_.damage, a_slow_damage = u_.slow_damage;
	unsigned int b_damage = opp.u_.damage, b_slow_damage = opp.u_.slow_damage;

	// To simulate stoning, we set to amount which kills, and re-adjust after.
	//! @todo FIXME: This doesn't work for rolling calculations, just first battle.
	if (u_.stones)
		a_damage = a_slow_damage = opp.u_.max_hp;
	if (opp.u_.stones)
		b_damage = b_slow_damage = u_.max_hp;

	do {
		for (unsigned int i = 0; i < max_attacks; i++) {
			if (i < hit_chances_.size()) {
				debug(("A strikes\n"));
				m.receive_blow_b(a_damage, a_slow_damage, hit_chances_[i],
								 u_.slows && !opp.u_.is_slowed, u_.drains);
				m.dump();
			}
			if (i < opp.hit_chances_.size()) {
				debug(("B strikes\n"));
				m.receive_blow_a(b_damage, b_slow_damage, opp.hit_chances_[i],
								 opp.u_.slows && !u_.is_slowed, opp.u_.drains);
				m.dump();
			}
		}

		debug(("Combat ends:\n"));
		m.dump();
	} while (--rounds && m.dead_prob() < 0.99);

	if (u_.stones)
		m.remove_stone_distortion_a(u_.damage, u_.slow_damage, opp.u_.hp);
	if (opp.u_.stones)
		m.remove_stone_distortion_b(opp.u_.damage, opp.u_.slow_damage, u_.hp);

	// We extract results separately, then combine.
	m.extract_results(summary, opp.summary);
}

// Two man enter.  One man leave!
// ... Or maybe two.  But definitely not three.
// Of course, one could be a woman.  Or both.
// And neither could be human, too.
// Um, ok, it was a stupid thing to say.
void combatant::fight(combatant &opp)
{
	unsigned int rounds = maximum<unsigned int>(u_.rounds, opp.u_.rounds);

	// If defender has firststrike and we don't, reverse.
	if (opp.u_.firststrike && !u_.firststrike) {
		opp.fight(*this);
		return;
	}

#ifdef ATTACK_PREDICTION_DEBUG
	printf("A:\n");
	u_.dump();
	printf("B:\n");
	opp.u_.dump();
#endif

	// If we've fought before and we have swarm, we must adjust cth array.
	adjust_hitchance();
	opp.adjust_hitchance();

#if 0
	std::vector<double> prev = summary[0], opp_prev = opp.summary[0];
	complex_fight(opp, 1);
	std::vector<double> res = summary[0], opp_res = opp.summary[0];
	summary[0] = prev;
	opp.summary[0] = opp_prev;
#endif

	// Optimize the simple cases.
	if (rounds == 1 && !u_.slows && !opp.u_.slows &&
		!u_.drains && !opp.u_.drains && !u_.stones && !opp.u_.stones &&
		summary[1].empty() && opp.summary[1].empty()) {
		if (hit_chances_.size() <= 1 && opp.hit_chances_.size() <= 1) {
			one_strike_fight(opp);
		} else if (hit_chances_.size() * u_.damage < opp.min_hp() &&
			opp.hit_chances_.size() * opp.u_.damage < min_hp()) {
			no_death_fight(opp);
		} else {
			complex_fight(opp, rounds);
		}
	} else {
			complex_fight(opp, rounds);
	}

#if 0
	assert(summary[0].size() == res.size());
	assert(opp.summary[0].size() == opp_res.size());
	for (unsigned int i = 0; i < summary[0].size(); i++) {
		if (fabs(summary[0][i] - res[i]) > 0.000001) {
			std::cerr << "Mismatch for " << i << " hp: " << summary[0][i] << " should have been " << res[i] << "\n";
			assert(0);
		}
	}
	for (unsigned int i = 0; i < opp.summary[0].size(); i++) {
		if (fabs(opp.summary[0][i] - opp_res[i])> 0.000001) {
			std::cerr << "Mismatch for " << i << " hp: " << opp.summary[0][i] << " should have been " << opp_res[i] << "\n";
			assert(0);
		}
	}
#endif

	// Combine summary into distribution.
	if (summary[1].empty())
		hp_dist = summary[0];
	else {
		for (unsigned int i = 0; i < hp_dist.size(); i++)
			hp_dist[i] = summary[0][i] + summary[1][i];
	}
	if (opp.summary[1].empty())
		opp.hp_dist = opp.summary[0];
	else {
		for (unsigned int i = 0; i < opp.hp_dist.size(); i++)
			opp.hp_dist[i] = opp.summary[0][i] + opp.summary[1][i];
	}

	// Make sure we don't try to access the vectors out of bounds,
	// drain increases HPs so we determine the number of HP here
	// and make sure it stays within bounds
	const unsigned int hp = minimum<unsigned int>(u_.hp, hp_dist.size() - 1);
	const unsigned int opp_hp = minimum<unsigned int>(opp.u_.hp, opp.hp_dist.size() - 1);

	// Chance that we / they were touched this time.
	double touched = untouched - hp_dist[hp];
	double opp_touched = opp.untouched - opp.hp_dist[opp_hp];
	if (opp.u_.poisons)
		poisoned += (1 - poisoned) * touched;
	if (u_.poisons)
		opp.poisoned += (1 - opp.poisoned) * opp_touched;

	if (opp.u_.slows)
		slowed += (1 - slowed) * touched;
	if (u_.slows)
		opp.slowed += (1 - opp.slowed) * opp_touched;

	//! @todo FIXME: This is approximate: we could drain, then get hit.
	untouched = hp_dist[hp];
	opp.untouched = opp.hp_dist[opp_hp];
}

double combatant::average_hp(unsigned int healing) const
{
	double total = 0;

	// Since sum of probabilities is 1.0, we can just tally weights.
	for (unsigned int i = 1; i < hp_dist.size(); i++) {
		total += hp_dist[i] * minimum<unsigned int>(i + healing, u_.max_hp);
	}
	return total;
}

#if defined(BENCHMARK) || defined(CHECK)
// We create a significant number of nasty-to-calculate units,
// and test each one against the others.
#define NUM_UNITS 50

// Stolen from glibc headers sys/time.h
#define timer_sub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

#ifdef CHECK
void combatant::print(const char label[], unsigned int battle) const
{
	printf("#%u: %s: %u %u %u %2g%% ", battle,
		   label, damage_, base_num_attacks_, hp_, base_hit_chance_*100.0);
	if (drains_)
		printf("drains,");
	if (slows_)
		printf("slows,");
	if (berserk_)
		printf("berserk,");
	if (swarm_)
		printf("swarm,");
	if (firststrike_)
		printf("firststrike,");
	printf("maxhp=%u ", hp_dist.size()-1);
	printf(" %.2f", untouched);
	for (unsigned int i = 0; i < hp_dist.size(); i++)
		printf(" %.2f", hp_dist[i] * 100);
	printf("\n");
}
#else  // ... BENCHMARK
void combatant::print(const char label[], unsigned int battle) const
{
}
#endif

static void run(unsigned specific_battle)
{
	// N^2 battles
	struct combatant *u[NUM_UNITS];
	unsigned int i, j, k, battle = 0;
	struct timeval start, end, total;

	for (i = 0; i < NUM_UNITS; i++) {
		unsigned hp = 1 + ((i*3)%23);
		u[i] = new combatant(hp, hp + (i+7)%17, false);
		u[i]->set_weapon((i % 4) + 1, (i % 9) == 0, (i % 5) == 0,
						 ((i+4) % 4) == 0,
						 ((i+3) % 5) == 0);
		u[i]->set_effectiveness((i % 7) + 2, 0.3 + (i % 6)*0.1, (i % 8) == 0);
	}

	gettimeofday(&start, NULL);
	for (i = 0; i < NUM_UNITS; i++) {
		for (j = 0; j < NUM_UNITS; j++) {
			if (i == j)
				continue;
			for (k = 0; k < NUM_UNITS; k++) {
				double untouched;
				if (i == k || j == k)
					continue;
				battle++;
				if (specific_battle && battle != specific_battle)
					continue;
				u[j]->fight(*u[i]);
				// We need this here, because swarm means
				// out num hits can change.
				u[i]->set_effectiveness((i % 7) + 2, 0.3 + (i % 6)*0.1,
										(i % 8) == 0);
				u[k]->fight(*u[i]);
				u[i]->print("Defender", battle);
				u[j]->print("Attacker #1", battle);
				u[k]->print("Attacker #2", battle);
				u[i]->reset();
				u[j]->reset();
				u[k]->reset();
			}
		}
	}
	gettimeofday(&end, NULL);

	timer_sub(&end, &start, &total);

#ifdef BENCHMARK
	printf("Total time for %i combats was %lu.%06lu\n",
	       NUM_UNITS*(NUM_UNITS-1)*(NUM_UNITS-2), total.tv_sec, total.tv_usec);
	printf("Time per calc = %li us\n",
	       ((end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec))
		   / (NUM_UNITS*(NUM_UNITS-1)*(NUM_UNITS-2)));
#else
	printf("Total combats: %i\n", NUM_UNITS*(NUM_UNITS-1)*(NUM_UNITS-2));
#endif

	for (i = 0; i < NUM_UNITS; i++) {
		delete u[i];
	}

	exit(0);
}

static combatant *parse_unit(char ***argv,
							 unsigned *damagep = NULL,
							 double *hit_chancep = NULL,
							 bool *slowsp = NULL)
{
	unsigned damage, num_attacks, hp, max_hp, hit_chance;
	bool slows, slowed, drains, berserk, swarm, firststrike;
	combatant *u;

	damage = atoi((*argv)[1]);
	num_attacks = atoi((*argv)[2]);
	hp = max_hp = atoi((*argv)[3]);
	hit_chance = atoi((*argv)[4]);
	slows = false;
	slowed = false;
	drains = false;
	berserk = false;
	swarm = false;
	firststrike = false;

	if (damagep)
		*damagep = damage;
	if (hit_chancep)
		*hit_chancep = hit_chance/100.0;
	if (slowsp)
		*slowsp = slows;

	if ((*argv)[5] && atoi((*argv)[5]) == 0) {
		char *max = strstr((*argv)[5], "maxhp=");

		if (max) {
			max_hp = atoi(max + strlen("maxhp="));
			if (max_hp < hp) {
				fprintf(stderr, "maxhp must be > hitpoints");
				exit(1);
			}
		}
		if (strstr((*argv)[5], "drain")) {
			if (!max) {
				fprintf(stderr, "drain needs maxhp set");
				exit(1);
			}
			drains = true;
		}
		if (strstr((*argv)[5], "slows"))
			slows = true;
		if (strstr((*argv)[5], "slowed"))
			slowed = true;
		if (strstr((*argv)[5], "berserk"))
			berserk = true;
		if (strstr((*argv)[5], "firststrike"))
			firststrike = true;
		if (strstr((*argv)[5], "swarm")) {
			if (!max) {
				fprintf(stderr, "swarm needs maxhp set");
				exit(1);
			}
			swarm = true;
		}
		*argv += 5;
	} else {
		*argv += 4;
	}
	u = new combatant(hp, max_hp, slowed, true);
	u->set_weapon(num_attacks, drains, berserk, swarm, firststrike);
	u->set_effectiveness(damage, hit_chance/100.0, slows);
	return u;
}

int main(int argc, char *argv[])
{
	combatant *def, *att[20];
	double hit_chance;
	unsigned damage;
	bool slows;
	unsigned int i;

	if (argc < 3)
		run(argv[1] ? atoi(argv[1]) : 0);

	if (argc < 9) {
		fprintf(stderr,"Usage: %s <damage> <attacks> <hp> <hitprob> [drain,slows,slowed,swarm,firststrike,berserk,maxhp=<num>] <damage> <attacks> <hp> <hitprob> [drain,slows,slowed,berserk,firststrike,swarm,maxhp=<num>] ...",
				argv[0]);
		exit(1);
	}

	def = parse_unit(&argv, &damage, &hit_chance, &slows);
	for (i = 0; argv[1]; i++)
		att[i] = parse_unit(&argv);
	att[i] = NULL;

	for (i = 0; att[i]; i++) {
		// In case defender has swarm, effectiveness changes.
		debug(("Fighting next attacker\n"));
		def->set_effectiveness(damage, hit_chance, slows);
		att[i]->fight(*def);
	}

	def->print("Defender", 0);
	for (i = 0; att[i]; i++)
		att[i]->print("Attacker", 0);

	delete def;
	for (i = 0; att[i]; i++)
		delete att[i];

	return 0;
}
#endif // Standalone program
