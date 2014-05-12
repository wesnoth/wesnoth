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
#include "seed_rng.hpp"
#include "config.hpp"
#include "log.hpp"
#include <sstream>
#include <iomanip>
static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)


namespace rand_rng
{

mt_rng::mt_rng() :
	random_seed_(seed_rng::next_seed()),
	mt_(random_seed_),
	random_calls_(0)
{
}

mt_rng::mt_rng(const config& cfg) :
	random_seed_(42),
	mt_(random_seed_), //we don't have the seed at construction time, we have to seed after construction in this case. Constructing an mt19937 is somewhat expensive, apparently has about 2kb of private memory. 
	random_calls_(0)
{
	config::attribute_value seed = cfg["random_seed"];
	seed_random(seed.str(), cfg["random_calls"].to_int(0));
} 

bool mt_rng::operator== (const mt_rng & other) const {
	return random_seed_ == other.random_seed_  
	    && random_calls_ == other.random_calls_
	    && mt_ == other.mt_;
}

uint32_t mt_rng::get_next_random()
{
	uint32_t result = mt_();
	++random_calls_;
	DBG_RND << "pulled user random " << result
		<< " for call " << random_calls_
		<< " with seed " << std::hex << random_seed_ << '\n';

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
	DBG_RND << "Seeded random with " << std::hex << random_seed_ << " with "
		<< random_calls_ << " calls." << std::endl;
}

void mt_rng::seed_random(const std::string & seed_str, const unsigned int call_count)
{
	uint32_t new_seed; 
	std::istringstream s(seed_str); 
	s >> std::hex >> new_seed;
	seed_random(new_seed, call_count);
}

std::string mt_rng::get_random_seed_str() const {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << random_seed_;
	return stream.str();
}

void mt_rng::discard(const unsigned int call_count)
{
	for(unsigned int i = 0; i < call_count; ++i) {
		get_next_random();
	}
}

} // ends rand_rng namespace

