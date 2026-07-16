/*
	Copyright (C) 2017 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "undo_action.hpp"
#include "shroud_clearing_action.hpp"
#include "units/ptr.hpp"

namespace actions::undo
{
struct move_action : undo_action, shroud_clearing_action
{
	int starting_moves;
	map_location::direction starting_dir;
	map_location goto_hex;


	move_action(const unit_const_ptr& moved,
	            const std::vector<map_location>::const_iterator & begin,
	            const std::vector<map_location>::const_iterator & end,
	            int sm, const map_location::direction dir);
	move_action(const config & cfg)
		: undo_action()
		, shroud_clearing_action(cfg)
		, starting_moves(cfg["starting_moves"].to_int())
		, starting_dir(map_location::parse_direction(cfg["starting_direction"]))
		, goto_hex(cfg.child_or_empty("unit")["goto_x"].to_int(-999),
			  cfg.child_or_empty("unit")["goto_y"].to_int(-999),
			  wml_loc())
	{
	}

	static const char* get_type_impl() { return "move"; }
	virtual const char* get_type() const { return get_type_impl(); }

	virtual ~move_action() {}

	/** Writes this into the provided config. */
	virtual void write(config & cfg) const;

	/** Undoes this action. */
	virtual bool undo(int side);

	/**
	 * Applies the state changes of undoing this step (unit position, movement points,
	 * facing, goto), without animating anything.
	 * @return the affected unit, or nullptr if the step could not be undone.
	 *
	 * This is split out from undo() so that undo_action_container::undo() can merge the
	 * animation of consecutive per-hex move_action steps (which all stem from a single
	 * multi-hex move) into one continuous motion instead of animating each hex separately.
	 */
	unit_ptr undo_state(int side);

	/** Animates a unit traversing the given (already start-to-end ordered) route, and
	 *  settles it into a standing animation at the end. */
	static void animate(const std::vector<map_location>& route, const unit_ptr& u, map_location::direction dir);
};

}
