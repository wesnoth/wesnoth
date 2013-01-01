/* $Id$ */
/*
   Copyright (C) 2008 - 2013 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_MOUSE_ACTION_UNIT_HPP
#define EDITOR_MOUSE_ACTION_UNIT_HPP

#include "editor/action/mouse/mouse_action.hpp"
#include "editor/palette/unit_palette.hpp"

class CKey;

namespace editor {

/**
 * Unit placement action class
 */
class mouse_action_unit : public mouse_action
{
public:
	mouse_action_unit(const CKey& key, unit_palette& palette)
		: mouse_action(palette, key)
		, click_(false)
		, start_hex_()
		, old_direction_(map_location::NORTH)
		, new_direction_(map_location::NORTH)
		, unit_palette_(palette)
	{
	}

	/**
	 * TODO
	 */
	editor_action* click_left(editor_display& disp, int x, int y);

	/**
	 * TODO
	 */
	editor_action* up_left(editor_display& disp, int x, int y);

	editor_action* drag_left(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Drag end replaces the unit when clicked left, or adjusts
	 * the facing when clicked right.
	 */
	editor_action* drag_end_left(editor_display& disp, int x, int y);

	/**
	 * TODO
	 */
	editor_action* click_right(editor_display& disp, int x, int y);

	/**
	 * Right click only erases the unit if there is one.
	 * Do this on mouse up to avoid drag issue.
	 */
	editor_action* up_right(editor_display& disp, int x, int y);

	/**
	 *
	 */
	editor_action* drag_right(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Drag end replaces the unit when clicked left, or adjusts
	 * the facing when clicked right.
	 */
	editor_action* drag_end_right(editor_display& disp, int x, int y);

	virtual void set_mouse_overlay(editor_display& disp);

private:
	bool click_;

	map_location start_hex_;
	map_location::DIRECTION old_direction_;
	map_location::DIRECTION new_direction_;
	unit_palette& unit_palette_;
};


} //end namespace editor

#endif
