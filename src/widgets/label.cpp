/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "label.hpp"
#include "../display.hpp"
#include "../font.hpp"

namespace gui {

label::label(display& d, const std::string& text, int size, const SDL_Color& colour) : widget(d), text_(text), size_(size), colour_(colour)
{
	update_label_size();
}

const std::string& label::set_text(const std::string& text)
{
	if (text_ == text)
		return text_;

	text_ = text;
	update_label_size();
	set_dirty();
	return text_;
}

const std::string& label::get_text() const
{
	return text_;
}

int label::set_size(int size)
{
	size_ = size;
	update_label_size();
	set_dirty();
	return size_;
}

int label::get_size() const 
{
	return size_;
}

const SDL_Color& label::set_colour(const SDL_Color& colour)
{
	colour_ = colour;
	set_dirty();
	return colour_;
}

const SDL_Color& label::get_colour() const
{
	return colour_;
}

void label::draw_contents()
{
	font::draw_text(&disp(), disp().screen_area(), size_, colour_, text_, location().x, location().y);
}

void label::update_label_size()
{
	SDL_Rect area = font::text_area(text_, size_);
	set_measurements(area.w, area.h);
}


}
