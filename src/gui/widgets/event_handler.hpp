/* $Id$ */
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

//! @file event_handler.hpp
//! Contains the information with an event.

#ifndef __GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED__
#define __GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED__

#include "SDL.h"


namespace gui2{

class twidget;

class tevent_info 
{
public:
	tevent_info();

	void handle_event(const SDL_Event& event, twidget* mouse_over);

	void mouse_capture(const bool capture = true);

private:

	int mouse_x_;                      //! The current mouse x.
	int mouse_y_;                      //! The current mouse y.

	bool mouse_left_button_down_;      //! Is the left mouse button down?
	bool mouse_middle_button_down_;    //! Is the middle mouse button down?
	bool mouse_right_button_down_;     //! Is the right mouse button down?

	Uint32 last_left_click_;	
	Uint32 last_middle_click_;	
	Uint32 last_right_click_;	

	bool hover_pending_;			   //! Is there a hover event pending?
	unsigned hover_id_;                //! Id of the pending hover event.
	SDL_Rect hover_box_;               //! The area the mouse can move in, moving outside
	                                   //! invalidates the pending hover event.
    bool had_hover_;                   //! A widget only gets one hover event per enter cycle. 


	twidget* mouse_focus_;
	bool mouse_captured_;


	void mouse_enter(const SDL_Event& event, twidget* mouse_over);
	void mouse_move(const SDL_Event& event, twidget* mouse_over);
	void mouse_hover(const SDL_Event& event, twidget* mouse_over);
	void mouse_leave(const SDL_Event& event, twidget* mouse_over);


	void mouse_left_button_down(const SDL_Event& event, twidget* mouse_over);
	void mouse_left_button_up(const SDL_Event& event, twidget* mouse_over);

	void set_hover(const bool test_on_widget = false);


};

} // namespace gui2

#endif
