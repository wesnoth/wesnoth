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

#include "actions/undo_move_action.hpp"
#include "actions/move.hpp"

#include "resources.hpp"
#include "replay.hpp"
#include "units/map.hpp"
#include "units/animation_component.hpp"
#include "log.hpp"
#include "game_display.hpp"
#include "units/udisplay.hpp"
#include "units/unit.hpp"
#include "game_board.hpp"
#include "map/map.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace actions
{
namespace undo
{

move_action::move_action(const unit_const_ptr moved,
			const std::vector<map_location>::const_iterator & begin,
			const std::vector<map_location>::const_iterator & end,
			int sm, int timebonus, int orig, const map_location::DIRECTION dir)
	: undo_action()
	, shroud_clearing_action(moved, begin, end, orig, timebonus != 0)
	, starting_moves(sm)
	, starting_dir(dir == map_location::NDIRECTIONS ? moved->facing() : dir)
	, goto_hex(moved->get_goto())
{
}

/**
 * Writes this into the provided config.
 */
void move_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);
	cfg["starting_direction"] = map_location::write_direction(starting_dir);
	cfg["starting_moves"] = starting_moves;
	config & child = cfg.child("unit");
	child["goto_x"] = goto_hex.wml_x();
	child["goto_y"] = goto_hex.wml_y();
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool move_action::undo(int)
{
	game_display & gui = *resources::screen;
	unit_map &   units = resources::gameboard->units();

	// Copy some of our stored data.
	const int saved_moves = starting_moves;
	std::vector<map_location> rev_route = route;
	std::reverse(rev_route.begin(), rev_route.end());

	// Check units.
	unit_map::iterator u = units.find(rev_route.front());
	const unit_map::iterator u_end = units.find(rev_route.back());
	if ( u == units.end()  ||  u_end != units.end() ) {
		//this can actually happen if the scenario designer has abused the [allow_undo] command
		ERR_NG << "Illegal 'undo' found. Possible abuse of [allow_undo]?" << std::endl;
		return false;
	}
	this->return_village();

	// Record the unit's current state so it can be redone.
	starting_moves = u->movement_left();
	goto_hex = u->get_goto();

	// Move the unit.
	unit_display::move_unit(rev_route, u.get_shared_ptr(), true, starting_dir);
	units.move(u->get_location(), rev_route.back());
	unit::clear_status_caches();

	// Restore the unit's old state.
	u = units.find(rev_route.back());
	u->set_goto(map_location());
	u->set_movement(saved_moves, true);
	u->anim_comp().set_standing();

	gui.invalidate_unit_after_move(rev_route.front(), rev_route.back());
	execute_undo_umc_wml();
	return true;
}


}
}
