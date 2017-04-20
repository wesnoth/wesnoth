/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#include <cstdlib> //needed for RAND_MAX
#include <cstdint>
#include <iterator> //needed for std::distance
#include <limits>

namespace randomness
{
	/**
		this class does not give synced random results derived classes might do.
	*/
	class rng : private std::numeric_limits<uint32_t>
	{
		using base = std::numeric_limits<uint32_t>;
	public:
		rng();
		/**
		 * Provides the next random draw. This is raw PRG output.
		 */
		uint32_t next_random();
		virtual ~rng();
		/**
		 * Provides the number of random calls to the rng in this context.
		 * Note that this may be different from the number of random calls to
		 * the underlying rng, and to the random_calls number in save files!
		 */
		unsigned int get_random_calls() const;

		/**
	         *  This helper method provides a random int from the underlying generator,
		 *  using results of next_random in a manner guaranteed to be cross platform.
		 *  The result will be random in range [min,max] inclusive.
		 *  @param min		The minimum value produced.
		 *  @param max		The maximum value produced.
		 */
		int get_random_int(int min, int max)
		{ return min + get_random_int_in_range_zero_to(max - min); }

		/**
		 * This helper method returns true with the probability supplied as a parameter.
		 * @param probability	The probability of returning true, from 0 to 1.
		 */
		bool get_random_bool(double probability);

		/**
		 * This helper method returns a floating-point number in the range [0,1[.
		 */
		double get_random_double();

		/**
		 * This helper method selects a random element from a container of floating-point numbers.
		 * Every number has a probability to be selected equal to the number itself
		 * (e.g. a number of 0.1 is selected with a probability of 0.1). The sum of numbers
		 * should be one.
		 * @param first	Iterator to the beginning of the container
		 * @param last	Iterator to the end of the container
		 * @ret			The index of the selected number
		 */
		template <typename T>
		unsigned int get_random_element(T first, T last);

		// For compatibility with the C++ UniformRandomBitGenerator concept
		using result_type = uint32_t;
		using base::min;
		using base::max;
		uint32_t operator()() { return next_random(); }

		static rng& default_instance();
	protected:
		virtual uint32_t next_random_impl() = 0;
		unsigned int random_calls_;

	private:
		/** Does the hard work of get_random_int.
		 *  The result will be random in range [0,max] inclusive.
		 *  @param max		The maximum value produced.
		 */
		int get_random_int_in_range_zero_to(int max);
	};

	/**
		This generator is automatically synced during synced context.
		Calling this rng during a synced context automatically makes undoing impossible.
		Outside a synced context this has the same effect as rand()
	*/
	extern rng* generator;

	template <typename T>
	unsigned int rng::get_random_element(T first, T last)
	{
		double target = get_random_double();
		double sum = 0.0;
		T it = first;
		sum += *it;
		while (sum <= target)
		{
			++it;
			if (it != last)
			{
				sum += *it;
			}
			else
			{
				break;
			}
		}
		return std::distance(first, it);
	}
}
#endif
