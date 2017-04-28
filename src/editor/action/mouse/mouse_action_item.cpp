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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/mouse/mouse_action_item.hpp"
#include "editor/action/action_item.hpp"

#include "editor/editor_display.hpp"
//#include "gui/dialogs/item_create.hpp"
#include "tooltips.hpp"
#include "gettext.hpp"

#include "map/location.hpp"

namespace editor {


void mouse_action_item::move(editor_display& disp, const map_location& hex)
{
	if (hex != previous_move_hex_) {

		update_brush_highlights(disp, hex);

		std::set<map_location> adjacent_set;
		map_location adjacent[6];
		get_adjacent_tiles(previous_move_hex_, adjacent);

		for (int i = 0; i < 6; i++)
			adjacent_set.insert(adjacent[i]);

		disp.invalidate(adjacent_set);
		previous_move_hex_ = hex;

	//	const item_map& items = disp.get_items();
	//	const item_map::const_item_iterator item_it = items.find(hex);
//		if (item_it != items.end()) {
//
//			disp.set_mouseover_hex_overlay(nullptr);
//
//			SDL_Rect rect;
//			rect.x = disp.get_location_x(hex);
//			rect.y = disp.get_location_y(hex);
//			rect.h = disp.hex_size();
//			rect.w = disp.hex_size();
//			std::stringstream str;
//			str << N_("ID: ")   << item_it->id()   << "\n"
//				<< N_("Name: ") << item_it->name() << "\n"
//				<< N_("Type: ") << item_it->type_name();
//			tooltips::clear_tooltips();
//			tooltips::add_tooltip(rect, str.str());
//		}
//		else {
//			set_mouse_overlay(disp);
//		}
	}
}

editor_action* mouse_action_item::click_left(editor_display& disp, int x, int y)
{
	start_hex_ = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(start_hex_)) {
		return nullptr;
	}

	const overlay& item = item_palette_.selected_fg_item();
	disp.add_overlay(start_hex_, item.image, item.halo, "", "", true);



//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//	if (item_it != items.end())
//		set_item_mouse_overlay(disp, item_it->type());

	click_ = true;
	return nullptr;
}

editor_action* mouse_action_item::drag_left(editor_display& disp, int x, int y, bool& /*partial*/, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);
	click_ = (hex == start_hex_);
	return nullptr;
}

editor_action* mouse_action_item::up_left(editor_display& disp, int x, int y)
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
//		ERR_ED << "create item dialog returned inexistent or unusable item_type id '" << type_id << "'" << std::endl;
//		return nullptr;
//	}
//
//	const item_type &ut = *new_item_type;
//	item_race::GENDER gender = ut.genders().front();
//
//	item new_item(ut, disp.viewing_side(), true, gender);
//	editor_action* action = new editor_action_item(hex, new_item);
//	return action;

	return nullptr;
}

editor_action* mouse_action_item::drag_end_left(editor_display& disp, int x, int y)
{
	if (click_) return nullptr;
	editor_action* action = nullptr;

	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))
		return nullptr;

//	const item_map& items = disp.get_items();
//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//	if (item_it == items.end())
//		return nullptr;

	action = new editor_action_item_replace(start_hex_, hex);
	return action;
}

/*
editor_action* mouse_action_item::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	start_hex_ = hex;
	previous_move_hex_ = hex;

	const item_map& items = disp.get_items();
	const item_map::const_item_iterator item_it = items.find(start_hex_);

	if (item_it != items.end()) {
		old_direction_ = item_it->facing();
	}

	click_ = true;
	return nullptr;
}
*/

//editor_action* mouse_action_item::drag_right(editor_display& disp, int x, int y, bool& /*partial*/, editor_action* /*last_undo*/)
//{
//	map_location hex = disp.hex_clicked_on(x, y);
//	if (previous_move_hex_ == hex)
//		return nullptr;
//
//	click_ = (start_hex_ == hex);
//	previous_move_hex_ = hex;
//
//	const item_map& items = disp.get_items();
//
//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//	if (item_it != items.end()) {
//		for (map_location::DIRECTION new_direction = map_location::NORTH;
//				new_direction <= map_location::NORTH_WEST;
//				new_direction = map_location::DIRECTION(new_direction +1)){
//			if (item_it->get_location().get_direction(new_direction, 1) == hex) {
//				return new editor_action_item_facing(start_hex_, new_direction, old_direction_);
//			}
//		}
//	}
//
//	return nullptr;
//}

//editor_action* mouse_action_item::up_right(editor_display& disp, int /*x*/, int /*y*/)
//{
//	if (!click_) return nullptr;
//	click_ = false;
//
//	const item_map& items = disp.get_items();
//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//	if (item_it != items.end()) {
//		return new editor_action_item_delete(start_hex_);
//	}
//
//	return nullptr;
//}

//editor_action* mouse_action_item::drag_end_right(editor_display& disp, int x, int y)
//{
//	if (click_) return nullptr;
//
//	map_location hex = disp.hex_clicked_on(x, y);
//	if (!disp.get_map().on_board(hex))
//		return nullptr;
//
//	if(new_direction_ != old_direction_) {
//
//	const item_map& items = disp.get_items();
//	const item_map::const_item_iterator item_it = items.find(start_hex_);
//		if (item_it != items.end()) {
//			return new editor_action_item_facing(start_hex_, new_direction_, old_direction_);
//		}
//	}
//
//	return nullptr;
//}


void mouse_action_item::set_mouse_overlay(editor_display& disp)
{
	const overlay& item = item_palette_.selected_fg_item();
	set_item_mouse_overlay(disp, item);
}

void mouse_action_item::set_item_mouse_overlay(editor_display& disp, const overlay& u)
{

	std::stringstream filename;
	filename << u.image; // << "~RC(" << u.flag_rgb() << '>'
		//	<< team::get_side_color_index(disp.viewing_side()) << ')';

	surface image(image::get_image(filename.str()));
	Uint8 alpha = 196;
	//TODO don't hardcode
	int size = 72;
	//int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	adjust_surface_alpha(image, alpha);
	image = scale_surface(image, zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


} //end namespace editor
