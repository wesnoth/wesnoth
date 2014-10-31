/*
   Copyright (C) 2014 by David White <dave@whitevine.net>
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

namespace random_new
{
	/**
		this class does not give synced random results derived classes might do.
	*/
	class rng
	{
	public:
		rng();
		int next_random();
		virtual ~rng();
		unsigned int get_random_calls();
	protected:
		virtual int next_random_impl();
		unsigned int random_calls_;
	};

	/**
		This generator is automatically synced during synced context.
		Calling this rng during a synced context automatically makes undoing impossible.
		Outside a synced context this has the same effect as rand()
	*/
	extern rng* generator;

}
#endif
