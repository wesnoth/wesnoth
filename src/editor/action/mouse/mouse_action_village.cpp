/*
   Copyright (C) 2008 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

editor_action* mouse_action_village::up_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return nullptr;
	if (!disp.get_map().is_village(hex)) return nullptr;

	return new editor_action_village(hex, disp.playing_team());
}

editor_action* mouse_action_village::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return nullptr;
	if (!disp.get_map().is_village(hex)) return nullptr;

	return new editor_action_village_delete(hex);
}

void mouse_action_village::set_mouse_overlay(editor_display& disp)
{
	surface image60 = image::get_image("icons/action/editor-tool-village_60.png");

	//TODO avoid hardcoded hex field size
	surface image = create_neutral_surface(72,72);

	SDL_Rect r {6, 6, 0, 0};
	sdl_blit(image60, nullptr, image, &r);

	uint8_t alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	adjust_surface_alpha(image, alpha);
	image = scale_surface(image, zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


} //end namespace editor
