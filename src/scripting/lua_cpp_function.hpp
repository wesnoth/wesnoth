/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
 * This namespace makes the possibility to push not just C style functions,
 * but CPP style functions to lua, if they are cast as a boost::function.
 * Using this, for example, C++ method functions may be boost::bind'ed and
 * then pushed into the lua environment and called like usual.
 *
 * They are represented as user data with a call operator, which uses a 
 * dispatcher implemented as a C-style function to retrieve the boost
 * function and execute it. Thus effectively all that we have to provide
 * is a "value type" user data (full userdata, not light userdata) in lua
 * which wraps the boost::function type and implements a garbage collector.
 */

#ifndef LUA_CPP_FUNCTION_HPP_INCLUDED
#define LUA_CPP_FUNCTION_HPP_INCLUDED

#include <boost/function.hpp>

struct lua_State;

namespace lua_cpp {

typedef boost::function<int(lua_State*)> lua_function;

void register_metatable ( lua_State* L );

void push_function( lua_State* L, const lua_function & f );

} // end namespace lua_cpp_func

#endif
