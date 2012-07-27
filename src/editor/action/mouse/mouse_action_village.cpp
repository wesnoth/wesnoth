/* $Id: mouse_action_village.cpp 49182 2011-04-12 02:32:34Z fendrin $ */
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
//TODO uncomment or remove
//#define GETTEXT_DOMAIN "wesnoth-editor"

#include "mouse_action_village.hpp"
#include "../action_village.hpp"

#include "../../editor_display.hpp"

namespace editor {

editor_action* mouse_action_village::up_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return NULL;
	if (!disp.get_map().is_village(hex)) return NULL;

	return new editor_action_village(hex, disp.playing_team());
}

editor_action* mouse_action_village::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))   return NULL;
	if (!disp.get_map().is_village(hex)) return NULL;

	return new editor_action_village_delete(hex);
}

void mouse_action_village::set_mouse_overlay(editor_display& /*disp*/)
{
	//TODO
	//set_mouse_overlay_image(disp, "editor/tool-overlay-village.png");
}


} //end namespace editor
