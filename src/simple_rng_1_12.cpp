/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2015 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "simple_rng_1_12.hpp"
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
simple_rng_1_12::simple_rng_1_12() :
	random_seed_(rand() & 0x7FFFFFFF),
	random_pool_(random_seed_),
	random_calls_(0),
	legacy_rng_()
{
}

simple_rng_1_12::simple_rng_1_12(const config& cfg) :
	random_seed_(cfg["random_seed"]),
	random_pool_(random_seed_),
	random_calls_(cfg["random_calls"].to_int(0)),
	legacy_rng_()
{
	if(fallback_to_legacy_rng())
	{
		legacy_rng_ = simple_rng_1_10(cfg);
	}
	for ( unsigned calls = 0; calls < random_calls_; ++calls )
		random_next();
}

void simple_rng_1_12::set_replay_data(const config& cfg)
{
	if(cfg.has_attribute("version"))
	{
		DBG_RND << "Received version: " << cfg["version"] << "\n";
		*loaded_version_ = version_info(cfg["version"]);

		if(fallback_to_legacy_rng())
		{
			legacy_rng_ = rand_rng::simple_rng_1_10(cfg);
			legacy_rng_.set_replay_data(cfg);
		}
	}
}

int simple_rng_1_12::get_next_random()
{
	random_next();
	++random_calls_;
	if(fallback_to_legacy_rng()) return legacy_rng_.get_next_random();
	DBG_RND << "simple_rng_1_12 pulled user random " << random_pool_
		<< " for call " << random_calls_
		<< " with seed " << random_seed_ << '\n';

	return (random_pool_ / 65536) % 32768;
}

void simple_rng_1_12::next_side_turn_simulation()
{
	if(fallback_to_legacy_rng()) legacy_rng_.next_side_turn_simulation();
}

void simple_rng_1_12::rotate_random()
{
	if(fallback_to_legacy_rng()) legacy_rng_.rotate_random();
	random_seed_ = random_pool_ & 0x7FFFFFFF;
	random_calls_ = 0;
}

int simple_rng_1_12::get_random_seed() const
{
	if(fallback_to_legacy_rng()) return legacy_rng_.get_random_seed();
	else return random_seed_;
}

int simple_rng_1_12::get_random_calls() const
{
	if(fallback_to_legacy_rng()) return legacy_rng_.get_random_calls();
	else return random_calls_;
}

void simple_rng_1_12::seed_random(const int seed, const unsigned call_count)
{
	random_pool_ = seed;
	random_seed_ = seed;
	for(random_calls_ = 0; random_calls_ < call_count; ++random_calls_) {
		random_next();
	}
	if(fallback_to_legacy_rng())
		legacy_rng_.seed_random(seed, call_count);
	else
		DBG_RND << "Seeded random with " << random_seed_ << " with "
			<< random_calls_ << " calls, pool is now at "
			<< random_pool_ << '\n';
}

void simple_rng_1_12::random_next()
{
	// Use the simple random generator as shown in man rand(3).
	// The division is done separately since we also want to
	// quickly go the the wanted index in the random list.
	random_pool_ = random_pool_ * 1103515245 + 12345;
}


} // ends rand_rng namespace

