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


#ifndef SCRIPTING_APP_LUA_KERNEL_HPP
#define SCRIPTING_APP_LUA_KERNEL_HPP

#include "scripting/lua_kernel_base.hpp"

class config;
class game_launcher;

class application_lua_kernel : public lua_kernel_base {
public:
	application_lua_kernel();
	bool initialize(game_launcher* gl);

	virtual std::string my_name() { return "Application Lua Kernel"; }

	static int intf_set_script(lua_State * L); /* Registers a lua function as the current script */
	void call_script(const config & cfg); /* Call the current script, with config passed as argument */
};

#endif
