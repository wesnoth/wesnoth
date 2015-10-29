/*
   Copyright (C) 2014 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "random_new.hpp"
#include "log.hpp"


#include <cassert>
#include <stdlib.h>
#include <boost/random/mersenne_twister.hpp>
#include <time.h>

static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)

namespace {

	class rng_default : public random_new::rng
	{
	public:
		rng_default()
			: gen_(time(NULL))
		{
		}
	protected:
		virtual uint32_t next_random_impl()
		{
			return gen_();
		}
	private:
		boost::mt19937 gen_;
	};
}

namespace random_new
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

	unsigned int rng::get_random_calls()
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
	 *  to boost::mt19937, since it guarantees that there are no "bad bits"
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
}
