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

#include "actions/undo_update_shroud_action.hpp"

#include "play_controller.hpp"
#include "resources.hpp"      // for screen, teams, units, etc
#include "synced_context.hpp" // for set_scontext_synced
#include "team.hpp"           // for team

namespace actions::undo
{
/**
 * Writes this into the provided config.
 */
void auto_shroud_action::write(config & cfg) const
{
	undo_action::write(cfg);
	cfg["active"] = active;
}

bool auto_shroud_action::undo(int)
{
	resources::controller->current_team().set_auto_shroud_updates(!active);
	return true;
}

static auto reg_auto_shroud = undo_action_container::subaction_factory<auto_shroud_action>();

}
