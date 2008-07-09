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


namespace editor2 {

class mouse_action
{
public:
	mouse_action(editor_mode& mode)
	: mode_(mode)
	{
	}
	virtual editor_action* click(editor_display& disp, int mousex, int mousey) const = 0;
//	virtual void drag_start();
//	virtual void drag_through();
//	virtual void drag_end();
protected:
	editor_mode& mode_;
};

class mouse_action_paint : public mouse_action
{
public:
	mouse_action_paint(editor_mode& mode)
	: mouse_action(mode)
	{
	}
	editor_action* click(editor_display& disp, int mousex, int mousey) const;
};


} //end namespace editor2

#endif
