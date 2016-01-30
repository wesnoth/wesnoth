/*
   Copyright (C) 2014 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef RANDOM_NEW_SYNCED_H_INCLUDED
#define RANDOM_NEW_SYNCED_H_INCLUDED



#include "random_new.hpp"
#include "mt_rng.hpp"

#include "utils/boost_function_guarded.hpp"


/*
todo: use a boost::random based solution.
*/
namespace random_new
{
	class synced_rng : public random_new::rng
	{
	public:
		synced_rng(boost::function0<std::string> seed_generator);
		virtual ~synced_rng();

	protected:
		virtual uint32_t next_random_impl();
	private:
		void initialize();
		bool has_valid_seed_;
		boost::function0<std::string> seed_generator_;
		rand_rng::mt_rng gen_;
	};
}
#endif
