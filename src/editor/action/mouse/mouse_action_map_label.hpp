/* $Id$ */
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

#ifndef EDITOR_MOUSE_ACTION_MAP_LABEL_HPP
#define EDITOR_MOUSE_ACTION_MAP_LABEL_HPP

#include "mouse_action.hpp"

class CKey;

namespace editor {

/**
 * Set map label action.
 */
class mouse_action_map_label : public mouse_action
{
public:
	mouse_action_map_label(const CKey& key)
	: mouse_action(key), click_(false), clicked_on_(), last_draged_(), tmp_label_(NULL)
	  {
	  }

	editor_action* click_left(editor_display& disp, int x, int y);

	/**
	 * Left click displays a dialog that is used for entering the label string.
	 */
	editor_action* up_left(editor_display& disp, int x, int y);

	editor_action* click_right(editor_display& disp, int x, int y);

	/**
	 * Right click erases the label under the mouse.
	 */
	editor_action* up_right(editor_display& disp, int x, int y);

	/**
	 * Drag operation. A click should have occured earlier. Defaults to no action.
	 */
	editor_action* drag_left(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * TODO
	 */
	editor_action* drag_end(editor_display& disp, int x, int y);

	virtual void set_mouse_overlay(editor_display& disp);

private:
	bool click_;
	map_location clicked_on_;
	map_location last_draged_;
	terrain_label* tmp_label_;
};

} //end namespace editor

#endif
