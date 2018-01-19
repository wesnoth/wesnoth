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

#include "actions/undo_dismiss_action.hpp"
#include "game_board.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"

namespace actions
{
namespace undo
{

/**
 * Writes this into the provided config.
 */
void dismiss_action::write(config & cfg) const
{
	undo_action::write(cfg);
	dismissed_unit->write(cfg.add_child("unit"));
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool dismiss_action::undo(int side)
{
	team &current_team = resources::gameboard->get_team(side);

	current_team.recall_list().add(dismissed_unit);
	execute_undo_umc_wml();
	return true;
}

}
}
