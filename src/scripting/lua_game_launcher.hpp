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

#ifndef SCRIPTING_LUA_GAME_LAUNCHER_HPP
#define SCRIPTING_LUA_GAME_LAUNCHER_HPP

class game_launcher;
struct lua_State;

namespace lua_game_launcher {
	void define_metatable(lua_State* L); /* Defines the metatable of the game launcher and adds to the registry */
	void add_table(lua_State* L, game_launcher * gl); /* Adds an instance of game launcher to the lua environment */
}

#endif
