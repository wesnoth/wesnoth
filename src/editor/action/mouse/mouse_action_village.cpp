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

#include "editor/action/mouse/mouse_action_village.hpp"
#include "editor/action/action_village.hpp"

#include "editor/editor_display.hpp"

namespace editor {

std::unique_ptr<editor_action> mouse_action_village::up_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return nullptr;
	if (!disp.get_map().is_village(hex)) return nullptr;

	return std::make_unique<editor_action_village>(hex, disp.playing_team_index());
}

std::unique_ptr<editor_action> mouse_action_village::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return nullptr;
	if (!disp.get_map().is_village(hex)) return nullptr;

	return std::make_unique<editor_action_village_delete>(hex);
}

void mouse_action_village::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(
		image::get_texture(
			// center 60px icon on blank hex template
			image::locator(
				"misc/blank-hex.png",
				"~BLIT(icons/action/editor-tool-village_60.png,6,6)"
			)
		)
	);
}


} //end namespace editor
