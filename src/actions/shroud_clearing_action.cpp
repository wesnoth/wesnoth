/*
   Copyright (C) 2017-2018 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "actions/shroud_clearing_action.hpp"

#include "actions/move.hpp" //get_village
#include "resources.hpp"
#include "team.hpp"
#include "log.hpp"
#include "units/udisplay.hpp"
#include "game_board.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"

namespace actions {
void shroud_clearing_action::return_village()
{
	team &current_team = resources::controller->current_team();
	const map_location back = route.back();
	if(resources::gameboard->map().is_village(back)) {
		get_village(back, original_village_owner, nullptr, false);
		//MP_COUNTDOWN take away capture bonus
		if(take_village_timebonus) {
			current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
		}
	}
}
void shroud_clearing_action::take_village()
{
	team &current_team = resources::controller->current_team();
	const map_location back = route.back();
	if(resources::gameboard->map().is_village(back)) {
		get_village(back, current_team.side(), nullptr, false);
		//MP_COUNTDOWN restore capture bonus
		if(take_village_timebonus) {
			current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
		}
	}
}
}
