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
 * Editor item action class
 */

// TODO is a textdomain needed?
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action_item.hpp"

#include "editor/map/map_context.hpp"

namespace editor
{
IMPLEMENT_ACTION(item)

editor_action* editor_action_item::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_item_delete(loc_));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_item::perform_without_undo(map_context& /*mc*/) const
{
	//
	//	mc.get_items().add(loc_,u_);
	//	mc.get_items().find(loc_)->set_location(loc_);
	//	mc.add_changed_location(loc_);
}

IMPLEMENT_ACTION(item_delete)

editor_action* editor_action_item_delete::perform(map_context& /*mc*/) const
{
	//	item_map& items = mc.get_items();
	//	item_map::const_item_iterator item_it = items.find(loc_);
	//
	//	editor_action_ptr undo;
	//	if (item_it != items.end()) {
	//		undo.reset(new editor_action_item(loc_, *item_it));
	//		perform_without_undo(mc);
	//		return undo.release();
	//	}
	return nullptr;
}

void editor_action_item_delete::perform_without_undo(map_context& /*mc*/) const
{
	//	item_map& items = mc.get_items();
	//	if (!items.erase(loc_)) {
	//		ERR_ED << "Could not delete item on " << loc_.x << "/" << loc_.y << std::endl;
	//	} else {
	//		mc.add_changed_location(loc_);
	//	}
}

IMPLEMENT_ACTION(item_replace)

editor_action* editor_action_item_replace::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_item_replace(new_loc_, loc_));

	perform_without_undo(mc);
	return undo.release();
}

void editor_action_item_replace::perform_without_undo(map_context& /*mc*/) const
{
	//	item_map& items = mc.get_items();
	//	items.move(loc_, new_loc_);
	//	item::clear_status_caches();
	//
	//	item& u = *items.find(new_loc_);
	//	//TODO do we still need set_standing?
	//	u.anim_comp().set_standing();
	//
	//	mc.add_changed_location(loc_);
	//	mc.add_changed_location(new_loc_);
	//
	//	/* @todo
	//	  if (mc.get_map().is_village(new_loc_)) {
	//		(*(resources::gameboard->teams()))[u.side()].get_village(new_loc_);
	//	}
	//	*/
	//
	////TODO check if that is useful
	////	resources::screen->invalidate_item_after_move(loc_, new_loc_);
	////	resources::screen->draw();
}

IMPLEMENT_ACTION(item_facing)

editor_action* editor_action_item_facing::perform(map_context& mc) const
{
	editor_action_ptr undo(new editor_action_item_facing(loc_, old_direction_, new_direction_));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_item_facing::perform_without_undo(map_context& /*mc*/) const
{
	//	item_map& items = mc.get_items();
	//	item_map::item_iterator item_it = items.find(loc_);
	//
	//	if (item_it != items.end()) {
	//		item_it->set_facing(new_direction_);
	//		item_it->set_standing();
	//	}
}

} // end namespace editor
