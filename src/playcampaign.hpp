/* $Id$ */
/*
   Copyright (C) 2003-2005 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAYCAMPAIGN_H_INCLUDED
#define PLAYCAMPAIGN_H_INCLUDED

#include "playlevel.hpp"

class display;
struct game_state;
class config;
struct game_data;
class CVideo;
class upload_log;

enum io_type_t {
	IO_NONE,
	IO_SERVER,
	IO_CLIENT
};

LEVEL_RESULT play_game(display& disp, game_state& state, const config& game_config,
		const game_data& units_data, CVideo& video,
		upload_log &log,
		io_type_t io_type=IO_NONE);


void play_replay(display& disp, game_state& state, const config& game_config,
		const game_data& units_data, CVideo& video,
		io_type_t io_type=IO_NONE);

#endif // PLAYCAMPAIGN_H_INCLUDED

