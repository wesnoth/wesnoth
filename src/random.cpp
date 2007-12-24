/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2007 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file random.cpp
//! Generate random numbers.
//!
//! There are various ways to get a random number.
//! rand()              This can be used for things that never are send over the 
//!                     network e.g. generate a random map (the final result the
//!                     map is send, but the other players don't need to generate
//!                     the map.
//!
//! get_random()        A random generator which is syncronized over the network
//!                     this only seems to work when it's used by 1 player at the
//!                     same time. It's syncronized after an event so if an event
//!                     runs at two clients at the same time it gets out of sync
//!                     and sets the entire game out of sync.
//!
//! game_state::get_random()
//!                     A random generator which is seeded by the host of an MP
//!                     game. This generator is (not yet) synchronized over the
//!                     network. It's only used by [set_variable]rand=. The map
//!                     designer has to make sure it stays in sync. This 
//!                     generator can be used at the same time at multiple client
//!                     since the generators are always in sync.

#include "global.hpp"

#include "config.hpp"
#include "random.hpp"
#include "util.hpp"

#include <cassert>
#include <cstdio>
#include <sstream>

rng::rng() : random_(NULL), random_child_(0)
{}

int rng::get_random()
{
	if (!random_)
		return rand() & 0x7FFFFFFF;

	const config::child_list random(random_->get_children("random"));
	if (random_child_ >= random.size()) {
		random_child_ = random.size() + 1;
		int res = rand() & 0x7FFFFFFF;
		(random_->add_child("random"))["value"] = lexical_cast<std::string>(res);
		return res;
	} else {
		return lexical_cast_default<int>((*random[random_child_++])["value"], 0);
	}
}

const config* rng::get_random_results()
{
	assert(random_ != NULL);

	const config::child_list random(random_->get_children("random"));
	if (random_child_ <= 0 ||random_child_ > random.size()) return NULL;
	return random[random_child_-1]->child("results");
}

void rng::set_random_results(const config& cfg)
{
	assert(random_ != NULL);

	const config::child_list random(random_->get_children("random"));
	if (random_child_ <= 0 || random_child_ > random.size()) return;
	random[random_child_-1]->clear_children("results");
	random[random_child_-1]->add_child("results",cfg);
}

config* rng::random()
{
	return random_;
}

void rng::set_random(config* random)
{
	random_ = random;
	random_child_ = 0;
	return;
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
	assert(random_generator!=NULL);
	return random_generator->get_random();
}

const config* get_random_results()
{
	assert(random_generator!=NULL);
	return random_generator->get_random_results();
}

void set_random_results(const config& cfg)
{
	assert(random_generator!=NULL);
	random_generator->set_random_results(cfg);
}
