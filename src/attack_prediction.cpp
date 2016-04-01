/*
   Copyright (C) 2006 - 2016 by Rusty Russell <rusty@rustcorp.com.au>
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
 * --check \<file name\>).
 * For either option, use -DHUMAN_READABLE if you want to see the results
 * from each combat displayed in a prettier format (but no longer suitable
 * for wesnoth-attack-sim.c).
 */

#include <cfloat>

#include "attack_prediction.hpp"

#include "actions/attack.hpp"
#include "array.hpp"
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
 * A shorthand utility for forcing a value between minimum and maximum values.
 */
template<typename T>
const T& limit(const T& val, const T& min, const T& max)
{
	if ( val < min )
		return min;
	if ( val > max )
		return max;
	return val;
}

/**
 * A matrix of A's hitpoints vs B's hitpoints vs. their slowed states.
 * This class is concerned only with the matrix implementation and
 * implements functionality for shifting and retrieving probabilities
 * (i.e. low-level stuff).
 */
class prob_matrix
{
	// Since this gets used very often (especially by the AI), it has
	// been optimized for speed as a sparse matrix.
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

	/// Move a column (adding it to the destination).
	void move_column(unsigned d_plane, unsigned s_plane,
	                 unsigned d_col, unsigned s_col);
	/// Move a row (adding it to the destination).
	void move_row(unsigned d_plane, unsigned s_plane,
	              unsigned d_row, unsigned s_row);

	// Move values within a row (or column) to a specified column (or row).
	void merge_col(unsigned d_plane, unsigned s_plane, unsigned col, unsigned d_row);
	void merge_cols(unsigned d_plane, unsigned s_plane, unsigned d_row);
	void merge_row(unsigned d_plane, unsigned s_plane, unsigned row, unsigned d_col);
	void merge_rows(unsigned d_plane, unsigned s_plane, unsigned d_col);

	/// What is the chance that an indicated combatant (one of them) is at zero?
	double prob_of_zero(bool check_a, bool check_b) const;
	/// Sums the values in the specified plane.
	void sum(unsigned plane, std::vector<double> & row_sums,
	         std::vector<double> & col_sums) const;

	/// Returns true if the specified plane might have data in it.
	bool plane_used(unsigned p) const { return p < NUM_PLANES  &&  plane_[p] != nullptr; }

	unsigned int num_rows() const { return rows_; }
	unsigned int num_cols() const { return cols_; }

	// Debugging tool.
	void dump() const;

	// We need four matrices, or "planes", reflecting the possible
	// "slowed" states (neither slowed, A slowed, B slowed, both slowed).
	enum {
		NEITHER_SLOWED,
		A_SLOWED,
		B_SLOWED,
		BOTH_SLOWED,
		NUM_PLANES	// Symbolic constant for the number of planes.
	};

private:
	// This gives me 10% speed improvement over std::vector<> (g++4.0.3 x86)
	double *new_plane();

	void initialize_plane(unsigned plane, unsigned a_cur, unsigned b_cur,
	                      const std::vector<double> & a_initial,
	                      const std::vector<double> & b_initial);
	void initialize_row(unsigned plane, unsigned row, double row_prob,
	                    unsigned b_cur, const std::vector<double> & b_initial);

	double &val(unsigned plane, unsigned row, unsigned col);
	const double &val(unsigned plane, unsigned row, unsigned col) const;

	/// Transfers a portion (value * prob) of one value in the matrix to another.
	void xfer(unsigned dst_plane, unsigned src_plane,
			  unsigned row_dst, unsigned col_dst,
			  unsigned row_src, unsigned col_src,
			  double prob);
	/// Transfers one value in the matrix to another.
	void xfer(unsigned dst_plane, unsigned src_plane,
	          unsigned row_dst, unsigned col_dst,
	          unsigned row_src, unsigned col_src);

	void shift_cols_in_row(unsigned dst, unsigned src, unsigned row,
	                       const std::vector<unsigned> & cols,
	                       unsigned damage, double prob, int drainmax,
	                       int drain_constant, int drain_percent);
	void shift_rows_in_col(unsigned dst, unsigned src, unsigned col,
	                       const std::vector<unsigned> & rows,
	                       unsigned damage, double prob, int drainmax,
	                       int drain_constant, int drain_percent);

private: // data
	const unsigned int rows_, cols_;
	util::array<double *, NUM_PLANES> plane_;

