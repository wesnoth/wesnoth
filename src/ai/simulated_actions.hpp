/*
   Copyright (C) 2014 by Guorui Xi <kevin.xgr@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Implement simulated actions
 * @file
 */

#ifndef AI_SIMULATED_ACTIONS_HPP_INCLUDED
#define AI_SIMULATED_ACTIONS_HPP_INCLUDED

#include "game_info.hpp"

#include "lua/unit_advancements_aspect.hpp"

class unit_type;

namespace ai{

bool simulated_attack(const map_location& attacker_loc, const map_location& defender_loc, double attacker_hp, double defender_hp);

bool simulated_move(int side, const map_location& from, const map_location& to, int steps, map_location& unit_location);

bool simulated_recall(int side, const std::string& unit_id, const map_location& recall_location);

bool simulated_recruit(int side, const unit_type* u, const map_location& recruit_location);

bool simulated_stopunit(const map_location& unit_location, bool remove_movement, bool remove_attacks);

bool simulated_synced_command();

}

#endif
