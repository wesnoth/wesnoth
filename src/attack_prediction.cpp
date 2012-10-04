/* $Id$ */
/*
   Copyright (C) 2006 - 2012 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.

   Full algorithm by Yogin.  Original typing and optimization by Rusty.

   This code has lots of debugging.  It is there for a reason:
   this code is kinda tricky.  Do not remove it.
*/

/**
 * @file
 * Simulate combat to calculate attacks.
 * This can be compiled as a stand-alone program to either verify
 * correctness or to benchmark performance.
 *
 * Compile with -O3 -DBENCHMARK for speed testing, and with -DCHECK for
 * testing correctness (redirect the output to a file, then compile
 * utils/wesnoth-attack-sim.c and run that with the arguments
 * --check <file name>).
 * For either option, use -DHUMAN_READABLE if you want to see the results
 * from each combat displayed in a prettier format (but no longer suitable
 * for wesnoth-attack-sim.c).
 */

#include <cfloat>

#include "attack_prediction.hpp"

#include "actions/attack.hpp"
#include "game_config.hpp"

#if defined(BENCHMARK) || defined(CHECK)
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

// Set some default values so this file can stand alone.
namespace game_config {
	int kill_experience = 8;
	int tile_size = 72; // Not really needed, but it's used in image.hpp.
}
#endif

#ifdef ATTACK_PREDICTION_DEBUG
#define debug(x) printf x
#else
#define debug(x)
#endif

#ifdef ATTACK_PREDICTION_DEBUG
namespace {
	/** Dumps the statistics of a unit on stdout. Remove it eventually. */
	// Moved from attack/attack.cpp
	void dump(const battle_context_unit_stats & stats)
	{
		printf("==================================\n");
		printf("is_attacker:    %d\n", static_cast<int>(stats.is_attacker));
		printf("is_poisoned:    %d\n", static_cast<int>(stats.is_poisoned));
		printf("is_slowed:      %d\n", static_cast<int>(stats.is_slowed));
		printf("slows:          %d\n", static_cast<int>(stats.slows));
		printf("drains:         %d\n", static_cast<int>(stats.drains));
		printf("petrifies:      %d\n", static_cast<int>(stats.petrifies));
		printf("poisons:        %d\n", static_cast<int>(stats.poisons));
		printf("backstab_pos:   %d\n", static_cast<int>(stats.backstab_pos));
		printf("swarm:          %d\n", static_cast<int>(stats.swarm));
		printf("rounds:         %u\n", stats.rounds);
		printf("firststrike:    %d\n", static_cast<int>(stats.firststrike));
		printf("\n");
		printf("hp:             %u\n", stats.hp);
		printf("max_hp:         %u\n", stats.max_hp);
		printf("chance_to_hit:  %u\n", stats.chance_to_hit);
		printf("damage:         %d\n", stats.damage);
		printf("slow_damage:    %d\n", stats.slow_damage);
		printf("drain_percent:  %d\n", stats.drain_percent);
		printf("drain_constant: %d\n", stats.drain_constant);
		printf("num_blows:      %u\n", stats.num_blows);
		printf("swarm_min:      %u\n", stats.swarm_min);
		printf("swarm_max:      %u\n", stats.swarm_max);
		printf("\n");
	}
}
#endif

namespace
{

/**
 * A matrix of A's hitpoints vs B's hitpoints vs. their slowed states.
 * This class is concerned only with the matrix implementation and
 * implements functionality for shifting and retrieving probabilities
 * (i.e. low-level stuff).
 */
class prob_matrix
{
public:
	prob_matrix(unsigned int a_max, unsigned int b_max,
	            bool need_a_slowed, bool need_b_slowed,
	            unsigned int a_cur, unsigned int b_cur,
	            const std::vector<double> a_initial[2],
	            const std::vector<double> b_initial[2]);

	~prob_matrix();

	// Shift columns on this plane (b taking damage).
	void shift_cols(unsigned dst, unsigned src, unsigned damage,
	                double prob, int drain_constant, int drain_percent);

	// Shift rows on this plane (a taking damage).
	void shift_rows(unsigned dst, unsigned src, unsigned damage,
	                double prob, int drain_constant, int drain_percent);

	/// What is the chance that an indicated combatant (one of them) is at zero?
	double prob_of_zero(bool check_a, bool check_b) const;

	/// Returns true if the specified plane might have data in it.
	bool plane_used(unsigned p) const { return p < 4  &&  plane_[p] != NULL; }

	unsigned int num_rows() const { return rows_; }
	unsigned int num_cols() const { return cols_; }

	// To allow a more optimized loop through data:
	unsigned int min_row(unsigned plane) const { return min_row_[plane]; }
	unsigned int min_col(unsigned plane) const { return min_col_[plane]; }
	// No error checking in these yet.
	void set_min_row(unsigned plane, unsigned row) { min_row_[plane] = row; }
	void set_min_col(unsigned plane, unsigned col) { min_col_[plane] = col; }

	// Debugging tool.
	void dump() const;

	// We need four matrices, or "planes", reflecting the possible
	// "slowed" states (neither slowed, A slowed, B slowed, both slowed).
	enum {
		NEITHER_SLOWED,
		A_SLOWED,
		B_SLOWED,
		BOTH_SLOWED
	};

	double &val(unsigned plane, unsigned row, unsigned col);
	const double &val(unsigned plane, unsigned row, unsigned col) const;

	// Move this much from src to dst.  Returns true if anything transferred.
	void xfer(unsigned dst_plane, unsigned src_plane,
			  unsigned row_dst, unsigned col_dst,
			  unsigned row_src, unsigned col_src,
			  double prob);

private:
	// This gives me 10% speed improvement over std::vector<> (g++4.0.3 x86)
	double *new_plane();

	void initialize_plane(unsigned plane, unsigned a_cur, unsigned b_cur,
	                      const std::vector<double> & a_initial,
	                      const std::vector<double> & b_initial);
	void initialize_row(unsigned plane, unsigned row, double row_prob,
	                    unsigned b_cur, const std::vector<double> & b_initial);

	void shift_cols_in_row(unsigned dst, unsigned src, unsigned row,
	                       unsigned damage, double prob, int drainmax,
	                       int drain_constant, int drain_percent);
	void shift_rows_in_col(unsigned dst, unsigned src, unsigned col,
	                       unsigned damage, double prob, int drainmax,
	                       int drain_constant, int drain_percent);

private: // data
	const unsigned int rows_, cols_;
	double *plane_[4];

