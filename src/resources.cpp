/*
   Copyright (C) 2009 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "resources.hpp"
#include <cstddef>

namespace resources
{
	game_config_manager  *config_manager = NULL;
	play_controller      *controller = NULL;
	game_data            *gamedata = NULL;
	gamemap              *game_map = NULL;
	LuaKernel            *lua_kernel = NULL;
	persist_manager      *persist = NULL;
	game_display         *screen = NULL;
	soundsource::manager *soundsources = NULL;
	game_state           *state_of_game = NULL;
	std::vector<team>    *teams = NULL;
	::tod_manager        *tod_manager = NULL;
	pathfind::manager    *tunnels = NULL;
	actions::undo_list   *undo_stack = NULL;
	unit_map             *units = NULL;
}