	// For optimization, we keep track of the rows and columns with data.
	// (The matrices are likely going to be rather sparse, with data on a grid.)
	util::array<std::set<unsigned>, NUM_PLANES> used_rows_, used_cols_;
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
	: rows_(a_max+1)
	, cols_(b_max+1)
	, plane_()
	, used_rows_()
	, used_cols_()
{
	// Make sure we do not access the matrix in invalid positions.
	a_cur = std::min<unsigned int>(a_cur, rows_ - 1);
	b_cur = std::min<unsigned int>(b_cur, cols_ - 1);

	// It will be convenient to always consider row/col 0 to be used.
	for ( unsigned plane = 0; plane != NUM_PLANES; ++plane ) {
		used_rows_[plane].insert(0u);
		used_cols_[plane].insert(0u);
	}

	// We will need slowed planes if the initial vectors have them.
	need_a_slowed =  need_a_slowed || !a_initial[1].empty();
	need_b_slowed =  need_b_slowed || !b_initial[1].empty();

	// Allocate the needed planes.
	plane_[NEITHER_SLOWED] = new_plane();
	plane_[A_SLOWED] = !need_a_slowed ? nullptr : new_plane();
	plane_[B_SLOWED] = !need_b_slowed ? nullptr : new_plane();
	plane_[BOTH_SLOWED] = !(need_a_slowed && need_b_slowed) ? nullptr : new_plane();

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
		// The probabilities for each row are contained in a_initial.
		for ( unsigned row = 0; row < row_count; ++row ) {
			if ( a_initial[row] != 0.0 ) {
				used_rows_[plane].insert(row);
				initialize_row(plane, row, a_initial[row], b_cur, b_initial);
			}
		}
	}
	else {
		used_rows_[plane].insert(a_cur);
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
		// The probabilities for each column are contained in b_initial.
		for ( unsigned col = 0; col < col_count; ++col ) {
			if ( b_initial[col] != 0.0 ) {
				used_cols_[plane].insert(col);
				val(plane, row, col) = row_prob * b_initial[col];
			}
		}
	}
	else {
		// Only the column indicated by b_cur is a possibility.
		used_cols_[plane].insert(b_cur);
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

		double &dst = val(dst_plane, row_dst, col_dst);
		if ( dst == 0.0 ) {
			// Track that this entry is now used.
			used_rows_[dst_plane].insert(row_dst);
			used_cols_[dst_plane].insert(col_dst);
		}
		dst += diff;

		debug(("Shifted %4.3g from %s(%u,%u) to %s(%u,%u).\n",
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
 * Transfers one value in the matrix to another.
 */
void prob_matrix::xfer(unsigned dst_plane, unsigned src_plane,
                       unsigned row_dst, unsigned col_dst,
                       unsigned row_src, unsigned col_src)
{
	if ( dst_plane == src_plane  &&  row_dst == row_src  &&  col_dst == col_src )
		// Transferring to itself. Nothing to do.
		return;

	double &src = val(src_plane, row_src, col_src);
	if ( src != 0.0 ) {
		debug(("Shifting %4.3g from %s(%u,%u) to %s(%u,%u).\n",
			   src, src_plane == NEITHER_SLOWED ? ""
			   : src_plane == A_SLOWED ? "[A_SLOWED]"
			   : src_plane == B_SLOWED ? "[B_SLOWED]"
			   : src_plane == BOTH_SLOWED ? "[BOTH_SLOWED]" : "INVALID",
			   row_src, col_src,
			   dst_plane == NEITHER_SLOWED ? ""
			   : dst_plane == A_SLOWED ? "[A_SLOWED]"
			   : dst_plane == B_SLOWED ? "[B_SLOWED]"
			   : dst_plane == BOTH_SLOWED ? "[BOTH_SLOWED]" : "INVALID",
			   row_dst, col_dst));

		double &dst = val(dst_plane, row_dst, col_dst);
		if ( dst == 0.0 ) {
			// Track that this entry is now used.
			used_rows_[dst_plane].insert(row_dst);
			used_cols_[dst_plane].insert(col_dst);
		}
		dst += src;
		src = 0.0;
	}
}

/**
 * Transfers a portion (value * prob) of the values in a row to another.
 * Part of shift_cols().
 */
void prob_matrix::shift_cols_in_row(unsigned dst, unsigned src, unsigned row,
                                    const std::vector<unsigned> & cols,
                                    unsigned damage, double prob, int drainmax,
                                    int drain_constant, int drain_percent)
{
	// Some conversions to (signed) int.
	int row_i = static_cast<int>(row);
	int max_row = static_cast<int>(rows_) - 1;

	// cols[0] is excluded since that should be 0, representing already dead.
	unsigned col_x = 1;

	// Killing blows can have different drain amounts, so handle them first
	for ( ; col_x < cols.size()  &&  cols[col_x] < damage; ++col_x ) {
		// These variables are not strictly necessary, but they make the
		// calculation easier to parse.
		int col_i = static_cast<int>(cols[col_x]);
		int drain_amount = col_i*drain_percent/100 + drain_constant;
		unsigned newrow = limit<int>(row_i + drain_amount, 1, max_row);
		xfer(dst, src, newrow, 0, row, cols[col_x], prob);
	}

	// The remaining columns use the specified drainmax.
	unsigned newrow = limit<int>(row_i + drainmax, 1, max_row);
	for ( ; col_x < cols.size(); ++col_x )
		xfer(dst, src, newrow, cols[col_x] - damage, row, cols[col_x], prob);
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

	// Get lists of indices currently used in the source plane.
	// (This needs to be cached since we might add indices while shifting.)
	const std::vector<unsigned> rows(used_rows_[src].begin(), used_rows_[src].end());
	const std::vector<unsigned> cols(used_cols_[src].begin(), used_cols_[src].end());

	// Loop downwards if we drain positive, but upwards if we drain negative,
	// so we write behind us (for when src == dst).
	if(drainmax > 0) {
		// rows[0] is excluded since that should be 0, representing already dead.
		for ( unsigned row_x = rows.size()-1; row_x != 0; --row_x )
			shift_cols_in_row(dst, src, rows[row_x], cols, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	} else {
		// rows[0] is excluded since that should be 0, representing already dead.
		for ( unsigned row_x = 1; row_x != rows.size(); ++row_x )
			shift_cols_in_row(dst, src, rows[row_x], cols, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	}
}

/**
 * Transfers a portion (value * prob) of the values in a column to another.
 * Part of shift_rows().
 */
void prob_matrix::shift_rows_in_col(unsigned dst, unsigned src, unsigned col,
                                    const std::vector<unsigned> & rows,
                                    unsigned damage, double prob, int drainmax,
                                    int drain_constant, int drain_percent)
{
	// Some conversions to (signed) int.
	int col_i = static_cast<int>(col);
	int max_col = static_cast<int>(cols_) - 1;

	// rows[0] is excluded since that should be 0, representing already dead.
	unsigned row_x = 1;

	// Killing blows can have different drain amounts, so handle them first
	for ( ; row_x < rows.size()  &&  rows[row_x] < damage; ++row_x ) {
		// These variables are not strictly necessary, but they make the
		// calculation easier to parse.
		int row_i = static_cast<int>(rows[row_x]);
		int drain_amount = row_i*drain_percent/100 + drain_constant;
		unsigned newcol = limit<int>(col_i + drain_amount, 1, max_col);
		xfer(dst, src, 0, newcol, rows[row_x], col, prob);
	}

	// The remaining rows use the specified drainmax.
	unsigned newcol = limit<int>(col_i + drainmax, 1, max_col);
	for ( ; row_x < rows.size(); ++row_x )
		xfer(dst, src, rows[row_x] - damage, newcol, rows[row_x], col, prob);
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

	// Get lists of indices currently used in the source plane.
	// (This needs to be cached since we might add indices while shifting.)
	const std::vector<unsigned> rows(used_rows_[src].begin(), used_rows_[src].end());
	const std::vector<unsigned> cols(used_cols_[src].begin(), used_cols_[src].end());

	// Loop downwards if we drain positive, but upwards if we drain negative,
	// so we write behind us (for when src == dst).
	if(drainmax > 0) {
		// cols[0] is excluded since that should be 0, representing already dead.
		for ( unsigned col_x = cols.size()-1; col_x != 0; --col_x )
			shift_rows_in_col(dst, src, cols[col_x], rows, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	} else {
		// cols[0] is excluded since that should be 0, representing already dead.
		for ( unsigned col_x = 1; col_x != cols.size(); ++col_x )
			shift_rows_in_col(dst, src, cols[col_x], rows, damage, prob, drainmax,
			                  drain_constant, drain_percent);
	}
}

/**
 * Move a column (adding it to the destination).
 */
void prob_matrix::move_column(unsigned d_plane, unsigned s_plane,
                              unsigned d_col, unsigned s_col)
{
	std::set<unsigned>::const_iterator rows_end = used_rows_[s_plane].end();
	std::set<unsigned>::const_iterator row_it = used_rows_[s_plane].begin();

	// Transfer the data.
	for ( ; row_it != rows_end; ++row_it )
		xfer(d_plane, s_plane, *row_it, d_col, *row_it, s_col);
}

/**
 * Move a row (adding it to the destination).
 */
void prob_matrix::move_row(unsigned d_plane, unsigned s_plane,
                           unsigned d_row, unsigned s_row)
{
	std::set<unsigned>::const_iterator cols_end = used_cols_[s_plane].end();
	std::set<unsigned>::const_iterator col_it = used_cols_[s_plane].begin();

	// Transfer the data.
	for ( ; col_it != cols_end; ++col_it )
		xfer(d_plane, s_plane, d_row, *col_it, s_row, *col_it);
}

/**
 * Move values in the specified column -- excluding row zero -- to the
 * specified row in that column (possibly shifting planes in the process).
 */
void prob_matrix::merge_col(unsigned d_plane, unsigned s_plane, unsigned col,
                            unsigned d_row)
{
	std::set<unsigned>::const_iterator rows_end = used_rows_[s_plane].end();
	std::set<unsigned>::const_iterator row_it = used_rows_[s_plane].begin();

	// Transfer the data, excluding row zero.
	for ( ++row_it; row_it != rows_end; ++row_it )
		xfer(d_plane, s_plane, d_row, col, *row_it, col);
}

/**
 * Move values within columns in the specified plane -- excluding row zero --
 * to the specified row (possibly shifting planes in the process).
 */
void prob_matrix::merge_cols(unsigned d_plane, unsigned s_plane, unsigned d_row)
{
	std::set<unsigned>::const_iterator rows_end = used_rows_[s_plane].end();
	std::set<unsigned>::const_iterator row_it = used_rows_[s_plane].begin();
	std::set<unsigned>::const_iterator cols_end = used_cols_[s_plane].end();
	std::set<unsigned>::const_iterator cols_begin = used_cols_[s_plane].begin();
	std::set<unsigned>::const_iterator col_it;

	// Transfer the data, excluding row zero.
	for ( ++row_it; row_it != rows_end; ++row_it )
		for ( col_it = cols_begin; col_it != cols_end; ++col_it )
			xfer(d_plane, s_plane, d_row, *col_it, *row_it, *col_it);
}

/**
 * Move values in the specified row -- excluding column zero -- to the
 * specified column in that row (possibly shifting planes in the process).
 */
void prob_matrix::merge_row(unsigned d_plane, unsigned s_plane, unsigned row,
                            unsigned d_col)
{
	std::set<unsigned>::const_iterator cols_end = used_cols_[s_plane].end();
	std::set<unsigned>::const_iterator col_it = used_cols_[s_plane].begin();

	// Transfer the data, excluding column zero.
	for ( ++col_it; col_it != cols_end; ++col_it )
		xfer(d_plane, s_plane, row, d_col, row, *col_it);
}


/**
 * Move values within rows in the specified plane -- excluding column zero --
 * to the specified column (possibly shifting planes in the process).
 */
void prob_matrix::merge_rows(unsigned d_plane, unsigned s_plane, unsigned d_col)
{
	std::set<unsigned>::const_iterator rows_end = used_rows_[s_plane].end();
	std::set<unsigned>::const_iterator row_it = used_rows_[s_plane].begin();
	std::set<unsigned>::const_iterator cols_end = used_cols_[s_plane].end();
	std::set<unsigned>::const_iterator cols_begin = used_cols_[s_plane].begin();
	// (excluding column zero)
	++cols_begin;
	std::set<unsigned>::const_iterator col_it;

	// Transfer the data, excluding column zero.
	for ( ; row_it != rows_end; ++row_it )
		for ( col_it = cols_begin; col_it != cols_end; ++col_it )
			xfer(d_plane, s_plane, *row_it, d_col, *row_it, *col_it);
}

/**
 * What is the chance that an indicated combatant (one of them) is at zero?
 */
double prob_matrix::prob_of_zero(bool check_a, bool check_b) const
{
	double prob = 0.0;

	for (unsigned p = 0; p < NUM_PLANES; ++p) {
		if ( !plane_used(p) )
			continue;

		// Column 0 is where b is at zero.
		if ( check_b ) {
			std::set<unsigned>::const_iterator rows_end = used_rows_[p].end();
			std::set<unsigned>::const_iterator row_it = used_rows_[p].begin();
			for ( ; row_it != rows_end; ++row_it )
				prob += val(p, *row_it, 0);
		}
		// Row 0 is where a is at zero.
		if ( check_a ) {
			std::set<unsigned>::const_iterator cols_end = used_cols_[p].end();
			std::set<unsigned>::const_iterator col_it = used_cols_[p].begin();
			for ( ; col_it != cols_end; ++col_it )
				prob += val(p, 0, *col_it);
		}
		// Theoretically, if checking both, we should subtract the chance that
		// both are dead, but that chance is zero, so don't worry about it.
	}
	return prob;
}

/**
 * Sums the values in the specified plane.
 * The sum of each row is added to the corresponding entry in row_sums.
 * The sum of each column is added to the corresponding entry in col_sums.
 */
void prob_matrix::sum(unsigned plane, std::vector<double> & row_sums,
                      std::vector<double> & col_sums) const
{
	std::set<unsigned>::const_iterator rows_end = used_rows_[plane].end();
	std::set<unsigned>::const_iterator row_it = used_rows_[plane].begin();
	std::set<unsigned>::const_iterator cols_end = used_cols_[plane].end();
	std::set<unsigned>::const_iterator cols_begin = used_cols_[plane].begin();
	std::set<unsigned>::const_iterator col_it;

	for ( ; row_it != rows_end; ++row_it )
		for ( col_it = cols_begin; col_it != cols_end; ++col_it ) {
			const double & prob = val(plane, *row_it, *col_it);
			row_sums[*row_it] += prob;
			col_sums[*col_it] += prob;
		}
}

#if defined(CHECK) && defined(ATTACK_PREDICTION_DEBUG)
void prob_matrix::dump() const
{
	unsigned int row, col, m;
	const char *names[]
		= { "NEITHER_SLOWED", "A_SLOWED", "B_SLOWED", "BOTH_SLOWED" };

	for (m = 0; m < NUM_PLANES; ++m) {
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
	combat_matrix(unsigned int a_max_hp, unsigned int b_max_hp,
	              unsigned int a_hp, unsigned int b_hp,
	              const std::vector<double> a_summary[2],
	              const std::vector<double> b_summary[2],
	              bool a_slows, bool b_slows,
	              unsigned int a_damage, unsigned int b_damage,
	              unsigned int a_slow_damage, unsigned int b_slow_damage,
	              int a_drain_percent, int b_drain_percent,
	              int a_drain_constant, int b_drain_constant);

	~combat_matrix() {}

	// A hits B.
	void receive_blow_b(double hit_chance);
	// B hits A.  Why can't they just get along?
	void receive_blow_a(double hit_chance);

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

private:
	unsigned a_max_hp_;
	bool     a_slows_;
	unsigned a_damage_;
	unsigned a_slow_damage_;
	int      a_drain_percent_;
	int      a_drain_constant_;

	unsigned b_max_hp_;
	bool     b_slows_;
	unsigned b_damage_;
	unsigned b_slow_damage_;
	int      b_drain_percent_;
	int      b_drain_constant_;
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
                             unsigned int a_hp, unsigned int b_hp,
                             const std::vector<double> a_summary[2],
                             const std::vector<double> b_summary[2],
                             bool a_slows, bool b_slows,
                             unsigned int a_damage, unsigned int b_damage,
                             unsigned int a_slow_damage, unsigned int b_slow_damage,
                             int a_drain_percent, int b_drain_percent,
                             int a_drain_constant, int b_drain_constant)
	// The inversion of the order of the *_slows parameters here is intentional.
	: prob_matrix(a_max_hp, b_max_hp, b_slows, a_slows, a_hp, b_hp, a_summary, b_summary),

	  a_max_hp_(a_max_hp), a_slows_(a_slows), a_damage_(a_damage), a_slow_damage_(a_slow_damage),
	  a_drain_percent_(a_drain_percent), a_drain_constant_(a_drain_constant),

	  b_max_hp_(b_max_hp), b_slows_(b_slows), b_damage_(b_damage), b_slow_damage_(b_slow_damage),
	  b_drain_percent_(b_drain_percent), b_drain_constant_(b_drain_constant)
{
}

// Shift combat_matrix to reflect the probability 'hit_chance' that damage
// is done to 'b'.
void combat_matrix::receive_blow_b(double hit_chance)
{
	// Walk backwards so we don't copy already-copied matrix planes.
	unsigned src = NUM_PLANES;
	while ( src-- != 0 ) {
		if ( !plane_used(src) )
			continue;

		// If A slows us, we go from 0=>2, 1=>3, 2=>2 3=>3.
		int dst = a_slows_ ? src|2 : src;

		// A is slow in planes 1 and 3.
		unsigned damage = src & 1 ? a_slow_damage_ : a_damage_;

		shift_cols(dst, src, damage, hit_chance, a_drain_constant_, a_drain_percent_);
	}
}

// Shift matrix to reflect probability 'hit_chance'
// that damage (up to) 'damage' is done to 'a'.
void combat_matrix::receive_blow_a(double hit_chance)
{
	// Walk backwards so we don't copy already-copied matrix planes.
	unsigned src = NUM_PLANES;
	while ( src-- != 0 ) {
		if ( !plane_used(src) )
			continue;

		// If B slows us, we go from 0=>1, 1=>1, 2=>3 3=>3.
		int dst = b_slows_ ? src|1 : src;

		// B is slow in planes 2 and 3.
		unsigned damage = src & 2 ? b_slow_damage_ : b_damage_;

		shift_rows(dst, src, damage, hit_chance, b_drain_constant_, b_drain_percent_);
	}
}

// We lied: actually did less damage, adjust matrix.
void combat_matrix::remove_petrify_distortion_a(unsigned damage, unsigned slow_damage,
                                                unsigned b_hp)
{
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( !plane_used(p) )
			continue;

		// A is slow in planes 1 and 3.
		unsigned actual_damage = (p & 1) ? slow_damage : damage;
		if ( b_hp > actual_damage )
			// B was actually petrified, not killed.
			move_column(p, p, b_hp - actual_damage, 0);
	}
}

void combat_matrix::remove_petrify_distortion_b(unsigned damage, unsigned slow_damage,
                                                unsigned a_hp)
{
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( !plane_used(p) )
			continue;

		// B is slow in planes 2 and 3.
		unsigned actual_damage = (p & 2) ? slow_damage : damage;
		if ( a_hp > actual_damage )
			// A was actually petrified, not killed.
			move_row(p, p, a_hp - actual_damage, 0);
	}
}

void combat_matrix::forced_levelup_a()
{
	/* Move all the values (except 0hp) of all the planes to the "fully healed"
	   row of the planes unslowed for A. */
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( plane_used(p) )
			merge_cols(p & -2, p, a_max_hp_);
	}
}

void combat_matrix::forced_levelup_b()
{
	/* Move all the values (except 0hp) of all the planes to the "fully healed"
	   column of planes unslowed for B. */
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( plane_used(p) )
			merge_rows(p & -3, p, b_max_hp_);
	}
}

void combat_matrix::conditional_levelup_a()
{
	/* Move the values of the first column (except 0hp) of all the
	   planes to the "fully healed" row of the planes unslowed for A. */
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( plane_used(p) )
			merge_col(p & -2, p, 0, a_max_hp_);
	}
}

void combat_matrix::conditional_levelup_b()
{
	/* Move the values of the first row (except 0hp) of all the
	   planes to the last column of the planes unslowed for B. */
	for (int p = 0; p < NUM_PLANES; ++p) {
		if ( plane_used(p) )
			merge_row(p & -3, p, 0, b_max_hp_);
	}
}

void combat_matrix::extract_results(std::vector<double> summary_a[2],
                                    std::vector<double> summary_b[2])
{
	// Reset the summaries.
	summary_a[0] = std::vector<double>(num_rows());
	summary_b[0] = std::vector<double>(num_cols());

	if ( plane_used(A_SLOWED) )
		summary_a[1] = std::vector<double>(num_rows());
	if ( plane_used(B_SLOWED) )
		summary_b[1] = std::vector<double>(num_cols());

	for (unsigned p = 0; p < NUM_PLANES; ++p) {
		if ( !plane_used(p) )
			continue;

		// A is slow in planes 1 and 3.
		unsigned dst_a = (p & 1) ? 1u : 0u;
		// B is slow in planes 2 and 3.
		unsigned dst_b = (p & 2) ? 1u : 0u;
		sum(p, summary_a[dst_a], summary_b[dst_b]);
	}
}

} // end anon namespace


/**
 * A struct to describe one possible combat scenario.
 * (Needed when the number of attacks can vary due to swarm.)
 */
struct combatant::combat_slice
{
	// The hit point range this slice covers.
	unsigned begin_hp; // included in the range.
	unsigned end_hp;   // excluded from the range.

	// The probability of this slice.
	double prob;

	// The number of strikes applicable with this slice.
	unsigned strikes;


	combat_slice(const std::vector<double> src_summary[2],
	             unsigned begin, unsigned end, unsigned num_strikes);
	combat_slice(const std::vector<double> src_summary[2], unsigned num_strikes);
};

/**
 * Creates a slice from a summary, and associates a number of strikes.
 */
combatant::combat_slice::combat_slice(const std::vector<double> src_summary[2],
                                      unsigned begin, unsigned end,
                                      unsigned num_strikes) :
	begin_hp(begin),
	end_hp(end),
	prob(0.0),
	strikes(num_strikes)
{
	if ( src_summary[0].empty() ) {
		// No summary; this should be the only slice.
		prob = 1.0;
		return;
	}

	// Avoid accessing beyond the end of the vectors.
	if ( end > src_summary[0].size() )
		end = src_summary[0].size();

	// Sum the probabilities in the slice.
	for ( unsigned i = begin; i < end; ++i )
		prob += src_summary[0][i];
	if ( !src_summary[1].empty() )
		for ( unsigned i = begin; i < end; ++i )
			prob += src_summary[1][i];
}


/**
 * Creates a slice from the summaries, and associates a number of strikes.
 * This version of the constructor creates a slice consisting of everything.
 */
combatant::combat_slice::combat_slice(const std::vector<double> src_summary[2],
                                      unsigned num_strikes) :
	begin_hp(0),
	end_hp(src_summary[0].size()),
	prob(1.0),
	strikes(num_strikes)
{
}


combatant::combatant(const battle_context_unit_stats &u, const combatant *prev)
	: hp_dist(u.max_hp + 1, 0.0),
	  untouched(0.0),
	  poisoned(0.0),
	  slowed(0.0),
	  u_(u)
{
	// We inherit current state from previous combatant.
	if (prev) {
		summary[0] = prev->summary[0];
		summary[1] = prev->summary[1];
		hp_dist = prev->hp_dist;
		untouched = prev->untouched;
		poisoned = prev->poisoned;
		slowed = prev->slowed;
	} else {
		hp_dist[std::min(u.hp, u.max_hp)] = 1.0;
		untouched = 1.0;
		poisoned = u.is_poisoned ? 1.0 : 0.0;
		slowed = u.is_slowed ? 1.0 : 0.0;
	}
}

// Copy constructor (except use this copy of battle_context_unit_stats)
combatant::combatant(const combatant &that, const battle_context_unit_stats &u)
	: hp_dist(that.hp_dist), untouched(that.untouched), poisoned(that.poisoned), slowed(that.slowed), u_(u)
{
		summary[0] = that.summary[0];
		summary[1] = that.summary[1];
}


namespace {
	/**
	 * Returns the number of hit points greater than cur_hp, and at most
	 * stats.max_hp+1, at which the unit would get another attack because
	 * of swarm.
	 * Helper function for split_summary().
	 */
	unsigned hp_for_next_attack(unsigned cur_hp,
	                            const battle_context_unit_stats & stats)
	{
		unsigned old_strikes = stats.calc_blows(cur_hp);

		// A formula would have to deal with rounding issues; instead
		// loop until we find more strikes.
		while ( ++cur_hp <= stats.max_hp )
			if ( stats.calc_blows(cur_hp) != old_strikes )
				break;

		return cur_hp;
	}
} // end anon namespace

/**
 * Split the combat by number of attacks per combatant (for swarm).
 * This also clears the current summaries.
 */
std::vector<combatant::combat_slice> combatant::split_summary() const
{
	std::vector<combat_slice> result;

	if ( u_.swarm_min == u_.swarm_max  ||  summary[0].empty() )
	{
		// We use the same number of blows for all possibilities.
		result.push_back(combat_slice(summary, u_.num_blows));
		return result;
	}

	debug(("Slicing:\n"));
	// Loop through our slices.
	unsigned cur_end = 0;
	do {
		// Advance to the next slice.
		const unsigned cur_begin = cur_end;
		cur_end = hp_for_next_attack(cur_begin, u_);

		// Add this slice.
		combat_slice slice(summary, cur_begin, cur_end, u_.calc_blows(cur_begin));
		if ( slice.prob != 0.0 ) {
			result.push_back(slice);
			debug(("\t%2u-%2u hp; strikes: %u; probability: %6.2f\n",
			       cur_begin, cur_end, slice.strikes, slice.prob*100.0));
		}
	} while ( cur_end <= u_.max_hp );

	return result;
}


namespace {

void forced_levelup(std::vector<double> &hp_dist)
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

void conditional_levelup(std::vector<double> &hp_dist, double kill_prob)
{
	/* If we kill, we will level up. So then the damage we had becomes
	   less probable since it's now conditional on us not leveling up.
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

/**
 * Returns the smallest HP we could possibly have based on the provided
 * hit point distribution.
 */
unsigned min_hp(const std::vector<double> & hp_dist, unsigned def)
{
	const unsigned size = hp_dist.size();

	// Look for a nonzero entry.
	for ( unsigned i = 0; i != size; ++i )
		if ( hp_dist[i] != 0.0 )
			return i;

	// Either the distribution is empty or is full of zeros, so
	// return the default.
	return def;
}

// Combat without chance of death, berserk, slow or drain is simple.
void no_death_fight(const battle_context_unit_stats &stats,
                    const battle_context_unit_stats &opp_stats,
                    unsigned strikes, unsigned opp_strikes,
                    std::vector<double> & hp_dist,
                    std::vector<double> & opp_hp_dist,
                    double & self_not_hit, double & opp_not_hit,
                    bool levelup_considered)
{
	// Our strikes.
	// If we were killed in an earlier fight, we don't get to attack.
	// (Most likely case: we are a first striking defender subject to a series
	// of attacks.)
	const double alive_prob = hp_dist.empty() ? 1.0 : 1.0 - hp_dist[0];
	const double hit_chance = (stats.chance_to_hit/100.0) * alive_prob;
	if ( opp_hp_dist.empty() ) {
		// Starts with a known HP, so Pascal's triangle.
		opp_hp_dist = std::vector<double>(opp_stats.max_hp+1);
		opp_hp_dist[opp_stats.hp] = 1.0;
		for (unsigned int i = 0; i < strikes; ++i) {
			for (int j = i; j >= 0; j--) {
				unsigned src_index = opp_stats.hp - j*stats.damage;
				double move = opp_hp_dist[src_index] * hit_chance;
				opp_hp_dist[src_index] -= move;
				opp_hp_dist[src_index - stats.damage] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < strikes; ++i) {
			for (unsigned int j = stats.damage; j < opp_hp_dist.size(); ++j) {
				double move = opp_hp_dist[j] * hit_chance;
				opp_hp_dist[j] -= move;
				opp_hp_dist[j - stats.damage] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	}

	// Opponent's strikes
	// If opponent was killed in an earlier fight, they don't get to attack.
	const double opp_alive_prob = opp_hp_dist.empty() ? 1.0 : 1.0 - opp_hp_dist[0];
	const double opp_hit_chance = (opp_stats.chance_to_hit/100.0) * opp_alive_prob;
	if ( hp_dist.empty() ) {
		// Starts with a known HP, so Pascal's triangle.
		hp_dist = std::vector<double>(stats.max_hp+1);
		hp_dist[stats.hp] = 1.0;
		for (unsigned int i = 0; i < opp_strikes; ++i) {
			for (int j = i; j >= 0; j--) {
				unsigned src_index = stats.hp - j*opp_stats.damage;
				double move = hp_dist[src_index] * opp_hit_chance;
				hp_dist[src_index] -= move;
				hp_dist[src_index - opp_stats.damage] += move;
			}
			self_not_hit *= 1.0 - opp_hit_chance;
		}
	} else {
		// HP could be spread anywhere, iterate through whole thing.
		for (unsigned int i = 0; i < opp_strikes; ++i) {
			for (unsigned int j = opp_stats.damage; j < hp_dist.size(); ++j) {
				double move = hp_dist[j] * opp_hit_chance;
				hp_dist[j] -= move;
				hp_dist[j - opp_stats.damage] += move;
			}
			self_not_hit *= 1.0 - opp_hit_chance;
		}
	}

	if (!levelup_considered) return;

	if ( stats.experience + opp_stats.level >= stats.max_experience ) {
		forced_levelup(hp_dist);
	}

	if ( opp_stats.experience + stats.level >= opp_stats.max_experience ) {
		forced_levelup(opp_hp_dist);
	}
}

// Combat with <= 1 strike each is simple, too.
void one_strike_fight(const battle_context_unit_stats &stats,
                      const battle_context_unit_stats &opp_stats,
                      unsigned strikes, unsigned opp_strikes,
                      std::vector<double> & hp_dist,
                      std::vector<double> & opp_hp_dist,
                      double & self_not_hit, double & opp_not_hit,
                      bool levelup_considered)
{
	// If we were killed in an earlier fight, we don't get to attack.
	// (Most likely case: we are a first striking defender subject to a series
	// of attacks.)
	const double alive_prob = hp_dist.empty() ? 1.0 : 1.0 - hp_dist[0];
	const double hit_chance = (stats.chance_to_hit/100.0) * alive_prob;
	if ( opp_hp_dist.empty() ) {
		opp_hp_dist = std::vector<double>(opp_stats.max_hp+1);
		if ( strikes == 1 ) {
			opp_hp_dist[opp_stats.hp] = 1.0 - hit_chance;
			opp_hp_dist[std::max<int>(opp_stats.hp - stats.damage, 0)] = hit_chance;
			opp_not_hit *= 1.0 - hit_chance;
		} else {
			assert(strikes == 0);
			opp_hp_dist[opp_stats.hp] = 1.0;
		}
	} else {
		if ( strikes == 1 ) {
			for (unsigned int i = 1; i < opp_hp_dist.size(); ++i) {
				double move = opp_hp_dist[i] * hit_chance;
				opp_hp_dist[i] -= move;
				opp_hp_dist[std::max<int>(i - stats.damage, 0)] += move;
			}
			opp_not_hit *= 1.0 - hit_chance;
		}
	}

	// If we killed opponent, it won't attack us.
	const double opp_alive_prob = 1.0 - opp_hp_dist[0] / alive_prob;
	const double opp_hit_chance = (opp_stats.chance_to_hit/100.0) * opp_alive_prob;
	if ( hp_dist.empty() ) {
		hp_dist = std::vector<double>(stats.max_hp+1);
		if ( opp_strikes == 1 ) {
			hp_dist[stats.hp] = 1.0 - opp_hit_chance;
			hp_dist[std::max<int>(stats.hp - opp_stats.damage, 0)] = opp_hit_chance;
			self_not_hit *= 1.0 - opp_hit_chance;
		} else {
			assert(opp_strikes == 0);
			hp_dist[stats.hp] = 1.0;
		}
	} else {
		if ( opp_strikes == 1) {
			for (unsigned int i = 1; i < hp_dist.size(); ++i) {
				double move = hp_dist[i] * opp_hit_chance;
				hp_dist[i] -= move;
				hp_dist[std::max<int>(i - opp_stats.damage, 0)] += move;
			}
			self_not_hit *= 1.0 - opp_hit_chance;
		}
	}

	if (!levelup_considered) return;

	if ( stats.experience + opp_stats.level >= stats.max_experience ) {
		forced_levelup(hp_dist);
	} else if ( stats.experience + game_config::kill_xp(opp_stats.level) >= stats.max_experience ) {
		conditional_levelup(hp_dist, opp_hp_dist[0]);
	}

	if ( opp_stats.experience + stats.level >= opp_stats.max_experience ) {
		forced_levelup(opp_hp_dist);
	} else if ( opp_stats.experience + game_config::kill_xp(stats.level) >= opp_stats.max_experience ) {
		conditional_levelup(opp_hp_dist, hp_dist[0]);
	}
}

void complex_fight(const battle_context_unit_stats &stats,
                   const battle_context_unit_stats &opp_stats,
                   unsigned strikes, unsigned opp_strikes,
                   std::vector<double> summary[2],
                   std::vector<double> opp_summary[2],
                   double & self_not_hit, double & opp_not_hit,
                   bool levelup_considered)
{
	unsigned int rounds = std::max<unsigned int>(stats.rounds, opp_stats.rounds);
	unsigned max_attacks = std::max(strikes, opp_strikes);

	debug(("A gets %u attacks, B %u.\n", strikes, opp_strikes));

	unsigned int a_damage = stats.damage, a_slow_damage = stats.slow_damage;
	unsigned int b_damage = opp_stats.damage, b_slow_damage = opp_stats.slow_damage;

	// To simulate stoning, we set to amount which kills, and re-adjust after.
	/** @todo FIXME: This doesn't work for rolling calculations, just first battle.
	                 It also does not work if combined with (percentage) drain. */
	if (stats.petrifies)
		a_damage = a_slow_damage = opp_stats.max_hp;
	if (opp_stats.petrifies)
		b_damage = b_slow_damage = stats.max_hp;

	// Prepare the matrix that will do our calculations.
	combat_matrix m(stats.max_hp, opp_stats.max_hp,
	                stats.hp, opp_stats.hp, summary, opp_summary,
	                stats.slows && !opp_stats.is_slowed,
	                opp_stats.slows && !stats.is_slowed,
	                a_damage, b_damage, a_slow_damage, b_slow_damage,
	                stats.drain_percent, opp_stats.drain_percent,
	                stats.drain_constant, opp_stats.drain_constant);
	const double hit_chance = stats.chance_to_hit / 100.0;
	const double opp_hit_chance = opp_stats.chance_to_hit / 100.0;

	do {
		for (unsigned int i = 0; i < max_attacks; ++i) {
			if ( i < strikes ) {
				debug(("A strikes\n"));
				opp_not_hit *= 1.0 - hit_chance*(1.0-m.dead_prob_a());
				m.receive_blow_b(hit_chance);
				m.dump();
			}
			if ( i < opp_strikes ) {
				debug(("B strikes\n"));
				self_not_hit *= 1.0 - opp_hit_chance*(1.0-m.dead_prob_b());
				m.receive_blow_a(opp_hit_chance);
				m.dump();
			}
		}

		debug(("Combat ends:\n"));
		m.dump();
	} while (--rounds && m.dead_prob() < 0.99);

	if (stats.petrifies)
		m.remove_petrify_distortion_a(stats.damage, stats.slow_damage, opp_stats.hp);
	if (opp_stats.petrifies)
		m.remove_petrify_distortion_b(opp_stats.damage, opp_stats.slow_damage, stats.hp);

	if (levelup_considered) {
		if ( stats.experience + opp_stats.level >= stats.max_experience ) {
			m.forced_levelup_a();
		} else if ( stats.experience + game_config::kill_xp(opp_stats.level) >= stats.max_experience ) {
			m.conditional_levelup_a();
		}

		if ( opp_stats.experience + stats.level >= opp_stats.max_experience ) {
			m.forced_levelup_b();
		} else if ( opp_stats.experience + game_config::kill_xp(stats.level) >= opp_stats.max_experience ) {
			m.conditional_levelup_b();
		}
	}

	// We extract results separately, then combine.
	m.extract_results(summary, opp_summary);
}


/**
 * Chooses the best of the various known combat calculations for the current
 * situation.
 */
void do_fight(const battle_context_unit_stats &stats,
              const battle_context_unit_stats &opp_stats,
              unsigned strikes, unsigned opp_strikes,
              std::vector<double> summary[2], std::vector<double> opp_summary[2],
              double & self_not_hit, double & opp_not_hit,
              bool levelup_considered)
{
	// Optimization only works in the simple cases (no slow, no drain,
	// no petrify, no berserk, and no slowed results from an earlier combat).
	if ( !stats.slows  &&  !opp_stats.slows  &&
	     !stats.drains  &&  !opp_stats.drains  &&
	     !stats.petrifies  &&  !opp_stats.petrifies  &&
	     stats.rounds == 1  &&  opp_stats.rounds == 1  &&
	     summary[1].empty()  &&  opp_summary[1].empty() )
	{
		if ( strikes <= 1  &&  opp_strikes <= 1 )
			one_strike_fight(stats, opp_stats, strikes, opp_strikes,
			                 summary[0], opp_summary[0], self_not_hit, opp_not_hit,
			                 levelup_considered);
		else if ( strikes*stats.damage < min_hp(opp_summary[0], opp_stats.hp)  &&
		          opp_strikes*opp_stats.damage < min_hp(summary[0], stats.hp) )
			no_death_fight(stats, opp_stats, strikes, opp_strikes,
			               summary[0], opp_summary[0], self_not_hit, opp_not_hit,
			               levelup_considered);
		else
			complex_fight(stats, opp_stats, strikes, opp_strikes,
			              summary, opp_summary, self_not_hit, opp_not_hit,
			              levelup_considered);
	}
	else
		complex_fight(stats, opp_stats, strikes, opp_strikes,
		              summary, opp_summary, self_not_hit, opp_not_hit,
		              levelup_considered);
}

/**
 * Initializes a hit point summary (assumed empty) based on the source.
 * Only the part of the source from begin_hp up to (not including) end_hp
 * is kept, and all values get scaled by prob.
 */
void init_slice_summary(std::vector<double> & dst, const std::vector<double> & src,
                        unsigned begin_hp, unsigned end_hp, double prob)
{
	if ( src.empty() )
		// Nothing to do.
		return;

	const unsigned size = src.size();
	// Avoid going over the end of the vector.
	if ( end_hp > size )
		end_hp = size;

	// Initialize the destination.
	dst.resize(size, 0.0);
	for ( unsigned i = begin_hp; i < end_hp; ++i )
		dst[i] = src[i] / prob;
}

/**
 * Merges the summary results of simulation into an overall summary.
 * This uses prob to reverse the scaling that was done in init_slice_summary().
 */
void merge_slice_summary(std::vector<double> & dst, const std::vector<double> & src,
                         double prob)
{
	const unsigned size = src.size();

	// Make sure we have enough space.
	if ( dst.size() < size )
		dst.resize(size, 0.0);

	// Merge the data.
	for ( unsigned i = 0; i != size; ++i )
		dst[i] += src[i] * prob;
}

} // end anon namespace

// Two man enter.  One man leave!
// ... Or maybe two.  But definitely not three.
// Of course, one could be a woman.  Or both.
// And either could be non-human, too.
// Um, ok, it was a stupid thing to say.
void combatant::fight(combatant &opp, bool levelup_considered)
{
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

	// If we've fought before and we have swarm, we might have to split the
	// calculation by number of attacks.
	const std::vector<combat_slice> split = split_summary();
	const std::vector<combat_slice> opp_split = opp.split_summary();

	if ( split.size() == 1  &&  opp_split.size() == 1 )
		// No special treatment due to swarm is needed. Ignore the split.
		do_fight(u_, opp.u_, u_.num_blows, opp.u_.num_blows,
		         summary, opp.summary, self_not_hit, opp_not_hit,
		         levelup_considered);
	else
	{
		// Storage for the accumulated hit point distributions.
		std::vector<double> summary_result[2], opp_summary_result[2];
		// The chance of not being hit becomes an accumulated chance:
		self_not_hit = 0.0;
		opp_not_hit = 0.0;

		// Loop through all the potential combat situations.
		for ( unsigned s = 0; s != split.size(); ++s )
			for ( unsigned t = 0; t != opp_split.size(); ++t ) {
				const double sit_prob = split[s].prob * opp_split[t].prob;

				// Create summaries for this potential combat situation.
				std::vector<double> sit_summary[2], sit_opp_summary[2];
				init_slice_summary(sit_summary[0], summary[0], split[s].begin_hp,
				                   split[s].end_hp, split[s].prob);
				init_slice_summary(sit_summary[1], summary[1], split[s].begin_hp,
				                   split[s].end_hp, split[s].prob);
				init_slice_summary(sit_opp_summary[0], opp.summary[0],
				                   opp_split[t].begin_hp, opp_split[t].end_hp,
				                   opp_split[t].prob);
				init_slice_summary(sit_opp_summary[1], opp.summary[1],
				                   opp_split[t].begin_hp, opp_split[t].end_hp,
				                   opp_split[t].prob);

				// Scale the "not hit" chance for this situation by the chance that
				// this situation applies.
				double sit_self_not_hit = sit_prob;
				double sit_opp_not_hit = sit_prob;

				do_fight(u_, opp.u_, split[s].strikes, opp_split[t].strikes,
				         sit_summary, sit_opp_summary, sit_self_not_hit,
				         sit_opp_not_hit, levelup_considered);

				// Collect the results.
				self_not_hit += sit_self_not_hit;
				opp_not_hit += sit_opp_not_hit;
				merge_slice_summary(summary_result[0], sit_summary[0], sit_prob);
				merge_slice_summary(summary_result[1], sit_summary[1], sit_prob);
				merge_slice_summary(opp_summary_result[0], sit_opp_summary[0], sit_prob);
				merge_slice_summary(opp_summary_result[1], sit_opp_summary[1], sit_prob);
			}

		// Swap in the results.
		summary[0].swap(summary_result[0]);
		summary[1].swap(summary_result[1]);
		opp.summary[0].swap(opp_summary_result[0]);
		opp.summary[1].swap(opp_summary_result[1]);
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
		const unsigned size = summary[0].size();
		hp_dist.resize(size);
		for (unsigned int i = 0; i < size; ++i)
			hp_dist[i] = summary[0][i] + summary[1][i];
	}
	if (opp.summary[1].empty())
		opp.hp_dist = opp.summary[0];
	else {
		const unsigned size = opp.summary[0].size();
		opp.hp_dist.resize(size);
		for (unsigned int i = 0; i < size; ++i)
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


#ifdef ATTACK_PREDICTION_DEBUG
void list_combatant(const battle_context_unit_stats & stats, unsigned fighter)
{
	printf("#%02u: %u-%d; %2uhp; %02u%% to hit; ",
	       fighter, stats.swarm_max, stats.damage, stats.hp, stats.chance_to_hit);
	if ( stats.drains )
		printf("drains,");
	if ( stats.slows )
		printf("slows,");
	if ( stats.rounds > 1 )
		printf("berserk,");
	if ( stats.swarm )
		printf("swarm(%u),", stats.num_blows);
	if ( stats.firststrike )
		printf("firststrike,");
	printf("maxhp=%u\n", stats.max_hp);
}
#else
void list_combatant(const battle_context_unit_stats &, unsigned)
{ }
#endif


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
	if ( u_.swarm )
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
	if ( u_.swarm )
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
		unsigned alt = i + 74; // To offset some cycles.
		// To get somewhat realistic performance data, try to approximate
		// hit point ranges for mainline units (say 25-60 max hitpoints?)
		unsigned max_hp = (i*2)%23 + (i*3)%14 + 25;
		unsigned hp = (alt*5)%max_hp + 1;
		stats[i] = new battle_context_unit_stats(alt%8 + 2,      // damage
		                                         (alt%19 + 3)/4, // number of strikes
		                                         hp, max_hp,
		                                         (i%6)*10 + 30,  // hit chance
		                                         (i%13)%4 == 0,  // drains
		                                         (i%11)%3 == 0,  // slows
		                                         false,          // slowed
		                                          i%7 == 0,      // berserk
		                                         (i%17)/2 == 0,  // firststrike
		                                          i%5 == 0);     // swarm
		u[i] = new combatant(*stats[i]);
		list_combatant(*stats[i], i+1);
	}

	gettimeofday(&start, nullptr);
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
	gettimeofday(&end, nullptr);

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
				fprintf(stderr, "WARNING: drain specified without maxhp; assuming uninjured.\n");
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
				fprintf(stderr, "WARNING: swarm specified without maxhp; assuming uninjured.\n");
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
	att[i] = nullptr;

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
