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

#ifndef SCRIPTING_LUA_KERNEL_BASE_HPP
#define SCRIPTING_LUA_KERNEL_BASE_HPP

#include <string>                       // for string

struct lua_State;

typedef int (*pcall_fcn_ptr)(lua_State *, int, int);

class lua_kernel_base {
protected:
	lua_State *mState;
	bool execute(char const *, int, int);
public:
	lua_kernel_base();
	virtual ~lua_kernel_base();

	/** Runs a plain script. */
	void run(char const *prog);

	void load_package();

	virtual pcall_fcn_ptr pcall_fcn(); //when running scripts, in the "base" kernel type we should just use pcall. But for the in-game kernel, we want to call the luaW_pcall function instead which extends it using things specific to that api, and returns errors on a WML channel
};

#endif
