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
#ifndef AI_MOVE_H_INCLUDED
#define AI_MOVE_H_INCLUDED

#include "map.hpp"
#include "pathfind.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <map>

namespace ai {

typedef gamemap::location location;

struct target {
	target(const location& pos, double val) : loc(pos), value(val)
	{}
	location loc;
	double value;
};

std::vector<target> find_targets(
                           const gamemap& map, std::map<location,unit>& units,
						   std::vector<team>& teams, int current_team, bool has_leader
						  );

std::pair<location,location> choose_move(
                           std::vector<target>& targets,
                           const std::multimap<location,location>& dstsrc,
						   std::map<location,unit>& units,
						   const gamemap& map, const std::vector<team>& teams,
						   int current_team,
						   const game_data& data
						  );

}

#endif
