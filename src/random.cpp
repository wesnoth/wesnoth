/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
/**
 *  @file
 *  Generate random numbers.
 *
 *  There are various ways to get a random number.
 *  rand()              This can be used for things that never are send over the
 *                      network e.g. generate a random map (the final result the
 *                      map is send, but the other players don't need to generate
 *                      the map.
 *
 *  get_random()        A random generator which is synchronized over the network
 *                      this only seems to work when it's used by 1 player at the
 *                      same time. It's synchronized after an event so if an event
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
#include "serialization/string_utils.hpp"
#include "simple_rng.hpp"
#include "util.hpp"

namespace rand_rng
{
}
