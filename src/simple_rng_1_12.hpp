/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2015 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SIMPLE_RNG_1_12_HPP_INCLUDED
#define SIMPLE_RNG_1_12_HPP_INCLUDED

#include "game_version.hpp"
#include "simple_rng_1_10.hpp"
#include "random.hpp"

class config;

namespace rand_rng
{

class simple_rng_1_12
{
public:
    simple_rng_1_12();
    simple_rng_1_12(const config& cfg);

	void set_replay_data(const config& cfg);

	/** Get a new random number. */
	int get_next_random();

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
	void rotate_random();

	int get_random_seed() const;
	int get_random_calls() const;

	void next_side_turn_simulation();

private:
	/** Initial seed for the pool. */
	int random_seed_;

	/** State for the random pool (modulo arithmetic). */
	unsigned long random_pool_;

	/** Number of time a random number is generated. */
	unsigned random_calls_;

	/** Sets the next random number in the pool. */
	void random_next();

	/** First version to use this RNG engine. */
	version_info first_version_ = version_info("1.11.0");

	/** The version for the RNG engine. */
	version_info* loaded_version_ = &randomness::loaded_version;

	bool fallback_to_legacy_rng() const { return *loaded_version_ < first_version_; }

	/** The legacy RNG engine use for old versions. */
	rand_rng::simple_rng_1_10 legacy_rng_;
};

} // ends rand_rng namespace

#endif
