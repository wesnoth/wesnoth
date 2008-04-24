/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TEXTBOX_HPP_INCLUDED
#define TEXTBOX_HPP_INCLUDED

#include "../serialization/string_utils.hpp"
#include "../sdl_utils.hpp"

#include "scrollarea.hpp"

#include "SDL.h"

namespace gui {

class textbox : public scrollarea
{
public:
	textbox(CVideo &video, int width, const std::string& text="", bool editable=true, size_t max_size = 256, double alpha = 0.4, double alpha_focus = 0.2, const bool auto_join = true);
	virtual ~textbox();

	const std::string text() const;
	void set_text(const std::string& text);
	void append_text(const std::string& text,bool auto_scroll = false);
	void clear();
	void process();

	void set_editable(bool value);
	bool editable() const;

	void scroll_to_bottom();

	void set_wrap(bool val);

	void set_location(const SDL_Rect& rect);

	//VC++ doesn't like a 'using scrollarea::set_location' directive here, so we declare
	//an inline forwarding function instead
	void set_location(int x, int y) { widget::set_location(x,y); }

protected:
	virtual void draw_contents();
	virtual void set_inner_location(SDL_Rect const &);
	virtual void scroll(unsigned int pos);

private:
	virtual void handle_text_changed(const wide_string&) {}

	size_t max_size_;

	wide_string text_;

	// mutable unsigned int firstOnScreen_;
	int cursor_;
	int selstart_;
	int selend_;
	bool grabmouse_;

	int text_pos_;
	int cursor_pos_;
	std::vector<int> char_x_, char_y_;

	bool editable_;

	bool show_cursor_;

	//records the time the cursor was shown at last
	//the cursor should be inverted every 500 ms.
	//this will be reset when keyboard input events occur
	int show_cursor_at_;
	surface text_image_;

	bool wrap_;

	size_t line_height_, yscroll_;

	double alpha_;
	double alpha_focus_;

	void handle_event(const SDL_Event& event);

	void draw_cursor(int pos, CVideo &video) const;
	void update_text_cache(bool reset = false);
	surface add_text_line(const wide_string& text);
	bool is_selection();
	void erase_selection();

	//make it so that only one textbox object can be receiving
	//events at a time.
	bool requires_event_focus(const SDL_Event *event=NULL) const;

	bool show_scrollbar() const;
};

}

#endif
