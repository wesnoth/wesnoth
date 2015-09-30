/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2015 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef PLAYCAMPAIGN_H_INCLUDED
#define PLAYCAMPAIGN_H_INCLUDED

#include "game_end_exceptions.hpp"

#include <boost/shared_ptr.hpp>

class display;
class game_display;
class saved_game;
class terrain_type_data;
typedef boost::shared_ptr<terrain_type_data> tdata_cache;

class config;

enum io_type_t {
	IO_SERVER,
	IO_CLIENT
};

LEVEL_RESULT play_game(game_display& disp, saved_game& state,
		const config& game_config,
		const tdata_cache & tdata,
		io_type_t io_type=IO_SERVER,
		bool skip_replay = false,
		bool network_game = false,
		bool blindfold_replay = false,
		bool is_unit_test = false,
		bool is_replay = false);
#endif // PLAYCAMPAIGN_H_INCLUDED

