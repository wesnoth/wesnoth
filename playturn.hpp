/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

#include "actions.hpp"
#include "ai.hpp"
#include "config.hpp"
#include "dialogs.hpp"
#include "display.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "key.hpp"
#include "menu.hpp"
#include "pathfind.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "unit.hpp"
#include "video.hpp"

#include <map>
#include <vector>

struct paths_wiper
{
	paths_wiper(display& gui) : gui_(gui)
	{}

	~paths_wiper() { gui_.set_paths(NULL); }
	
private:
	display& gui_;
};

void play_turn(game_data& gameinfo, game_state& state_of_game,
               gamestatus& status, config& terrain_config, config* level,
			   CVideo& video, CKey& key, display& gui,
               game_events::manager& events_manager, gamemap& map,
			   std::vector<team>& teams, int team_num,
			   std::map<gamemap::location,unit>& units);

#endif
