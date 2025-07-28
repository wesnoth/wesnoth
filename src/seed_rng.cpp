/*
	Copyright (C) 2014 - 2025
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
   it should most likely be the system time. (Currently boost::random_device
   is available on all supported platforms.)
*/

#include "seed_rng.hpp"

#include <boost/nondet_random.hpp>

#include <sstream>
#include <iomanip>

namespace seed_rng {

	uint32_t next_seed() {
		static boost::random_device rnd_;
		return rnd_();
	}

	std::string next_seed_str() {
		uint32_t random_seed_ = next_seed();
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << random_seed_;

		return stream.str();
	}

} //ends seed_rng namespace
