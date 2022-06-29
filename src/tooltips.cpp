/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "tooltips.hpp"

#include "floating_label.hpp"
#include "font/standard_colors.hpp"
#include "game_display.hpp"
#include "help/help.hpp"
#include "video.hpp"

#include <SDL2/SDL_rect.h> // Travis doesn't like this, although it works on my machine -> '#include <SDL2/SDL_sound.h>

namespace {

static const int font_size = font::SIZE_SMALL;
static const int text_width = 400;

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act = "", bool use_markup = false, const surface& fg = surface())
	: rect_(r), message(msg), action(act), markup(use_markup), foreground(fg)
	{}
	rect rect_;
	std::string message;
	std::string action;
	bool markup;
	surface foreground;
};

std::map<int, tooltip> tips;
std::map<int, tooltip>::const_iterator current_tooltip = tips.end();

int tooltip_handle = 0;
int tooltip_id = 0;

surface current_background = nullptr;

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
	CVideo& video = CVideo::get_singleton();

	if(video.faked()) {
		return;
	}

	clear_tooltip();

	const color_t bgcolor {0,0,0,192};
	SDL_Rect area = video.draw_area();

	unsigned int border = 10;

	font::floating_label flabel(tip.message, tip.foreground);
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
	if(tip.rect_.y > rect.h) {
		rect.y = tip.rect_.y - rect.h;
	} else {
		rect.y = tip.rect_.y + tip.rect_.h;
	}

	rect.x = tip.rect_.x;
	if(rect.x < 0) {
		rect.x = 0;
	} else if(rect.x + rect.w > area.w) {
		rect.x = area.w - rect.w;
	}

	font::move_floating_label(tooltip_handle,rect.x,rect.y);
}

namespace tooltips {

manager::manager()
{
	clear_tooltips();
}

manager::~manager()
{
	try {
	clear_tooltips();
	} catch (...) {}
}

void clear_tooltips()
{
	clear_tooltip();
	tips.clear();
	current_tooltip = tips.end();
}

void clear_tooltips(const SDL_Rect& r)
{
	for(std::map<int,tooltip>::iterator i = tips.begin(); i != tips.end(); ) {
		if(i->second.rect_.overlaps(r)) {
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



bool update_tooltip(int id, const SDL_Rect& rect, const std::string& message,
		const std::string& action, bool use_markup)
{
	std::map<int, tooltip>::iterator it = tips.find(id);
	if (it == tips.end() ) return false;
	it->second.action = action;
	it->second.markup = use_markup;
	it->second.message = message;
	it->second.rect_ = rect;
	return true;
}

bool update_tooltip(int id, const SDL_Rect& rect, const std::string& message,
		const std::string& action, bool use_markup, const surface& foreground)
{
	std::map<int, tooltip>::iterator it = tips.find(id);
	if (it == tips.end() ) return false;
	it->second.action = action;
	it->second.foreground = foreground;
	it->second.markup = use_markup;
	it->second.message = message;
	it->second.rect_ = rect;
	return true;
}

void remove_tooltip(int id)
{
	tips.erase(id);
	clear_tooltip();
}

int add_tooltip(const SDL_Rect& rect, const std::string& message, const std::string& action, bool use_markup, const surface& foreground)
{
	clear_tooltips(rect);

	int id = tooltip_id++;

	tips.emplace(id, tooltip(rect, message, action, use_markup, foreground));

	current_tooltip = tips.end();
	return id;
}

void process(int mousex, int mousey)
{
	for(std::map<int, tooltip>::const_iterator i = tips.begin(); i != tips.end(); ++i) {
		if(mousex > i->second.rect_.x && mousey > i->second.rect_.y &&
		   mousex < i->second.rect_.x + i->second.rect_.w && mousey < i->second.rect_.y + i->second.rect_.h) {
			if(current_tooltip != i) {
				show_tooltip(i->second);
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
	for(std::map<int, tooltip>::const_iterator i = tips.begin(); i != tips.end(); ++i) {
		if(!i->second.action.empty() && i->second.rect_.contains(mousex, mousey)) {
			help::show_help(i->second.action);
			return true;
		}
	}
	return false;
}

}
