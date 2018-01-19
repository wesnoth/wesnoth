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
	std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex) override;

	/**
	 * Force a fake "move" event to update brush overlay on key event
	 */
	editor_action* key_event(editor_display& disp, const SDL_Event& e) override;

	/**
	 * Left click/drag selects
	 */
	editor_action* click_perform_left(editor_display& disp, const std::set<map_location>& hexes) override;

	/**
	 * Right click does nothing for now
	 */
	editor_action* click_right(editor_display& disp, int x, int y) override;


	/**
	 * Right click/drag
	 */
	editor_action* click_perform_right(editor_display& disp, const std::set<map_location>& hexes) override;

	virtual void set_mouse_overlay(editor_display& disp) override;

	virtual bool has_context_menu() const override { return true; }
	virtual bool supports_brushes() const override { return true; }

};

} //end namespace editor
