/*
   Copyright (C) 2014 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "random_synced.hpp"
#include "log.hpp"

static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)

namespace randomness
{
	synced_rng::synced_rng(std::function<std::string()> seed_generator)
		: has_valid_seed_(false), seed_generator_(seed_generator), gen_()
	{
	}
	uint32_t synced_rng::next_random_impl()
	{
		if(!has_valid_seed_)
		{
			initialize();
		}
		//getting here means random was called form inside a synced context.
		uint32_t retv = gen_.get_next_random();

		LOG_RND << "randomness::rng::next_random_impl returned " << retv << std::endl;
		return retv;
	}

	void synced_rng::initialize()
	{
		std::string return_val = seed_generator_();
		std::string new_seed;
		std::string random_calls = "0";
		if(return_val.front() == ';')
		{
			size_t pos = 1;
			size_t split = return_val.find(';', pos);
			new_seed.assign(return_val, /*pos=*/ pos, /*len=*/ split-1);
			random_calls.assign(return_val, /*pos=*/ split+1, /* len=npos*/ std::string::npos);
		} else {
			new_seed = return_val;
		}
		gen_.seed_random(new_seed, std::stoi(random_calls));
		has_valid_seed_ = true;
	}

	synced_rng::~synced_rng()
	{

	}

	bool synced_rng::is_networked() const
	{
		return true;
	}
}
