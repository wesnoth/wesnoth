/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Introduction sequence at start of a scenario, End-screen after end of
 * campaign.
 */

#include "intro.hpp"

#include "video.hpp"
#include "gettext.hpp"
#include "font/marked-up_text.hpp"
#include "color.hpp"
#include "sdl/rect.hpp"
#include "font/sdl_ttf.hpp"

void the_end(CVideo &video, std::string text, unsigned int duration)
{
	//
	// Some sane defaults.
	//
	if(text.empty())
		text = _("The End");
	if(!duration)
		duration = 3500;

	SDL_Rect area = screen_area();
	sdl::fill_rect(video.getSurface(),&area,0);

	video.flip();

	const size_t font_size = font::SIZE_XLARGE;

	area = font::text_area(text,font_size);
	area.x = screen_area().w/2 - area.w/2;
	area.y = screen_area().h/2 - area.h/2;

	for(size_t n = 0; n < 255; n += 5) {
		if(n)
			sdl::fill_rect(video.getSurface(),&area,0);

		const color_t col = color_t(uint8_t(n), uint8_t(n), uint8_t(n), uint8_t(n));
		font::draw_text(&video,area,font_size,col,text,area.x,area.y);

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		video.flip();
		CVideo::delay(10);
	}

	//
	// Delay after the end of fading.
	// Rounded to multiples of 10.
	//
	unsigned int count = duration/10;
	while(count) {
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		video.flip();
		CVideo::delay(10);
		--count;
	}
}
