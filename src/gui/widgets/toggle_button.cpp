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

#include "gui/widgets/toggle_button.hpp"

#include "log.hpp"

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

namespace gui2 {


void ttoggle_button::mouse_enter(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: mouse enter.\n"; 

	if(is_selected()) {
		set_state(FOCUSSED_UP);
	} else {
		set_state(FOCUSSED_DOWN);
	}
}

void ttoggle_button::mouse_leave(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: mouse leave.\n"; 

	if(is_selected()) {
		set_state(ENABLED_UP);
	} else {
		set_state(ENABLED_DOWN);
	}
}

void ttoggle_button::mouse_left_button_click(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: left mouse button click.\n"; 

	if(is_selected()) {
		set_state(ENABLED_DOWN);
	} else {
		set_state(ENABLED_UP);
	}

	if(callback_mouse_left_click_) {
		callback_mouse_left_click_(this);
	}
}

void ttoggle_button::set_active(const bool active)
{
	if(active) {
		if(is_selected()) {
			set_state(ENABLED_UP);
		} else {
			set_state(ENABLED_DOWN);
		}
	} else {
		if(is_selected()) {
			set_state(DISABLED_UP);
		} else {
			set_state(DISABLED_DOWN);
		}
	}
}

void ttoggle_button::set_selected(const bool selected)
{
	if(selected == is_selected()) {
		return;
	}

	if(selected) {
		set_state(static_cast<tstate>(state_ - ENABLED_DOWN));
	} else {
		set_state(static_cast<tstate>(state_ + ENABLED_DOWN));
	}
}

void ttoggle_button::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

} // namespace gui2

