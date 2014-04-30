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

#include "mt_rng.hpp"
#include "config.hpp"
#include "log.hpp"
#include <stdlib.h>
static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)


namespace rand_rng
{
#ifdef MT_RNG_USE_BOOST_RANDOM_DEVICE
boost::random_device mt_rng::rnd_; 

mt_rng::mt_rng() :
	random_seed_(mt_rng::rnd_()), //TODO: Is this the proper way to get the seed in this scenario?
	mt_(random_seed_),
	random_calls_(0)
{
}
#else
mt_rng::mt_rng() :
	random_seed_(static_cast<unsigned int>(std::time(0))),
	mt_(random_seed_),
	random_calls_(0)
{
}
#endif

mt_rng::mt_rng(const config& cfg) :
	random_seed_(cfg["random_seed"]),
	mt_(random_seed_),
	random_calls_(cfg["random_calls"].to_int(0))
{
	discard(random_calls_); //mt_.discard(random_calls_);
}

uint32_t mt_rng::get_next_random()
{
	uint32_t result = mt_();
	++random_calls_;
	DBG_RND << "pulled user random " << result
		<< " for call " << random_calls_
		<< " with seed " << random_seed_ << '\n';

	return result;
}

void mt_rng::rotate_random()
{
	seed_random(mt_(),0);
}

void mt_rng::seed_random(const uint32_t seed, const unsigned int call_count)
{
	random_seed_ = seed;
	mt_.seed(random_seed_);
	discard(call_count); //mt_.discard(call_count);
	DBG_RND << "Seeded random with " << random_seed_ << " with "
		<< random_calls_ << " calls." << std::endl;
}

void mt_rng::discard(const unsigned int call_count)
{
	for(unsigned int i = 0; i < call_count; ++i) {
		mt_();
	}
}

} // ends rand_rng namespace

