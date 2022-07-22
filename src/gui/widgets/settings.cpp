/*
	Copyright (C) 2007 - 2022
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

#include "gui/widgets/settings.hpp"

#include "display.hpp"
#include "video.hpp"

namespace gui2
{
bool new_widgets = false;

namespace settings
{
unsigned screen_width = 0;
unsigned screen_height = 0;
/** screen_pitch_microns is deprecated. Do not use it.
 *
 * This value corresponds to a physical DPI of 96. But physical DPI should
 * not be used to make rendering decisions. With the ability to set pixel
 * scale, it can be assumed that one pixel in draw-space is neither too
 * small nor too large.
 */
const unsigned screen_pitch_microns = 265;
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
	point canvas_size = video::game_canvas_size();

	screen_width = canvas_size.x;
	screen_height = canvas_size.y;

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
