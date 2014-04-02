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

#ifndef RANDOM_NEW_SYNCED_H_INCLUDED
#define RANDOM_NEW_SYNCED_H_INCLUDED



#include "random_new.hpp"
#include "simple_rng.hpp"

#include <boost/function.hpp>


/*
todo: use a boost::random based solution.
*/
namespace random_new
{
	class synced_rng : public random_new::rng
	{
	public:
		synced_rng(boost::function0<int> seed_generator);
		virtual ~synced_rng();
	protected:
		virtual int next_random_impl();
	private:
		void initialize();
		bool has_valid_seed_;
		boost::function0<int> seed_generator_; 
		//TODO: replayce this by boost::random::mt19937 or similar
		rand_rng::simple_rng gen_;
	};
}
#endif
