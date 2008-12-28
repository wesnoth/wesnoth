/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/button.hpp"

#include "gui/widgets/window.hpp"

namespace gui2 {

void tbutton::mouse_enter(tevent_handler&)
{
	DBG_G_E << "Button: mouse enter.\n";

	set_state(FOCUSSED);
}

void tbutton::mouse_leave(tevent_handler&)
{
	DBG_G_E << "Button: mouse leave.\n";

	set_state(ENABLED);
}

void tbutton::mouse_left_button_down(tevent_handler& event)
{
	DBG_G_E << "Button: left mouse button down.\n";

	event.mouse_capture();

	set_state(PRESSED);
}

void tbutton::mouse_left_button_up(tevent_handler&)
{
	DBG_G_E << "Button: left mouse button up.\n";

	set_state(FOCUSSED);
}

void tbutton::mouse_left_button_click(tevent_handler&)
{
	DBG_G_E << "Button: left mouse button click.\n";

	// If a button has a retval do the default handling.
	if(retval_ != 0) {
		twindow* window = get_window();
		if(window) {
			window->set_retval(retval_);
			return;
		}
	}

	if(callback_mouse_left_click_) {
		callback_mouse_left_click_(this);
	}
}

void tbutton::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_block_easy_close(get_visible() && get_active());
		set_dirty(true);
	}
}

} // namespace gui2
