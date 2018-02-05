/*
   Copyright (C) 2008 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Editor unit action class
 */

// TODO is a textdomain needed?
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action_unit.hpp"

#include "editor/map/map_context.hpp"

#include "units/animation_component.hpp"
#include "units/unit.hpp"

namespace editor
{
IMPLEMENT_ACTION(unit)

editor_action* editor_action_unit::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_unit_delete(loc_));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_unit::perform_without_undo(map_context& mc) const
{
	mc.units().add(loc_, u_);
	mc.units().find(loc_)->set_location(loc_);
	mc.add_changed_location(loc_);
}

IMPLEMENT_ACTION(unit_delete)

editor_action* editor_action_unit_delete::perform(map_context& mc) const
{
	unit_map& units = mc.units();
	unit_map::const_unit_iterator unit_it = units.find(loc_);

	editor_action_ptr undo;
	if(unit_it != units.end()) {
		undo.reset(new editor_action_unit(loc_, *unit_it));
		perform_without_undo(mc);
		return undo.release();
	}

	return nullptr;
}

void editor_action_unit_delete::perform_without_undo(map_context& mc) const
{
	unit_map& units = mc.units();
	if(!units.erase(loc_)) {
		ERR_ED << "Could not delete unit on " << loc_ << std::endl;
	} else {
		mc.add_changed_location(loc_);
	}
}

IMPLEMENT_ACTION(unit_replace)

editor_action* editor_action_unit_replace::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_unit_replace(new_loc_, loc_));

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_unit_replace::perform_without_undo(map_context& mc) const
{
	unit_map& units = mc.units();
	units.move(loc_, new_loc_);
	unit::clear_status_caches();

	unit& u = *units.find(new_loc_);

	// TODO do we still need set_standing?
	u.anim_comp().set_standing();

	mc.add_changed_location(loc_);
	mc.add_changed_location(new_loc_);

	/* @todo
	  if (mc.map().is_village(new_loc_)) {
		(*(resources::gameboard->teams()))[u.side()].get_village(new_loc_);
	}
	*/

	// TODO check if that is useful
	//	game_display::get_singleton()->invalidate_unit_after_move(loc_, new_loc_);
	//	display::get_singleton()->draw();
}

IMPLEMENT_ACTION(unit_facing)

editor_action* editor_action_unit_facing::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_unit_facing(loc_, old_direction_, new_direction_));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_unit_facing::perform_without_undo(map_context& mc) const
{
	unit_map& units = mc.units();
	unit_map::unit_iterator unit_it = units.find(loc_);

	if(unit_it != units.end()) {
		unit_it->set_facing(new_direction_);
		unit_it->anim_comp().set_standing();
	}
}

} // end namespace editor
