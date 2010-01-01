/* $Id$ */
/*
   Copyright (C) 2004 - 2010 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/label.hpp"
#include "marked-up_text.hpp"

namespace gui {

label::label(CVideo& video, const std::string& text, int size, const SDL_Color& colour, const bool auto_join) : widget(video, auto_join), text_(text), size_(size), colour_(colour)
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

const SDL_Color& label::set_colour(const SDL_Color& colour)
{
	colour_ = colour;
	set_dirty();
	return get_colour();
}

const SDL_Color& label::get_colour() const
{
	return (enabled()) ? colour_ : font::DISABLED_COLOUR;
}

void label::draw_contents()
{
	const SDL_Rect& loc = location();
	if (!text_.empty() && loc.w > 0 && loc.h > 0)
		font::draw_text(&video(), loc, size_, get_colour(), text_, loc.x, loc.y);
}

void label::update_label_size()
{
	SDL_Rect area = font::text_area(text_, size_);
	set_measurements(area.w, area.h);
}


}
