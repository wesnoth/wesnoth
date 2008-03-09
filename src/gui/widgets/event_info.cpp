/* $Id: boilerplate-header.cpp 20001 2007-08-31 19:09:40Z soliton $ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file event_info.cpp
//! Implementation of event_info.hpp.

#include "gui/widgets/event_info.hpp"

#include "config.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2{

//! At construction we should get the state and from that moment on we keep
//! track of the changes ourselves, not yet sure what happens when an input
//! blocker is used.
tevent_info::tevent_info() :
	// fixme get state at construction
	mouse_x(-1),
	mouse_y(-1),

	mouse_last_x(-1),
	mouse_last_y(-1),

	mouse_left_button_down(false),
	mouse_middle_button_down(false),
	mouse_right_button_down(false),

	mouse_last_left_button_down(false),
	mouse_last_middle_button_down(false),
	mouse_last_right_button_down(false),

	event_mouse_button(-1)
{
}

//! Sets status for next event.
//!
//! This should be called before processing an event, that way
//! it's possible to track the state of the system.
void tevent_info::cycle() 
{
	mouse_last_x = mouse_x;
	mouse_last_y = mouse_y;

	mouse_last_left_button_down = mouse_left_button_down;
	mouse_last_middle_button_down = mouse_middle_button_down;
	mouse_last_right_button_down = mouse_right_button_down;

	event_mouse_button = -1;
}

} // namespace gui2
