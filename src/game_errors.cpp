/*
   Copyright (C) 2011 - 2014 by mark de wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_errors.hpp"

std::string game::load_game_exception::game;
bool game::load_game_exception::show_replay;
bool game::load_game_exception::cancel_orders;
bool game::load_game_exception::select_difficulty;
std::string game::load_game_exception::difficulty;
