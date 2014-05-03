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
#ifndef RANDOM_NEW_DETERMINISTIC_H_INCLUDED
#define RANDOM_NEW_DETERMINISTIC_H_INCLUDED
#include "random_new.hpp"
#include "simple_rng.hpp"

namespace random_new
{
	/*
		this is the determinstic random class, it behaves similar to the old random class.
		it's only application is at the very start of the scneario.

		or durign the "Deterministic SP mode"
	*/
	class rng_deterministic : public random_new::rng
	{
	public:
		rng_deterministic(rand_rng::simple_rng& gen);
		virtual ~rng_deterministic();
	protected:
		virtual int next_random_impl();
	private:
		rand_rng::simple_rng& generator_;
	};

	//RAII class
	class set_random_determinstic
	{
	public:
		set_random_determinstic(rand_rng::simple_rng& rng);
		~set_random_determinstic();
	private :
		rng* old_rng_;
		rng_deterministic new_rng_;
	};
}

#endif
