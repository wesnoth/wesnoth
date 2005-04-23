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
#include "config.hpp"
#include "random.hpp"
#include "wassert.hpp"

rng::rng() : random_(NULL), separator_(false)
{}

int rng::get_random(int value)
{
	separator_ = false;

	if(random_ == NULL) {
		return value >= 0 ? value : rand();
	}

	//random numbers are in a 'list' meaning that each random
	//number contains another random numbers unless it's at
	//the end of the list. Generating a new random number means
	//nesting a new node inside the current node, and making
	//the current node the new node
	config* const random = random_->child("random");
	if(random == NULL) {
		int res = value;
		if(value < 0) 
			res = rand();
		random_ = &random_->add_child("random");

		char buf[100];
		sprintf(buf,"%d",res);
		(*random_)["value"] = buf;

		return res;
	} else {
		const int res = atol((*random)["value"].c_str());
		random_ = random;
		return res;
	}
}

const config* rng::get_random_results() 
{
	wassert(random_ != NULL);

	if(separator_) {
		get_random(0);
	}
	return random_->child("results");
}

void rng::set_random_results(const config& cfg)
{
	wassert(random_ != NULL);

	if(separator_) {
		get_random(0);
	}
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

