/* $Id$ */
/*
   Copyright (C) 2006 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.

   Full algorithm by Yogin.  Typing and optimization by Rusty.

   This code has lots of debugging.  It is there for a reason: this
   code is kinda tricky.  Do not remove it.
*/
#include <cstring> // For memset
#include <vector>

#include "global.hpp"
#include "wassert.hpp"

// Compile with -O3 -DBENCHMARK for speed testing, -DCHECK for testing
// correctness (run tools/wesnoth-attack-sim.c --check on output)
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

#ifdef DEBUG
#define debug(x) printf x
#else
#define debug(x)
#endif

namespace 
{
// A matrix of A's hitpoints vs B's hitpoints.
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
	void receive_blow_b(unsigned damage, double hit_chance,
						bool a_slows, bool a_drains);

	// B hits A.  Why can't they just get along?
	void receive_blow_a(unsigned damage, double hit_chance,
						bool b_slows, bool b_drains);

	// Its over, and here's the bill.
	void extract_results(std::vector<double> summary_a[2],
						 std::vector<double> summary_b[2]);

	// What's the chance one is dead?
	double dead_prob() const;

	void dump() const;

	// We need four matrices, or "planes", reflecting the possible
	// "slowed" states (neither slowed, A slowed, B slowed, both
	// slowed).
	enum {
		NEITHER_SLOWED,
		A_SLOWED,
		B_SLOWED,
		BOTH_SLOWED,
	};

private:
	// This gives me 10% speed improvement over std::vector<> (g++4.0.3 x86)
	double *prob_matrix::new_arr(unsigned int size);

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

	// FIXME: rename using _ at end.
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
		wassert(b_summary[0].empty());
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
		// FIXME: Can optimize here.
		min_row[NEITHER_SLOWED] = 0;
		min_row[A_SLOWED] = 0;
		min_col[A_SLOWED] = b_hp - 1;
		for (unsigned int row = 0; row < rows; row++)
			val(NEITHER_SLOWED, row, b_hp) = a_summary[0][row];
		if (!a_summary[1].empty()) {
			for (unsigned int row = 0; row < rows; row++)
				val(A_SLOWED, row, b_hp) = a_summary[1][row];
		}
		debug(("A has fought before\n"));
		dump();
	} else if (!b_summary[0].empty()) {
		min_col[NEITHER_SLOWED] = 0;
		min_col[B_SLOWED] = 0;
		min_row[B_SLOWED] = a_hp - 1;
		for (unsigned int col = 0; col < cols; col++)
			val(NEITHER_SLOWED, a_hp, col) = b_summary[0][col];
		if (!b_summary[1].empty()) {
			for (unsigned int col = 0; col < cols; col++)
				val(B_SLOWED, a_hp, col) = b_summary[1][col];
		}
		debug(("B has fought before\n"));
		dump();
	} else
		val(NEITHER_SLOWED, a_hp, b_hp) = 1.0;
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
	return plane[p][row * cols + col];
}

