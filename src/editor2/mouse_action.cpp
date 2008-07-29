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

#include "action.hpp"
#include "brush.hpp"
#include "editor_common.hpp"
#include "editor_display.hpp"
#include "mouse_action.hpp"

#include "../foreach.hpp"
#include "../pathutils.hpp"

namespace editor2 {

void mouse_action::move(editor_display& disp, int x, int y)
{
}

editor_action* mouse_action::drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	return NULL;
}

editor_action* mouse_action::drag_end(editor_display& disp, int x, int y)
{
	return NULL;
}


void brush_drag_mouse_action::move(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	if (hex != previous_move_hex_) {
		disp.set_brush_locs(get_brush().project(disp.hex_clicked_on(x,y)));
		previous_move_hex_ = hex;
	}
}

editor_action* brush_drag_mouse_action::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform(disp, hex);
}

editor_action* brush_drag_mouse_action::drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	move(disp, x, y);
	gamemap::location hex = disp.hex_clicked_on(x, y);
	if (hex != previous_drag_hex_) {
		editor_action* a = click_perform(disp, hex);
		previous_drag_hex_ = hex;
		return a;
	} else {
		return NULL;
	}
}
	
editor_action* brush_drag_mouse_action::drag_end(editor_display& disp, int x, int y)
{
	return NULL;
}

const brush& brush_drag_mouse_action::get_brush()
{
	assert(brush_);
	assert(*brush_);
	return **brush_;
}


editor_action* mouse_action_paint::click_perform(editor_display& disp, const gamemap::location& hex)
{
	return new editor_action_paint_area(get_brush().project(hex), terrain_);
}

editor_action* mouse_action_select::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	selecting_ = !disp.map().in_selection(hex);
	return brush_drag_mouse_action::click(disp, x, y);
}

editor_action* mouse_action_select::click_perform(editor_display& disp, const gamemap::location& hex)
{
	editor_action* a(NULL);
	if (selecting_) {
		a = new editor_action_select(get_brush().project(hex));
	} else {
		a = new editor_action_deselect(get_brush().project(hex));
	}
	return a;
}


void mouse_action_paste::move(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	std::set<gamemap::location> affected = paste_.get_offset_area(hex);
	disp.set_brush_locs(affected);
}

editor_action* mouse_action_paste::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	editor_action_paste* a = new editor_action_paste(hex, paste_);
	return a;
}


void mouse_action_fill::move(editor_display& disp, int x, int y)
{
	std::set<gamemap::location> affected = 
		disp.map().get_contigious_terrain_tiles(disp.hex_clicked_on(x, y));
	disp.set_brush_locs(affected);
}

editor_action* mouse_action_fill::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	editor_action_fill* a = new editor_action_fill(hex, terrain_);
	return a;
}

} //end namespace editor2
