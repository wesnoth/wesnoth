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

#ifndef TEXTBOX_HPP_INCLUDED
#define TEXTBOX_HPP_INCLUDED

#include "../display.hpp"
#include "../events.hpp"
#include "../key.hpp"
#include "../sdl_utils.hpp"

#include "widget.hpp"

#include "SDL.h"

namespace gui {

class textbox : public widget
{
public:
	textbox(display& disp, int width, const std::string& text="");

	const std::string& text() const;
	void set_text(std::string text);
	void clear();

	void set_position(int x, int y);
	void set_width(int w);

	void set_focus(bool new_focus);

	using widget::location;

private:
	display& disp_;
	std::string text_;
	unsigned int firstOnScreen_, cursor_;

	scoped_sdl_surface buffer_;

	bool focus_;

	void handle_event(const SDL_Event& event);

	void draw() const;

	void draw_cursor(int pos) const;
};

}

#endif
