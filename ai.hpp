/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef AI_HPP_INCLUDED
#define AI_HPP_INCLUDED

#include "ai_move.hpp"
#include "display.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <map>

namespace ai {
typedef gamemap::location location;

void do_move(display& disp, const gamemap& map, const game_data& gameinfo,
             std::map<gamemap::location,unit>& units,
             std::vector<team>& teams, int team_num, const gamestatus& state,
             bool consider_combat=true,
             std::vector<target>* additional_targets=NULL);
}

#endif
