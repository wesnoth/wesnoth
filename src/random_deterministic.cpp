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
#include "random_deterministic.hpp"


namespace randomness
{

	rng_deterministic::rng_deterministic(rand_rng::mt_rng& gen)
		: generator_(gen)
	{

	}

	rng_deterministic::~rng_deterministic()
	{

	}

	uint32_t rng_deterministic::next_random_impl()
	{
		return generator_.get_next_random();
	}


	set_random_determinstic::set_random_determinstic(rand_rng::mt_rng& rng)
		: old_rng_(generator), new_rng_(rng)
	{
		generator = &new_rng_;
	}

	set_random_determinstic::~set_random_determinstic()
	{
		generator = old_rng_;
	}
}
