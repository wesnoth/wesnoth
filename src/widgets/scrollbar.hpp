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
#include "button.hpp"
#include "widget.hpp"

namespace gui {

class scrollable
{
public:
	virtual void scroll(int pos) = 0;
};

class scrollbar : public widget
{
public:
	/// Create a scrollbar.
	/// \param d the display object
	/// \param pane the widget where wheel events take place
	/// \param callback a callback interface for warning that the grip has been moved
	scrollbar(display &d, widget const &pane, scrollable *callback);

	virtual void set_location(SDL_Rect const &rect);
	using widget::set_location;
	virtual void hide(bool value = true);

	/// This function is used to determine where the scrollbar is.
	/// \return the position. For example, will return 0 if the scrollbar
	///  is at the top, and (full_size - shown_size) if it is at the bottom.
	unsigned get_position() const;

	/// Used to manually update the scrollbar.
	void set_position(unsigned pos);

	/// Ensure the viewport contains the position.
	void adjust_position(unsigned pos);

	///Move the scrollbar.
	void move_position(int dep);

	/// Set the relative size of the grip.
	void set_shown_size(unsigned h);

	/// Set the relative size of the scrollbar.
	void set_full_size(unsigned h);

protected:
	virtual void handle_event(const SDL_Event& event);
	virtual void process_event();
	virtual void draw_contents();

private:
	SDL_Rect grip_area() const;
	SDL_Rect groove_area() const;
	widget const &pane_;
	surface mid_scaled_, groove_scaled_;

	scrollable* callback_;
	button uparrow_, downarrow_;

	enum STATE { UNINIT, NORMAL, ACTIVE, DRAGGED };
	STATE state_;

	int minimum_grip_height_, mousey_on_grip_;
	// Relative data
	int grip_position_, old_position_, grip_height_, full_height_;
};

}

#endif
