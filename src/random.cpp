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
#include "log.hpp"
#include "network.hpp"
#include "random.hpp"
#include "rng.hpp"
#include "serialization/string_utils.hpp"
#include "simple_rng.hpp"
#include "util.hpp"

static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)

namespace {
  rand_rng::rng *random_generator = NULL ;
  rand_rng::seed_t last_seed;
  bool seed_valid = false;
  boost::function<void (rand_rng::seed_t)> new_seed_callback;
}


int get_random()
{
  assert(random_generator!=NULL);
  int r = random_generator->get_random();
  return r ;
}

int get_random_nocheck()
{
  assert(random_generator!=NULL);
  int r = random_generator->get_random_nocheck();
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

void set_seed(seed_t seed)
{
	LOG_RND << "set_seed with " << seed << "\n";
	assert(random_generator!=NULL);
	last_seed = seed;
	seed_valid = true;
	random_generator->set_seed(seed);
	if (new_seed_callback) {
		LOG_RND << "set_seed calling new_seed_callback\n";
		new_seed_callback(seed);
	}
}

void invalidate_seed()
{
	LOG_RND << "invalidate_seed\n";
	assert(random_generator!=NULL);
	last_seed = rand();
	if (has_valid_seed()) { //aka SRNG is disabled
		random_generator->set_seed(last_seed);
	}
	seed_valid = false;
}

bool has_valid_seed()
{
	//if we're in a SP game the seed is always valid
	return (network::nconnections() == 0) || seed_valid;
}

seed_t get_last_seed()
{
	return last_seed;
}

void set_new_seed_callback(boost::function<void (seed_t)> f)
{
	DBG_RND << "set_new_seed_callback\n";
	new_seed_callback = f;
}

void clear_new_seed_callback()
{
	DBG_RND << "clear_new_seed_callback\n";
	new_seed_callback = NULL;
}



rng::rng() : random_(NULL), random_child_(0), generator_()
{
}

int rng::get_random()
{
	return get_random_private(true);
}

int rng::get_random_nocheck()
{
	return get_random_private(false);
}

int rng::get_random_private(bool check)
{
	if (!random_) {
		int r = generator_.get_next_random();
		LOG_RND << "get_random() returning " << r << " (random_ is null)\n";
		return r;
	}

	size_t random_size = random_->child_count("random");
	if (random_child_ >= random_size) {
		random_child_ = random_size + 1;
		int res = generator_.get_next_random() & 0x7FFFFFFF;
		(random_->add_child("random"))["value"] = res;
		LOG_RND << "get_random() returning " << res << " (added to random_)\n";
		return res;
	} else {
		int mine = generator_.get_next_random();
		int stored = random_->child("random", random_child_++)["value"];
		if (mine != stored) {
			if (check) {
				ERR_RND << "Random number mismatch, mine " << mine << " vs " << stored << "\n";
				//OOS here?
			} else {
				LOG_RND << "Random number mismatch (nocheck), mine " << mine << " vs " << stored << "\n";
			}
		}
		LOG_RND << "get_random() returning " << stored << "\n";
		return stored;
	}
}

const config* rng::get_random_results()
{
	assert(random_ != NULL);

	if (random_child_ <= 0 ||random_child_ > random_->child_count("random")) return NULL;
	const config &res = random_->child("random", random_child_ - 1).child("results");
	return res ? &res : NULL;
}

void rng::set_random_results(const config& cfg)
{
	assert(random_ != NULL);

	if (random_child_ <= 0 ||random_child_ > random_->child_count("random")) return;
	config &r = random_->child("random", random_child_ - 1);
	r.clear_children("results");
	r.add_child("results", cfg);
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

void rng::set_seed(seed_t seed)
{
	LOG_RND << "Set random seed to " << seed << "\n";
	generator_.seed_random(seed, 0);
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
{
}

simple_rng::simple_rng(const config& cfg) :
    /**
	 * @todo  older savegames don't have random_seed stored, evaluate later
     * whether default can be removed again. Look after branching 1.5.
	 */
    random_seed_(cfg["random_seed"].to_int(42)),
    random_pool_(random_seed_),
    random_calls_(0)
{
}

int simple_rng::get_next_random()
{
	random_next();
	++random_calls_;
	DBG_RND << "pulled user random " << random_pool_
		<< " for call " << random_calls_
		<< " with seed " << random_seed_ << '\n';

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
	DBG_RND << "Seeded random with " << random_seed_ << " with "
		<< random_calls_ << " calls, pool is now at "
		<< random_pool_ << '\n';
}

void simple_rng::random_next()
{
	// Use the simple random generator as shown in man rand(3).
	// The division is done separately since we also want to
	// quickly go the the wanted index in the random list.
	random_pool_ = random_pool_ * 1103515245 + 12345;
}


} // ends rand_rng namespace

