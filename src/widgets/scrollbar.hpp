/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCROLLBAR_HPP_INCLUDED
#define SCROLLBAR_HPP_INCLUDED

#include "SDL.h"
#include "../sdl_utils.hpp"
#include "widget.hpp"
#include <vector>

namespace gui {

class scrollable
{
public:
	virtual void scroll(int pos) = 0;
};

/// class scrollbar implements a rather stupid scrollbar widget. It requires
/// some hand-holding from the widget using it. Many of these functions will
/// likely be removed (possibly replaced) at a later date, as the "widget"
/// class hierarchy is expanded
class scrollbar : public widget
{
public:
	/// Create a scrollbar
	/// \param d The display object
	///
	scrollbar(display& d, scrollable* callback);

	/// \return the current width of the scrollbar (if it is disabled,
	///			that value is zero
	///
	int get_width() const;

	/// \return the maximum width of the scrollbar, determined by the
	///			image files used and a constant "padding" value
	///
	int get_max_width() const;
	

	/// Process any scrollbar usage
	///
	void process();

	/// Turn scrollbar on or off
	///
	/// \param en true to enable, false otherwise
	///
	void enable(bool en);
	
	/// \return true if scrollbar enabled, false otherwise
	bool enabled() const;

	void redraw();

	/// This function is used to determine where the scrollbar is. This 
	/// should be used when trying to find out if the user has moved
	/// the scrollbar.
	///
	/// \return The distance in pixels from the highest
	///  (vertically) position. For example, will return 0 if the scrollbar
	///  is at the top.
	int get_grip_position() const;

	/// Used to manually update the scrollbar; for example, if the user has
	/// changed the scroll position some other way (via button or mousewheel)
	///
	/// \param the distance from the top of the scrollbar
	///
	/// \return true if the function succeeds, false otherwise.
	bool set_grip_position(int pos); 

	/// \return the smallest the scrollbar "grip" can be
	int get_minimum_grip_height() const;

	/// \return the current height of the grip.
	int get_grip_height() const;

	/// Set the size of the grip
	///
	/// \param pos the size for the grip
	///
	/// \return true if successful, false otherwise.
	bool set_grip_height(int pos);

	/// This function determines whether the user clicked on the scrollbar
	/// groove, and whether it was above or below the grip
	/// 
	/// \return -1 if click was above, 1 if click was below, 0 otherwise
	int  groove_clicked() const;

private:
	SDL_Rect scroll_grip_area() const;
	void draw();

	scrollable* callback_;

	int minimum_grip_height_;
	int width_;
	bool highlight_;
	bool clicked_;

	bool dragging_;

	// location of scrollbar grip (number of pixels below top of scrollbar)
	int grip_position_;

	// vertical extent of grip
	int grip_height_;

	int enabled_;

	// -1 if user just clicked above the groove, 1 if they just clicked below
	// 0 otherwise
	int groove_click_code_;
};
	
}

#endif
