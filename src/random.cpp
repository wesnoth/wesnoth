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
#include "util.hpp"
#include "wassert.hpp"
#include <sstream>

rng::rng() : random_(NULL), random_child_(0)
{}

int rng::get_random()
{
	if (!random_)
		return rand() & 0x7FFFFFFF;

	const config::child_list random(random_->get_children("random"));
	if (random_child_ >= random.size()) {
		random_child_ = random.size();
		int res = rand() & 0x7FFFFFFF;
		(random_->add_child("random"))["value"] = lexical_cast<std::string>(res);
		return res;
	} else {
		return lexical_cast_default<int>((*random[random_child_++])["value"], 0);
	}
}

const config* rng::get_random_results()
{
	wassert(random_ != NULL);

	const config::child_list random(random_->get_children("random"));
	if (random_child_ >= random.size()) return NULL;
	return random[random_child_]->child("results");
}

void rng::set_random_results(const config& cfg)
{
	wassert(random_ != NULL);

	const config::child_list random(random_->get_children("random"));
	if (random_child_ >= random.size()) return;
	random[random_child_]->clear_children("results");
	random[random_child_]->add_child("results",cfg);
}

config* rng::random()
{
	return random_;
}

config* rng::set_random(config* random)
{
	config* old = random_;
	random_ = random;
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
