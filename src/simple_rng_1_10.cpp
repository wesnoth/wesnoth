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

#include "simple_rng_1_10.hpp"
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
std::vector<int> simulation;
std::vector<int> side_turn_simulation;
int current_simulation;
int current_side_simulation;
bool do_simulation = false;
unsigned last_used_seed_calls = 0;

int disable_simulation = -1;

simple_rng_1_10::simple_rng_1_10() :
	random_seed_(rand() & 0x7FFFFFFF),
	random_pool_(random_seed_),
	random_pool_1_8_(random_seed_),
	random_calls_(0),
	legacy_rng_()
{
}

simple_rng_1_10::simple_rng_1_10(const config& cfg) :
	random_seed_(cfg["random_seed"]),
	random_pool_(random_seed_),
	random_pool_1_8_(random_seed_),
	random_calls_(cfg["random_calls"].to_int(0)),
	legacy_rng_()
{
	if(fallback_to_legacy_rng())
	{
		legacy_rng_ = simple_rng_1_6(cfg);
	}
	for ( unsigned calls = 0; calls < random_calls_; ++calls )
		random_next();
}

void simple_rng_1_10::set_replay_data(const config& cfg)
{
	if(cfg.has_attribute("version"))
	{
		DBG_RND << "Received version: " << cfg["version"] << "\n";
		*loaded_version_ = version_info(cfg["version"]);

		if(fallback_to_legacy_rng())
		{
			legacy_rng_ = rand_rng::simple_rng_1_6(cfg);
			legacy_rng_.set_replay_data(cfg);
		} else {
			//LOG_RND<<"cfg to set_replay_data "<<cfg<<"\n";
			fill_simulation_stack(cfg);
		}
	}
}

int simple_rng_1_10::get_next_random()
{
	int output_random_number;
	random_next();
	++random_calls_;
	if(fallback_to_legacy_rng()) return legacy_rng_.get_next_random();
	if(*do_simulation_) return get_next_simulation();
	if(fallback_to_1_8_rng())
	{
		output_random_number = (random_pool_1_8_ / 65536) % 32768;
		while (output_random_number < 0)
		{
			output_random_number += 32768;
		}
		DBG_RND << "simple_rng_1_10 pulled user random " << random_pool_1_8_
			<< " for call " << random_calls_
			<< " with seed " << random_seed_ << '\n';
	} else {
		output_random_number = (random_pool_ / 65536) % 32768;
		DBG_RND << "simple_rng_1_10 pulled user random " << random_pool_
			<< " for call " << random_calls_
			<< " with seed " << random_seed_ << '\n';
	}
	*last_used_seed_calls_ = random_calls_;
	return output_random_number;
}

void simple_rng_1_10::set_simulation(bool do_simulation)
{
	*do_simulation_ = do_simulation;
}

void simple_rng_1_10::fill_simulation_stack(const config& cfg)
{
	simulation_->clear();
	side_turn_simulation_->clear();

	bool skip_first_init = true;
	bool add_to_stack = true;
	int default_simulation_value = disable_simulation;
	DBG_RND << "In fill_simulation_stack\n";
	for(const config& command: cfg.child_range("command"))
	{
		if(const config& init_side = command.child("init_side"))
		{
			// First [init_side] is garbage, it will be skipped
			if(skip_first_init)
			{
				skip_first_init = false;
				continue;
			}
			side_turn_simulation_->push_back(default_simulation_value);
			add_to_stack = true;
			default_simulation_value = disable_simulation;
		}
		if(const config& recruit = command.child("recruit"))
		{
			if(!add_to_stack) continue;
			// First recruit command since init_side
			if(command.child_count("random") > 0)
			{
				if(default_simulation_value == disable_simulation) default_simulation_value = simulation_->size();
				DBG_RND << "In fill_simulation_stack, append " << recruit["type"] << " to stack\n";
			}
			config::const_child_itors it_random = command.child_range("random");
			for(const config& random: it_random)
			{
				simulation_->push_back(random["value"].to_int());
			}
		}
		if(const config& attack = command.child("attack"))
		{
			// Don't add future recruit numbers for this side-turn
			add_to_stack = false;
		}
	}
	// The savegame is unlikely to finish with an [init_side]
	side_turn_simulation_->push_back(default_simulation_value); // Might be sometimes a duplicate

	DBG_RND << "simulation_ stack size is " << simulation_->size() << "\n";
	DBG_RND << "side_turn_simulation_ stack size is " << side_turn_simulation_->size() << "\n";
	std::stringstream ss;
	ss << "Side turn stack: ";
	for (auto it=side_turn_simulation_->begin() ; it!=side_turn_simulation_->end(); it++)
	{
		ss << *it << "|";
	}
	DBG_RND << ss.str() << "\n";
	*current_simulation_ = 0;
	*current_side_simulation_ = 0;
}

