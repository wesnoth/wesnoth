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

#pragma once

#include "random.hpp"
#include "mt_rng.hpp"

namespace randomness
{
	/**
		This rng is used when the normal synced rng is not available
		this is currently only he case at the very start of the scenario (random generation of starting units traits).

		or during the "Deterministic SP mode"
	*/
	class rng_deterministic : public randomness::rng
	{
	public:
		rng_deterministic(mt_rng& gen);
		virtual ~rng_deterministic();

	protected:
		virtual uint32_t next_random_impl();
	private:
		mt_rng& generator_;
	};

	/**
		RAII class to use rng_deterministic in the current scope.
	*/
	class set_random_determinstic
	{
	public:
		set_random_determinstic(mt_rng& rng);
		~set_random_determinstic();
	private :
		rng* old_rng_;
		rng_deterministic new_rng_;
	};
}
