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

#include "SDL.h"

namespace gui {

#define INPUT_CHAR_START (' ')
#define INPUT_CHAR_END ('~' + 1)
#define CHAR_LENGTH (INPUT_CHAR_END - INPUT_CHAR_START)

class textbox : public events::handler
{
	display& disp_;
	std::string text_;
	unsigned int firstOnScreen_, cursor_;
	int height_, width_;

	scoped_sdl_surface buffer_;
	int x_, y_;

	CKey key_;
	bool previousKeyState_[CHAR_LENGTH];

	bool focus_;

	void handle_event(const SDL_Event& event);

	void draw_cursor(int pos) const;

public:
	textbox(display& disp, int width, const std::string& text="");

	int x() const;
	int y() const;
	int height() const;
	int width() const;
	const std::string& text() const;
	void set_text(std::string text);
	void clear();

	void draw() const;
	void process();

	void set_location(int x, int y);
	void set_width(int w);

	void set_focus(bool new_focus);
};

}

#endif
