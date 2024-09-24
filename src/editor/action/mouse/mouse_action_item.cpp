/*
	Copyright (C) 2008 - 2024
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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/mouse/mouse_action_item.hpp"
#include "editor/action/action_item.hpp"

#include "editor/editor_display.hpp"
#include "gettext.hpp"

#include "map/location.hpp"

namespace editor {


void mouse_action_item::move(editor_display& disp, const map_location& hex)
{
	if (hex == previous_move_hex_) {
		return;
	}

	update_brush_highlights(disp, hex);

	std::set<map_location> adjacent_set;
	for(const map_location& adj : get_adjacent_tiles(previous_move_hex_)) {
		adjacent_set.insert(adj);
	}

	disp.invalidate(adjacent_set);
	previous_move_hex_ = hex;
}

std::unique_ptr<editor_action> mouse_action_item::click_left(editor_display& disp, int x, int y)
{
	start_hex_ = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(start_hex_)) {
		return nullptr;
	}

	overlay item = item_palette_.selected_fg_item();
	disp.add_overlay(start_hex_, std::move(item));

	click_ = true;
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_item::click_right(editor_display& disp, int x, int y)
{
	start_hex_ = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(start_hex_)) {
		return nullptr;
	}

	disp.remove_overlay(start_hex_);

	click_ = true;
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_item::drag_left(editor_display& disp, int x, int y, bool& /*partial*/, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);
	click_ = (hex == start_hex_);
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_item::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return nullptr;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex)) {
		return nullptr;
	}

//	item_type type = item_palette_.selected_fg_item();
//
//	// Does this serve a purpose other than making sure the type is built?
//	// (Calling item_types.build_item_type(type) would now accomplish that
//	// with less overhead.)
//	const std::string& type_id = type.id();
//	const item_type *new_item_type = item_types.find(type_id);
//	if (!new_item_type) {
//		//TODO rewrite the error message.
//		ERR_ED << "create item dialog returned inexistent or unusable item_type id '" << type_id << "'";
//		return nullptr;
//	}
//
//	const item_type &ut = *new_item_type;
//	item_race::GENDER gender = ut.genders().front();
//
//	item new_item(ut, disp.viewing_side(), true, gender);
//	editor_action* action = new editor_action_item(hex, new_item);
//	return action;

// \todo in #5070: there's a load of commented-out code in this file, it should probably
// all be deleted. For the function that this comment is in, I've left the commented-out
// code in because it seems the not-commented code should also be reviewed. AFAICS, the
// entire function (including the not-commented code) could be deleted, and fall back to
// the parent class' implementation of just returning nullptr.

	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_item::drag_end_left(editor_display& disp, int x, int y)
{
	if (click_) return nullptr;

	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))
		return nullptr;

//	const item_map& items = disp.get_items();
//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//	if (item_it == items.end())
//		return nullptr;

	return std::make_unique<editor_action_item_replace>(start_hex_, hex);
}

void mouse_action_item::set_mouse_overlay(editor_display& disp)
{
	const overlay& item = item_palette_.selected_fg_item();
	set_item_mouse_overlay(disp, item);
}

void mouse_action_item::set_item_mouse_overlay(editor_display& disp, const overlay& u)
{
	disp.set_mouseover_hex_overlay(image::get_texture(u.image));
}


} //end namespace editor
