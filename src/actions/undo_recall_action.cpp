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

#include "actions/undo_recall_action.hpp"

#include "game_board.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/animation_component.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "game_display.hpp"
#include "whiteboard/manager.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace actions::undo
{
recall_action::recall_action(const unit_const_ptr& recalled, const map_location& loc,
			  const map_location& from)
	: undo_action()
	, shroud_clearing_action(recalled, loc)
	, id(recalled->id())
	, recall_from(from)
{}

recall_action::recall_action(const config & cfg)
	: undo_action()
	, shroud_clearing_action(cfg)
	, id(cfg["id"])
	, recall_from(map_location(cfg.child_or_empty("leader"), nullptr))
{}

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
	game_display & gui = *game_display::get_singleton();
	unit_map &   units = resources::gameboard->units();
	team &current_team = resources::gameboard->get_team(side);

	const map_location & recall_loc = route.front();
	unit_map::iterator un_it = units.find(recall_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	unit_ptr un = un_it.get_shared_ptr();
	if (!un) {
		return false;
	}

	resources::controller->statistics().un_recall_unit(*un);
	int cost = un->recall_cost();
	if (cost < 0) {
		current_team.spend_gold(-current_team.recall_cost());
	}
	else {
		current_team.spend_gold(-cost);
	}

	current_team.recall_list().add(un);
	// Invalidate everything, not just recall_loc, in case the sprite
	// extends into adjacent hexes (Horseman) or even farther away (Fire
	// Dragon)
	gui.invalidate_all();
	units.erase(recall_loc);
	resources::whiteboard->on_kill_unit();
	un->anim_comp().clear_haloes();
	return true;
}
static auto reg_undo_recall = undo_action_container::subaction_factory<recall_action>();

}
