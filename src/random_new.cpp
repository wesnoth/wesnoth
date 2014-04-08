/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "random_new.hpp"
#include "log.hpp"



#include <stdlib.h>
static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)

namespace random_new
{
	rng* generator = new rng();

	rng::rng()
		: random_calls_(0)
	{

	}

	rng::~rng()
	{

	}
	
	unsigned int rng::get_random_calls()
	{
		return random_calls_;
	}

	int rng::next_random()
	{
		random_calls_++;
		return next_random_impl();
	}

	int rng::next_random_impl()
	{
		//getting here means random was called form outsiude a synced context.
		int retv = rand();
		
		LOG_RND << "random_new::rng::next_random returned " << retv;
		return retv;
	}

}