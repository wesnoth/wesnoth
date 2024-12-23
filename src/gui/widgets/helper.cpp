/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/helper.hpp"

#include "color.hpp"
#include "formula/callable.hpp"
#include "formula/string_utils.hpp"
#include "gui/core/log.hpp"
#include "sdl/point.hpp"
#include "gui/widgets/settings.hpp"
#include "tstring.hpp"
#include "sdl/input.hpp" // get_mouse_location

namespace gui2
{
font::pango_text::FONT_STYLE decode_font_style(const std::string& style)
{
	if(style == "bold") {
		return font::pango_text::STYLE_BOLD;
	} else if(style == "italic") {
		return font::pango_text::STYLE_ITALIC;
	} else if(style == "underline") {
		return font::pango_text::STYLE_UNDERLINE;
	}

	if(!style.empty() && style != "normal") {
		ERR_GUI_G << "Unknown style '" << style << "', using 'normal' instead.";
	}

	return font::pango_text::STYLE_NORMAL;
}

color_t decode_color(const std::string& color)
{
	return color_t::from_rgba_string(color);
}

PangoWeight decode_text_weight(const std::string& weight)
{
	if(weight == "thin") {
		return PANGO_WEIGHT_THIN;
	} else if (weight == "light") {
		return PANGO_WEIGHT_LIGHT;
	} else if (weight == "semibold") {
		return PANGO_WEIGHT_SEMIBOLD;
	} else if (weight == "bold") {
		return PANGO_WEIGHT_BOLD;
	} else if (weight == "heavy") {
		return PANGO_WEIGHT_HEAVY;
	}

	if(!weight.empty() && weight != "normal") {
		ERR_GUI_E << "Invalid text weight '" << weight << "', falling back to 'normal'.";
	}

	return PANGO_WEIGHT_NORMAL;
}

PangoStyle decode_text_style(const std::string& style)
{
	if(style == "italic") {
		return PANGO_STYLE_ITALIC;
	} else if(style == "oblique") {
		return PANGO_STYLE_OBLIQUE;
	}

	if(!style.empty() && style != "normal") {
		ERR_GUI_E << "Invalid text style '" << style << "', falling back to 'normal'.";
	}

	return PANGO_STYLE_NORMAL;
}

PangoAlignment decode_text_alignment(const std::string& alignment)
{
	if(alignment == "center") {
		return PANGO_ALIGN_CENTER;
	} else if(alignment == "right") {
		return PANGO_ALIGN_RIGHT;
	}

	if(!alignment.empty() && alignment != "left") {
		ERR_GUI_E << "Invalid text alignment '" << alignment << "', falling back to 'left'.";
	}

	return PANGO_ALIGN_LEFT;
}

PangoEllipsizeMode decode_ellipsize_mode(const std::string& ellipsize_mode)
{
	if(ellipsize_mode == "start") {
		return PANGO_ELLIPSIZE_START;
	} else if(ellipsize_mode == "middle") {
		return PANGO_ELLIPSIZE_MIDDLE;
	} else if(ellipsize_mode == "end") {
		return PANGO_ELLIPSIZE_END;
	}

	if(!ellipsize_mode.empty() && ellipsize_mode != "none") {
		ERR_GUI_E << "Invalid text ellipsization mode '" << ellipsize_mode << "', falling back to 'none'.";
	}

	return PANGO_ELLIPSIZE_NONE;
}

std::string encode_ellipsize_mode(const PangoEllipsizeMode ellipsize_mode)
{
	switch(ellipsize_mode) {
		case PANGO_ELLIPSIZE_START:
			return "start";
		case PANGO_ELLIPSIZE_MIDDLE:
			return "middle";
		case PANGO_ELLIPSIZE_END:
			return "end";
		default:
			return "none";
	}
}

std::string encode_text_alignment(const PangoAlignment alignment)
{
	switch(alignment) {
		case PANGO_ALIGN_LEFT:
			return "left";
		case PANGO_ALIGN_RIGHT:
			return "right";
		case PANGO_ALIGN_CENTER:
			return "center";
	}

	assert(false);
	// FIXME: without this "styled_widget reaches end of non-void function" in release mode
	throw "Control should not reach this point.";
}

t_string missing_widget(const std::string& id)
{
	return t_string(VGETTEXT("Mandatory widget ‘$id’ hasn't been defined.", {{"id", id}}));
}

void get_screen_size_variables(wfl::map_formula_callable& variable)
{
	variable.add("screen_width", wfl::variant(settings::screen_width));
	variable.add("screen_height", wfl::variant(settings::screen_height));
	variable.add("gamemap_width", wfl::variant(settings::gamemap_width));
	variable.add("gamemap_height", wfl::variant(settings::gamemap_height));
	variable.add("gamemap_x_offset", wfl::variant(settings::gamemap_x_offset));
}

wfl::map_formula_callable get_screen_size_variables()
{
	wfl::map_formula_callable result;
	get_screen_size_variables(result);

	return result;
}

point get_mouse_position()
{
	return sdl::get_mouse_location();
}

std::string_view debug_truncate(std::string_view text)
{
	return text.substr(0, 15);
}

} // namespace gui2