	// For optimization, we keep track of the lower row/col we need to consider
	unsigned int min_row_[4], min_col_[4];
};


/**
 * Constructor.
 * @param  a_max          The maximum value we will track for A.
 * @param  b_max          The maximum value we will track for B.
 * @param  need_a_slowed  Set to true if there might be transfers to a "slow" plane for A.
 * @param  need_b_slowed  Set to true if there might be transfers to a "slow" plane for B.
 * @param  a_cur          The current value for A. (Ignored if a_initial[0] is not empty.)
 * @param  b_cur          The current value for B. (Ignored if b_initial[0] is not empty.)
 * @param  a_initial      The initial distribution of values for A. Element [0] is for normal A. while [1] is for slowed A.
 * @param  b_initial      The initial distribution of values for B. Element [0] is for normal B. while [1] is for slowed B.
 */
prob_matrix::prob_matrix(unsigned int a_max, unsigned int b_max,
                         bool need_a_slowed, bool need_b_slowed,
                         unsigned int a_cur, unsigned int b_cur,
                         const std::vector<double> a_initial[2],
                         const std::vector<double> b_initial[2])
	: rows_(a_max+1), cols_(b_max+1)
{
	// Make sure we do not access the matrix in invalid positions.
	a_cur = std::min<unsigned int>(a_cur, rows_ - 1);
	b_cur = std::min<unsigned int>(b_cur, cols_ - 1);

	// We will need slowed planes if the initial vectors have them.
	need_a_slowed =  need_a_slowed || !a_initial[1].empty();
	need_b_slowed =  need_b_slowed || !b_initial[1].empty();

	// Allocate the needed planes.
	plane_[NEITHER_SLOWED] = new_plane();
	plane_[A_SLOWED] = !need_a_slowed ? NULL : new_plane();
	plane_[B_SLOWED] = !need_b_slowed ? NULL : new_plane();
	plane_[BOTH_SLOWED] = !(need_a_slowed && need_b_slowed) ? NULL : new_plane();

	// Default min_row_ and min_col_ for the planes that might not need
	// initialization.
	min_row_[A_SLOWED] = min_row_[B_SLOWED] = min_row_[BOTH_SLOWED] = rows_;
	min_col_[A_SLOWED] = min_col_[B_SLOWED] = min_col_[BOTH_SLOWED] = cols_;

	// Initialize the probability distribution.
	initialize_plane(NEITHER_SLOWED, a_cur, b_cur, a_initial[0], b_initial[0]);
	if ( !a_initial[1].empty() )
		initialize_plane(A_SLOWED, a_cur, b_cur, a_initial[1], b_initial[0]);
	if ( !b_initial[1].empty() )
		initialize_plane(B_SLOWED, a_cur, b_cur, a_initial[0], b_initial[1]);
	if ( !a_initial[1].empty() && !b_initial[1].empty() )
		// Currently will not happen, but to be complete...
		initialize_plane(BOTH_SLOWED, a_cur, b_cur, a_initial[1], b_initial[1]);

	// Some debugging messages.
	if ( !a_initial[0].empty() ) {
		debug(("A has fought before.\n"));
		dump();
	}
	else if ( !b_initial[0].empty() ) {
		debug(("B has fought before.\n"));
		dump();
	}
}

prob_matrix::~prob_matrix()
{
	delete[] plane_[NEITHER_SLOWED];
	delete[] plane_[A_SLOWED];
	delete[] plane_[B_SLOWED];
	delete[] plane_[BOTH_SLOWED];
}

/** Allocate a new probability array, initialized to 0. */
double *prob_matrix::new_plane()
{
	unsigned int size = rows_ * cols_;
	double *arr = new double[size];
	memset(arr, 0, sizeof(double) * size);
	return arr;
}

/**
 * Fills the indicated plane with its initial (non-zero) values.
 * (Part of construction)
 * @param  plane          The plane to initialize.
 * @param  a_cur          The current value for A. (Ignored if a_initial is not empty.)
 * @param  b_cur          The current value for B. (Ignored if b_initial is not empty.)
 * @param  a_initial      The initial distribution of values for A for this plane.
 * @param  b_initial      The initial distribution of values for B for this plane.
 */
void prob_matrix::initialize_plane(unsigned plane, unsigned a_cur, unsigned b_cur,
                                   const std::vector<double> & a_initial,
                                   const std::vector<double> & b_initial)
{
	if ( !a_initial.empty() ) {
		unsigned row_count = std::min<unsigned>(a_initial.size(), rows_);
		// @todo FIXME: Can optimize here.
		min_row_[plane] = 0;
		// The probabilities for each row are contained in a_initial.
		for ( unsigned row = 0; row < row_count; ++row ) {
			if ( a_initial[row] != 0.0 )
				initialize_row(plane, row, a_initial[row], b_cur, b_initial);
		}
	}
	else {
		min_row_[plane] = a_cur - 1;
		// Only the row indicated by a_cur is a possibility.
		initialize_row(plane, a_cur, 1.0, b_cur, b_initial);
	}
}

/**
 * Fills the indicated row with its initial (non-zero) values.
 * (Part of construction)
 * @param  plane          The plane containing the row to initialize.
 * @param  row            The row to initialize.
 * @param  row_prob       The probability of A having the value for this row.
 * @param  b_cur          The current value for B. (Ignored if b_initial is not empty.)
 * @param  b_initial      The initial distribution of values for B for this plane.
 */
void prob_matrix::initialize_row(unsigned plane, unsigned row, double row_prob,
                                 unsigned b_cur, const std::vector<double> & b_initial)
{
	if ( !b_initial.empty() ) {
		unsigned col_count = std::min<unsigned>(b_initial.size(), cols_);
		// @todo FIXME: Can optimize here.
		min_col_[plane] = 0;
		// The probabilities for each column are contained in b_initial.
		for ( unsigned col = 0; col < col_count; ++col ) {
			if ( b_initial[col] != 0.0 )
				val(plane, row, col) = row_prob * b_initial[col];
		}
	}
	else {
		min_col_[plane] = b_cur - 1;
		// Only the column indicated by b_cur is a possibility.
		val(plane, row, b_cur) = row_prob;
	}
}

double &prob_matrix::val(unsigned p, unsigned row, unsigned col)
{
	assert(row < rows_);
	assert(col < cols_);
	return plane_[p][row * cols_ + col];
}

const double &prob_matrix::val(unsigned p, unsigned row, unsigned col) const
{
	assert(row < rows_);
	assert(col < cols_);
	return plane_[p][row * cols_ + col];
}

// xfer, shift_cols and shift_rows use up most of our time.  Careful!
/**
 * Transfers a portion (value * prob) of one value in the matrix to another.
 */
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
		if (col_dst >= cols_)
			col_dst = cols_ - 1;
		if (row_dst >= rows_)
			row_dst = rows_ - 1;

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

/**
 * Transfers a portion (value * prob) of the values in a row to another.
 * Part of shift_cols(). @a damage is assumed to be less than cols_.
 */
void prob_matrix::shift_cols_in_row(unsigned dst, unsigned src, unsigned row,
                                    unsigned damage, double prob, int drainmax,
                                    int drain_constant, int drain_percent)
{
	// Column 0 is already dead, so skip it.
	unsigned col = std::max(1u, min_col_[src]);

	// Killing blows can have different drain amounts, so handle them first
	for ( ; col < damage; ++col ) {
		// These variables are not strictly necessary, but they make the
		// calculation easier to parse.
		int drain_amount = static_cast<int>(col)*drain_percent/100 + drain_constant;
		unsigned newrow = std::max(1, static_cast<int>(row) + drain_amount);
		xfer(dst, src, newrow, 0, row, col, prob);
	}

	// The remaining columns use the specified drainmax.
	unsigned newrow = std::max(1, static_cast<int>(row) + drainmax);
	for ( ; col < cols_; ++col )
		xfer(dst, src, newrow, col - damage, row, col, prob);
}

/**
 * Transfers a portion (value * prob) of each column in a plane to another.
 * Each column in the @a src plane gets shifted @a damage columns towards 0, and
 * also shifted into the @a dst plane. In addition, the rows can shift if
 * @a drain constant or @a drain_percent is nonzero.
 */
void prob_matrix::shift_cols(unsigned dst, unsigned src, unsigned damage,
                             double prob, int drain_constant, int drain_percent)
{
	int drainmax = (drain_percent*(static_cast<signed>(damage))/100+drain_constant);

	if(drain_constant || drain_percent) {
		debug(("Drains %i (%i%% of %u plus %i)\n", drainmax, drain_percent, damage, drain_constant));
	}

	// Damage cannot effectively exceed the maximum value.
	if (damage >= cols_)
		damage = cols_ - 1;

	// Loop downwards if we drain positive, but upwards if we drain negative,
	// so we write behind us (for when src == dst).
	if(drainmax > 0) {
		for (unsigned row = rows_ - 1; row > min_row_[src]; --row)
			shift_cols_in_row(dst, src, row, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	} else {
		for (unsigned row = min_row_[src] + 1; row < rows_; ++row)
			shift_cols_in_row(dst, src, row, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	}
}

/**
 * Transfers a portion (value * prob) of the values in a column to another.
 * Part of shift_rows(). @a damage is assumed to be less than rows_.
 */
void prob_matrix::shift_rows_in_col(unsigned dst, unsigned src, unsigned col,
                                    unsigned damage, double prob, int drainmax,
                                    int drain_constant, int drain_percent)
{
	// Column 0 is already dead, so skip it.
	unsigned row = std::max(1u, min_row_[src]);

	// Killing blows can have different drain amounts, so handle them first
	for ( ; row < damage; ++row ) {
		// These variables are not strictly necessary, but they make the
		// calculation easier to parse.
		int drain_amount = static_cast<int>(row)*drain_percent/100 + drain_constant;
		unsigned newcol = std::max(1, static_cast<int>(col) + drain_amount);
		xfer(dst, src, 0, newcol, row, col, prob);
	}

	// The remaining columns use the specified drainmax.
	unsigned newcol = std::max(1, static_cast<int>(col) + drainmax);
	for ( ; row < rows_; ++row )
		xfer(dst, src, row - damage, newcol, row, col, prob);
}

/**
 * Transfers a portion (value * prob) of each row in a plane to another.
 * Each row in the @a src plane gets shifted @a damage columns towards 0, and
 * also shifted into the @a dst plane. In addition, the columns can shift if
 * @a drain constant or @a drain_percent is nonzero.
 */
void prob_matrix::shift_rows(unsigned dst, unsigned src, unsigned damage,
                             double prob, int drain_constant, int drain_percent)
{
	int drainmax = (drain_percent*(static_cast<signed>(damage))/100+drain_constant);

	if(drain_constant || drain_percent) {
		debug(("Drains %i (%i%% of %u plus %i)\n", drainmax, drain_percent, damage, drain_constant));
	}

	// Damage cannot effectively exceed the maximum value.
	if (damage >= rows_)
		damage = rows_ - 1;

	// Loop downwards if we drain positive, but upwards if we drain negative,
	// so we write behind us (for when src == dst).
	if(drainmax > 0) {
		for (unsigned col = cols_ - 1; col > min_col_[src]; --col)
			shift_rows_in_col(dst, src, col, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	} else {
		for (unsigned col = min_col_[src] + 1; col < cols_; ++col)
			shift_rows_in_col(dst, src, col, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	}
}

/**
 * What is the chance that an indicated combatant (one of them) is at zero?
 */
double prob_matrix::prob_of_zero(bool check_a, bool check_b) const
{
	double prob = 0.0;

	for (unsigned p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;
		// Column 0 is where b is dead.
		if ( check_b )
			for (unsigned row = min_row_[p]; row < rows_; ++row)
				prob += val(p, row, 0);
		// Row 0 is where a is dead.
		if ( check_a )
			for (unsigned col = min_col_[p]; col < cols_; ++col)
				prob += val(p, 0, col);
		// Theoretically, if checking both, we should subtract the chance that
		// both are dead, but that chance is zero, so don't worry about it.
	}
	return prob;
}

#if defined(CHECK) && defined(ATTACK_PREDICTION_DEBUG)
void prob_matrix::dump() const
{
	unsigned int row, col, m;
	const char *names[]
		= { "NEITHER_SLOWED", "A_SLOWED", "B_SLOWED", "BOTH_SLOWED" };

	for (m = 0; m < 4; ++m) {
		if ( !plane_used(m) )
			continue;
		debug(("%s:\n", names[m]));
		for (row = 0; row < rows_; ++row) {
			debug(("  "));
			for (col = 0; col < cols_; ++col)
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


/**
 * A matrix for calculating the outcome of combat.
 * This class is concerned with translating things that happen in combat
 * to matrix operations that then get passed on to the (private) matrix
 * implementation.
 */
class combat_matrix : private prob_matrix
{
public:
	// Simple matrix, both known HP.
	combat_matrix(unsigned int a_max_hp, unsigned int b_max_hp,
				bool a_slows, bool b_slows,
				unsigned int a_hp, unsigned int b_hp,
				const std::vector<double> a_summary[2],
				const std::vector<double> b_summary[2]);

	~combat_matrix() {}

	// A hits B.
	void receive_blow_b(unsigned damage, unsigned slow_damage, double hit_chance,
						bool a_slows, int a_drain_constant, int a_drain_percent);

	// B hits A.  Why can't they just get along?
	void receive_blow_a(unsigned damage, unsigned slow_damage, double hit_chance,
						bool b_slows, int b_drain_constant, int b_drain_percent);

	// We lied: actually did less damage, adjust matrix.
	void remove_petrify_distortion_a(unsigned damage, unsigned slow_damage, unsigned b_hp);
	void remove_petrify_distortion_b(unsigned damage, unsigned slow_damage, unsigned a_hp);

	void forced_levelup_a();
	void conditional_levelup_a();

	void forced_levelup_b();
	void conditional_levelup_b();

	// Its over, and here's the bill.
	void extract_results(std::vector<double> summary_a[2],
						 std::vector<double> summary_b[2]);

	/// What is the chance that one of the combatants is dead?
	double dead_prob() const    { return prob_of_zero(true, true); }
	/// What is the chance that combatant 'a' is dead?
	double dead_prob_a() const  { return prob_of_zero(true, false); }
	/// What is the chance that combatant 'b' is dead?
	double dead_prob_b() const  { return prob_of_zero(false, true); }

	void dump() const { prob_matrix::dump(); }
};


/**
 * Constructor.
 * @param  a_max_hp       The maximum hit points for A.
 * @param  b_max_hp       The maximum hit points for B.
 * @param  a_slows        Set to true if A slows B when A hits B.
 * @param  b_slows        Set to true if B slows A when B hits A.
 * @param  a_hp           The current hit points for A. (Ignored if a_summary[0] is not empty.)
 * @param  b_hp           The current hit points for B. (Ignored if b_summary[0] is not empty.)
 * @param  a_summary      The hit point distribution for A (from previous combats). Element [0] is for normal A. while [1] is for slowed A.
 * @param  b_summary      The hit point distribution for B (from previous combats). Element [0] is for normal B. while [1] is for slowed B.
 */
combat_matrix::combat_matrix(unsigned int a_max_hp, unsigned int b_max_hp,
                             bool a_slows, bool b_slows,
                             unsigned int a_hp, unsigned int b_hp,
                             const std::vector<double> a_summary[2],
                             const std::vector<double> b_summary[2])
	// The inversion of the order of the *_slows parameters here is intentional.
	: prob_matrix(a_max_hp, b_max_hp, b_slows, a_slows, a_hp, b_hp, a_summary, b_summary)
{
}

// Shift combat_matrix to reflect probability 'hit_chance'
// that damage (up to) 'damage' is done to 'b'.
void combat_matrix::receive_blow_b(unsigned damage, unsigned slow_damage, double hit_chance,
								 bool a_slows, int a_drain_constant, int a_drain_percent)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned int actual_damage;

		if ( !plane_used(src) )
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

		shift_cols(dst, src, actual_damage, hit_chance, a_drain_constant, a_drain_percent);
		if ( min_col(src) < damage )
			set_min_col(dst, 0);
		else if ( min_col(src) - damage < min_col(dst) )
			set_min_col(dst, min_col(src) - damage);
		if ( min_row(src) < min_row(dst) )
			set_min_row(dst, min_row(src));

		int drain_worst = std::min<int>(a_drain_percent*(static_cast<signed>(actual_damage))/100+a_drain_constant, a_drain_constant);
		if(drain_worst < 0) {
			unsigned int max_drain_damage = static_cast<unsigned>(-drain_worst);
			if ( max_drain_damage >= min_row(dst) )
				set_min_row(dst, 0);
			else
				set_min_row(dst, min_row(dst) - max_drain_damage);
		}
	}
}

// Shift matrix to reflect probability 'hit_chance'
// that damage (up to) 'damage' is done to 'a'.
void combat_matrix::receive_blow_a(unsigned damage, unsigned slow_damage, double hit_chance,
								 bool b_slows, int b_drain_constant, int b_drain_percent)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned actual_damage;

		if ( !plane_used(src) )
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

		shift_rows(dst, src, actual_damage, hit_chance, b_drain_constant, b_drain_percent);
		if ( min_row(src) < damage )
			set_min_row(dst, 0);
		else if ( min_row(src) - damage < min_row(dst) )
			set_min_row(dst, min_row(src) - damage);
		if ( min_col(src) < min_col(dst) )
			set_min_col(dst, min_col(src));

		int drain_worst = std::min<int>(b_drain_percent*(static_cast<signed>(actual_damage))/100+b_drain_constant, b_drain_constant);
		if(drain_worst < 0) {
			unsigned int max_drain_damage = static_cast<unsigned>(-drain_worst);
			if ( max_drain_damage >= min_col(dst) )
				set_min_col(dst, 0);
			else
				set_min_col(dst, min_col(dst) - max_drain_damage);
		}
	}
}

// We lied: actually did less damage, adjust matrix.
void combat_matrix::remove_petrify_distortion_a(unsigned damage, unsigned slow_damage,
                                                unsigned b_hp)
{
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;

		// A is slow in planes 1 and 3.
		if (p & 1) {
			if (b_hp > slow_damage)
				for (unsigned int row = 0; row < num_rows(); ++row)
					xfer(p, p, row, b_hp - slow_damage, row, 0, 1.0);
		} else {
			if (b_hp > damage)
				for (unsigned int row = 0; row < num_rows(); ++row)
					xfer(p, p, row, b_hp - damage, row, 0, 1.0);
		}
	}
}

void combat_matrix::remove_petrify_distortion_b(unsigned damage, unsigned slow_damage,
                                                unsigned a_hp)
{
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;

		// B is slow in planes 2 and 3.
		if (p & 2) {
			if (a_hp > slow_damage)
				for (unsigned int col = 0; col < num_cols(); ++col)
					xfer(p, p, a_hp - slow_damage, col, 0, col, 1.0);
		} else {
			if (a_hp > damage)
				for (unsigned int col = 0; col < num_cols(); ++col)
					xfer(p, p, a_hp - damage, col, 0, col, 1.0);
		}
	}
}

void combat_matrix::forced_levelup_a()
{
	/* Move all the values (except 0hp) of all the planes to the last
	   row of the planes unslowed for A. */
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;
		for (unsigned row = std::max(min_row(p), 1u); row < num_rows(); ++row) {
			for (unsigned col = min_col(p); col < num_cols(); ++col) {
				double v = val(p, row, col);
				val(p, row, col) = 0;
				val(p & -2, num_rows() - 1, col) += v;
			}
		}
	}
}

void combat_matrix::forced_levelup_b()
{
	/* Move all the values (except 0hp) of all the planes to the last
	   column of planes unslowed for B. */
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;
		for (unsigned row = min_row(p); row < num_rows(); ++row) {
			for (unsigned col = std::max(min_col(p), 1u); col < num_cols(); ++col) {
				double v = val(p, row, col);
				val(p, row, col) = 0;
				val(p & -3, row, num_cols() - 1) += v;
			}
		}
	}
}

