/*
   Copyright (C) 2016 by Jyrki Vesterinen <sandgtx@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_UNIT_PREFERENCES_HPP
#define SCRIPTING_LUA_UNIT_PREFERENCES_HPP

#include <string>

struct lua_State;

namespace lua_preferences
{
	std::string register_table(lua_State* L);
}

#endif
