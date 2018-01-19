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

#pragma once

#include "editor/action/mouse/mouse_action.hpp"
#include "editor/palette/item_palette.hpp"

class CKey;

namespace editor {

/**
 * item placement action class
 */
class mouse_action_item : public mouse_action
{
public:
	mouse_action_item(const CKey& key, item_palette& palette)
		: mouse_action(palette, key)
		, click_(false)
		, start_hex_()
		, item_palette_(palette)
	{
	}

	bool has_context_menu() const {
		return true;
	}

	void move(editor_display& disp, const map_location& hex);

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
	 * Drag end replaces the item when clicked left, or adjusts
	 * the facing when clicked right.
	 */
	editor_action* drag_end_left(editor_display& disp, int x, int y);

	editor_action* click_right(editor_display& /*disp*/, int /*x*/, int /*y*/) {
		return nullptr;
	}

	virtual void set_mouse_overlay(editor_display& disp);
	void set_item_mouse_overlay(editor_display& disp, const overlay& u);

private:
	bool click_;

	map_location start_hex_;
	item_palette& item_palette_;
};


} //end namespace editor
