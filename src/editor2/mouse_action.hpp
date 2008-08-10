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

#ifndef EDITOR2_MOUSE_ACTION_HPP
#define EDITOR2_MOUSE_ACTION_HPP

#include "action_base.hpp"
#include "editor_map.hpp"
#include "../map.hpp"
#include "../terrain.hpp"

class CKey;

namespace editor2 {


/**
 * A mouse action receives events from the controller, and responds to them by creating 
 * an appropriate editor_action object. Mouse actions may store some temporary data
 * such as the last clicked hex for better handling of click-drag.
 */
class mouse_action
{
public:
	mouse_action()
	: toolbar_button_(NULL)
	{
	}

	virtual ~mouse_action() {}
	
	virtual void move(editor_display& disp, int x, int y);
	
	/**
	 * A click, possibly the beginning of a drag
	 */
	virtual editor_action* click(editor_display& disp, int x, int y) = 0;
	
	/**
	 * Drag operation. A click should have occured earlier.
	 */
	virtual editor_action* drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);
	
	/**
	 * The end of dragging.
	 */
	virtual editor_action* drag_end(editor_display& disp, int x, int y);
	
	void set_toolbar_button(const theme::menu* value) { toolbar_button_ = value; }
	const theme::menu* toolbar_button() const { return toolbar_button_; }

protected:
	gamemap::location previous_move_hex_;
	
private:
	const theme::menu* toolbar_button_;
};

class brush_drag_mouse_action : public mouse_action
{
public:
	brush_drag_mouse_action(const brush* const * const brush)
	: mouse_action(), brush_(brush)
	{
	}
	void move(editor_display& disp, int x, int y);
	editor_action* click(editor_display& disp, int x, int y);
	virtual editor_action* click_perform(editor_display& disp, const gamemap::location& hex) = 0;
	editor_action* drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo);
	editor_action* drag_end(editor_display& disp, int x, int y);	
protected:
	const brush& get_brush();
	gamemap::location previous_drag_hex_;
private:
	const brush* const * const brush_;
};

class mouse_action_paint : public brush_drag_mouse_action
{
public:
	mouse_action_paint(const t_translation::t_terrain& terrain, const brush* const * const brush, const CKey& key)
	: brush_drag_mouse_action(brush), terrain_(terrain), key_(key)
	{
	}
	editor_action* click_perform(editor_display& disp, const gamemap::location& hex);
protected:
	const t_translation::t_terrain& terrain_;
	const CKey& key_;
};

class mouse_action_select : public brush_drag_mouse_action
{
public:
	mouse_action_select(const brush* const * const brush)
	: brush_drag_mouse_action(brush), selecting_(true)
	{
	}
	editor_action* click(editor_display& disp, int x, int y);
	editor_action* click_perform(editor_display& disp, const gamemap::location& hex);
protected:
	bool selecting_;
};

class mouse_action_paste : public mouse_action
{
public:
	mouse_action_paste(const map_fragment& paste)
	: mouse_action(), paste_(paste)
	{
	}
	void move(editor_display& disp, int x, int y);
	editor_action* click(editor_display& disp, int x, int y);
protected:
	const map_fragment& paste_;
};

class mouse_action_fill : public mouse_action
{
public:
	mouse_action_fill(const t_translation::t_terrain& terrain, const CKey& key)
	: mouse_action(), terrain_(terrain), key_(key)
	{
	}
	void move(editor_display& disp, int x, int y);
	editor_action* click(editor_display& disp, int x, int y);
protected:
	const t_translation::t_terrain& terrain_;
	const CKey& key_;
};

class mouse_action_starting_position : public mouse_action
{
public:
	mouse_action_starting_position()
	: mouse_action()
	{
	}
	void move(editor_display& disp, int x, int y);
	editor_action* click(editor_display& disp, int x, int y);
};

} //end namespace editor2

#endif
