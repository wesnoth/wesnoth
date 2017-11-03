/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "editor/action/action_base.hpp"
#include "editor/map/editor_map.hpp"
#include "theme.hpp"
#include "editor/palette/editor_palettes.hpp"
#include "editor/palette/terrain_palettes.hpp"
#include "editor/palette/location_palette.hpp"
#include "editor/palette/empty_palette.hpp"

class CKey;

namespace editor {

/**
 * A mouse action receives events from the controller, and responds to them by creating
 * an appropriate editor_action object. Mouse actions may store some temporary data
 * such as the last clicked hex for better handling of click-drag. They should *not* modify
 * the map or trigger refreshes, but may set brush locations and similar overlays that
 * should be visible around the mouse cursor, hence the display references are not const.
 */
class mouse_action
{
public:
	mouse_action(common_palette& palette, const CKey& key)
		: previous_move_hex_()
		, key_(key)
		, toolbar_button_(nullptr)
		, palette_(palette)
	{
	}

	virtual ~mouse_action() {}

	virtual bool has_context_menu() const;

	/**
	 * Mouse move (not a drag). Never changes anything (other than temporary highlights and similar)
	 */
	virtual void move(editor_display& disp, const map_location& hex);

	/**
	 * Unconditionally update the brush highlights for the current tool when hex is the center location
	 */
	void update_brush_highlights(editor_display& disp, const map_location& hex);

	/**
	 * Locations that would be affected by a click, used by move to update highlights. Defaults to highlight the mouseover hex.
	 * Maybe also used for actually performing the action in click() or drag().
	 */
	virtual std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex);

	/**
	 * A click, possibly the beginning of a drag. Must be overridden.
	 */
	virtual editor_action* click_left(editor_display& disp, int x, int y) = 0;

	/**
	 * A click, possibly the beginning of a drag. Must be overridden.
	 */
	virtual editor_action* click_right(editor_display& disp, int x, int y) = 0;

