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

#include "gui/widgets/canvas.hpp"
#include "gui/widgets/event_handler.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/helper.hpp" 
#include "gui/widgets/settings.hpp"

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
class twindow : public tpanel, public tevent_handler
{
public:
	twindow(CVideo& video, const int x, const int y, const int w, const int h);

	// show the window
	// The flip function is the disp_.flip() if ommitted the video_flip() is used
	int show(const bool restore = true, void* flip_function = 0);

	// layout the window
	void layout(const SDL_Rect position);

	enum tstatus{ NEW, SHOWING, REQUEST_CLOSE, CLOSED };

	void close() { status_ = REQUEST_CLOSE; }

	void set_retval(const int retval, const bool close_window = true)
		{ retval_ = retval; if(close_window) close(); }

	void set_width(const unsigned width);

	void set_height(const unsigned height);

	twindow& get_window() { return *this; }

	//! Inherited from tevent_handler
	twidget* get_widget(const tpoint& coordinate) { return tpanel::get_widget(coordinate); }

	tpoint client_position(const tpoint& screen_position) const
		{ return tpoint(screen_position.x - get_x(), screen_position.y - get_y()); }

	void window_resize(tevent_handler&, 
		const unsigned new_width, const unsigned new_height);

	//! A window is always active atm so ignore the request.
	void set_active(const bool active) {}
	bool get_active() const { return true; }
	unsigned get_state() const { return 0; }
	bool full_redraw() const { return false; /* FIXME IMPLEMENT */ }

	// Inherited from twidget.
	tpoint get_minimum_size() const { /*FIXME IMPLEMENT*/ return tpoint(0,0); } 
	tpoint get_best_size() const { /*FIXME IMPLEMENT*/ return tpoint(0,0); } 
	tpoint get_maximum_size() const { /*FIXME IMPLEMENT*/ return tpoint(0,0); }

protected:
private:


	void flip();

	CVideo& video_;

	tstatus status_;

	// return value of the window, 0 default.
	int retval_;

	//! When set the form needs a full layout redraw cycle.
	bool need_layout_;

	surface restorer_;
	
	tcanvas
		canvas_background_,
		canvas_foreground_;

	std::vector<twindow_definition::tresolution>::const_iterator definition_;

	void resolve_definition();

	SDL_Rect get_client_rect();
};

} // namespace gui2

#endif
