/*
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#ifndef LUA_FORMULA_BRIDGE_HPP_INCLUDED
#define LUA_FORMULA_BRIDGE_HPP_INCLUDED

struct lua_State;

namespace lua_formula_bridge {
	
	int intf_eval_formula(lua_State*);
	
} // end namespace lua_formula_bridge

#endif
