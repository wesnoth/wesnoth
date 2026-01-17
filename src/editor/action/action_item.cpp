/*
	Copyright (C) 2008 - 2025
	by Fabian Mueller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action_item.hpp"
#include "editor/map/map_context.hpp"


namespace editor
{
IMPLEMENT_ACTION(item)

std::unique_ptr<editor_action> editor_action_item::perform(map_context& mc) const
{
	auto undo = std::make_unique<editor_action_item_delete>(loc_);
	perform_without_undo(mc);
	return undo;
}

void editor_action_item::perform_without_undo(map_context& /*mc*/) const
{
	//
	//	mc.get_items().add(loc_,u_);
	//	mc.get_items().find(loc_)->set_location(loc_);
	//	mc.add_changed_location(loc_);
}

IMPLEMENT_ACTION(item_delete)

std::unique_ptr<editor_action> editor_action_item_delete::perform(map_context& /*mc*/) const
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
	//		ERR_ED << "Could not delete item on " << loc_.x << "/" << loc_.y;
	//	} else {
	//		mc.add_changed_location(loc_);
	//	}
}

IMPLEMENT_ACTION(item_replace)

std::unique_ptr<editor_action> editor_action_item_replace::perform(map_context& mc) const
{
	auto undo = std::make_unique<editor_action_item_replace>(new_loc_, loc_);

	perform_without_undo(mc);
	return undo;
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
	//	  if (mc.map().is_village(new_loc_)) {
	//		(*(resources::gameboard->teams()))[u.side()].get_village(new_loc_);
	//	}
	//	*/
	//
	// TODO: check if that is useful
	//  	game_display::get_singleton()->invalidate_item_after_move(loc_, new_loc_);
	//  	display::get_singleton()->draw();
}

IMPLEMENT_ACTION(item_move_all)

std::unique_ptr<editor_action> editor_action_item_move_all::perform(map_context& mc) const
{
	auto undo = std::make_unique<editor_action_item_move_all>(-x_offset_, -y_offset_);
	perform_without_undo(mc);
	return undo;
}

void editor_action_item_move_all::perform_without_undo(map_context& mc) const
{
	auto ovls = mc.get_overlays();
	decltype(ovls) new_overlays_map;
	for (auto ov: ovls) {
		map_location new_loc{ov.first.x - x_offset_, ov.first.y - y_offset_};
		new_overlays_map.emplace(new_loc, ov.second);
	}
	mc.set_overlays(new_overlays_map);
}

IMPLEMENT_ACTION(item_facing)

std::unique_ptr<editor_action> editor_action_item_facing::perform(map_context& mc) const
{
	auto undo = std::make_unique<editor_action_item_facing>(loc_, old_direction_, new_direction_);
	perform_without_undo(mc);
	return undo;
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
