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
/** Ensures that the next "random results" operations will not happen on the
 * current random context, eventually adding a dummy random context if
 * necessary.
 */
void add_random_separator();

class rng
{
public:
	rng();
	int get_random();

	const config* get_random_results();
	void set_random_results(const config& cfg);

	void add_random_separator();
protected:
	config* random();
	config* set_random(config*);

private:
	config* random_;
	bool separator_, started_;
	std::string remaining_values_;
};

struct set_random_generator {
	set_random_generator(rng* r);
	~set_random_generator();

private:
	rng* old_;
};

#endif
