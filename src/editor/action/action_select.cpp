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
 * Editor label action classes
 */
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action_select.hpp"
#include "editor/map/map_context.hpp"

namespace editor
{
IMPLEMENT_ACTION(select)

void editor_action_select::extend(const editor_map& /*map*/, const std::set<map_location>& locs)
{
	for(const map_location& loc : locs) {
		LOG_ED << "Extending by " << loc << "\n";
		area_.insert(loc);
	}
}

editor_action* editor_action_select::perform(map_context& mc) const
{
	std::set<map_location> undo_locs;
	for(const map_location& loc : area_) {
		undo_locs.insert(loc);
		mc.add_changed_location(loc);
	}

	perform_without_undo(mc);
	return new editor_action_select(undo_locs);
}

void editor_action_select::perform_without_undo(map_context& mc) const
{
	for(const map_location& loc : area_) {
		mc.map().add_to_selection(loc);
		mc.add_changed_location(loc);
	}
}

IMPLEMENT_ACTION(deselect)

void editor_action_deselect::extend(const editor_map& map, const std::set<map_location>& locs)
{
	for(const map_location& loc : locs) {
		LOG_ED << "Checking " << loc << "\n";
		if(!map.in_selection(loc)) {
			LOG_ED << "Extending by " << loc << "\n";
			area_.insert(loc);
		}
	}
}

editor_action* editor_action_deselect::perform(map_context& mc) const
{
	std::set<map_location> undo_locs;
	for(const map_location& loc : area_) {
		if(mc.map().in_selection(loc)) {
			undo_locs.insert(loc);
			mc.add_changed_location(loc);
		}
	}

	perform_without_undo(mc);
	return new editor_action_select(undo_locs);
}

void editor_action_deselect::perform_without_undo(map_context& mc) const
{
	for(const map_location& loc : area_) {
		mc.map().remove_from_selection(loc);
		mc.add_changed_location(loc);
	}
}

IMPLEMENT_ACTION(select_all)

editor_action_select* editor_action_select_all::perform(map_context& mc) const
{
	std::set<map_location> current = mc.map().selection();
	mc.map().select_all();

	std::set<map_location> all = mc.map().selection();
	std::set<map_location> undo_locs;

	std::set_difference(
		all.begin(), all.end(), current.begin(), current.end(), std::inserter(undo_locs, undo_locs.begin()));

	mc.set_everything_changed();
	return new editor_action_select(undo_locs);
}

void editor_action_select_all::perform_without_undo(map_context& mc) const
{
	mc.map().select_all();
	mc.set_everything_changed();
}

IMPLEMENT_ACTION(select_none)

editor_action_select* editor_action_select_none::perform(map_context& mc) const
{
	std::set<map_location> current = mc.map().selection();
	mc.map().clear_selection();
	mc.set_everything_changed();
	return new editor_action_select(current);
}

void editor_action_select_none::perform_without_undo(map_context& mc) const
{
	mc.map().clear_selection();
	mc.set_everything_changed();
}

IMPLEMENT_ACTION(select_inverse)

editor_action_select_inverse* editor_action_select_inverse::perform(map_context& mc) const
{
	perform_without_undo(mc);
	return new editor_action_select_inverse();
}

void editor_action_select_inverse::perform_without_undo(map_context& mc) const
{
	mc.map().invert_selection();
	mc.set_everything_changed();
}

} // end namespace editor
