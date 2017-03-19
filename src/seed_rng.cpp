/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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
   it should most likely be the system time.
*/

#include "seed_rng.hpp"

/*****
  Use preprocessor tests to decide whether to try to include and
  use boost random device, or fallback to system time.
 *****/

#define SEED_RNG_USE_BOOST_RANDOM_DEVICE

/*****
  End preprocessor checks
 *****/

#ifdef SEED_RNG_USE_BOOST_RANDOM_DEVICE
#include <boost/nondet_random.hpp>
#else
#include <ctime>
#endif

#include <sstream>
#include <iomanip>

namespace seed_rng {

	uint32_t next_seed() {
	#ifdef SEED_RNG_USE_BOOST_RANDOM_DEVICE
		static boost::random_device rnd_;
		return rnd_();
	#else
		return static_cast<uint32_t> (std::time(0));
	#endif
	}

	std::string next_seed_str() {
		uint32_t random_seed_ = next_seed();
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << random_seed_;

		return stream.str();
	}

} //ends seed_rng namespace
