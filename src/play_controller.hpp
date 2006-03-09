/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAY_CONTROLLER_H_INCLUDED
#define PLAY_CONTROLLER_H_INCLUDED

#include "global.hpp"
#include "gamestatus.hpp"
#include "hotkeys.hpp"
#include "playlevel.hpp"
#include "statistics.hpp"

#include <vector>

class play_controller
{
public:
	play_controller(const config& level, game_state& state_of_game, 
		int ticks, int num_turns, const config& game_config);

protected:
	virtual void init();

	const config& level_;
	const config& game_config_;
	game_state& gamestate_;
	gamestatus status_;
	gamemap map_;
	const statistics::scenario_context statistics_context_;

	const int ticks_;
private:
};


#endif
