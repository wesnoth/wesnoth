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
#ifndef RNG_HPP_INCLUDED
#define RNG_HPP_INCLUDED

#include <cstddef>

#include "random.hpp"
#include "simple_rng.hpp"

#include <boost/function.hpp>

namespace rand_rng
{

class rng
{
public:
	rng();
	int get_random();

	const config* get_random_results();
	void set_random_results(const config& cfg);

	void set_seed(seed_t seed);

protected:
	config* random();
	void set_random(config*);

private:
	config* random_;
	size_t random_child_;

	simple_rng generator_;
};

/**
 * Set the random seed for the current random number generator, sets the seed
 * as valid nd calls the new seed callback if set
 */
void set_seed(seed_t seed);

/**
 * Mark the RNG seed as invalid
 */
void invalidate_seed();

/**
 * Function to check whether the RNG has been updated with a new seed since
 * the las invalidate_seed() call
 */
bool has_valid_seed();

/**
 * Get the last seed the RNG was seeded with in set_seed
 */
seed_t get_last_seed();

/**
 * Set the callback for a fuction that will be called on subsequent set_seed
 * calls.
 * @TODO needs a reliable way of clearing the callback when things don't go as
 * normal (e.g. player quit the game while the callback is set)
 */
void set_new_seed_callback(boost::function<void (seed_t)> f);

/**
 * Clear the new seed callback
 */
void clear_new_seed_callback();

/**
 * Disable server RNG support, will cause OOS if playing MP with a client with
 * SRNG enabled. For debugging purposes, should be removed before 1.8.0
 */
void disable_server_rng();

} // ends rand_rng namespace

#endif
