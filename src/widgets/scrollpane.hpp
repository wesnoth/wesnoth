/*
   Copyright (C) 2004 - 2016 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef SCROLLPANE_HPP_INCLUDED
#define SCROLLPANE_HPP_INCLUDED

#include <map>
#include <vector>

#include "SDL.h"
#include "sdl/utils.hpp"
#include "scrollarea.hpp"

namespace gui {

/** Scrollpane. */
class scrollpane : public scrollarea
{
public:
	struct scrollpane_widget {
		scrollpane_widget(widget* w, int x=0, int y=0, int z_order=0)
			: w(w), x(x), y(y), z_order(z_order) {}

		widget* w;
		int x;
		int y;
		int z_order;
	};

	/**
	 * Create a scrollpane.
	 * @todo FIXME: parameterlist ??
	 */
	//- @param d the display object
	//- @param pane the widget where wheel events take place
	//- @param callback a callback interface for warning that the grip has been moved
	scrollpane(CVideo &video);

	virtual void set_location(SDL_Rect const &rect);

	// VC++ doesn't like a 'using scrollarea::set_location' directive here,
	// so we declare an inline forwarding function instead.
	void set_location(int x, int y) { widget::set_location(x,y); }

	virtual void hide(bool value=true);

	void add_widget(widget* w, int x, int y, int z_order = 0);
	void remove_widget(widget* w);
	void clear();

protected:
	//virtual void handle_event(const SDL_Event& event);
	//virtual void process_event();
	virtual void draw();
	virtual void set_inner_location(SDL_Rect const &rect);
	virtual void scroll(unsigned int pos);

private:
	void update_widget_positions();
	void position_widget(scrollpane_widget& spw);
	SDL_Rect client_area() const;
	void update_content_size();

	int border_;
	typedef std::multimap<int, scrollpane_widget> widget_map;
	widget_map content_;
	SDL_Rect content_pos_;
};

} // namespace gui

#endif
