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
#include "global.hpp"

#include "tooltips.hpp"

#include "font.hpp"
#include "game_display.hpp"
#include "help.hpp"
#include "marked-up_text.hpp"
#include "resources.hpp"
#include "video.hpp"

#include <boost/foreach.hpp>

namespace {

CVideo* video_ = NULL;

static const int font_size = font::SIZE_SMALL;
static const int text_width = 400;

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act = "", bool use_markup = false)
	: rect(r), message(msg), action(act), markup(use_markup)
	{}
	SDL_Rect rect;
	std::string message;
	std::string action;
	bool markup;
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

	const SDL_Color bgcolor = {0,0,0,160};
	SDL_Rect area = screen_area();

	unsigned int border = 10;

	font::floating_label flabel(tip.message);
	flabel.use_markup(tip.markup);
	flabel.set_font_size(font_size);
	flabel.set_color(font::NORMAL_COLOR);
	flabel.set_clip_rect(area);
	flabel.set_width(text_width);
	flabel.set_alignment(font::LEFT_ALIGN);
	flabel.set_bg_color(bgcolor);
	flabel.set_border_size(border);

	tooltip_handle = font::add_floating_label(flabel);

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
	try {
	clear_tooltips();
	} catch (...) {}
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
		if(sdl::rects_overlap(i->rect,rect)) {
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

void add_tooltip(const SDL_Rect& rect, const std::string& message, const std::string& action, bool use_markup)
{
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ++i) {
		if(sdl::rects_overlap(i->rect,rect)) {
			*i = tooltip(rect, message, action, use_markup);
			return;
		}
	}

	tips.push_back(tooltip(rect, message, action, use_markup));
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

bool click(int mousex, int mousey)
{
	BOOST_FOREACH(tooltip tip, tips) {
		if(!tip.action.empty() && sdl::point_in_rect(mousex, mousey, tip.rect)) {
			display* disp = resources::screen;
			help::show_help(*disp, tip.action);
			return true;
		}
	}
	return false;
}

}
