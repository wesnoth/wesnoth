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

#include "button.hpp"
#include "scrollbar.hpp"
#include "widget.hpp"

#include "SDL.h"

namespace gui {

class textbox : public widget, public scrollable
{
public:
	textbox(display& d, int width, const std::string& text="", bool editable=true);

	const std::string text() const;
	void set_text(std::string text);
	void clear();
	void process();

	void set_editable(bool value);
	bool editable() const;

	void scroll_to_bottom();

	void set_wrap(bool val);

	void draw();

private:
	void scroll(int pos);

	std::wstring text_;
	
	// mutable unsigned int firstOnScreen_;
	int cursor_;
	int selstart_;
	int selend_;
	bool grabmouse_;

	int text_pos_;
	int cursor_pos_;
	std::vector<int> char_pos_;

	bool editable_;

	bool show_cursor_;

	//records the time the cursor was shown at last
	//the cursor should be inverted every 500 ms.
	//this will be reset when keyboard input events occur
	int show_cursor_at_;
	shared_sdl_surface text_image_;
	SDL_Rect text_size_;

	//variables used for multi-line textboxes which support scrolling
	scrollbar scrollbar_;
	button uparrow_, downarrow_;

	bool scroll_bottom_;

	bool wrap_;

	void handle_event(const SDL_Event& event);

	void draw_cursor(int pos, display &disp) const;
	void update_text_cache(bool reset = false);
	bool is_selection();
	void erase_selection();

	//make it so that only one textbox object can be receiving
	//events at a time.
	bool requires_event_focus() const { return editable_; }

	bool show_scrollbar() const;
};

}

#endif
