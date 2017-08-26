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

/**
 * This namespace makes the possibility to push not just C style functions,
 * but CPP style functions to lua, if they are cast as a std::function.
 * Using this, for example, C++ method functions may be std::bind'ed and
 * then pushed into the lua environment and called like usual.
 *
 * They are represented as user data with a call operator, which uses a
 * dispatcher implemented as a C-style function to retrieve the boost
 * function and execute it. Thus effectively all that we have to provide
 * is a "value type" user data (full userdata, not light userdata) in lua
 * which wraps the std::function type and implements a garbage collector.
 *
 *
 * -- Why? --
 *
 * There are a few basic approaches to connect lua and C++, with various
 * degrees of power. Lua, being a C library, has no concept of C++ specific
 * types, even when compiled as C++. Lua has only two functions which
 * introduce C functions to the scripting environment:
 * lua_pushcfunction, and lua_pushcclosure. (The helper library only provides
 * functions which use these.) These functions can only accept C-style function
 * pointers of type int (lua_State*). Boost bind cannot be used to make a match
 * to this type signature, nor can C++ method functions be used.
 *
 * In many cases C-style functions are sufficient, but if one ever wants to
 * refer to an instance of a class or a member variable (which one does as most
 * of our project is written in C++ object oriented style), it's not enough.
 *
 * The basic lua-provided approach to this is to pass objects as "userdata".
 * Userdata comes in two flavors, "regular" and "light", for representing
 * value and reference types respectively. Lightuserdata is meant to hold a
 * pointer to an object -- full userdata is meant to hold an instance of an
 * object which is either copied with memcpy or constructed using placement new.
 * Proper memory management may require using lua's __gc metamethod to call a
 * destructor. In the normal idiom, every class which is passed to lua this
 * way should have C-function shim provided for each method which may be called
 * by lua -- the object's "this" is retrieved from the userdata type on the
 * stack, and used to call the appopriate method. A metatable is defined, which
 * may be the same as the "lua module" placed in the global namespace which may
 * also provide access to a constructor.
 *
 * This approach is often quite good for small objects. Wesnoth uses full userdata
 * to represent "rng" objects used in lua random map generation, to represent lua
 * "private units", to represent vconfigs and translatable strings. Wesnoth uses
 * lightuserdata for many AI related objects.
 *
 * However it is not ideal for "large" objects like the game engine itself or its
 * helpers. In this case full translation to userdata is out of the question --
 * in case of lightuserdata, it is problematic because the lua api is not actually
 * trying to directly represent a wesnoth engine object (which doesn't exist!)
 * the wesnoth table in lua instead has a medley of callbacks implemented variously
 * by grabbing from the gamemap, the unit map, the game_config namespace, etc. etc.
 * for instance even the wesnoth.game_config table is not backed up by any one object,
 * instead its callbacks may variously alter game_config namespace or the current tod
 * manager object etc. etc.
 *
 * Circa 2012, the solution was to implement every callback function in the wesnoth
 * table simply as a C function, which grabs whatever engine features it needs from
 * a collection of pointers with external linkage in resources.hpp. The pointers
 * must be reset by the play controller object whenver it is created or destroyed...
 * or reset (replay controller), and always in a particular order because eventually
 * all of the objects themselves are also grabbing these pointers, leading to segfaults
 * if they are initialized in the wrong order, or left with danging pointers...
 * in short it was very messy. While the organization of everything as pure C functions
 * allows us to flexibly organize the wesnoth table using subtables holding functions
 * as we like, (which wouldn't be possible if it were based on a lightuserdata), the
 * requirement to base it all on externally-linked pointer variables comes at great cost.
 * Further it means that we can never hope to have a correct deep-copy constructor of the
 * gamestate, meaning that features like "replay moves since my last turn" or "AI based
 * on exploratory simulations" are much more difficult to produce if not impossible.
 *
 * The lua cpp functions code permits us to refactor this by (1) passing all the engine
 * resources needed by the lua environment, to the "lua kernel" at construction time,
 * which holds them as private variables (2) declaring all callbacks which need these
 * as private member functions (3) using boost bind to bind them to the lua kernel and
 * this code to push the result into the scripting environment, at construction time
 * of the lua kernel. Now there is no longer any question about dangling pointers in lua,
 * or issues about making deep copies, since a lua state may be copied and the pointers
 * in the kernel rebound, and the pointers in lua all die when the kernel is destroyed.
 *
 * More generally, this code makes it easy to assemble "mixins" for lua using the member
 * functions of several different classes, if this is desired.
 *
 * The implementation details are also extremely simple -- whereas there are many popular
 * lua -> C++ binding utilities like LuaPlus, luabind, etc. many of these only truly
 * support automatic generation of bindings for *all* of the methods of an object, rely
 * on the userdata approach, and require substantial template metaprogramming which will
 * turn up in any associated stacktraces. This technique used here essentially delegates
 * all of the templating work to boost, and is implemented in only a few hundred lines.
 *
 *
 * -- Caveats: --
 *
 * Essentially, we provide C++ versions of the lua library calls 'lua_pushcfunction',
 * 'lua_setfuncs', 'lua_pushcclosure'.
 *
 * - They are "C++" versions in that they take std::function<int (lua_State*)> rather
 * than int(lua_State*).
 * - While for lua, "lua_pushcfunction(L, f)" is essentially the same as
 * "lua_pushcclosure(L, f, 0)", for the functions below that is not the case.
 * lua_cpp::push_function generates a userdata "helper" object with a _call operator,
 * not technically a function. lua_cpp::push_closure generates a true lua closure.
 * Therefore push_closure is the most general and most compatible form -- push_function
 * is slightly simpler and more efficient though.
 * - Similarly lua_cpp::set_functions(L, l) differs from lua_cpp::set_functions(L,l,nups).
 * - Closures created by lua_cpp::push_closure are not *exactly* the same as lua closures,
 * in that the first upvalue is used by the implementation. A closure created with two
 * upvalues will find them at upvalue indices 2 and 3, and should not touch upvalue 1.
 */

