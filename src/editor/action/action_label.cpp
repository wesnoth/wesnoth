/*
   Copyright (C) 2010 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "editor/action/action_label.hpp"

#include "editor/map/map_context.hpp"

namespace editor
{
IMPLEMENT_ACTION(label)

editor_action* editor_action_label::perform(map_context& mc) const
{
	std::unique_ptr<editor_action> undo;

	const terrain_label* old_label = mc.get_labels().get_label(loc_);
	if(old_label) {
		undo.reset(new editor_action_label(
			loc_,
			old_label->text(),
			old_label->team_name(),
			old_label->color(),
			old_label->visible_in_fog(),
			old_label->visible_in_shroud(),
			old_label->immutable(),
			old_label->category())
		);
	} else {
		undo.reset(new editor_action_label_delete(loc_));
	}

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_label::perform_without_undo(map_context& mc) const
{
	mc.get_labels().set_label(
		loc_, text_, -1, team_name_, color_, visible_fog_, visible_shroud_, immutable_, category_);
}

IMPLEMENT_ACTION(label_delete)

editor_action* editor_action_label_delete::perform(map_context& mc) const
{
	std::unique_ptr<editor_action> undo;

	const terrain_label* deleted = mc.get_labels().get_label(loc_);

	if(!deleted) {
		return nullptr;
	}

	undo.reset(new editor_action_label(
		loc_,
		deleted->text(),
		deleted->team_name(),
		deleted->color(),
		deleted->visible_in_fog(),
		deleted->visible_in_shroud(),
		deleted->immutable(),
		deleted->category())
	);

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_label_delete::perform_without_undo(map_context& mc) const
{
	mc.get_labels().set_label(loc_, "");
}

} // end namespace editor
