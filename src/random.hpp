/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2010 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file random.hpp */

#ifndef RANDOM_HPP_INCLUDED
#define RANDOM_HPP_INCLUDED

class config;

int get_random();
const config* get_random_results();
void set_random_results(const config& cfg);

namespace rand_rng
{

typedef unsigned int seed_t;

class rng;

struct set_random_generator {
	set_random_generator(rng* r);
	~set_random_generator();
private:
	rng* old_;
};

} // ends rand_rng namespace

#endif
