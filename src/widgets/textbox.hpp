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

#include "../events.hpp"
#include "../key.hpp"
#include "../sdl_utils.hpp"

#include "widget.hpp"

#include "SDL.h"

namespace gui {

class textbox : public widget
{
public:
	textbox(display& d, int width, const std::string& text="");

	const std::string& text() const;
	void set_text(std::string text);
	void clear();
	void process();

protected:
	using widget::bg_restore;
	using widget::set_dirty;
	using widget::dirty;

private:
	std::string text_;
	mutable unsigned int firstOnScreen_;
	unsigned int cursor_;
	bool show_cursor_;

	void handle_event(const SDL_Event& event);

	void draw();
	void draw_cursor(int pos, display &disp) const;
};

}

#endif
