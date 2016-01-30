/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MT_RNG_HPP_INCLUDED
#define MT_RNG_HPP_INCLUDED

#include <boost/cstdint.hpp>
#include <boost/random/mersenne_twister.hpp>

using boost::uint32_t;

class config;

namespace rand_rng
{

/*
   This class provides an interface, similar to simple_rng, to the
   boost mt19937 generator.
*/

class mt_rng
{
public:
	mt_rng();
	explicit mt_rng(const config& cfg);
	explicit mt_rng(boost::uint32_t seed);
	/** Get a new random number. */
	uint32_t get_next_random();

	/**
	 *  Same as uint32_t version, but uses a stringstream to convert given
         *  hex string.
         *  @param seed         A hex string. Should not have 0x leading.
         *  @param call_count   Value to set internal call counter to after seeding.
         */
	void seed_random(const std::string & seed, const unsigned int call_count = 0);

	/**
	 * Resets the random to the 0 calls and the seed to the random
	 *  this way we stay in the same sequence but don't have a lot
	 *  calls. Used when moving to the next scenario.
	 */
	void rotate_random();

	uint32_t get_random_seed() const { return random_seed_; }
	std::string get_random_seed_str() const;
	unsigned int get_random_calls() const { return random_calls_; }

	//Comparisons, mainly used for testing
	bool operator== (const mt_rng &other) const;
	bool operator!= (const mt_rng &other) const
	{ return !operator==(other); }

private:
	/** Initial seed for the pool. */
	uint32_t random_seed_;

	/** State for the random pool (boost mersenne twister random generator). */
	boost::mt19937 mt_;

	/** Number of time a random number is generated. */
	unsigned int random_calls_;

	/** On my local version of boost::random, I can use mt_.discard to discard a number of rng results.
	In older versions this seems to be unavailable. I'm implementing as a private method of mt_rng,
	following description here: http://www.boost.org/doc/libs/1_51_0/doc/html/boost/random/mersenne_twister_engine.html#id1408119-bb
	*/
	void discard(const unsigned int call_count);

	/**
	 *  Seeds the random pool. This is the old version, I would like to mark this private.
	 *
	 *  @param seed         The initial value for the random engine.
	 *  @param call_count   Upon loading we need to restore the state at saving
	 *                      so set the number of times a random number is
	 *                      generated for replays the orginal value is
	 *                      required.
	 */
	void seed_random(const uint32_t seed, const unsigned int call_count = 0);
};

} // ends rand_rng namespace

#endif
