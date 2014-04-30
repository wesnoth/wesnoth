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
#ifndef RANDOM_NEW_H_INCLUDED
#define RANDOM_NEW_H_INCLUDED

#include <stdint.h>

namespace random_new
{
	//this class does NOT give synced random results.
	class rng
	{
	public:
		rng();
		uint32_t next_random();
		virtual ~rng();
		unsigned int get_random_calls();
	protected:
		virtual uint32_t next_random_impl();
		unsigned int random_calls_;
	};

	// this generator is autmatilcy synced during synced context.
	// calling this should automaticly clear the undo stack.
	// 
	extern rng* generator;
	
}
#endif