void combat_matrix::conditional_levelup_a()
{
	/* Move the values of the first column (except 0hp) of all the
	   planes to the last row of the planes unslowed for A. */
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;
		for (unsigned row = std::max(min_row(p), 1u); row < num_rows(); ++row) {
			double v = val(p, row, 0);
			val(p, row, 0) = 0;
			val(p & -2, num_rows() - 1, 0) += v;
		}
	}
}

void combat_matrix::conditional_levelup_b()
{
	/* Move the values of the first row (except 0hp) of all the
	   planes to the last column of the planes unslowed for B. */
	for (int p = 0; p < 4; ++p) {
		if ( !plane_used(p) )
			continue;
		for (unsigned col = std::max(min_col(p), 1u); col < num_cols(); ++col) {
			double v = val(p, 0, col);
			val(p, 0, col) = 0;
			val(p & -3, 0, num_cols() - 1) += v;
		}
	}
}

void combat_matrix::extract_results(std::vector<double> summary_a[2],
								  std::vector<double> summary_b[2])
{
	unsigned int p, row, col;

	summary_a[0] = std::vector<double>(num_rows());
	summary_b[0] = std::vector<double>(num_cols());

	if ( plane_used(A_SLOWED) )
		summary_a[1] = std::vector<double>(num_rows());
	if ( plane_used(B_SLOWED) )
		summary_b[1] = std::vector<double>(num_cols());

	for (p = 0; p < 4; ++p) {
		int dst_a, dst_b;
		if ( !plane_used(p) )
			continue;

		// A is slow in planes 1 and 3.
		dst_a = (p & 1);
		// B is slow in planes 2 and 3.
		dst_b = !!(p & 2);
		for (row = 0; row < num_rows(); ++row) {
			for (col = 0; col < num_cols(); ++col) {
				summary_a[dst_a][row] += val(p, row, col);
				summary_b[dst_b][col] += val(p, row, col);
			}
		}
	}
}

} // end anon namespace

