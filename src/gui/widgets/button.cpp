/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/button.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"

#include <boost/bind.hpp>

namespace gui2 {

tbutton::tbutton()
	: tcontrol(COUNT)
	, state_(ENABLED)
	, retval_(0)
	, callback_mouse_left_click_(0)
{
	connect_signal<event::MOUSE_ENTER>(boost::bind(
				&tbutton::signal_handler_mouse_enter, this, _1, _2));
	connect_signal<event::MOUSE_LEAVE>(boost::bind(
				&tbutton::signal_handler_mouse_leave, this, _1, _2));
}

void tbutton::mouse_enter(tevent_handler&)
{
	DBG_GUI_E << "Button: mouse enter.\n";

	set_state(FOCUSSED);
}

void tbutton::mouse_leave(tevent_handler&)
{
	DBG_GUI_E << "Button: mouse leave.\n";

	set_state(ENABLED);
}

void tbutton::mouse_left_button_down(tevent_handler& event)
{
	DBG_GUI_E << "Button: left mouse button down.\n";

	event.mouse_capture();

	set_state(PRESSED);
}

void tbutton::mouse_left_button_up(tevent_handler&)
{
	DBG_GUI_E << "Button: left mouse button up.\n";

	set_state(FOCUSSED);
}

void tbutton::mouse_left_button_click(tevent_handler&)
{
	DBG_GUI_E << "Button: left mouse button click.\n";

	sound::play_UI_sound(settings::sound_button_click);

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
		set_dirty(true);
	}
}

const std::string& tbutton::get_control_type() const
{
	static const std::string type = "button";
	return type;
}

void tbutton::signal_handler_mouse_enter(
		const event::tevent event, bool& handled)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".\n";

	set_state(FOCUSSED);
	handled = true;
}

void tbutton::signal_handler_mouse_leave(
		const event::tevent event, bool& handled)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

} // namespace gui2
