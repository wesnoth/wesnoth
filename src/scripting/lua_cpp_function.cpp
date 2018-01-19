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

#include "scripting/lua_cpp_function.hpp"

#include "log.hpp"

#include <sstream>
#include <string>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "scripting/lua_common.hpp" // for new(L)

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua_cpp {

char const * cpp_function = "CPP_Function";

static int intf_dispatcher ( lua_State* L )
{
	//make a temporary copy, in case lua_remove(L,1) might cause lua to garbage collect and destroy it
	lua_function f = * static_cast<lua_function *> (luaL_checkudata(L, 1, cpp_function));
	// remove from the stack before executing, so that like all other callbacks, f finds only its intended arguments on the stack.
	lua_remove(L,1);
	int result = (f)(L);
	return result;
}

static int intf_cleanup ( lua_State* L )
{
	lua_function * d = static_cast< lua_function *> (luaL_testudata(L, 1, cpp_function));
	if (d == nullptr) {
		ERR_LUA << "lua_cpp::intf_cleanup called on data of type: " << lua_typename( L, lua_type( L, 1 ) ) << std::endl;
		ERR_LUA << "This may indicate a memory leak, please report at bugs.wesnoth.org" << std::endl;
		lua_pushstring(L, "C++ function object garbage collection failure");
		lua_error(L);
	} else {
		d->~lua_function();
	}
	return 0;
}

static int intf_tostring( lua_State* L )
{
	lua_function * d = static_cast< lua_function *> (luaL_checkudata(L, 1, cpp_function));
	// d is not null, if it was null then checkudata raised a lua error and a longjump was executed.
	std::stringstream result;
	result << "c++ function: " << std::hex << d;
	lua_pushstring(L, result.str().c_str());
	return 1;
}

void register_metatable ( lua_State* L )
{
	luaL_newmetatable(L, cpp_function);
	lua_pushcfunction(L, intf_dispatcher);
	lua_setfield(L, -2, "__call");
	lua_pushcfunction(L, intf_cleanup);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, intf_tostring);
	lua_setfield(L, -2, "__tostring");
	lua_pushvalue(L, -1); //make a copy of this table, set it to be its own __index table
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1);
}

void push_function( lua_State* L, const lua_function & f )
{
	new(L) lua_function(f);
	luaL_setmetatable(L, cpp_function);
}

void set_functions( lua_State* L, const std::vector<lua_cpp::Reg>& functions)
{
	luaL_checkversion(L);
	for (const lua_cpp::Reg& l : functions) {  /* fill the table with given functions */
		if (l.name != nullptr) {
			push_function(L, l.func);
			lua_setfield(L, -2, l.name);
		}
	}
}

static int intf_closure_dispatcher( lua_State* L )
{
	lua_function * f = static_cast< lua_function *> (luaL_checkudata(L, lua_upvalueindex(1), cpp_function)); //assume the std::function is the first upvalue
	return (*f)(L);
}

void push_closure( lua_State* L, const lua_function & f, int nup)
{
	push_function(L, f);
	lua_insert(L, -(1+nup)); //move the function beneath the upvalues
	lua_pushcclosure(L, &intf_closure_dispatcher, 1+nup);
}

void set_functions( lua_State* L, const std::vector<lua_cpp::Reg>& functions, int nup )
{
	luaL_checkversion(L);
	luaL_checkstack(L, nup+1, "too many upvalues");
	for (const lua_cpp::Reg& l : functions) {  /* fill the table with given functions */
		if (l.name == nullptr) {
			continue;
		}
		int i;
		for (i = 0; i < nup; ++i)  /* copy upvalues to the top */
			lua_pushvalue(L, -nup);
		push_closure(L, l.func, nup);  /* closure with those upvalues */
		lua_setfield(L, -(nup + 2), l.name);
	}
	lua_pop(L, nup);  /* remove upvalues */
}

} // end namespace lua_cpp
