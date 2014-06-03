/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "widgets/progressbar.hpp"

#include "font.hpp"
#include "marked-up_text.hpp"
#include "sdl/rect.hpp"
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
	surface surf = video().getSurface();
	SDL_Rect area = location();

	if(area.w >= 2 && area.h >= 2) {
		int fcr =  21, fcg =  53, fcb =  80;		// RGB-values for finished piece.
		int bcr =   0, bcg =   0, bcb =   0;		// Border color.
		int gcr = 255, gcg = 255, gcb = 255;		// Groove color.
		int	lightning_thickness = 2;
		static const SDL_Color selected_text_color = {0xCC,0xCC,0xCC,0};

		SDL_Rect inner_area = sdl::create_rect(area.x + 1
				, area.y + 1
				, area.w - 2
				, area.h - 2);

		sdl::fill_rect(surf,&area,SDL_MapRGB(surf->format,bcr,bcg,bcb));
		sdl::fill_rect(surf,&inner_area,SDL_MapRGB(surf->format,gcr,gcg,gcb));

		inner_area.w = (inner_area.w*progress_)/100;
		sdl::fill_rect(surf,&inner_area,SDL_MapRGB(surf->format,fcr,fcg,fcb));

		SDL_Rect lightning = inner_area;
		lightning.h = lightning_thickness;
		//we add 25% of white to the color of the bar to simulate a light effect
		sdl::fill_rect(surf,&lightning,SDL_MapRGB(surf->format,(fcr*3+255)/4,(fcg*3+255)/4,(fcb*3+255)/4));
		lightning.y = inner_area.y+inner_area.h-lightning.h;
		//remove 50% of color to simulate a shadow effect
		sdl::fill_rect(surf,&lightning,SDL_MapRGB(surf->format,fcr/2,fcg/2,fcb/2));

		const std::string text = text_.empty() ? str_cast(progress_) + "%" :
		                         text_ + " (" + str_cast(progress_) + "%)";
		SDL_Rect text_area = font::text_area(text,font::SIZE_NORMAL);

		text_area.x = area.x + area.w/2 - text_area.w/2;
		text_area.y = area.y + area.h/2 - text_area.h/2;

		font::draw_text(
			&video(),
			location(),
			font::SIZE_NORMAL,
			font::BLACK_COLOR,
			text,
			text_area.x,
			text_area.y
		);

		// Draw a white text section for the highlighted area
		// of the bar
		SDL_Rect selected_text_location = location();
		selected_text_location.w = inner_area.w;
		selected_text_location.h = inner_area.h;
		{
			clip_rect_setter clippy(surf, &selected_text_location);
			font::draw_text(
				&video(),
				selected_text_location,
				font::SIZE_NORMAL,
				selected_text_color,
				text,
				text_area.x,
				text_area.y
			);
		}
	}

	update_rect(location());
}

}
