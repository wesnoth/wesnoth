/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

/**
 * This namespace contains bindings for lua to hold a mt19937 object
 * as userdata, draw from it, and do the automatic memory management
 */

struct lua_State;

namespace lua_rng {

/** Implementations for lua callbacks */
int impl_rng_create(lua_State*);
int impl_rng_destroy(lua_State*);
int impl_rng_seed(lua_State*);
int impl_rng_draw(lua_State*);

/** Creates the metatable for RNG objects, and adds the Rng table which contains the constructor */
void load_tables(lua_State*);

} // end namespace lua_rng
