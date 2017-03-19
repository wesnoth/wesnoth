/*
   Copyright (C) 2013 - 2017 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MP_GAME_UTILS_HPP_INCLUDED
#define MP_GAME_UTILS_HPP_INCLUDED

#include "mp_game_settings.hpp"

class config;
class saved_game;

namespace mp {

config initial_level_config(saved_game& state);

void level_to_gamestate(const config& level, saved_game& state);

void check_response(bool res, const config& data);

} // end namespace mp

#endif

