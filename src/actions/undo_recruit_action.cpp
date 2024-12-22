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

#include "actions/undo_recruit_action.hpp"

#include "game_board.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "game_display.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace actions::undo
{
recruit_action::recruit_action(const unit_const_ptr& recruited, const map_location& loc,
			   const map_location& from)
	: undo_action()
	, shroud_clearing_action(recruited, loc)
	, u_type(recruited->type())
	, recruit_from(from)
{}

static const unit_type& get_unit_type(const config& cfg)
{
	const config& child = cfg.mandatory_child("unit");
	const unit_type* u_type = unit_types.find(child["type"]);

	if(!u_type) {
		// Bad data.
		throw config::error("Invalid recruit; unit type '" + child["type"].str() + "' was not found.\n");
	}
	return *u_type;
}
recruit_action::recruit_action(const config & cfg)
	: undo_action()
	, shroud_clearing_action(cfg)
	, u_type(get_unit_type(cfg))
	, recruit_from(map_location(cfg.child_or_empty("leader"), nullptr))
{
}

/**
 * Writes this into the provided config.
 */
void recruit_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);

	recruit_from.write(cfg.add_child("leader"));
	config & child = cfg.mandatory_child("unit");
	child["type"] = u_type.parent_id();
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool recruit_action::undo(int side)
{
	game_display & gui = *game_display::get_singleton();
	unit_map &   units = resources::gameboard->units();
	team &current_team = resources::gameboard->get_team(side);

	const map_location & recruit_loc = route.front();
	unit_map::iterator un_it = units.find(recruit_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	const unit &un = *un_it;
	resources::controller->statistics().un_recruit_unit(un);
	current_team.spend_gold(-un.type().cost());

	//MP_COUNTDOWN take away recruit bonus
	current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);

	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recruit_loc);
	units.erase(recruit_loc);
	return true;
}
static auto reg_undo_recruit = undo_action_container::subaction_factory<recruit_action>();

}
