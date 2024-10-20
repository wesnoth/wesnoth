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

#include "editor/action/mouse/mouse_action_select.hpp"
#include "editor/action/action_select.hpp"

#include "editor/editor_display.hpp"

namespace editor {

std::set<map_location> mouse_action_select::affected_hexes(
	editor_display& disp, const map_location& hex)
{
	if (has_shift_modifier()) {
		return disp.get_map().get_contiguous_terrain_tiles(hex);
	} else {
		return brush_drag_mouse_action::affected_hexes(disp, hex);
	}
}

std::unique_ptr<editor_action> mouse_action_select::key_event(
		editor_display& disp, const SDL_Event& event)
{
	auto ret = mouse_action::key_event(disp, event);
	update_brush_highlights(disp, previous_move_hex_);
	return ret;
}

std::unique_ptr<editor_action> mouse_action_select::click_perform_left(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	auto chain = std::make_unique<editor_action_chain>();
	if (has_ctrl_modifier())
		chain->append_action(std::make_unique<editor_action_deselect>(hexes));
	else
		chain->append_action(std::make_unique<editor_action_select>(hexes));
	return chain;
}

std::unique_ptr<editor_action> mouse_action_select::click_perform_right(
		editor_display& /*disp*/, const std::set<map_location>& /*hexes*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_select::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

void mouse_action_select::set_mouse_overlay(editor_display& disp)
{
	texture tex;
	if (has_shift_modifier()) {
		tex = image::get_texture(image::locator{"editor/tool-overlay-select-wand.png"});
	} else {
		tex = image::get_texture(image::locator{"editor/tool-overlay-select-brush.png"});
	}
	disp.set_mouseover_hex_overlay(tex);
}


} //end namespace editor
