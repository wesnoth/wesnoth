/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2010 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SIMPLE_RNG_HPP_INCLUDED
#define SIMPLE_RNG_HPP_INCLUDED

class config;

namespace rand_rng
{

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
