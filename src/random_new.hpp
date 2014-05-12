/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef RANDOM_NEW_H_INCLUDED
#define RANDOM_NEW_H_INCLUDED

#include <stdint.h>
#include <cstdlib> //needed for RAND_MAX

namespace random_new
{
	//this class does NOT give synced random results.
	class rng
	{
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
		unsigned int get_random_calls();

		/** 
	         *  This helper method provides a random int from the underlying generator,
		 *  using results of next_random in a manner guaranteed to be cross platform.
		 *  The result will be random in range [min,max] inclusive.
		 *  @param min		The minimum value produced.
		 *  @param max		The maximum value produced.
		 */
		int get_random_int(int min, int max) 
		{ return min + get_random_int_in_range_zero_to(max - min); }

	protected:
		virtual uint32_t next_random_impl();
		unsigned int random_calls_;

	private:
		/** Does the hard work of get_random_int. 
		 *  The result will be random in range [0,max] inclusive.
		 *  @param max		The maximum value produced.
		 */
		int get_random_int_in_range_zero_to(int max);
	};

	// this generator is autmatilcy synced during synced context.
	// calling this should automaticly clear the undo stack.
	// 
	extern rng* generator;
	
}
#endif
