/* $Id$ */
/*
   Copyright (C) 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_HPP
#define SCRIPTING_LUA_HPP

#include "game_events.hpp"

struct lua_State;

class LuaKernel
{
	lua_State *mState;
	bool execute(char const *, int, int);
public:
	LuaKernel();
	~LuaKernel();
	void run_event(vconfig const &, game_events::queued_event const &);
	bool run_filter(char const *name, unit const &u);
	/** Runs a plain script. */
	void run(char const *prog) { execute(prog, 0, 0); }
};

#endif
