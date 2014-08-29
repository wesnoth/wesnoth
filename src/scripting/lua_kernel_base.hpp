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

class lua_kernel_base {
protected:
	lua_State *mState;
	bool execute(char const *, int, int);
public:
	lua_kernel_base();
	~lua_kernel_base();

	/** Runs a plain script. */
	void run(char const *prog) { execute(prog, 0, 0); }

	void load_package();
};

#endif
