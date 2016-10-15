#pragma once

#include "font/text.hpp"
#include <pango/pango.h>

namespace font {

/** Small helper class to make sure the pango font object is destroyed properly. */
class p_font
{
public:
	p_font(const std::string& name, const unsigned size, const unsigned style) :
		font_(pango_font_description_new())
	{
		pango_font_description_set_family(font_, name.c_str());
		pango_font_description_set_size(font_, size * PANGO_SCALE);

		if(style != ttext::STYLE_NORMAL) {
			if(style & ttext::STYLE_ITALIC) {
				pango_font_description_set_style(font_, PANGO_STYLE_ITALIC);
			}
			if(style & ttext::STYLE_BOLD) {
				pango_font_description_set_weight(font_, PANGO_WEIGHT_BOLD);
			}
			if(style & ttext::STYLE_UNDERLINE) {
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
