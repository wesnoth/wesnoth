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

#include "editor_common.hpp"

#include "../mouse_handler_base.hpp"

namespace editor2 {

class editor_mouse_handler : public events::mouse_handler_base
{
public:
	editor_mouse_handler(editor_display* gui, editor_map& map, editor_mode& mode);
	void mouse_motion(int x, int y, const bool browse, bool update);
	void set_gui(editor_display* gui);
	editor_display& gui() { return *gui_; }
	const editor_display& gui() const { return *gui_; }
	bool left_click(const SDL_MouseButtonEvent& event, const bool /*browse*/);
	
private:
	editor_display* gui_;
	editor_mode& mode_;
};

} //end namespace editor2

#endif
