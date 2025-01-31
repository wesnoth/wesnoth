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

#include "editor/action/mouse/mouse_action_unit.hpp"
#include "editor/action/action_unit.hpp"

#include "editor/editor_display.hpp"
#include "formula/string_utils.hpp"
#include "tooltips.hpp"
#include "gettext.hpp"

#include "map/location.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"

namespace editor {


void mouse_action_unit::move(editor_display& disp, const map_location& hex)
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

	const unit_map& units = disp.context().units();
	const unit_map::const_unit_iterator unit_it = units.find(hex);
	if (unit_it != units.end()) {

		disp.clear_mouseover_hex_overlay();

		SDL_Rect rect = disp.get_location_rect(hex);
		std::stringstream str;
		str << _("Identifier: ") << unit_it->id()     << "\n";
		if(unit_it->name() != "") {
			str	<< _("Name: ")    << unit_it->name()      << "\n";
		}
		str	<< _("Type: ")    << unit_it->type_name() << "\n"
			<< _("Level: ")   << unit_it->level()     << "\n"
			<< _("Cost: ")    << unit_it->cost()      << "\n";
		tooltips::clear_tooltips();
		tooltips::add_tooltip(rect, str.str());
	}
	else {
		set_mouse_overlay(disp);
	}
}

std::unique_ptr<editor_action> mouse_action_unit::click_left(editor_display& disp, int x, int y)
{
	start_hex_ = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(start_hex_)) {
		return nullptr;
	}

	const unit_map& units = disp.context().units();
	const unit_map::const_unit_iterator unit_it = units.find(start_hex_);
	if (unit_it != units.end())
		set_unit_mouse_overlay(disp, unit_it->type());

	click_ = true;
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_unit::drag_left(editor_display& disp, int x, int y, bool& /*partial*/, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);
	click_ = (hex == start_hex_);
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_unit::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return nullptr;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex)) {
		return nullptr;
	}

	const unit_type& type = unit_palette_.selected_fg_item();

	// Does this serve a purpose other than making sure the type is built?
	// (Calling unit_types.build_unit_type(type) would now accomplish that
	// with less overhead.)
	const std::string& type_id = type.id();
	const unit_type *new_unit_type = unit_types.find(type_id);
	if (!new_unit_type) {
		//TODO rewrite the error message.
		ERR_ED << "create unit dialog returned inexistent or unusable unit_type id '" << type_id << "'";
		return nullptr;
	}

	const unit_type &ut = *new_unit_type;
	unit_race::GENDER gender = ut.genders().front();

	unit_ptr new_unit = unit::create(ut, disp.viewing_team().side(), true, gender);
	auto action = std::make_unique<editor_action_unit>(hex, *new_unit);
	return action;
}

std::unique_ptr<editor_action> mouse_action_unit::drag_end_left(editor_display& disp, int x, int y)
{
	if (click_) return nullptr;

	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))
		return nullptr;

	const unit_map& units = disp.context().units();
	const unit_map::const_unit_iterator unit_it = units.find(start_hex_);
	if (unit_it == units.end())
		return nullptr;

	return std::make_unique<editor_action_unit_replace>(start_hex_, hex);
}

void mouse_action_unit::set_mouse_overlay(editor_display& disp)
{
	const unit_type& u = unit_palette_.selected_fg_item();
	set_unit_mouse_overlay(disp, u);
}

void mouse_action_unit::set_unit_mouse_overlay(editor_display& disp, const unit_type& u)
{
	std::stringstream filename;
	filename << u.image() << "~RC(" << u.flag_rgb() << '>'
			<< team::get_side_color_id(disp.viewing_team().side()) << ')';

	disp.set_mouseover_hex_overlay(image::get_texture(filename.str()));
}


} //end namespace editor
