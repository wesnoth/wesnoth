/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "actions/undo_recall_action.hpp"
#include "actions/create.hpp"

#include "gui/dialogs/transient_message.hpp"
#include "game_board.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "replay.hpp"
#include "units/map.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "game_display.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace actions
{
namespace undo
{

/**
 * Writes this into the provided config.
 */
void recall_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);

	recall_from.write(cfg.add_child("leader"));
	cfg["id"] = id;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool recall_action::undo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = resources::gameboard->units();
	team &current_team = resources::gameboard->teams()[side-1];

	const map_location & recall_loc = route.front();
	unit_map::iterator un_it = units.find(recall_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	unit_ptr un = un_it.get_shared_ptr();
	if (!un) {
		return false;
	}

	statistics::un_recall_unit(*un);
	int cost = statistics::un_recall_unit_cost(*un);
	if (cost < 0) {
		current_team.spend_gold(-current_team.recall_cost());
	}
	else {
		current_team.spend_gold(-cost);
	}

	current_team.recall_list().add(un);
	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recall_loc);
	units.erase(recall_loc);
	this->return_village();
	execute_undo_umc_wml();
	return true;
}

}
}
