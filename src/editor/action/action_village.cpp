/*
   Copyright (C) 2008 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "editor/action/action_village.hpp"

#include "editor/map/map_context.hpp"
#include "team.hpp"

namespace editor
{
IMPLEMENT_ACTION(village)

editor_action* editor_action_village::perform(map_context& mc) const
{
	std::unique_ptr<editor_action> undo;

	if(!mc.get_map().is_village(loc_)) {
		return nullptr;
	}

	std::vector<team>& teams = mc.get_teams();

	team* t = unsigned(side_number_) < teams.size() ? &teams[side_number_] : nullptr;
	if(t && t->owns_village(loc_)) {
		return nullptr;
	}

	undo.reset(new editor_action_village_delete(loc_));

	for(const team& t : teams) {
		if(t.owns_village(loc_)) {
			undo.reset(new editor_action_village(loc_, t.side() - 1));
		}
	}

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_village::perform_without_undo(map_context& mc) const
{
	std::vector<team>& teams = mc.get_teams();

	for(team& t : teams) {
		if(t.owns_village(loc_)) {
			t.lose_village(loc_);
		}
	}

	// TODO 0 is a bad argument
	teams[side_number_].get_village(loc_, 0, nullptr);
}

IMPLEMENT_ACTION(village_delete)

editor_action* editor_action_village_delete::perform(map_context& mc) const
{
	std::unique_ptr<editor_action> undo;

	for(const team& t : mc.get_teams()) {
		if(t.owns_village(loc_)) {
			perform_without_undo(mc);
			undo.reset(new editor_action_village(loc_, t.side() - 1));
		}
	}

	return undo.release();
}

void editor_action_village_delete::perform_without_undo(map_context& mc) const
{
	for(team& t : mc.get_teams()) {
		if(t.owns_village(loc_)) {
			t.lose_village(loc_);
			mc.add_changed_location(loc_);
		}
	}
}

} // end namespace editor
