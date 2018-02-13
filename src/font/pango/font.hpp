/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "font/text.hpp"
#include <pango/pango.h>

namespace font {

/** Small helper class to make sure the pango font object is destroyed properly. */
class p_font
{
public:
	p_font(const std::string& name, const unsigned size, font::pango_text::FONT_STYLE style)
		: font_(pango_font_description_new())
	{
		pango_font_description_set_family(font_, name.c_str());
		pango_font_description_set_size(font_, size * PANGO_SCALE);

		if(style != pango_text::STYLE_NORMAL) {
			if(style & pango_text::STYLE_ITALIC) {
				pango_font_description_set_style(font_, PANGO_STYLE_ITALIC);
			}
			if(style & pango_text::STYLE_BOLD) {
				pango_font_description_set_weight(font_, PANGO_WEIGHT_BOLD);
			}
			if(style & pango_text::STYLE_UNDERLINE) {
				/* Do nothing here, underline is a property of the layout. */
			}
		}
	}

	p_font(const p_font &) = delete;
	p_font & operator = (const p_font &) = delete;

	~p_font() { pango_font_description_free(font_); }

	PangoFontDescription* get() { return font_; }

private:
	PangoFontDescription *font_;
};

} // namespace font
