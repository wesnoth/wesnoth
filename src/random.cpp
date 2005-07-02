/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <cstdio>

#include "config.hpp"
#include "random.hpp"
#include "wassert.hpp"
#include <sstream>

rng::rng() : random_(NULL)
{}

int rng::get_random()
{
	if (!random_)
		return rand();

	config *random;
	if (!started_ || separator_) {
		// setup the first [random] or find a nested [random]
		started_ = true;
		separator_ = false;
		random = random_->child("random");
		if (random == NULL) {
			random_ = &random_->add_child("random");
			remaining_values_ = "";
			goto new_value;
		}
		random_ = random;
		remaining_values_ = (*random_)["value"];
	}

	// find some remaining value
	while (remaining_values_.empty()) {
		random = random_->child("random");
		if (random == NULL) {
			// no remaining value nor child
			// create a new value and store it, then return it
			new_value:
			int res = rand() & 0x7FFFFFFF;
			std::ostringstream tmp;
			if (!(*random_)["value"].empty())
				tmp << ',';
			tmp << res;
			(*random_)["value"] += tmp.str();
			return res;
		}
		random_ = random;
		remaining_values_ = (*random_)["value"];
	}

	// read the first remaining value and erase it
	int res = atoi(remaining_values_.c_str()); // atoi stops at the comma
	std::string::size_type pos = remaining_values_.find(',');
	if (pos != std::string::npos)
		remaining_values_.erase(0, pos + 1);
	else
		remaining_values_ = "";
	return res;
}

const config* rng::get_random_results()
{
	wassert(random_ != NULL);

	if (separator_)
		get_random();
	return random_->child("results");
}

void rng::set_random_results(const config& cfg)
{
	wassert(random_ != NULL);

	if (separator_)
		get_random();
	random_->clear_children("results");
	random_->add_child("results",cfg);
}

void rng::add_random_separator()
{
	separator_ = true;
}

config* rng::random()
{
	return random_;
}

config* rng::set_random(config* random)
{
	config* old = random_;
	random_ = random;
	started_ = false;
	separator_ = false;
	return old;
}

namespace {
rng* random_generator = NULL;
}

set_random_generator::set_random_generator(rng* r) : old_(random_generator)
{
	random_generator = r;
}

set_random_generator::~set_random_generator()
{
	random_generator = old_;
}

int get_random()
{
	wassert(random_generator!=NULL);
	return random_generator->get_random();
}

const config* get_random_results()
{
	wassert(random_generator!=NULL);
	return random_generator->get_random_results();
}

void set_random_results(const config& cfg)
{
	wassert(random_generator!=NULL);
	random_generator->set_random_results(cfg);
}

void add_random_separator()
{
	wassert(random_generator!=NULL);
	random_generator->add_random_separator();
}

