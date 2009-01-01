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
/**
 *  @file random.cpp
 *  Generate random numbers.
 * 
 *  There are various ways to get a random number.
 *  rand()              This can be used for things that never are send over the
 *                      network e.g. generate a random map (the final result the
 *                      map is send, but the other players don't need to generate
 *                      the map.
 * 
 *  get_random()        A random generator which is syncronized over the network
 *                      this only seems to work when it's used by 1 player at the
 *                      same time. It's syncronized after an event so if an event
 *                      runs at two clients at the same time it gets out of sync
 *                      and sets the entire game out of sync.
 * 
 *  game_state::get_random()
 *                      A random generator which is seeded by the host of an MP
 *                      game. This generator is (not yet) synchronized over the
 *                      network. It's only used by [set_variable]rand=. The map
 *                      designer has to make sure it stays in sync. This
 *                      generator can be used at the same time at multiple client
 *                      since the generators are always in sync.
 */

#include "global.hpp"

#include "config.hpp"
#include "random.hpp"


namespace {
  rand_rng::rng *random_generator = NULL ;
}


int get_random()
{
  assert(random_generator!=NULL);
  int r = random_generator->get_random(); 
  return r ;
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


namespace rand_rng
{

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


set_random_generator::set_random_generator(rng* r) : old_(random_generator)
{
	random_generator = r;
}

set_random_generator::~set_random_generator()
{
	random_generator = old_;
}

simple_rng::simple_rng() :
    random_seed_(rand()),
    random_pool_(random_seed_),
    random_calls_(0)
{ }

simple_rng::simple_rng(const config& cfg) :
    /**
	 * @todo  older savegames don't have random_seed stored, evaluate later
     * whether default can be removed again. Look after branching 1.5.
	 */
    random_seed_(lexical_cast_default<int>(cfg["random_seed"], 42)),
    random_pool_(random_seed_),
    random_calls_(0)
{}

int simple_rng::get_random()
{
	random_next();
	++random_calls_;
	//DBG_NG << "pulled user random " << random_pool_
	//	<< " for call " << random_calls_ << '\n';

	return (static_cast<unsigned>(random_pool_ / 65536) % 32768);
}

void simple_rng::seed_random(const unsigned call_count)
{
    seed_random(random_seed_, call_count);
}

void simple_rng::seed_random(const int seed, const unsigned call_count)
{
	random_pool_ = seed;
	random_seed_ = seed;
	for(random_calls_ = 0; random_calls_ < call_count; ++random_calls_) {
		random_next();
	}
	//DBG_NG << "Seeded random with " << random_seed_ << " with "
	//	<< random_calls_ << " calls, pool is now at "
	//	<< random_pool_ << '\n';
}

void simple_rng::random_next()
{
	// Use the simple random generator as shown in man rand(3).
	// The division is done separately since we also want to
	// quickly go the the wanted index in the random list.
	random_pool_ = random_pool_ * 1103515245 + 12345;
}


} // ends rand_rng namespace

