/*
   Copyright (C) 2014 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SINGLEPLAYER_HPP_INCLUDED
#define SINGLEPLAYER_HPP_INCLUDED

#include "game_launcher.hpp"
#include "create_engine.hpp"
#include "configure_engine.hpp"
#include "connect_engine.hpp"

namespace sp {

bool enter_create_mode(game_display& disp, const config& game_config,
	saved_game& state, jump_to_campaign_info jump_to, bool local_players_only = true);

bool enter_configure_mode(game_display& disp, const config& game_config,
	saved_game& state, jump_to_campaign_info& jump_to_campaign, bool local_players_only = true);

bool enter_connect_mode(game_display& disp, const config& game_config,
	saved_game& state, bool local_players_only = true);

} // end namespace sp

#endif
