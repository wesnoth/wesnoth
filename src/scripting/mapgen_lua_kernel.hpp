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

#ifndef INCLUDED_MAPGEN_LUA_KERNEL
#define INCLUDED_MAPGEN_LUA_KERNEL

#include "scripting/lua_kernel_base.hpp"

class config;

#include <string>

class mapgen_lua_kernel : public lua_kernel_base {
public:
	mapgen_lua_kernel();

	std::string create_map(const char * prog, const config & generator); // throws game::lua_error
	config create_scenario(const char * prog, const config & generator); // throws game::lua_error

private:
	void run_generator(const char * prog, const config & generator);
};

#endif
