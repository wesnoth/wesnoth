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

//! @file event_info.hpp
//! Contains the information with an event.

#ifndef __GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED__
#define __GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED__

namespace gui2{

struct tevent_info 
{
	tevent_info();

	//! Sets status for next event.
	void cycle();

	int mouse_x;                       //! The current mouse x.
	int mouse_y;                       //! The current mouse y.

	int mouse_last_x;                  //! The mouse x in the last event.
	int mouse_last_y;                  //! The mouse y in the last event.

	bool mouse_left_button_down;       //! Is the left mouse button down?
	bool mouse_middle_button_down;     //! Is the middle mouse button down?
	bool mouse_right_button_down;      //! Is the right mouse button down?

	bool mouse_last_left_button_down;  //! Was the left mouse button down in the last event?
	bool mouse_last_middle_button_down;//! Was the middle mouse button down in the last event?
	bool mouse_last_right_button_down; //! Was the right mouse button down in the last event?

	int event_mouse_button;            //! If a mouse event it shows which button changed.
};

} // namespace gui2

#endif
