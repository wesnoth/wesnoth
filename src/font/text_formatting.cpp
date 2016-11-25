/*
   Copyright (C) 2003 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/text_formatting.hpp"

#include "formatter.hpp"
#include "game_config.hpp"
#include "gettext.hpp"

#include <iomanip>

namespace font {

std::string unit32_to_pango_color(uint32_t rgb)
{
	std::ostringstream h;

	// Must match what the pango expects
	h << "#"
	  << std::hex << std::setfill('0') << std::setw(2) << ((rgb & 0xFF0000) >> 16)
	  << std::hex << std::setfill('0') << std::setw(2) << ((rgb & 0x00FF00) >> 8)
	  << std::hex << std::setfill('0') << std::setw(2) <<  (rgb & 0x0000FF);

	return h.str();
}

std::string color2hexa(const SDL_Color& color)
{
	return unit32_to_pango_color((color.r << 16 ) + (color.g << 8) + (color.b));
}

std::string span_color(const SDL_Color& color)
{
	return "<span color='" + color2hexa(color) + "'>";
}

std::string get_pango_color_from_id(const std::string& id)
{
	const auto color = game_config::team_rgb_colors.find(id);
	if(color != game_config::team_rgb_colors.end()) {
		return unit32_to_pango_color(color->second[0]);
	}

	return "";
}

std::string get_color_string_pango(const std::string& id)
{
	const auto name = game_config::team_rgb_name.find(id);
	if(name != game_config::team_rgb_name.end()) {
		return formatter() << "<span color='" << get_pango_color_from_id(id) << "'>" << name->second << "</span>";
	}

	return _("Invalid Color");
}

}
