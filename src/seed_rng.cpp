/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/* This file selects a seed source -- "nondeterministic" random number
   generator in boost documentation. It should be a wrapper for 
   boost::random_device on platforms where this is available, otherwise
   it should most likely be the system time. On versions of boost after
   1.43.0, windows has an available boost random_device. Before that,
   we should put "ifdef WIN32" in the preprocessor guard below, for
   example.
*/

#include "seed_rng.hpp"

// #ifdef WE_ARE_ON_A_SYSTEM_WITH_NO_BOOST_RANDOM_DEVICE
// #undef SEED_RNG_USE_BOOST_RANDOM_DEVICE
// #else
#define SEED_RNG_USE_BOOST_RANDOM_DEVICE
// #endif


#ifdef SEED_RNG_USE_BOOST_RANDOM_DEVICE
#include <boost/nondet_random.hpp>
#else
#include <ctime>
#endif

namespace seed_rng {

	#ifdef SEED_RNG_USE_BOOST_RANDOM_DEVICE
	uint32_t next_seed() {
		static boost::random_device rnd_;
		return rnd_();
	}
	#else
	uint32_t next_seed() {
		return static_cast<uint32_t> (std::time(0));
	}
	#endif

} //ends seed_rng namespace
