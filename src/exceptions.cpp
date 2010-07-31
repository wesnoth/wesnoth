/* $Id$ */
/*
   Copyright (C) 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "exceptions.hpp"
#include "game_errors.hpp"
#include "game_end_exceptions.hpp"
#include "video.hpp"

char const *game::exception::sticky;

void game::exception::rethrow()
{
	if (!sticky) return;
	if (strcmp(sticky, "quit") == 0) throw CVideo::quit();
	if (strcmp(sticky, "load game") == 0) throw game::load_game_exception();
	if (strcmp(sticky, "end level") == 0) throw end_level_exception(QUIT);
	throw game::exception("Unknown exception", "unknown");
}

std::string game::load_game_exception::game;
bool game::load_game_exception::show_replay;
bool game::load_game_exception::cancel_orders;