unsigned combatant::hp_dist_size(const battle_context_unit_stats &u, const combatant *prev)
{
	// Our summary must be as big as previous one.
	if (prev) {
		return prev->hp_dist.size();
	}

	// If this unit drains, HP can increase, so alloc full array.
	// Do this anyway in case we level up.
	return u.max_hp + 1;
}

combatant::combatant(const battle_context_unit_stats &u, const combatant *prev)
	: hp_dist(hp_dist_size(u, prev)),
	  untouched(0.0),
	  poisoned(0.0),
	  slowed(0.0),
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

// Copy constructor (except use this copy of battle_context_unit_stats)
combatant::combatant(const combatant &that, const battle_context_unit_stats &u)
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
	for (i = 1; i <= u_.max_hp; ++i) {
		double prob = 0.0;
		if(i < summary[0].size()) {
			prob = summary[0][i];
		}
		if (!summary[1].empty())
			prob += summary[1][i];
		for (unsigned int j = 0; j < u_.swarm_min + (u_.swarm_max -
                u_.swarm_min) * i / u_.max_hp; ++j)

			hit_chances_[j] += prob * u_.chance_to_hit / 100.0 / alive_prob;
	}

	debug(("\nhit_chances_ (base %u%%):", u_.chance_to_hit));
	for (i = 0; i < u_.swarm_max; ++i)
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
	for (i = 0; summary[0][i] == 0; ++i) {};
	return i;
}

