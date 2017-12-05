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

#include "editor/action/mouse/mouse_action_select.hpp"
#include "editor/action/action_select.hpp"

#include "editor/editor_display.hpp"

namespace editor {

std::set<map_location> mouse_action_select::affected_hexes(
	editor_display& disp, const map_location& hex)
{
	if (has_shift_modifier()) {
		return disp.map().get_contiguous_terrain_tiles(hex);
	} else {
		return brush_drag_mouse_action::affected_hexes(disp, hex);
	}
}

editor_action* mouse_action_select::key_event(
		editor_display& disp, const SDL_Event& event)
{
	editor_action* ret = mouse_action::key_event(disp, event);
	update_brush_highlights(disp, previous_move_hex_);
	return ret;
}

editor_action* mouse_action_select::click_perform_left(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	if (has_ctrl_modifier())
		return new editor_action_chain(new editor_action_deselect(hexes));
	else
		return new editor_action_chain(new editor_action_select(hexes));
}

editor_action* mouse_action_select::click_perform_right(
		editor_display& /*disp*/, const std::set<map_location>& /*hexes*/)
{
	return nullptr;
}

editor_action* mouse_action_select::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

void mouse_action_select::set_mouse_overlay(editor_display& disp)
{
	surface image;
	if (has_shift_modifier()) {
		image = image::get_image("editor/tool-overlay-select-wand.png");
	} else {
		image = image::get_image("editor/tool-overlay-select-brush.png");
	}
	uint8_t alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	adjust_surface_alpha(image, alpha);
	image = scale_surface(image, zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


} //end namespace editor
