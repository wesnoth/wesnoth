/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAYMP_CONTROLLER_H_INCLUDED
#define PLAYMP_CONTROLLER_H_INCLUDED

#include "hotkeys.hpp"
#include "playlevel.hpp"
#include "play_controller.hpp"
#include "random.hpp"

#include <vector>

class playmp_controller : public play_controller
{
public:
	playmp_controller(const config& level, const game_data& gameinfo, game_state& state_of_game, 
		const int ticks, const int num_turns, const config& game_config, CVideo& video);

	LEVEL_RESULT play_scenario(const game_data& gameinfo, const config& terrain_config,
		const config* level, CVideo& video,	game_state& state_of_game,
		const std::vector<config*>& story, upload_log& log, bool skip_replay);

protected:
	const set_random_generator generator_setter;
	const cursor::setter cursor_setter;

private:
};


LEVEL_RESULT playmp_scenario(const game_data& gameinfo, const config& terrain_config,
		const config* level, CVideo& video,	game_state& state_of_game,
		const std::vector<config*>& story, upload_log& log, bool skip_replay);

#endif
