/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Introduction sequence at start of a scenario, End-screen after end of
 * campaign.
 */

#include "global.hpp"

#include "intro.hpp"

#include "display.hpp"
#include "foreach.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "sound.hpp"
#include "storyscreen/interface.hpp"
#include "unit.hpp"
#include "variable.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

static bool use_shadowm_storyscreen = false;

static void the_end_old(display &disp, std::string text, unsigned int duration)
{
	//
	// Some sane defaults.
	//
	if(text.empty())
		text = _("The End");
	if(!duration)
		duration = 3500;

	SDL_Rect area = screen_area();
	CVideo &video = disp.video();
	SDL_FillRect(video.getSurface(),&area,0);

	update_whole_screen();
	disp.flip();

	const size_t font_size = font::SIZE_XLARGE;

	area = font::text_area(text,font_size);
	area.x = screen_area().w/2 - area.w/2;
	area.y = screen_area().h/2 - area.h/2;

	for(size_t n = 0; n < 255; n += 5) {
		if(n)
			SDL_FillRect(video.getSurface(),&area,0);

		const SDL_Color col = create_color(n, n, n, n);
		font::draw_text(&video,area,font_size,col,text,area.x,area.y);
		update_rect(area);

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp.flip();
		disp.delay(10);
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
		disp.flip();
		disp.delay(10);
		--count;
	}
}

void set_new_storyscreen(bool enabled)
{
	use_shadowm_storyscreen = enabled;
	LOG_NG << "enabled experimental story/endscreen code\n";
}

bool get_new_storyscreen_status()
{
	return use_shadowm_storyscreen;
}

void the_end(display &disp, std::string text, unsigned int duration)
{
	if(use_shadowm_storyscreen) {
		show_endscreen(disp, t_string(text) /* dumb! */, duration);
	}
	else {
		the_end_old(disp,text,duration);
	}
}