#pragma once

#include "utils/functional.hpp"

#include <vector>

struct lua_State;

namespace lua_cpp {

typedef std::function<int(lua_State*)> lua_function;

typedef struct {
	const char * name;
	lua_function func;
} Reg;

void register_metatable ( lua_State* L );

/**
 * Pushes a std::function wrapper object onto the stack. It does
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
void set_functions( lua_State* L, const std::vector<lua_cpp::Reg>& functions);

/**
 * Analogous to lua_setfuncs, it registers a collection of function wrapper
 * objects into a table, using push_function.
 *
 * The note above applies.
 */
template<int N>
void set_functions( lua_State* L, const lua_cpp::Reg(& functions)[N])
{
	std::vector<lua_cpp::Reg> l;
	l.reserve(N);
	for(int i = 0; i < N; i++) {
		l.push_back(functions[i]);
	}
	set_functions(L, l);
}

/**
 * Pushes a closure which retains a std::function object as its first up-value.
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
void set_functions( lua_State* L, const std::vector<lua_cpp::Reg>& functions, int nup);

/**
 * Analogous to lua_setfuncs and set_functions above, but pushes closures.
 *
 * NOTE: set_functions(L, l, 0) is NOT the same as set_functions(L, l), as
 * the latter produces userdata and the former doesn't.
 */
template<int N>
void set_functions( lua_State* L, const lua_cpp::Reg(& functions)[N], int nup)
{
	std::vector<lua_cpp::Reg> l;
	l.reserve(N);
	for(int i = 0; i < N; i++) {
		l.push_back(functions[i]);
	}
	set_functions(L, l, nup);
}

} // end namespace lua_cpp_func
