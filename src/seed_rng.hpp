/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/*
   This file provides a name space to store a source for seeds for
   prgs. It should be boost::random_device on platforms that provide
   this with our version of boost random, and otherwise should be the
   system time I suppose.

   The seed_rng::next_seed function provided probably shouldn't be used
   anywhere except for default constructors of prg classes, or similar.
*/

#pragma once

#include <cstdint>
#include <string>

namespace seed_rng {

uint32_t next_seed();
std::string next_seed_str();

} // ends seed_rng namespace
