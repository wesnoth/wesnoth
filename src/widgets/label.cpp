/*
   Copyright (C) 2004 - 2014 by Philippe Plantier <ayin@anathas.org>
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

#include "global.hpp"

#include "widgets/label.hpp"
#include "marked-up_text.hpp"
#include "text.hpp"
#include "video.hpp"

namespace gui {

label::label(CVideo& video, const std::string& text, int size, const SDL_Color& color, const bool auto_join) : widget(video, auto_join), text_(text), size_(size), color_(color)
{
#ifdef SDL_GPU
	render_text();
#else
	update_label_size();
#endif
}

const std::string& label::set_text(const std::string& text)
{
	if (text_ == text)
		return text_;

	text_ = text;
#ifdef SDL_GPU
	render_text();
#else
	update_label_size();
#endif
	set_dirty();
	return text_;
}

const std::string& label::get_text() const
{
	return text_;
}

const SDL_Color& label::set_color(const SDL_Color& color)
{
	color_ = color;
#ifdef SDL_GPU
	render_text();
#endif
	set_dirty();
	return get_color();
}

const SDL_Color& label::get_color() const
{
	return (enabled()) ? color_ : font::DISABLED_COLOR;
}

void label::draw_contents()
{
	const SDL_Rect& loc = location();
	if (!text_.empty() && loc.w > 0 && loc.h > 0)
#ifdef SDL_GPU
		video().draw_texture(text_image_, loc.x, loc.y);
#else
		font::draw_text(&video(), loc, size_, get_color(), text_, loc.x, loc.y);
#endif
}

void label::update_label_size()
{
	SDL_Rect area = font::text_area(text_, size_);
	set_measurements(area.w, area.h);
}

#ifdef SDL_GPU
void label::render_text()
{
	font::ttext txt;

	txt.set_text(text_, true);
	const Uint32 color = (color_.r << 16) + (color_.g << 16) + (color_.b);
	txt.set_foreground_color(color);
	txt.set_font_size(size_);

	text_image_ = txt.render_as_texture();
	set_measurements(text_image_.width(), text_image_.height());
}
#endif


}
