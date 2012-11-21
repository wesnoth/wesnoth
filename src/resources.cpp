/* $Id$ */
/*
   Copyright (C) 2009 - 2012 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
	game_display *screen = NULL;
	soundsource::manager *soundsources = NULL;
	gamemap *game_map = NULL;
	unit_map *units = NULL;
	std::vector<team> *teams = NULL;
	game_state *state_of_game = NULL;
	game_data *gamedata = NULL;
	LuaKernel *lua_kernel = NULL;
	play_controller *controller = NULL;
	::tod_manager *tod_manager = NULL;
	pathfind::manager *tunnels = NULL;
	wb::manager *whiteboard = NULL;
	undo_list *undo_stack = NULL;
	persist_manager *persist = NULL;
}
