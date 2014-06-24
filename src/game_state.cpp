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

#include "game_state.hpp"

#include "pathfind/teleport.hpp"


game_state::game_state(const config & level, const config & game_config) :
	level_(level),
	gamedata_(level_),
	board_(game_config,level_),
	tod_manager_(level_),
	pathfind_manager_()
{}

game_state::~game_state() {}

