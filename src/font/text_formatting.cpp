/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "color.hpp"

#include <iomanip>

namespace font {

std::string span_color(const color_t& color)
{
	return formatter() << "<span color='" << color.to_hex_string() << "'>";
}

std::string span_color(const color_t& color, const std::string& data)
{
	return span_color(color) + data + "</span>";
}

std::string get_pango_color_from_id(const std::string& id)
{
	const auto color = game_config::team_rgb_colors.find(id);
	if(color != game_config::team_rgb_colors.end()) {
		return (color->second[0]).to_hex_string();
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
