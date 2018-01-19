/*
   Copyright (C) 2004 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "font/text.hpp"
#include "widgets/label.hpp"
#include "font/marked-up_text.hpp"
#include "video.hpp"

namespace gui {

label::label(CVideo& video, const std::string& text, int size, const color_t& color, const bool auto_join) : widget(video, auto_join), text_(text), size_(size), color_(color)
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

const color_t& label::set_color(const color_t& color)
{
	color_ = color;
	set_dirty();
	return get_color();
}

const color_t& label::get_color() const
{
	return (enabled()) ? color_ : font::DISABLED_COLOR;
}

void label::draw_contents()
{
	const SDL_Rect& loc = location();
	if (!text_.empty() && loc.w > 0 && loc.h > 0)
		font::draw_text(&video(), loc, size_, get_color(), text_, loc.x, loc.y);
}

void label::update_label_size()
{
	SDL_Rect area = font::text_area(text_, size_);
	set_measurements(area.w, area.h);
}



}