const double &prob_matrix::val(unsigned p, unsigned row, unsigned col) const
{
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

// Shift prob_matrix to reflect probability 'hit_chance' that damage (up
// to) 'damage' is done to 'b'.
void prob_matrix::receive_blow_b(unsigned damage, double hit_chance,
								 bool a_slows, bool a_drains)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned int actual_damage;

		if (!plane[src])
			continue;

		// If a slows us we go from 0=>2, 1=>3, 2=>2 3=>3. 
		if (a_slows)
			dst = (src|2);
		else
			dst = src;

		// A is slow in planes 1 and 3.
		if (src & 1)
			actual_damage = damage / 2;
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

// Shift matrix to reflect probability 'hit_chance' that damage (up
// to) 'damage' is done to 'a'.
void prob_matrix::receive_blow_a(unsigned damage, double hit_chance,
								 bool b_slows, bool b_drains)
{
	int src, dst;

	// Walk backwards so we don't copy already-copied matrix planes.
	for (src = 3; src >=0; src--) {
		unsigned actual_damage;

		if (!plane[src])
			continue;

		// If b slows us we go from 0=>1, 1=>1, 2=>3 3=>3.
		if (b_slows)
			dst = (src|1);
		else
			dst = src;

		// B is slow in planes 2 and 3.
		if (src & 2)
			actual_damage = damage/2;
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
	// FIXME: damage if slowed.
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

combatant::combatant(unsigned hp, unsigned max_hp, bool slowed,
					 bool could_ever_drain)
	: hp_dist(could_ever_drain ? max_hp+1: hp+1), hp_(hp), max_hp_(max_hp),
	  slowed_(slowed)
{
}

// Select a weapon.
void combatant::set_weapon(unsigned num_attacks, bool drains, bool berserk,
						   bool swarm, bool firststrike)
{
	base_num_attacks_ = num_attacks;
	drains_ = drains;
	berserk_ = berserk;
	swarm_ = swarm;
	firststrike_ = firststrike;
}

unsigned combatant::num_attacks(unsigned int hp) const
{
	if (swarm_)
		return base_num_attacks_ - (base_num_attacks_*(max_hp_-hp)/max_hp_);
	else
		return base_num_attacks_;
}

// Set effect against this particular opponent.
void combatant::set_effectiveness(unsigned damage, double hit_chance,
								  bool slows)
{
	slows_ = slows;
	base_hit_chance_ = hit_chance;
	if (slowed_)
		damage_ = damage / 2;
	else
		damage_ = damage;

	if (!swarm_ || summary[0].empty())
		hit_chances_ = std::vector<double>(num_attacks(hp_), hit_chance);
	else {
		// Whether we get an attack depends on HP distribution from previous
		// combat.  So we roll this into our P(hitting), since no attack is
		// equivalent to missing.
		hit_chances_ = std::vector<double>(base_num_attacks_);
		double alive_prob;

		if (summary[1].empty())
			alive_prob = 1 - summary[0][0];
		else
			alive_prob = 1 - summary[0][0] - summary[1][0];

		for (unsigned int i = 1; i <= max_hp_; i++) {
			double prob = summary[0][i];
			if (!summary[1].empty())
				prob += summary[1][i];
			for (unsigned int j = 0; j < num_attacks(i); j++)
				hit_chances_[j] += prob * hit_chance / alive_prob;
		}
	}
	debug(("\nhit_chances_ (base %u%%):", (unsigned)(hit_chance * 100.0 + 0.5)));
	for (unsigned int i = 0; i < base_num_attacks_; i++)
		debug((" %.2f", hit_chances_[i]));
	debug(("\n"));
}

void combatant::reset()
{
	for (unsigned int i = 0; i < hp_dist.size(); i++)
		hp_dist[i] = 0.0;
	summary[0] = std::vector<double>();
	summary[1] = std::vector<double>();
	hit_chances_ = std::vector<double>(num_attacks(hp_), base_hit_chance_);
}

// Two man enter.  One man leave!
// ... Or maybe two.  But definitely not three.  Of course, one could
// be a woman.  Or both.  And neither could be human, too.
// Um, ok, it was a stupid thing to say.
void combatant::fight(combatant &opp)
{
	unsigned int i, rounds = berserk_ || opp.berserk_ ? 30 : 1;

	// If defender has firststrike and we don't, reverse.
	if (opp.firststrike_ && !firststrike_) {
		opp.fight(*this);
		return;
	}

	debug(("A: %u %u %u %2g%% ",
		   damage_, base_num_attacks_, hp_, base_hit_chance_*100.0));
	if (drains_)
		debug(("drains,"));
	if (slows_ && !opp.slowed_)
		debug(("slows,"));
	if (berserk_)
		debug(("berserk,"));
	if (swarm_)
		debug(("swarm,"));
	debug(("maxhp=%u\n", hp_dist.size()-1));
	debug(("B: %u %u %u %2g%% ", opp.damage_, opp.base_num_attacks_, opp.hp_,
		   opp.base_hit_chance_*100.0));
	if (opp.drains_)
		debug(("drains,"));
	if (opp.slows_ && !slowed_)
		debug(("slows,"));
	if (opp.berserk_)
		debug(("berserk,"));
	if (opp.swarm_)
		debug(("swarm,"));
	debug(("maxhp=%u\n", opp.hp_dist.size()-1));

	prob_matrix m(hp_dist.size()-1, opp.hp_dist.size()-1,
				  slows_ && !opp.slowed_, opp.slows_ && !slowed_, hp_, opp.hp_,
				  summary, opp.summary);

	untouched = 1.0;
	opp.untouched = 1.0;
	unsigned max_attacks = maximum(hit_chances_.size(),
								   opp.hit_chances_.size());

	debug(("A gets %u attacks, B %u\n",
		   hit_chances_.size(),
		   opp.hit_chances_.size()));
	do {
		for (i = 0; i < max_attacks; i++) {
			if (i < hit_chances_.size()) {
				debug(("A strikes\n"));
				m.receive_blow_b(damage_, hit_chances_[i],
								 slows_ && !opp.slowed_, drains_);
				m.dump();
				opp.untouched *= (1 - hit_chances_[i]);
			}
			if (i < opp.hit_chances_.size()) {
				debug(("B strikes\n"));
				m.receive_blow_a(opp.damage_, opp.hit_chances_[i],
								 opp.slows_ && !slowed_, opp.drains_);
				m.dump();
				untouched *= (1 - opp.hit_chances_[i]);
			}
		}

		debug(("Combat ends:\n"));
		m.dump();
	} while (--rounds && m.dead_prob() < 0.99);

	// We extract results separately, then combine.
	m.extract_results(summary, opp.summary);

	if (summary[1].empty())
		hp_dist = summary[0];
	else {
		for (i = 0; i < hp_dist.size(); i++)
			hp_dist[i] = summary[0][i] + summary[1][i];
	}
	if (opp.summary[1].empty())
		opp.hp_dist = opp.summary[0];
	else {
		for (i = 0; i < opp.hp_dist.size(); i++)
			opp.hp_dist[i] = opp.summary[0][i] + opp.summary[1][i];
	}
}
};

#if defined(BENCHMARK) || defined(CHECK)
// We create a significant number of nasty-to-calculate units, and
// test each one against the others.
#ifdef BENCHMARK
#define NUM_UNITS 50
#else
#define NUM_UNITS 25
#endif

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
		unsigned hp = 1 + i/2 + ((i*2)%40);
		u[i] = new combatant(hp, hp + (i+7)%20, false);
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
				if (battle < specific_battle)
					continue;
				u[j]->fight(*u[i]);
				untouched = u[i]->untouched;
				// We need this here, because swarm means out num hits
				// can change.
				u[i]->set_effectiveness((i % 7) + 2, 0.3 + (i % 6)*0.1,
										(i % 8) == 0);
				u[k]->fight(*u[i]);
				// We want cumulative untouched for defender.
				u[i]->untouched *= untouched;
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
	return 0;
}
#endif // Standalone program
