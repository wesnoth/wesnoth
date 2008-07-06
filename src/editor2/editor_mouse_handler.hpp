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

#ifndef EDITOR2_EDITOR_MOUSE_HANDLER_HPP
#define EDITOR2_EDITOR_MOUSE_HANDLER_HPP

#include "../mouse_handler_base.hpp"

#include "editor_common.hpp"

namespace editor2 {

class editor_mouse_handler : public events::mouse_handler_base
{
public:
	editor_mouse_handler(editor_display* disp, editor_map& map);
	void mouse_motion(int x, int y, const bool browse, bool update);
	void set_gui(editor_display* gui);
	editor_display& gui() { return static_cast<editor_display&>(*gui_); }
};

} //end namespace editor2

#endif
