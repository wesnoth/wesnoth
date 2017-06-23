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

#pragma once

#include "mouse_action.hpp"
#include "editor/palette/empty_palette.hpp"
#include "map/label.hpp"

class CKey;

namespace editor {

/**
 * Set map label action.
 */
class mouse_action_map_label : public mouse_action
{
public:
	mouse_action_map_label(const CKey& key, empty_palette& palette)
	: mouse_action(palette, key), click_(false), clicked_on_(), last_draged_()
	  {
	  }

	editor_action* click_left(editor_display& disp, int x, int y);

	/**
	 * Drag operation. A click should have occurred earlier. Defaults to no action.
	 */
	editor_action* drag_left(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Left click displays a dialog that is used for entering the label string.
	 */
	editor_action* up_left(editor_display& disp, int x, int y);

	editor_action* click_right(editor_display& disp, int x, int y);

	/**
	 * Right click erases the label under the mouse.
	 */
	editor_action* up_right(editor_display& disp, int x, int y);

	virtual void set_mouse_overlay(editor_display& disp);

private:
	bool click_;
	map_location clicked_on_;
	map_location last_draged_;
};

} //end namespace editor
