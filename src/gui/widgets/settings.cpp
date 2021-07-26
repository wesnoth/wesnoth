/*
	Copyright (C) 2007 - 2021
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/widgets/settings.hpp"

#include "display.hpp"

#define MAGIC_DPI_MATCH_VIDEO 96
#define MICRONS_PER_INCH 25400

namespace gui2
{
bool new_widgets = false;

namespace settings
{
unsigned screen_width = 0;
unsigned screen_height = 0;
unsigned screen_pitch_microns = MICRONS_PER_INCH / MAGIC_DPI_MATCH_VIDEO;
unsigned gamemap_x_offset = 0;

unsigned gamemap_width = 0;
unsigned gamemap_height = 0;

unsigned popup_show_delay = 0;
unsigned popup_show_time = 0;
unsigned help_show_time = 0;
unsigned double_click_time = 0;
unsigned repeat_button_repeat_time = 0;

std::string sound_button_click = "";
std::string sound_toggle_button_click = "";
std::string sound_toggle_panel_click = "";
std::string sound_slider_adjust = "";

t_string has_helptip_message;

std::vector<game_tip> tips;

void update_screen_size_variables()
{
	CVideo& vid = CVideo::get_singleton();
	const SDL_Rect rect = vid.screen_area();

	screen_width = rect.w;
	screen_height = rect.h;

	auto [scalew, scaleh] = vid.get_dpi_scale_factor();
	float avgscale = (scalew + scaleh)/2;
	screen_pitch_microns = MICRONS_PER_INCH / (avgscale * MAGIC_DPI_MATCH_VIDEO);

	gamemap_width = screen_width;
	gamemap_height = screen_height;

	if(display* display = display::get_singleton()) {
		const SDL_Rect rect_gm = display->map_outside_area();

		if(rect_gm.w && rect_gm.h) {
			gamemap_width = rect_gm.w;
			gamemap_height = rect_gm.h;
			gamemap_x_offset = rect_gm.x;
		}
	}
}

} // namespace settings

} // namespace gui2
