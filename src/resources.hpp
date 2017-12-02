/*
   Copyright (C) 2009 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <memory>
#include <vector>

class game_board;
class game_display;
class game_data;
class filter_context;
class game_lua_kernel;
class play_controller;
class fake_unit_manager;
class tod_manager;
class unit_map;
class persist_manager;
class game_classification;
struct mp_game_settings;
class replay;
namespace actions { class undo_list; }

namespace game_events { class manager; }

namespace halo { class manager; }

namespace soundsource { class manager; }

namespace pathfind { class manager; }

namespace wb { class manager; } //whiteboard manager

namespace resources
{
	extern play_controller        *controller;
	extern game_board             *gameboard;
	extern game_data              *gamedata;
	extern game_events::manager   *game_events;
	extern game_lua_kernel              *lua_kernel;     // Set by game_events::manager.
	extern persist_manager        *persist;
	extern game_classification    *classification;
	extern game_display           *screen;
	extern filter_context	      *filter_con;
	extern soundsource::manager   *soundsources;
	extern replay                 *recorder;
	extern fake_unit_manager      *fake_units;
	extern ::tod_manager          *tod_manager;
	extern pathfind::manager      *tunnels;
	extern actions::undo_list     *undo_stack;
	extern unit_map               *units;
	extern std::shared_ptr<wb::manager> whiteboard;
	extern bool                   simulation_;
}
