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

namespace gui{

class scrollbar : public widget
{
public:
	/// Create a scrollbar
	/// \param d The display object
	///
	scrollbar(display& d);

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

protected:
	using widget::bg_restore;
	using widget::set_dirty;
	using widget::dirty;

private:
	SDL_Rect scroll_grip_area() const;
	void draw();

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
};
	
}

#endif
