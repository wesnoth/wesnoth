/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2009 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file random.hpp */

#ifndef RNG_HPP_INCLUDED
#define RNG_HPP_INCLUDED

#include "random.hpp"

namespace rand_rng
{

class rng
{
public:
	rng();
	int get_random();

	const config* get_random_results();
	void set_random_results(const config& cfg);

protected:
	config* random();
	void set_random(config*);

private:
	config* random_;
	size_t random_child_;
};

} // ends rand_rng namespace

#endif
