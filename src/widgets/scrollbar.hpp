/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
                 2004 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

class scrollarea;

class scrollbar : public widget
{
public:
	/// Create a scrollbar.
	/// \param d the display object
	/// \param pane the widget where wheel events take place
	/// \param callback a callback interface for warning that the grip has been moved
	scrollbar(CVideo &video);

	virtual void hide(bool value = true);

	/// This function is used to determine where the scrollbar is.
	/// \return the position. For example, will return 0 if the scrollbar
	///  is at the top, and (full_size - shown_size) if it is at the bottom.
	unsigned get_position() const;

	unsigned get_max_position() const;

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

	/// Set scroll rate.
	void set_scroll_rate(unsigned r);

	/// Return true if the scrollbar has a valid size.
	bool is_valid_height(int height) const;

protected:
	virtual void update_location(SDL_Rect const &rect);
	virtual void handle_event(const SDL_Event& event);
	virtual void process_event();
	virtual void draw_contents();

private:
	SDL_Rect grip_area() const;
	SDL_Rect groove_area() const;
	surface mid_scaled_, groove_scaled_;

	button uparrow_, downarrow_;

	enum STATE { UNINIT, NORMAL, ACTIVE, DRAGGED };
	STATE state_;

	int minimum_grip_height_, mousey_on_grip_;
	// Relative data
	int grip_position_, old_position_, grip_height_, full_height_, scroll_rate_;

	friend class scrollarea;
};

}

#endif
