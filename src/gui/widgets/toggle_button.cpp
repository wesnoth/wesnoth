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

#include "foreach.hpp"
#include "gui/widgets/canvas.hpp"
#include "gui/widgets/window.hpp"

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


void ttoggle_button::set_members(const std::map<std::string, t_string>& data)
{
	// Inherit
	tcontrol::set_members(data);

	std::map<std::string, t_string>::const_iterator itor = data.find("icon");
	if(itor != data.end()) {
		set_icon_name(itor->second);
	}
}

void ttoggle_button::mouse_enter(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: mouse enter.\n"; 

	if(is_selected()) {
		set_state(FOCUSSED_SELECTED);
	} else {
		set_state(FOCUSSED);
	}
}

void ttoggle_button::mouse_leave(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: mouse leave.\n"; 

	if(is_selected()) {
		set_state(ENABLED_SELECTED);
	} else {
		set_state(ENABLED);
	}
}

void ttoggle_button::mouse_left_button_click(tevent_handler&) 
{ 
	DBG_G_E << "Toggle button: left mouse button click.\n"; 

	if(is_selected()) {
		set_state(ENABLED);
	} else {
		set_state(ENABLED_SELECTED);
	}

	if(callback_mouse_left_click_) {
		callback_mouse_left_click_(this);
	}
}

void ttoggle_button::mouse_left_button_double_click(tevent_handler&)
{
	DBG_G_E << "Toggle button: left mouse button double click.\n"; 

	assert(retval_ != 0);

	twindow* window = get_window();
	assert(window);

	window->set_retval(retval_);
}

void ttoggle_button::set_active(const bool active)
{
	if(active) {
		if(is_selected()) {
			set_state(ENABLED_SELECTED);
		} else {
			set_state(ENABLED);
		}
	} else {
		if(is_selected()) {
			set_state(DISABLED_SELECTED);
		} else {
			set_state(DISABLED);
		}
	}
}

void ttoggle_button::set_canvas_text()
{
	// Inherit.
	tcontrol::set_canvas_text();

	// set icon in canvases
	foreach(tcanvas& canvas, tcontrol::canvas()) {
		canvas.set_variable("icon", variant(icon_name_));
	}
}

void ttoggle_button::set_selected(const bool selected)
{
	if(selected == is_selected()) {
		return;
	}

	if(selected) {
		set_state(static_cast<tstate>(state_ + ENABLED_SELECTED));
	} else {
		set_state(static_cast<tstate>(state_ - ENABLED_SELECTED));
	}
}

void ttoggle_button::set_retval(const int retval)
{
	if(retval == retval_) {
		return;
	}

	retval_ = retval;
	set_wants_mouse_left_double_click(retval_ != 0);
}

void ttoggle_button::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

} // namespace gui2

