/*
   Copyright (C) 2009 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
	game_board           *gameboard = nullptr;
	play_controller      *controller = nullptr;
	game_data            *gamedata = nullptr;
	filter_context	     *filter_con = nullptr;
	game_events::manager *game_events = nullptr;
	game_lua_kernel            *lua_kernel = nullptr;
	persist_manager      *persist = nullptr;
	soundsource::manager *soundsources = nullptr;
	replay               *recorder = nullptr;
	::tod_manager        *tod_manager = nullptr;
	fake_unit_manager    *fake_units = nullptr;
	pathfind::manager    *tunnels = nullptr;
	actions::undo_list   *undo_stack = nullptr;
	unit_map             *units = nullptr;
	std::shared_ptr<wb::manager> whiteboard = std::shared_ptr<wb::manager>();
	game_classification  *classification = nullptr;
	bool                 simulation_ = false;
}
