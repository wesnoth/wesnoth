/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "unit_types.hpp"
#include "video.hpp"

//an object which guarantees that when it is destroyed, a 'leave game'
//message will be sent to any hosts still connected.
struct network_game_manager {
	network_game_manager() {}
	~network_game_manager();
};

//function to host a multiplayer game. If server is true, then the
//game will accept connections from foreign hosts. Otherwise it'll
//just use existing network connections for players, or the game
//is an entirely local game
int play_multiplayer(display& disp, game_data& units_data,
                      const config& cfg, game_state& state, bool server=true);

#endif
