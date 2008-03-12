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

//! @file window.hpp
//! This file contains the window object, this object is a top level container 
//! which has the event management as well.

#ifndef __GUI_WIDGETS_WINDOW_HPP_INCLUDED__
#define __GUI_WIDGETS_WINDOW_HPP_INCLUDED__

#include "gui/widgets/widget.hpp"
#include "gui/widgets/canvas.hpp"
#include "gui/widgets/event_info.hpp"

#include "sdl_utils.hpp"
#include "video.hpp"

#include "tstring.hpp"
#include "config.hpp"
#include "variable.hpp"

#include "events.hpp"
#include "SDL.h"

#include <string>
#include <map>

namespace gui2{
/**
 * base class of top level items, the only item 
 * which needs to store the final canvase to draw on
 */

// we kunnen de timer gebruiken om een http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fAddTimer
// event aan ons te sturen, oftewel een movemove can dit genereren indien gewenst
//
// mogelijk dit ook gebruiken in de toekomst als aansturing van flip()
class twindow : public tpanel, public events::handler/*, public virtual tevent_executor */
{
public:
	twindow(CVideo& video, const int x, const int y, const int w, const int h);

	~twindow() 
		{ 
			// We have to leave the event context before it's destroyed.
			leave(); 
		}

	// show the window
	// The flip function is the disp_.flip() if ommitted the video_flip() is used
	void show(const bool restore = true, void* flip_function = 0);

	// layout the window
	void layout(const SDL_Rect position);

	enum tstatus{ NEW, SHOWING, REQUEST_CLOSE, CLOSED };

protected:
private:



	void flip();

	CVideo& video_;

	tstatus status_;

	tevent_info event_info_;

	/***** The event processing stuff *****/

	//! we create a new event context so we're always modal.
	//! Maybe this has to change, but not sure yet.
	events::event_context event_context_;

	//! Implement events::handler::handle_event().
	void handle_event(const SDL_Event& event);

	//! Handler for a mouse down.
	void handle_event_mouse_down(const SDL_Event& event);
	
	//! Handler for a mouse up.
	void handle_event_mouse_up(const SDL_Event& event);
	
	//! Handler for a mouse movement.
	void handle_event_mouse_move(const SDL_Event& event);

	//! Handler for a resize of the main window.
	void handle_event_resize(const SDL_Event& event);

	//! When set the form needs a full layout redraw cycle.
	bool need_layout_;
	
};

} // namespace gui2

#endif
