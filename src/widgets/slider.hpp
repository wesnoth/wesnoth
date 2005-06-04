/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SLIDER_HPP_INCLUDED
#define SLIDER_HPP_INCLUDED

#include "SDL.h"

#include "../sdl_utils.hpp"

#include "widget.hpp"

#include <vector>

namespace gui {

class slider : public widget
{
public:
	slider(CVideo &video);

	void set_min(int value);
	void set_max(int value);
	void set_value(int value);
	void set_increment(int increment);

	int value() const;
	int max_value() const;
	int min_value() const;

	virtual void set_location(SDL_Rect const &rect);

	//VC++ doesn't like a 'using scrollarea::set_location' directive here, so we declare
	//an inline forwarding function instead
	void set_location(int x, int y) { widget::set_location(x,y); }

protected:
	virtual void handle_event(const SDL_Event& event);
	virtual void draw_contents();

private:
	void mouse_motion(const SDL_MouseMotionEvent& event);
	void mouse_down(const SDL_MouseButtonEvent& event);
	void set_slider_position(int x);
	SDL_Rect slider_area() const;
	surface image_, highlightedImage_;

	int min_;
	int max_;
	int value_;
	int increment_;

	enum STATE { UNINIT, NORMAL, ACTIVE, CLICKED, DRAGGED };
	STATE state_;
};

}

#endif
