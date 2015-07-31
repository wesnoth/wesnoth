/*
   Copyright (C) 2008 - 2015 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_MOUSE_ACTION_SELECT_HPP
#define EDITOR_MOUSE_ACTION_SELECT_HPP

#include "mouse_action.hpp"

class CKey;
class empty_palette;

namespace editor {

/**
 * Select (and deselect) action, by brush or "magic wand" (via keyboard modifier)
 */
class mouse_action_select : public brush_drag_mouse_action
{
public:
	mouse_action_select(const brush* const * const brush, const CKey& key, empty_palette& palette)
	: brush_drag_mouse_action(palette, brush, key)
	{
	}

	/**
	 * Overridden to allow special behavior based on modifier keys
	 */
	std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex);

	/**
	 * Force a fake "move" event to update brush overlay on key event
	 */
	editor_action* key_event(editor_display& disp, const SDL_Event& e);

	/**
	 * Left click/drag selects
	 */
	editor_action* click_perform_left(editor_display& disp, const std::set<map_location>& hexes);

	/**
	 * Right click does nothing for now
	 */
	editor_action* click_right(editor_display& disp, int x, int y);


	/**
	 * Right click/drag
	 */
	editor_action* click_perform_right(editor_display& disp, const std::set<map_location>& hexes);

	virtual void set_mouse_overlay(editor_display& disp);

	bool has_context_menu() const { return true; }
	bool supports_brushes() { return true; }

};

} //end namespace editor

#endif
