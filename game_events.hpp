/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_EVENTS_H_INCLUDED
#define GAME_EVENTS_H_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <map>

namespace game_events
{

bool conditional_passed(game_state& state_of_game,
                        const std::map<gamemap::location,unit>* units,
                        config& cond);

struct manager {
	manager(config& cfg, display& disp, gamemap& map,
	        std::map<gamemap::location,unit>& units,
		    game_state& state_of_game, game_data& data);
	~manager();
};

void raise(const std::string& event,
           const gamemap::location& loc1=gamemap::location::null_location,
           const gamemap::location& loc2=gamemap::location::null_location);

bool fire(const std::string& event,
          const gamemap::location& loc1=gamemap::location::null_location,
          const gamemap::location& loc2=gamemap::location::null_location);

bool pump();

}

#endif
