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
	slider(display& d, const SDL_Rect& rect);

	void set_min(int value);
	void set_max(int value);
	void set_value(int value);
	int value() const;
	int max_value() const;
	int min_value() const;

	void process();

private:
	SDL_Rect slider_area() const;
	void draw();

	int min_;
	int max_;
	int value_;

	bool highlight_;
	bool clicked_;
	bool dragging_;
};

}

#endif
