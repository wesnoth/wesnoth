/*
   Copyright (C) 2013 - 2015 by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include "game_display.hpp"
#include "mp_game_settings.hpp"
#include "network.hpp"

class config;
class game_state;

namespace mp {

config initial_level_config(game_display& disp, const mp_game_settings& params,
	game_state& state);

void level_to_gamestate(config& level, game_state& state);

void check_response(network::connection res, const config& data);

} // end namespace mp

#endif

