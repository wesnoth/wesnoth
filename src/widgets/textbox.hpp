/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "serialization/unicode.hpp"
#include "font/constants.hpp"
#include "font/standard_colors.hpp"

#include "scrollarea.hpp"

namespace gui {

class textbox : public scrollarea
{
public:
	textbox(CVideo &video, int width, const std::string& text="", bool editable=true, size_t max_size = 256, int font_size = font::SIZE_PLUS, double alpha = 0.4, double alpha_focus = 0.2, const bool auto_join = true);
	virtual ~textbox();

	const std::string text() const;
	void set_text(const std::string& text, const color_t& color =font::NORMAL_COLOR);
	void append_text(const std::string& text,bool auto_scroll = false, const color_t& color =font::NORMAL_COLOR);
	void clear();

	void set_selection(const int selstart, const int selend);
	void set_cursor_pos(const int cursor_pos);

	void set_editable(bool value);
	bool editable() const;

	int font_size() const;
	void set_font_size(int fs);

	void scroll_to_bottom();

	void set_wrap(bool val);

	void set_edit_target(textbox* target);

protected:
	virtual void draw_contents();
	virtual void update_location(SDL_Rect const &rect);
	virtual void set_inner_location(SDL_Rect const &);
	virtual void scroll(unsigned int pos);

private:
	virtual void handle_text_changed(const ucs4::string&) {}

	size_t max_size_;

	int font_size_;

	ucs4::string text_;

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

	textbox* edit_target_;

	/* This boolean is used to filter out any TextInput events that are received without
	 * the corresponding KeyPress events. This is needed to avoid a bug when creating a
	 * textbox using a hotkey.
	 * */
	bool listening_;

	void handle_event(const SDL_Event& event, bool was_forwarded);

	void handle_event(const SDL_Event& event);

	void pass_event_to_target(const SDL_Event& event);

	void draw_cursor(int pos, CVideo &video) const;
	void update_text_cache(bool reset = false, const color_t& color =font::NORMAL_COLOR);
	surface add_text_line(const ucs4::string& text, const color_t& color =font::NORMAL_COLOR);
	bool is_selection();
	void erase_selection();

	//make it so that only one textbox object can be receiving
	//events at a time.
	bool requires_event_focus(const SDL_Event *event=nullptr) const;

	bool show_scrollbar() const;
	bool handle_text_input(const SDL_Event& event);
	bool handle_key_down(const SDL_Event &event);
};

}
