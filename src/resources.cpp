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
	game_board           *gameboard = NULL;
	game_config_manager  *config_manager = NULL;
	play_controller      *controller = NULL;
	game_data            *gamedata = NULL;
	filter_context	     *filter_con = NULL;
	LuaKernel            *lua_kernel = NULL;
	persist_manager      *persist = NULL;
	game_display         *screen = NULL;
	soundsource::manager *soundsources = NULL;
	std::vector<team>    *teams = NULL;
	::tod_manager        *tod_manager = NULL;
	fake_unit_manager    *fake_units = NULL;
	pathfind::manager    *tunnels = NULL;
	actions::undo_list   *undo_stack = NULL;
	unit_map             *units = NULL;
	boost::shared_ptr<wb::manager> whiteboard = boost::shared_ptr<wb::manager>();
	game_classification  *classification = NULL;
	const mp_game_settings *mp_settings = NULL;
	bool                 simulation_ = false;
}
