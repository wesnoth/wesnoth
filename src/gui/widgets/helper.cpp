/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/settings.hpp"
#include "sdl/rect.hpp"

#include "formula_string_utils.hpp"

#include "SDL_ttf.h"

namespace gui2
{

namespace
{
static bool initialized_ = false;
}

bool init()
{
	if(initialized_) {
		return true;
	}

	load_settings();

	initialized_ = true;

	return initialized_;
}

SDL_Rect create_rect(const tpoint& origin, const tpoint& size)
{
	return sdl::create_rect(origin.x, origin.y, size.x, size.y);
}

unsigned decode_font_style(const std::string& style)
{
	if(style == "bold") {
		return TTF_STYLE_BOLD;
	} else if(style == "italic") {
		return TTF_STYLE_ITALIC;
	} else if(style == "underline") {
		return TTF_STYLE_UNDERLINE;
	} else if(style.empty() || style == "normal") {
		return TTF_STYLE_NORMAL;
	}

	ERR_GUI_G << "Unknown style '" << style << "' using 'normal' instead."
			  << std::endl;

	return TTF_STYLE_NORMAL;
}

boost::uint32_t decode_color(const std::string& color)
{
	std::vector<std::string> fields = utils::split(color);

	// make sure we have four fields
	while(fields.size() < 4)
		fields.push_back("0");

	boost::uint32_t result = 0;
	for(int i = 0; i < 4; ++i) {
		// shift the previous value before adding, since it's a nop on the
		// first run there's no need for an if.
		result = result << 8;
		result |= lexical_cast_default<int>(fields[i]);
	}

	return result;
}

PangoAlignment decode_text_alignment(const std::string& alignment)
{
	if(alignment == "center") {
		return PANGO_ALIGN_CENTER;
	} else if(alignment == "right") {
		return PANGO_ALIGN_RIGHT;
	} else {
		if(!alignment.empty() && alignment != "left") {
			ERR_GUI_E << "Invalid text alignment '" << alignment
					  << "' falling back to 'left'.\n";
		}
		return PANGO_ALIGN_LEFT;
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
	// FIXME: without this "control reaches end of non-void function" in release
	// mode
	throw "Control should not reach this point.";
}

t_string missing_widget(const std::string& id)
{
	utils::string_map symbols;
	symbols["id"] = id;

	return t_string(
			vgettext("Mandatory widget '$id' hasn't been defined.", symbols));
}

void get_screen_size_variables(game_logic::map_formula_callable& variable)
{
	variable.add("screen_width", variant(settings::screen_width));
	variable.add("screen_height", variant(settings::screen_height));
	variable.add("gamemap_width", variant(settings::gamemap_width));
	variable.add("gamemap_height", variant(settings::gamemap_height));
	variable.add("gamemap_x_offset", variant(settings::gamemap_x_offset));
}

game_logic::map_formula_callable get_screen_size_variables()
{
	game_logic::map_formula_callable result;
	get_screen_size_variables(result);

	return result;
}

tpoint get_mouse_position()
{
	int x, y;
	SDL_GetMouseState(&x, &y);

	return tpoint(x, y);
}

std::string debug_truncate(const std::string& text)
{
	return text.substr(0, 15);
}

} // namespace gui2
