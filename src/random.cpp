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

#include "random.hpp"
#include "log.hpp"


#include <cassert>
#include <cstdlib>
#include <random>
#include <boost/random/random_device.hpp>

static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)

namespace {

	class rng_default : public randomness::rng
	{
	public:
		rng_default()
			: gen_()
		{
			/* Note: do not replace this with std::random_device.
			 * @cbeck88 told in IRC (2016-10-16) that std::random_device
			 * is very poorly implemented in MinGW. */
			boost::random_device entropy_source;
			gen_.seed(entropy_source());
		}
	protected:
		virtual uint32_t next_random_impl()
		{
			return gen_();
		}
	private:
		std::mt19937 gen_;
	};
}

namespace randomness
{

	rng* generator = &rng::default_instance();

	rng::rng()
		: random_calls_(0)
	{

	}

	rng::~rng()
	{

	}

	rng& rng::default_instance()
	{
		static rng* def = new rng_default();
		return *def;
	}

	unsigned int rng::get_random_calls() const
	{
		return random_calls_;
	}

	uint32_t rng::next_random()
	{
		random_calls_++;
		return next_random_impl();
	}

	/**
	 *  This code is based on the boost implementation of uniform_smallint.
	 *  http://www.boost.org/doc/libs/1_55_0/boost/random/uniform_smallint.hpp
	 *  Using that code would be ideal, except that boost, and C++11, do not
	 *  guarantee that it will work the same way on all platforms, or that the
	 *  results may not be different in future versions of the library.
	 *  The simplified version I have written should work the same on all
	 *  platforms, which is the most important thing for us.
	 *  The existence of "modulo bias" seems less important when we have moved
	 *  to std::mt19937, since it guarantees that there are no "bad bits"
	 *  and has a very large range.
	 *
	 *  If a standard cross platform version becomes available then this should
	 *  be replaced.
	 */
	int rng::get_random_int_in_range_zero_to(int max)
	{
		assert(max >= 0);
		return static_cast<int> (next_random() % (static_cast<uint32_t>(max)+1));
	}

	double rng::get_random_double()
	{
		union
		{
			double floating_point_number;
			uint64_t integer;
		} number;

		number.integer = 0u;
		/* Exponent. It's set to zero.
		Exponent bias is 1023 in double precision, and therefore the value 1023
		needs to be encoded. */
		number.integer |= static_cast<uint64_t>(1023) << 52;
		/* Significand. A double-precision floating point number stores 52 significand bits.
		The underlying RNG only gives us 32 bits, so we need to shift the bits 20 positions
		to the left. The last 20 significand bits we can leave at zero, we don't need
		the full 52 bits of randomness allowed by the double-precision format. */
		number.integer |= static_cast<uint64_t>(next_random()) << (52 - 32);
		/* At this point, the exponent is zero. The significand, taking into account the
		implicit leading one bit, is at least exactly one and at most almost two.
		In other words, interpreting the value as a double gives us a number in the range
		[1, 2[. Simply subtract one from that value and return it. */
		return number.floating_point_number - 1.0;
	}

	bool rng::get_random_bool(double probability)
	{
		assert(probability >= 0.0 && probability <= 1.0);
		return get_random_double() < probability;
	}
}
