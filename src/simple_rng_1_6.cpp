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

#include "simple_rng_1_6.hpp"
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
int disable_simulation_1_6 = -1;

simple_rng_1_6::simple_rng_1_6() :
	random_seed_(rand() & 0x7FFFFFFF),
	random_pool_(random_seed_),
	random_pool_1_8_(random_seed_),
	random_calls_(0)
{
}

simple_rng_1_6::simple_rng_1_6(const config& cfg) :
	random_seed_(cfg["random_seed"]),
	random_pool_(random_seed_),
	random_pool_1_8_(random_seed_),
	random_calls_(cfg["random_calls"].to_int(0))
{
	if(cfg.has_attribute("version"))
	{
		DBG_RND << "Received version: " << cfg["version"] << "\n";
		*loaded_version_ = version_info(cfg["version"]);
	}
	for ( unsigned calls = 0; calls < random_calls_; ++calls )
		random_next();
}

void simple_rng_1_6::set_replay_data(const config& cfg)
{
	if(cfg.has_attribute("version"))
	{
		DBG_RND << "Received version: " << cfg["version"] << "\n";
		*loaded_version_ = version_info(cfg["version"]);
	}
	fill_simulation_stack(cfg);
	set_simulation(false);
}

int simple_rng_1_6::get_next_random()
{
	int output_random_number;
	random_next();
	++random_calls_;
	if(*do_simulation_) return get_next_simulation();
	{
		output_random_number = (random_pool_1_8_ / 65536) % 32768;
		while (output_random_number < 0)
		{
			output_random_number += 32768;
		}
		DBG_RND << "simple_rng_1_6 pulled user random " << random_pool_1_8_
			<< " for call " << random_calls_
			<< " with seed " << random_seed_ << '\n';
	}
	return output_random_number;
}

void simple_rng_1_6::set_simulation(bool do_simulation)
{
	*do_simulation_ = do_simulation;
}

void simple_rng_1_6::fill_simulation_stack(const config& cfg)
{
	simulation_->clear();

	DBG_RND << "In fill_simulation_stack\n";
	for(const config& command: cfg.child_range("command"))
	{
		if(const config& recruit = command.child("recruit"))
		{
			std::stringstream ss;
			ss << "Recruit related stack: ";
			DBG_RND << "In fill_simulation_stack, append " << recruit["type"] << " to stack\n";
			config::const_child_itors it_random = command.child_range("random");
			for(const config& random: it_random)
			{
				simulation_->push_back(random["value"].to_int());
				ss << random["value"] << "|";
			}
			DBG_RND << ss.str() << "\n";
		}
		if(const config& attack = command.child("attack"))
		{
			config::const_child_itors it_random = command.child_range("random");
			for(const config& random: it_random)
			{
				simulation_->push_back(random["value"].to_int());
			}
		}
	}

	DBG_RND << "simulation_ stack size is " << simulation_->size() << "\n";
	*current_simulation_ = 0;
}

void simple_rng_1_6::next_side_turn_simulation()
{
	set_simulation(true);
}

int simple_rng_1_6::get_next_simulation()
{
	int output = simulation_->at(*current_simulation_);
	DBG_RND << "Pulling out " << *current_simulation_ << " number from simulation " << output << std::endl;
	++*current_simulation_;
	return output;
}

void simple_rng_1_6::rotate_random()
{
	random_seed_ = random_pool_1_8_;
	random_calls_ = 0;
}

int simple_rng_1_6::get_random_seed() const
{
	return random_seed_;
}

int simple_rng_1_6::get_random_calls() const
{
	return random_calls_;
}

void simple_rng_1_6::seed_random(const int seed, const unsigned call_count)
{
	random_pool_ = seed;
	random_pool_1_8_ = seed;
	random_seed_ = seed;
	for(random_calls_ = 0; random_calls_ < call_count; ++random_calls_) {
		random_next();
	}
	{
		DBG_RND << "Seeded random with " << random_seed_ << " with "
			<< random_calls_ << " calls, pool is now at "
			<< random_pool_1_8_ << '\n';
	}
}

void simple_rng_1_6::random_next()
{
	// Use the simple random generator as shown in man rand(3).
	// The division is done separately since we also want to
	// quickly go the the wanted index in the random list.
	random_pool_ = random_pool_ * 1103515245 + 12345;
	random_pool_1_8_ = random_pool_1_8_ * 1103515245 + 12345;
}


} // ends rand_rng namespace

