/* $Id: action_label.cpp 49121 2011-04-07 21:57:57Z fendrin $ */
/*
   Copyright (C) 2008 - 2011 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "editor/map_context.hpp"


namespace editor {

editor_action_label* editor_action_label::clone() const
{
	return new editor_action_label(*this);
}

editor_action* editor_action_label::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo;
	const terrain_label *old_label = mc.get_map().get_game_labels().get_label(loc_);
	if (old_label) {
		undo.reset(new editor_action_label(loc_, old_label->text(), old_label->team_name(), old_label->color()
				, old_label->visible_in_fog(), old_label->visible_in_shroud(), old_label->immutable()) );
	} else {
		undo.reset(new editor_action_label_delete(loc_));
	}

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_label::perform_without_undo(map_context& mc) const
{
	terrain_label *old_label = mc.get_map().get_game_labels().get_label(loc_);

	if (old_label) {
		//old_label->set_text(text_);
		old_label->update_info(text_, team_name_, color_, visible_shroud_, visible_fog_, immutable_);
	} else {
		mc.get_map().get_game_labels().set_label(loc_, text_, team_name_, color_, visible_fog_, visible_shroud_, immutable_);
		mc.set_needs_labels_reset();
	}
}

editor_action_label_delete* editor_action_label_delete::clone() const
{
	return new editor_action_label_delete(*this);
}

editor_action* editor_action_label_delete::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo;

	//	std::vector<terrain_label> deleted = mc.get_map().get_game_labels().get_label(loc_);
	const terrain_label* deleted = mc.get_map().get_game_labels().get_label(loc_);

	editor_action_chain* undo_chain = new editor_action_chain();

	//TODO
	//for (std::vector<terrain_label>::const_iterator it = deleted.begin(); it != deleted.end(); it++) {
	//	ERR_ED << it->text();
	undo_chain->append_action(new editor_action_label(loc_, deleted->text(), deleted->team_name()
			, deleted->color(), deleted->visible_in_fog(), deleted->visible_in_shroud(), deleted->immutable()));
	//}

	undo.reset(undo_chain);
	mc.set_needs_labels_reset();
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_label_delete::perform_without_undo(map_context& mc) const
{
	const terrain_label* deleted = mc.get_map().get_game_labels().get_label(loc_);
	mc.get_map().get_game_labels().delete_labels(loc_);
	delete deleted;
}


} //end namespace editor