	/**
	 * Drag operation. A click should have occurred earlier. Defaults to no action.
	 */
	virtual editor_action* drag_left(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Drag operation. A click should have occurred earlier. Defaults to no action.
	 */
	virtual editor_action* drag_right(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * The end of dragging. Defaults to no action.
	 */
	virtual editor_action* drag_end_left(editor_display& disp, int x, int y);

	virtual editor_action* drag_end_right(editor_display& disp, int x, int y);

	virtual editor_action* up_left(editor_display& disp, int x, int y);

	virtual editor_action* up_right(editor_display& disp, int x, int y);

	/**
	 * Function called by the controller on a key event for the current mouse action.
	 * Defaults to starting position processing.
	 */
	virtual editor_action* key_event(editor_display& disp, const SDL_Event& e);

	/**
	 * Helper variable setter - pointer to a toolbar menu/button used for highlighting
	 * the current action. Should always be nullptr or point to a valid menu.
	 */
	void set_toolbar_button(const theme::menu* value) { toolbar_button_ = value; }

	/**
	 * Getter for the (possibly nullptr) associated menu/button.
	 */
	const theme::menu* toolbar_button() const { return toolbar_button_; }

	/**
	 * Getter for the associated palette.
	 */
	common_palette& get_palette() { return palette_; }

	/** Whether we need the brush bar, is used to grey it out.*/
	virtual bool supports_brushes() const { return false; }

	/**
	 * Set the mouse overlay for this action. Defaults to an empty overlay.
	 */
	virtual void set_mouse_overlay(editor_display& disp);


protected:
	bool has_alt_modifier() const;
	bool has_shift_modifier() const;
	bool has_ctrl_modifier() const;

	/**
	 * Helper function for derived classes that need a active-terrain mouse overlay
	 */
	void set_terrain_mouse_overlay(editor_display& disp, const t_translation::terrain_code & fg,
		const t_translation::terrain_code & bg);

	/**
	 * The hex previously used in move operations
	 */
	map_location previous_move_hex_;

	/**
	 * Key presses, used for modifiers (alt, shift) in some operations
	 */
	const CKey& key_;

private:
	/**
	 * Pointer to an associated menu/button, if such exists
	 */
	const theme::menu* toolbar_button_;

	/**
	 * Pointer to an associated palette, if such exists
	 */
	common_palette& palette_;
};

/**
 * A brush-drag mouse action base class which adds brush and drag processing to a basic mouse action
 */
class brush_drag_mouse_action : public mouse_action
{
public:
	brush_drag_mouse_action(common_palette& palette, const brush* brush, const CKey& key)
		: mouse_action(palette, key)
		, previous_drag_hex_()
		, brush_(brush)
	{
	}

	/**
	 * The affected hexes of a brush action are the result of projecting the current brush on the mouseover hex
	 */
	std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex);

	/**
	 * The actual action function which is called by click() and drag(). Derived classes override this instead of click() and drag().
	 */
	virtual editor_action* click_perform_left(editor_display& disp, const std::set<map_location>& hexes) = 0;

	/**
	 * The actual action function which is called by click() and drag(). Derived classes override this instead of click() and drag().
	 */
	virtual editor_action* click_perform_right(editor_display& disp, const std::set<map_location>& hexes) = 0;

	/**
	 * Calls click_perform_left()
	 */
	editor_action* click_left(editor_display& disp, int x, int y);

	/**
	 * Calls click_perform_right()
	 */
	editor_action* click_right(editor_display& disp, int x, int y);

	/**
	 * Calls click_perform() for every new hex the mouse is dragged into.
	 * @todo partial actions support and merging of many drag actions into one
	 */
	editor_action* drag_left(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Calls click_perform for every new hex the mouse is dragged into.
	 * @todo partial actions support and merging of many drag actions into one
	 */
	editor_action* drag_right(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * End of dragging.
	 * @todo partial actions (the entire drag should end up as one action)
	 */
	editor_action* drag_end(editor_display& disp, int x, int y);

protected:
	/** Brush accessor */
	const brush& get_brush();

	/**
	 * The previous hex dragged into.
	 * @todo keep a set of all "visited" locations to reduce action count in long drags that hit the same hexes multiple times?
	 */
	map_location previous_drag_hex_;

private:
	/**
	 * Template helper gathering actions common for both drag_right and drag_left.
	 * The drags differ only in the worker function called, which should be
	 * passed as the template parameter. This exists only to avoid copy-pasting code.
	 */
	template <editor_action* (brush_drag_mouse_action::*perform_func)(editor_display&, const std::set<map_location>&)>
	editor_action* drag_generic(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);

	/**
	 * Current brush handle. Currently pointer to brush with full constness.
	 * The mouse action does not modify the brush, does not modify the pointer
	 * to the current brush, and we allow setting this pointr only once, hence
	 * the three "consts".
	 */
	const brush* const brush_;
};

/**
 * Brush paint mouse action. Uses keyboard modifiers for one-layer painting.
 */
class mouse_action_paint : public brush_drag_mouse_action
{
public:
	mouse_action_paint(
		const brush* brush, const CKey& key, terrain_palette& palette)
	: brush_drag_mouse_action(palette, brush, key)
	, terrain_palette_(palette)
	{
	}

	/**
	 * Handle terrain sampling before calling generic handler
	 */
	editor_action* click_left(editor_display& disp, int x, int y) override;

	/**
	 * Handle terrain sampling before calling generic handler
	 */
	editor_action* click_right(editor_display& disp, int x, int y) override;

	/**
	 * Create an appropriate editor_action and return it
	 */
	editor_action* click_perform_left(editor_display& disp, const std::set<map_location>& hexes) override;

	/**
	 * Create an appropriate editor_action and return it
	 */
	editor_action* click_perform_right(editor_display& disp, const std::set<map_location>& hexes) override;

	void set_mouse_overlay(editor_display& disp) override;

	virtual bool supports_brushes() const override { return true; }

protected:

	terrain_palette& terrain_palette_;

};



/**
 * Paste action. No dragging capabilities.
 */
class mouse_action_paste : public mouse_action
{
public:
	mouse_action_paste(const map_fragment& paste, const CKey& key, common_palette& palette)
	: mouse_action(palette, key), paste_(paste)
	{
	}

	virtual bool has_context_menu() const override;

	/**
	 * Show an outline of where the paste will go
	 */
	std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex) override;

	/**
	 * Return a paste with offset action
	 */
	editor_action* click_left(editor_display& disp, int x, int y) override;

	/**
	 * Right click does nothing for now
	 */
	editor_action* click_right(editor_display& disp, int x, int y) override;

	virtual void set_mouse_overlay(editor_display& disp) override;

protected:
	/**
	 * Reference to the buffer used for pasting (e.g. the clipboard)
	 */
	const map_fragment& paste_;
};

/**
 * Fill action. No dragging capabilities. Uses keyboard modifiers for one-layer painting.
 */
class mouse_action_fill : public mouse_action
{
public:
	mouse_action_fill(const CKey& key,
			terrain_palette& terrain_palette)
	: mouse_action(terrain_palette, key)
	, terrain_palette_(terrain_palette)
	{
	}

	/**
	 * Tiles that will be painted to, possibly use modifier keys here
	 */
	std::set<map_location> affected_hexes(editor_display& disp, const map_location& hex);

	/**
	 * Left / right click fills with the respective terrain
	 */
	editor_action* click_left(editor_display& disp, int x, int y);

	/**
	 * Left / right click fills with the respective terrain
	 */
	editor_action* click_right(editor_display& disp, int x, int y);

	virtual void set_mouse_overlay(editor_display& disp);

protected:
	terrain_palette& terrain_palette_;
};

/**
 * Set starting position action.
 */
class mouse_action_starting_position : public mouse_action
{
public:
	mouse_action_starting_position(const CKey& key, location_palette& palette)
	: mouse_action(palette, key), click_(false), location_palette_(palette)
	{
	}

	/**
	 * Left click displays a player-number-selector dialog and then creates an action
	 * or returns nullptr if cancel was pressed or there would be no change.
	 * Do this on mouse up to avoid drag issue.
	 */
	editor_action* up_left(editor_display& disp, int x, int y);

	editor_action* click_left(editor_display& disp, int x, int y);
	/**
	 * Right click only erases the starting position if there is one.
	 * Do this on mouse up to avoid drag issue,
	 */
	editor_action* up_right(editor_display& disp, int x, int y);

	editor_action* click_right(editor_display& disp, int x, int y);

	virtual void set_mouse_overlay(editor_display& disp);

private:
	bool click_;
	location_palette& location_palette_;
};



} //end namespace editor
