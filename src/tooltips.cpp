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

#include "font.hpp"
#include "marked-up_text.hpp"
#include "sdl_utils.hpp"
#include "tooltips.hpp"
#include "video.hpp"

#include <vector>

static bool rectangles_overlap(const SDL_Rect& a, const SDL_Rect& b)
{
	const bool xoverlap = a.x >= b.x && a.x < b.x + b.w ||
	                      b.x >= a.x && b.x < a.x + a.w;

	const bool yoverlap = a.y >= b.y && a.y < b.y + b.h ||
	                      b.y >= a.y && b.y < a.y + a.h;

	return xoverlap && yoverlap;
}

namespace {

CVideo* video_ = NULL;

static const int font_size = font::SIZE_SMALL;
#ifdef USE_TINY_GUI
static const int text_width = 260;
#else
static const int text_width = 400;
#endif

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg) : rect(r), message(msg)
	{}
	SDL_Rect rect;
	std::string message;
};

std::vector<tooltip> tips;
std::vector<tooltip>::const_iterator current_tooltip = tips.end();

int tooltip_handle = 0;

surface current_background = NULL;

}

static void clear_tooltip()
{
	if(tooltip_handle != 0) {
		font::remove_floating_label(tooltip_handle);
		tooltip_handle = 0;
	}
}

static void show_tooltip(const tooltip& tip)
{
	if(video_ == NULL) {
		return;
	}

	clear_tooltip();

	const SDL_Color bgcolour = {0,0,0,128};
	SDL_Rect area = screen_area();

#ifdef USE_TINY_GUI
	unsigned int border = 2;
#else
	unsigned int border = 10;
#endif

	const std::string wrapped_message = font::word_wrap_text(tip.message, font_size, text_width);
	tooltip_handle = font::add_floating_label(wrapped_message,font_size,font::NORMAL_COLOUR,
	                                          0,0,0,0,-1,area,font::LEFT_ALIGN,&bgcolour,border);

	SDL_Rect rect = font::get_floating_label_rect(tooltip_handle);

	//see if there is enough room to fit it above the tip area
	if(tip.rect.y > rect.h) {
		rect.y = tip.rect.y - rect.h;
	} else {
		rect.y = tip.rect.y + tip.rect.h;
	}

	rect.x = tip.rect.x;
	if(rect.x < 0) {
		rect.x = 0;
	} else if(rect.x + rect.w > area.w) {
		rect.x = area.w - rect.w;
	}

	font::move_floating_label(tooltip_handle,rect.x,rect.y);
}

namespace tooltips {

manager::manager(CVideo& video)
{
	clear_tooltips();
	video_ = &video;
}

manager::~manager()
{
	clear_tooltips();
	video_ = NULL;
}

void clear_tooltips()
{
	clear_tooltip();
	tips.clear();
	current_tooltip = tips.end();
}

void clear_tooltips(const SDL_Rect& rect)
{
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ) {
		if(rectangles_overlap(i->rect,rect)) {
			if (i==current_tooltip) {
				clear_tooltip();
			}
			i = tips.erase(i);
			current_tooltip = tips.end();
		} else {
			++i;
		}
	}
}

void add_tooltip(const SDL_Rect& rect, const std::string& message)
{
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ++i) {
		if(rectangles_overlap(i->rect,rect)) {
			*i = tooltip(rect,message);
			return;
		}
	}

	tips.push_back(tooltip(rect,message));
	current_tooltip = tips.end();
}

void process(int mousex, int mousey)
{
	for(std::vector<tooltip>::const_iterator i = tips.begin(); i != tips.end(); ++i) {
		if(mousex > i->rect.x && mousey > i->rect.y &&
		   mousex < i->rect.x + i->rect.w && mousey < i->rect.y + i->rect.h) {
			if(current_tooltip != i) {
				show_tooltip(*i);
				current_tooltip = i;
			}

			return;
		}
	}

	clear_tooltip();
	current_tooltip = tips.end();
}

}
