/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "widgets/progressbar.hpp"

#include "font.hpp"
#include "marked-up_text.hpp"
#include "util.hpp"
#include "video.hpp"

namespace gui {

progress_bar::progress_bar(CVideo& video) : widget(video), progress_(0)
{}

void progress_bar::set_progress_percent(int progress)
{
	progress_ = progress;
	set_dirty();
}

void progress_bar::set_text(const std::string& text)
{
	text_ = text;
	set_dirty();
}

void progress_bar::draw_contents()
{
	surface const surf = video().getSurface();
	SDL_Rect area = location();

	if(area.w >= 2 && area.h >= 2) {
		SDL_Rect inner_area = {area.x+1,area.y+1,area.w-2,area.h-2};
		SDL_FillRect(surf,&area,SDL_MapRGB(surf->format,0,0,0));
		SDL_FillRect(surf,&inner_area,SDL_MapRGB(surf->format,255,255,255));

		inner_area.w = (inner_area.w*progress_)/100;
		SDL_FillRect(surf,&inner_area,SDL_MapRGB(surf->format,21,53,80));

		const std::string text = text_.empty() ? str_cast(progress_) + "%" :
		                         text_ + " (" + str_cast(progress_) + "%)";
		SDL_Rect text_area = font::text_area(text,font::SIZE_NORMAL);

		text_area.x = area.x + area.w/2 - text_area.w/2;
		text_area.y = area.y + area.h/2 - text_area.h/2;

		font::draw_text(&video(),location(),font::SIZE_NORMAL,font::BLACK_COLOUR,text,text_area.x,text_area.y);
	}

	update_rect(location());
}

}