static void forced_levelup(std::vector<double> &hp_dist)
{
	/* If we survive the combat, we will level up. So the probability
	   of death is unchanged, but all other cases get merged into the
	   fully healed case. */
	std::vector<double>::iterator i = hp_dist.begin();
	++i; // Skip to the second value.
	for (; i != hp_dist.end(); ++i) *i = 0;
	// Full heal unless dead.
	hp_dist.back() = 1 - hp_dist.front();
}

static void conditional_levelup(std::vector<double> &hp_dist, double kill_prob)
{
	/* If we kill, we will level up. So then the damage we had becomes
	   less probable since it's now conditional on us not levelling up.
	   This doesn't apply to the probability of us dying, of course. */
	double scalefactor = 0;
	const double chance_to_survive = 1 - hp_dist.front();
	if(chance_to_survive > DBL_MIN) scalefactor = 1 - kill_prob / chance_to_survive;
	std::vector<double>::iterator i = hp_dist.begin();
	++i; // Skip to the second value.
	for (; i != hp_dist.end(); ++i) *i *= scalefactor;
	// Full heal if leveled up.
	hp_dist.back() += kill_prob;
}

// Combat without chance of death, berserk, slow or drain is simple.
void combatant::no_death_fight(combatant &opp, bool levelup_considered,
                               double & self_not_hit, double & opp_not_hit)
{
	// If opponent was killed in an earlier fight, they don't get to attack.
	double opp_alive_prob = opp.summary[0].empty() ? 1.0 : 1.0 - opp.summary[0][0];
	if (summary[0].empty()) {
		// Starts with a known HP, so Pascal's triangle.
		summary[0] = std::vector<double>(u_.max_hp+1);
		summary[0][u_.hp] = 1.0;
		for (unsigned int i = 0; i < opp.hit_chances_.size(); ++i) {
			const double hit_chance = opp.hit_chances_[i] * opp_alive_prob;
			for (int j = i; j >= 0; j--) {
				double move = summary[0][u_.hp - j * opp.u_.damage] * hit_chance;
				summary[0][u_.hp - j * opp.u_.damage] -= move;
				summary[0][u_.hp - (j+1) * opp.u_.damage] += move;
			}
			self_not_hit *= 1.0 - hit_chance;
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < opp.hit_chances_.size(); ++i) {
			const double hit_chance = opp.hit_chances_[i] * opp_alive_prob;
			for (unsigned int j = opp.u_.damage; j <= u_.hp; ++j) {
				double move = summary[0][j] * hit_chance;
				summary[0][j] -= move;
				summary[0][j - opp.u_.damage] += move;
			}
			self_not_hit *= 1.0 - hit_chance;
		}
	}

	// If we were killed in an earlier fight, we don't get to attack.
	// (Most likely case: we are a first striking defender subject to a series
	// of attacks.)
	double alive_prob = summary[0].empty() ? 1.0 : 1.0 - summary[0][0];
	if (opp.summary[0].empty()) {
		// Starts with a known HP, so Pascal's triangle.
		opp.summary[0] = std::vector<double>(opp.u_.max_hp+1);
		opp.summary[0][opp.u_.hp] = 1.0;
		for (unsigned int i = 0; i < hit_chances_.size(); ++i) {
			const double hit_chance = hit_chances_[i] * alive_prob;
			for (int j = i; j >= 0; j--) {
				double move = opp.summary[0][opp.u_.hp - j * u_.damage] * hit_chance;
				opp.summary[0][opp.u_.hp - j * u_.damage] -= move;
				opp.summary[0][opp.u_.hp - (j+1) * u_.damage] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < hit_chances_.size(); ++i) {
			const double hit_chance = hit_chances_[i] * alive_prob;
			for (unsigned int j = u_.damage; j <= opp.u_.hp; ++j) {
				double move = opp.summary[0][j] * hit_chance;
				opp.summary[0][j] -= move;
				opp.summary[0][j - u_.damage] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	}

	if (!levelup_considered) return;

	if (u_.experience + opp.u_.level >= u_.max_experience) {
		forced_levelup(summary[0]);
	}

	if (opp.u_.experience + u_.level >= opp.u_.max_experience) {
		forced_levelup(opp.summary[0]);
	}
}

// Combat with <= 1 strike each is simple, too.
void combatant::one_strike_fight(combatant &opp, bool levelup_considered,
                                 double & self_not_hit, double & opp_not_hit)
{
	// If we were killed in an earlier fight, we don't get to attack.
	// (Most likely case: we are a first striking defender subject to a series
	// of attacks.)
	double alive_prob = summary[0].empty() ? 1.0 : 1.0 - summary[0][0];
	if (opp.summary[0].empty()) {
		opp.summary[0] = std::vector<double>(opp.u_.max_hp+1);
		if (hit_chances_.size() == 1) {
			const double hit_chance = hit_chances_[0] * alive_prob;
			opp.summary[0][opp.u_.hp] = 1.0 - hit_chance;
			opp.summary[0][std::max<int>(opp.u_.hp - u_.damage, 0)] = hit_chance;
			opp_not_hit *= 1.0 - hit_chance;
		} else {
			assert(hit_chances_.empty());
			opp.summary[0][opp.u_.hp] = 1.0;
		}
	} else {
		if (hit_chances_.size() == 1) {
			const double hit_chance = hit_chances_[0] * alive_prob;
			for (unsigned int i = 1; i < opp.summary[0].size(); ++i) {
				double move = opp.summary[0][i] * hit_chance;
				opp.summary[0][i] -= move;
				opp.summary[0][std::max<int>(i - u_.damage, 0)] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	}

	// If we killed opponent, it won't attack us.
	double opp_alive_prob = 1.0 - opp.summary[0][0] / alive_prob;
	if (summary[0].empty()) {
		summary[0] = std::vector<double>(u_.max_hp+1);
		if (opp.hit_chances_.size() == 1) {
			const double hit_chance = opp.hit_chances_[0] * opp_alive_prob;
			summary[0][u_.hp] = 1.0 - hit_chance;
			summary[0][std::max<int>(u_.hp - opp.u_.damage, 0)] = hit_chance;
			self_not_hit *= 1.0 - hit_chance;
		} else {
			assert(opp.hit_chances_.empty());
			summary[0][u_.hp] = 1.0;
		}
	} else {
		if (opp.hit_chances_.size() == 1) {
			const double hit_chance = opp.hit_chances_[0] * opp_alive_prob;
			for (unsigned int i = 1; i < summary[0].size(); ++i) {
				double move = summary[0][i] * hit_chance;
				summary[0][i] -= move;
				summary[0][std::max<int>(i - opp.u_.damage, 0)] += move;
			}
			self_not_hit *= 1.0 - hit_chance;
		}
	}

	if (!levelup_considered) return;

	if (u_.experience + opp.u_.level >= u_.max_experience) {
		forced_levelup(summary[0]);
	} else if (u_.experience + game_config::kill_xp(opp.u_.level) >= u_.max_experience) {
		conditional_levelup(summary[0], opp.summary[0][0]);
	}

	if (opp.u_.experience + u_.level >= opp.u_.max_experience) {
		forced_levelup(opp.summary[0]);
	} else if (opp.u_.experience + game_config::kill_xp(u_.level) >= opp.u_.max_experience) {
		conditional_levelup(opp.summary[0], summary[0][0]);
	}
}

void combatant::complex_fight(combatant &opp, unsigned rounds, bool levelup_considered,
                              double & self_not_hit, double & opp_not_hit)
{
	combat_matrix m(hp_dist.size()-1, opp.hp_dist.size()-1,
	                u_.slows && !opp.u_.is_slowed, opp.u_.slows && !u_.is_slowed,
	                u_.hp, opp.u_.hp, summary, opp.summary);

	unsigned max_attacks = std::max(hit_chances_.size(), opp.hit_chances_.size());

	debug(("A gets %zu attacks, B %zu\n", hit_chances_.size(), opp.hit_chances_.size()));

	unsigned int a_damage = u_.damage, a_slow_damage = u_.slow_damage;
	unsigned int b_damage = opp.u_.damage, b_slow_damage = opp.u_.slow_damage;

	// To simulate stoning, we set to amount which kills, and re-adjust after.
	/** @todo FIXME: This doesn't work for rolling calculations, just first battle. */
	if (u_.petrifies)
		a_damage = a_slow_damage = opp.u_.max_hp;
	if (opp.u_.petrifies)
		b_damage = b_slow_damage = u_.max_hp;

	do {
		for (unsigned int i = 0; i < max_attacks; ++i) {
			if (i < hit_chances_.size()) {
				debug(("A strikes\n"));
				m.receive_blow_b(a_damage, a_slow_damage, hit_chances_[i],
								 u_.slows && !opp.u_.is_slowed, u_.drain_constant, u_.drain_percent);
				m.dump();
				opp_not_hit *= 1.0 - hit_chances_[i]*(1.0-m.dead_prob_a());
			}
			if (i < opp.hit_chances_.size()) {
				debug(("B strikes\n"));
				m.receive_blow_a(b_damage, b_slow_damage, opp.hit_chances_[i],
								 opp.u_.slows && !u_.is_slowed, opp.u_.drain_constant, opp.u_.drain_percent);
				m.dump();
				self_not_hit *= 1.0 - opp.hit_chances_[i]*(1.0-m.dead_prob_b());
			}
		}

		debug(("Combat ends:\n"));
		m.dump();
	} while (--rounds && m.dead_prob() < 0.99);

	if (u_.petrifies)
		m.remove_petrify_distortion_a(u_.damage, u_.slow_damage, opp.u_.hp);
	if (opp.u_.petrifies)
		m.remove_petrify_distortion_b(opp.u_.damage, opp.u_.slow_damage, u_.hp);

	if (levelup_considered) {
		if (u_.experience + opp.u_.level >= u_.max_experience) {
			m.forced_levelup_a();
		} else if (u_.experience + game_config::kill_xp(opp.u_.level) >= u_.max_experience) {
			m.conditional_levelup_a();
		}

		if (opp.u_.experience + u_.level >= opp.u_.max_experience) {
			m.forced_levelup_b();
		} else if (opp.u_.experience + game_config::kill_xp(u_.level) >= opp.u_.max_experience) {
			m.conditional_levelup_b();
		}
	}

	// We extract results separately, then combine.
	m.extract_results(summary, opp.summary);
}

// Two man enter.  One man leave!
// ... Or maybe two.  But definitely not three.
// Of course, one could be a woman.  Or both.
// And neither could be human, too.
// Um, ok, it was a stupid thing to say.
void combatant::fight(combatant &opp, bool levelup_considered)
{
	unsigned int rounds = std::max<unsigned int>(u_.rounds, opp.u_.rounds);

	// If defender has firststrike and we don't, reverse.
	if (opp.u_.firststrike && !u_.firststrike) {
		opp.fight(*this, levelup_considered);
		return;
	}

#ifdef ATTACK_PREDICTION_DEBUG
	printf("A:\n");
	dump(u_);
	printf("B:\n");
	dump(opp.u_);
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

	// The chance so far of not being hit this combat:
	double self_not_hit = 1.0;
	double opp_not_hit = 1.0;

	// Optimize the simple cases.
	if (rounds == 1 && !u_.slows && !opp.u_.slows &&
		!u_.drains && !opp.u_.drains && !u_.petrifies && !opp.u_.petrifies &&
		summary[1].empty() && opp.summary[1].empty()) {
		if (hit_chances_.size() <= 1 && opp.hit_chances_.size() <= 1) {
			one_strike_fight(opp, levelup_considered, self_not_hit, opp_not_hit);
		} else if (hit_chances_.size() * u_.damage < opp.min_hp() &&
			opp.hit_chances_.size() * opp.u_.damage < min_hp()) {
			no_death_fight(opp, levelup_considered, self_not_hit, opp_not_hit);
		} else {
			complex_fight(opp, rounds, levelup_considered, self_not_hit, opp_not_hit);
		}
	} else {
			complex_fight(opp, rounds, levelup_considered, self_not_hit, opp_not_hit);
	}

#if 0
	assert(summary[0].size() == res.size());
	assert(opp.summary[0].size() == opp_res.size());
	for (unsigned int i = 0; i < summary[0].size(); ++i) {
		if (fabs(summary[0][i] - res[i]) > 0.000001) {
			std::cerr << "Mismatch for " << i << " hp: " << summary[0][i] << " should have been " << res[i] << "\n";
			assert(0);
		}
	}
	for (unsigned int i = 0; i < opp.summary[0].size(); ++i) {
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
		for (unsigned int i = 0; i < hp_dist.size(); ++i)
			hp_dist[i] = summary[0][i] + summary[1][i];
	}
	if (opp.summary[1].empty())
		opp.hp_dist = opp.summary[0];
	else {
		for (unsigned int i = 0; i < opp.hp_dist.size(); ++i)
			opp.hp_dist[i] = opp.summary[0][i] + opp.summary[1][i];
	}

	// Chance that we / they were touched this time.
	double touched = 1.0 - self_not_hit;
	double opp_touched = 1.0 - opp_not_hit;
	if (opp.u_.poisons)
		poisoned += (1 - poisoned) * touched;
	if (u_.poisons)
		opp.poisoned += (1 - opp.poisoned) * opp_touched;

	if (opp.u_.slows)
		slowed += (1 - slowed) * touched;
	if (u_.slows)
		opp.slowed += (1 - opp.slowed) * opp_touched;

	untouched *= self_not_hit;
	opp.untouched *= opp_not_hit;
}

double combatant::average_hp(unsigned int healing) const
{
	double total = 0;

	// Since sum of probabilities is 1.0, we can just tally weights.
	for (unsigned int i = 1; i < hp_dist.size(); ++i) {
		total += hp_dist[i] * std::min<unsigned int>(i + healing, u_.max_hp);
	}
	return total;
}

	/* ** The stand-alone program ** */

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


#ifdef HUMAN_READABLE
void combatant::print(const char label[], unsigned int battle, unsigned int fighter) const
{
	printf("#%06u: (%02u) %s%*c %u-%d; %uhp; %02u%% to hit; %.2f%% unscathed; ",
	       battle, fighter, label, int(strlen(label))-12, ':',
	       u_.swarm_max, u_.damage, u_.hp, u_.chance_to_hit, untouched * 100.0);
	if ( u_.drains )
		printf("drains,");
	if ( u_.slows )
		printf("slows,");
	if ( u_.rounds > 1 )
		printf("berserk,");
	if ( u_.swarm_max != u_.swarm_min )
		printf("swarm,");
	if ( u_.firststrike )
		printf("firststrike,");
	printf("maxhp=%zu ", hp_dist.size()-1);

	int num_outputs = 0;
	for ( unsigned int i = 0; i < hp_dist.size(); ++i )
		if ( hp_dist[i] != 0.0 )
		{
			if ( num_outputs++ % 6 == 0 )
				printf("\n\t");
			else
				printf("  ");
			printf("%2u: %5.2f", i, hp_dist[i] * 100);
		}

	printf("\n");
}
#elif defined(CHECK)
void combatant::print(const char label[], unsigned int battle, unsigned int /*fighter*/) const
{
	printf("#%u: %s: %d %u %u %2g%% ", battle, label,
		   u_.damage, u_.swarm_max, u_.hp, static_cast<float>(u_.chance_to_hit));
	if ( u_.drains )
		printf("drains,");
	if ( u_.slows )
		printf("slows,");
	if ( u_.rounds > 1 )
		printf("berserk,");
	if ( u_.swarm_max != u_.swarm_min )
		printf("swarm,");
	if ( u_.firststrike )
		printf("firststrike,");
	printf("maxhp=%zu ", hp_dist.size()-1);
	printf(" %.2f", untouched);
	for (unsigned int i = 0; i < hp_dist.size(); ++i)
		printf(" %.2f", hp_dist[i] * 100);
	printf("\n");
}
#else  // ... BENCHMARK
void combatant::print(const char /*label*/[], unsigned int /*battle*/, unsigned int /*fighter*/) const
{
}
#endif

void combatant::reset()
{
	for ( unsigned int i = 0; i < hp_dist.size(); ++i )
		hp_dist[i] = 0.0;
	untouched = 1.0;
	poisoned = u_.is_poisoned ? 1.0 : 0.0;
	slowed = u_.is_slowed ? 1.0 : 0.0;
	hit_chances_ = std::vector<double>(u_.num_blows, u_.chance_to_hit / 100.0);
	summary[0] = std::vector<double>();
	summary[1] = std::vector<double>();
}


static void run(unsigned specific_battle)
{
	// N^2 battles
	struct battle_context_unit_stats *stats[NUM_UNITS];
	struct combatant *u[NUM_UNITS];
	unsigned int i, j, k, battle = 0;
	struct timeval start, end, total;

	for (i = 0; i < NUM_UNITS; ++i) {
		unsigned hp = 1 + ((i*3)%23);
		stats[i] = new battle_context_unit_stats(i%7 + 2,        // damage
		                                         i%4 + 1,        // number of strikes
		                                         hp, hp + (i+7)%17,
		                                         30 + (i%6)*10,  // hit chance
		                                         i % 9 == 0,     // drains
		                                         i % 8 == 0,     // slows
		                                         false,          // slowed
		                                         i % 5 == 0,     // berserk
		                                         (i+3) % 5 == 0, // firststrike
		                                         (i+4) % 4 == 0);// swarm
		u[i] = new combatant(*stats[i]);
	}

	gettimeofday(&start, NULL);
	// Go through all fights with two attackers (j and k attacking i).
	for (i = 0; i < NUM_UNITS; ++i) {
		for (j = 0; j < NUM_UNITS; ++j) {
			if (i == j)
				continue;
			for (k = 0; k < NUM_UNITS; ++k) {
				if (i == k || j == k)
					continue;
				++battle;
				if (specific_battle && battle != specific_battle)
					continue;
				// Fight!
				u[j]->fight(*u[i]);
				u[k]->fight(*u[i]);
				// Results.
				u[i]->print("Defender", battle, i+1);
				u[j]->print("Attacker #1", battle, j+1);
				u[k]->print("Attacker #2", battle, k+1);
				// Start the next fight fresh.
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

	for (i = 0; i < NUM_UNITS; ++i) {
		delete u[i];
		delete stats[i];
	}

	exit(0);
}

// Note: The exits in this function are not really necessary, unless they
//       are intended as (extreme) warnings.
static battle_context_unit_stats *parse_unit(char ***argv)
{
	// There are four required parameters.
	int add_to_argv = 4;
	int damage      = atoi((*argv)[1]);
	int num_attacks = atoi((*argv)[2]);
	int hitpoints   = atoi((*argv)[3]), max_hp = hitpoints;
	int hit_chance  = atoi((*argv)[4]);

	// Parse the optional (fifth) parameter.
	bool drains = false, slows = false, slowed = false, berserk = false,
	     firststrike = false, swarm = false;
	if ((*argv)[5] && atoi((*argv)[5]) == 0) {
		// Optional parameter is present.
		++add_to_argv;

		char *max = strstr((*argv)[5], "maxhp=");
		if (max) {
			max_hp = atoi(max + strlen("maxhp="));
			if ( max_hp < hitpoints ) {
				fprintf(stderr, "maxhp must be at least hitpoints.");
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
	}

	// Update argv.
	*argv += add_to_argv;

	// Construct the stats and return.
	return new battle_context_unit_stats(damage, num_attacks, hitpoints, max_hp,
	                                     hit_chance, drains, slows, slowed,
	                                     berserk, firststrike, swarm);
}

int main(int argc, char *argv[])
{
	battle_context_unit_stats *def_stats, *att_stats[20];
	combatant *def, *att[20];
	unsigned int i;

	if (argc < 3)
		run(argv[1] ? atoi(argv[1]) : 0);

	if (argc < 9) {
		fprintf(stderr, "Usage: %s [<battle>]\n"
		        "\t%s <damage> <attacks> <hp> <hitprob> [drain,slows,slowed,swarm,firststrike,berserk,maxhp=<num>] <damage> <attacks> <hp> <hitprob> [drain,slows,slowed,berserk,firststrike,swarm,maxhp=<num>] ...\n",
				argv[0], argv[0]);
		exit(1);
	}

	def_stats = parse_unit(&argv);
	def = new combatant(*def_stats);
	for (i = 0; argv[1] && i < 19; ++i) {
		att_stats[i] = parse_unit(&argv);
		att[i] = new combatant(*att_stats[i]);
	}
	att[i] = NULL;

	for (i = 0; att[i]; ++i) {
		debug(("Fighting next attacker\n"));
		att[i]->fight(*def);
	}

	def->print("Defender", 0, 0);
	for (i = 0; att[i]; ++i)
		att[i]->print("Attacker", 0, i+1);

	for (i = 0; att[i]; ++i) {
		delete att[i];
		delete att_stats[i];
	}
	delete def;
	delete def_stats;

	return 0;
}
#endif // Standalone program
