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

typedef struct {
	const char * name;
	lua_function func;
} Reg;

void register_metatable ( lua_State* L );

/**
 * Pushes a boost::function wrapper object onto the stack. It does
 * not support up-values. If you need that use push_closure (a little slower).
 *
 * NOTE: This object has type userdata, not function. Its metatable has a call operator.
 * If this is not sufficient for your needs then use push_closure.
 */
void push_function( lua_State* L, const lua_function & f );

/**
 * Analogous to lua_setfuncs, it registers a collection of function wrapper
 * objects into a table, using push_function.
 *
 * The note above applies.
 */
void set_functions( lua_State* L, const lua_cpp::Reg * l);

/**
 * Pushes a closure which retains a boost::function object as its first up-value.
 * Note that this is *NOT* strictly compatible with the lua c function push_closure --
 * if you request additional upvalues they will be indexed starting at 2 rather than 1.
 *
 * Note that unlike push_function above this results in a function and not userdata
 * being pushed on the stack.
 */
void push_closure( lua_State* L, const lua_function & f, int nup);

/**
 * Analogous to lua_setfuncs and set_functions above, but pushes closures.
 *
 * NOTE: set_functions(L, l, 0) is NOT the same as set_functions(L, l), as
 * the latter produces userdata and the former doesn't.
 */
void set_functions( lua_State* L, const lua_cpp::Reg * l, int nup);

} // end namespace lua_cpp_func

#endif