void simple_rng_1_10::next_side_turn_simulation()
{
	if(fallback_to_legacy_rng()) return legacy_rng_.next_side_turn_simulation();
	DBG_RND << "In next_side_turn_simulation\n";
	if(side_turn_simulation_->size() == 0) return;
	*current_simulation_ = side_turn_simulation_->at(*current_side_simulation_);
	DBG_RND << "In next_side_turn_simulation, side turn simulation index: " << *current_side_simulation_ << "\n";
	DBG_RND << "In next_side_turn_simulation, simulation index: " << *current_simulation_ << "\n";
	if(*current_simulation_ != disable_simulation && *current_simulation_ < int(simulation_->size()))
	{
		DBG_RND << "In next_side_turn_simulation, simulated number: " << simulation_->at(*current_simulation_) << "\n";
	}
	if(*current_simulation_ == disable_simulation || *current_simulation_ >= int(simulation_->size())) set_simulation(false);
	else set_simulation(true);
	++*current_side_simulation_;
}

int simple_rng_1_10::get_next_simulation()
{
	int output = simulation_->at(*current_simulation_);
	// If last number for this side_turn, turn simulation off
	int next_simulation = disable_simulation;
	for(int i=*current_side_simulation_; i<int(side_turn_simulation_->size()); i++)
	{
		if(side_turn_simulation_->at(i) != disable_simulation)
		{
			next_simulation = side_turn_simulation_->at(i);
			break;
		}
	}
	if (next_simulation != disable_simulation && *current_simulation_ >= next_simulation-1) set_simulation(false);
	if (*current_simulation_ >= int(simulation_->size())-1) set_simulation(false);
	++*current_simulation_;
	return output;
}

void simple_rng_1_10::rotate_random()
{
	if(fallback_to_legacy_rng()) legacy_rng_.rotate_random();
	if(fallback_to_1_8_rng()) random_seed_ = random_pool_1_8_;
	else random_seed_ = random_pool_ & 0x7FFFFFFF;
	random_calls_ = 0;
}

int simple_rng_1_10::get_random_seed() const
{
	if(fallback_to_legacy_rng()) return legacy_rng_.get_random_seed();
	else return random_seed_;
}

int simple_rng_1_10::get_random_calls() const
{
	if(fallback_to_legacy_rng()) return legacy_rng_.get_random_calls();
	else return random_calls_;
}

void simple_rng_1_10::seed_random(const int seed, const unsigned call_count)
{
	unsigned count = call_count;
	if(!fallback_to_legacy_rng() && call_count != 0)
	{
		if(call_count < *last_used_seed_calls_) // The unusual case: could be in case of berserk, ...
		{
			DBG_RND << "Pulled more random_calls " << *last_used_seed_calls_ << " than expected " << call_count << std::endl;
			count = *last_used_seed_calls_;
		}
		else if(call_count > *last_used_seed_calls_) // The usual case: arbitrary in replay or attack less than total strikes
		{
			// Big hack (based on wild assumptions) with magic numbers
			if((call_count - *last_used_seed_calls_) <= 7) // Assume this is attack less than estimated (as total strikes)
			{
			    DBG_RND << "Override random_calls " << call_count << " with current used number " << *last_used_seed_calls_ << std::endl;
				count = *last_used_seed_calls_;
			}
			else // Assume this is arbitrary random number calls in attack in replay
			{
				// Keep call_count
			}
		}
		*last_used_seed_calls_ = 0;
	}
	random_pool_ = seed;
	random_pool_1_8_ = seed;
	random_seed_ = seed;
	for(random_calls_ = 0; random_calls_ < count; ++random_calls_) {
		++(*last_used_seed_calls_);
		random_next();
	}
	if(fallback_to_legacy_rng())
		legacy_rng_.seed_random(seed, call_count);
	else if(fallback_to_1_8_rng())
		DBG_RND << "Seeded random with " << random_seed_ << " with "
			<< random_calls_ << " calls, pool is now at "
			<< random_pool_1_8_ << '\n';
	else
		DBG_RND << "Seeded random with " << random_seed_ << " with "
			<< random_calls_ << " calls, pool is now at "
			<< random_pool_ << '\n';
}

void simple_rng_1_10::random_next()
{
	// Use the simple random generator as shown in man rand(3).
	// The division is done separately since we also want to
	// quickly go the the wanted index in the random list.
	random_pool_ = random_pool_ * 1103515245 + 12345;
	random_pool_1_8_ = random_pool_1_8_ * 1103515245 + 12345;
}


} // ends rand_rng namespace

