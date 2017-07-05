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

#include "actions/undo_recruit_action.hpp"
#include "actions/create.hpp"

#include "gui/dialogs/transient_message.hpp"
#include "game_board.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "replay.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"
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

recruit_action::recruit_action(const unit_const_ptr recruited, const map_location& loc,
			   const map_location& from, int orig_village_owner, bool time_bonus)
	: undo_action()
	, shroud_clearing_action(recruited, loc, orig_village_owner, time_bonus)
	, u_type(recruited->type())
	, recruit_from(from)
{}

recruit_action::recruit_action(const config & cfg, const unit_type & type, const map_location& from)
	: undo_action(cfg)
	, shroud_clearing_action(cfg)
	, u_type(type)
	, recruit_from(from)
{}

/**
 * Writes this into the provided config.
 */
void recruit_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);

	recruit_from.write(cfg.add_child("leader"));
	config & child = cfg.child("unit");
	child["type"] = u_type.base_id();
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool recruit_action::undo(int side)
{
	unit_map &   units = resources::gameboard->units();
	team &current_team = resources::gameboard->get_team(side);

	const map_location & recruit_loc = route.front();
	unit_map::iterator un_it = units.find(recruit_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	const unit &un = *un_it;
	statistics::un_recruit_unit(un);
	current_team.spend_gold(-un.type().cost());

	//MP_COUNTDOWN take away recruit bonus
	current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);

	units.erase(recruit_loc);
	this->return_village();
	execute_undo_umc_wml();
	return true;
}

}
}
