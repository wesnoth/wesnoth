/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

class config;

int get_random();
const config* get_random_results();
void set_random_results(const config& cfg);

class rng
{
public:
	rng();
	int get_random();
	const config* get_random_results() const;
	void set_random_results(const config& cfg);

protected:
	config* random_;
};

struct set_random_generator {
	set_random_generator(rng* r);
	~set_random_generator();

private:
	rng* old_;
};

#endif
