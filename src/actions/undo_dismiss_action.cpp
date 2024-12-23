/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"

namespace actions::undo
{
dismiss_action::dismiss_action(const unit_const_ptr& dismissed)
	: undo_action()
	, dismissed_unit(dismissed->clone())
{
}

dismiss_action::dismiss_action(const config& cfg)
	: undo_action()
	, dismissed_unit(unit::create(cfg.mandatory_child("unit")))
{
}

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
	return true;
}

static auto red_undo_dismiss = undo_action_container::subaction_factory<dismiss_action>();

}
