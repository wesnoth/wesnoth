/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file action.cpp
//! Editor action classes

#include "editor_display.hpp"
#include "editor_map.hpp"
#include "editor_mode.hpp"
#include "editor_mouse_handler.hpp"
#include "mouse_action.hpp"
#include "../foreach.hpp"

namespace editor2 {

editor_mouse_handler::editor_mouse_handler(editor_display* gui, editor_map& map, editor_mode& mode)
: mouse_handler_base(map), gui_(gui), mode_(mode)
{
}

void editor_mouse_handler::mouse_motion(int x, int y, const bool /*browse*/, bool update)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	const gamemap::location new_hex = gui().hex_clicked_on(x,y);
	gui().highlight_hex(new_hex);
}

void editor_mouse_handler::set_gui(editor_display* gui)
{
	gui_ = gui;
}

bool editor_mouse_handler::left_click(const SDL_MouseButtonEvent& event, const bool /*browse*/)
{
	if (mode_.get_mouse_action() != NULL) {
		editor_action* a = mode_.get_mouse_action()->click(*gui_, event.x, event.y);
	}
	return true;
}
		
} //end namespace editor2
