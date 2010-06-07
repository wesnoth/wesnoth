/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "resources.hpp"

namespace resources
{
	game_display *screen;
	soundsource::manager *soundsources;
	gamemap *game_map;
	unit_map *units;
	std::vector<team> *teams;
	game_state *state_of_game;
	lua::LuaKernel *lua_kernel;
	play_controller *controller;
	::tod_manager *tod_manager;
	wb::manager *whiteboard;
}
