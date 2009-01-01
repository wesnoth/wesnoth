/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2009 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file random.hpp */

#ifndef RANDOM_HPP_INCLUDED
#define RANDOM_HPP_INCLUDED

#include "SDL_types.h"

class config;

int get_random();
const config* get_random_results();
void set_random_results(const config& cfg);

namespace rand_rng
{

class rng
{
public:
	rng();
	int get_random();

	const config* get_random_results();
	void set_random_results(const config& cfg);

protected:
	config* random();
	void set_random(config*);

private:
	config* random_;
	size_t random_child_;
};

struct set_random_generator {
	set_random_generator(rng* r);
	~set_random_generator();

private:
	rng* old_;
};

class simple_rng
{
public:
    simple_rng();
    simple_rng(const config& cfg);

	/** Get a new random number. */
	int get_random();

	/**
	 *  Seeds the random pool.
	 *
	 *  @param call_count   Upon loading we need to restore the state at saving
	 *                      so set the number of times a random number is
	 *                      generated for replays the orginal value is
	 *                      required.
	 */
	void seed_random(const unsigned call_count = 0);

	/**
	 *  Seeds the random pool.
	 *
	 *  @param seed         The initial value for the random engine.
	 *  @param call_count   Upon loading we need to restore the state at saving
	 *                      so set the number of times a random number is
	 *                      generated for replays the orginal value is
	 *                      required.
	 */
	void seed_random(const int seed, const unsigned call_count = 0);

	/**
	 * Resets the random to the 0 calls and the seed to the random
	 *  this way we stay in the same sequence but don't have a lot
	 *  calls. Used when moving to the next scenario.
	 */
	void rotate_random()
		{ random_seed_ = random_pool_; random_calls_ = 0; }


	int get_random_seed() const { return random_seed_; }
	int get_random_calls() const { return random_calls_; }

private:
	/** Initial seed for the pool. */
	int random_seed_;

	/** State for the random pool. */
	int random_pool_;

	/** Number of time a random number is generated. */
	unsigned random_calls_;

	/** Sets the next random number in the pool. */
	void random_next();
};

} // ends rand_rng namespace

#endif
