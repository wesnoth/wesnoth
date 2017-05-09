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

#pragma once

/**
 * This namespace contains the implementations for wesnoth's
 * lua bindings for the C++ map location operations.
 */

struct lua_State;

namespace lua_map_location {

int intf_get_direction(lua_State*);
int intf_vector_sum(lua_State*);
int intf_vector_diff(lua_State*);
int intf_vector_negation(lua_State*);
int intf_rotate_right_around_center(lua_State*);
int intf_tiles_adjacent(lua_State*);
int intf_get_adjacent_tiles(lua_State*);
int intf_distance_between(lua_State*);
int intf_get_in_basis_N_NE(lua_State*);
int intf_get_relative_dir(lua_State*);

} // end namespace lua_map_location
