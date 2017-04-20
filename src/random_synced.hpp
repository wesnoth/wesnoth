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

#ifndef RANDOM_SYNCED_H_INCLUDED
#define RANDOM_SYNCED_H_INCLUDED

#include "random.hpp"
#include "mt_rng.hpp"

#include "utils/functional.hpp"

/*
todo: use a boost::random based solution.
*/
namespace randomness
{
	class synced_rng : public randomness::rng
	{
	public:
		synced_rng(std::function<std::string()> seed_generator);
		virtual ~synced_rng();

	protected:
		virtual uint32_t next_random_impl();
	private:
		void initialize();
		bool has_valid_seed_;
		std::function<std::string()> seed_generator_;
		mt_rng gen_;
	};
}
#endif
