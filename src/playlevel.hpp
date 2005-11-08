/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PLAY_LEVEL_HPP_INCLUDED
#define PLAY_LEVEL_HPP_INCLUDED

class config;
class CVideo;
struct game_state;

#include "game_config.hpp"
#include "unit_types.hpp"

#include <vector>

enum LEVEL_RESULT { VICTORY, DEFEAT, QUIT, LEVEL_CONTINUE, LEVEL_CONTINUE_NO_SAVE };

struct end_level_exception {
	end_level_exception(LEVEL_RESULT res, bool bonus=true)
	                     : result(res), gold_bonus(bonus)
	{}
	LEVEL_RESULT result;
	bool gold_bonus;
};

struct end_turn_exception {
	end_turn_exception(int r = 0): redo(r) {}
	int redo;
};

LEVEL_RESULT play_level(const game_data& gameinfo, const config& terrain_config,
		config const* level, CVideo& video,
		game_state& state_of_game,
		const std::vector<config*>& story);

namespace play{
	void place_sides_in_preferred_locations(gamemap& map, const config::child_list& sides);
}

#endif
