/*
	Copyright (C) 2004 - 2025
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "button.hpp"

namespace gui {


/** Scrollbar */
class scrollbar : public widget
{
public:
	/**
	 * Create a scrollbar.
	 * @todo FIXME: parameterlist ??
	 */
	//- @param d         the display object
	//- @param pane      the widget where wheel events take place
	//- @param callback  a callback interface for warning that the grip has been moved
	scrollbar();

	/**
	 * Determine where the scrollbar is.
	 *
	 * @return  the position.
	 * @retval  returns 0 if the scrollbar is at the top or (full_size - shown_size) if it is at the bottom.
	 */
	unsigned get_position() const;

	unsigned get_max_position() const;

	/** Manually update the scrollbar. */
	void set_position(unsigned pos);

	/** Ensure the viewport contains the position. */
	void adjust_position(unsigned pos);

	/** Move the scrollbar. */
	void move_position(int dep);

	/** Set the relative size of the grip. */
	void set_shown_size(unsigned h);

	/** Set the relative size of the scrollbar. */
	void set_full_size(unsigned h);

	/** Set scroll rate. */
	void set_scroll_rate(unsigned r);

	/** Scrolls down one step */
	void scroll_down();

	/** Scrolls up one step */
	void scroll_up();

protected:
	virtual void handle_event(const SDL_Event& event);
	virtual void draw_contents();

private:
	rect grip_area() const;

	enum STATE { UNINIT, NORMAL, ACTIVE, DRAGGED };
	STATE state_;

	int minimum_grip_height_, mousey_on_grip_;
	// Relative data
	unsigned int grip_position_, grip_height_, full_height_;
	int scroll_rate_;

};

} // end namespace gui
