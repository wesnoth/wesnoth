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

#include "draw_manager.hpp"
#include "floating_label.hpp"
#include "font/standard_colors.hpp"
#include "game_display.hpp"
#include "help/help.hpp"
#include "log.hpp"
#include "video.hpp"

#include <SDL2/SDL_rect.h>

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)

namespace {

static const int font_size = font::SIZE_SMALL;
static const int text_width = 400;

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act = "");
	rect origin;
	rect loc = {};
	std::string message;
	std::string action;
	font::floating_label label;
};

tooltip::tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act)
	: origin(r), message(msg), action(act), label(msg)
{
	const color_t bgcolor {0,0,0,192};
	rect game_canvas = video::game_canvas();
	unsigned int border = 10;

	label.set_font_size(font_size);
	label.set_color(font::NORMAL_COLOR);
	label.set_clip_rect(game_canvas);
	label.set_width(text_width);
	label.set_alignment(font::LEFT_ALIGN);
	label.set_bg_color(bgcolor);
	label.set_border_size(border);

	label.create_texture();
	point lsize = label.get_draw_size();
	loc = {0, 0, lsize.x, lsize.y};

	// See if there is enough room to fit it above the tip area
	if(origin.y > loc.h) {
		loc.y = origin.y - loc.h;
	} else {
		loc.y = origin.y + origin.h;
	}

	// Try to keep it within the screen
	loc.x = origin.x;
	if(loc.x < 0) {
		loc.x = 0;
	} else if(loc.x + loc.w > game_canvas.w) {
		loc.x = game_canvas.w - loc.w;
	}

	label.move(loc.x, loc.y);

	DBG_FT << "created tooltip for " << origin << " at " << loc;
}


std::map<int, tooltip> tips;
int active_tooltip = 0;

int tooltip_id = 1;

surface current_background = nullptr;

// Is this a freaking singleton or is it not?
// This is horrible, but that's how the usage elsewhere is.
// If you want to fix this, either make it an actual singleton,
// or ensure that tooltips:: functions are called on an instance.
tooltips::manager* current_manager = nullptr;

} // anon namespace

/** Clear/hide the active tooltip. */
static void clear_active()
{
	if(!active_tooltip) {
		return;
	}
	DBG_FT << "clearing active tooltip " << active_tooltip;
	tips.at(active_tooltip).label.undraw();
	active_tooltip = 0;
}

namespace tooltips
{

manager::manager()
{
	clear_tooltips();
	current_manager = this;
}

manager::~manager()
{
	try {
	clear_tooltips();
	} catch (...) {}
	current_manager = nullptr;
}

void manager::layout()
{
	if(!active_tooltip) {
		return;
	}
	// Update the active tooltip's draw state.
	// This will trigger redraws if necessary.
	tips.at(active_tooltip).label.update(SDL_GetTicks());
}

bool manager::expose(const SDL_Rect& region)
{
	// Only the active tip is shown.
	if(!active_tooltip) {
		return false;
	}
	tooltip& tip = tips.at(active_tooltip);
	if(!tip.loc.overlaps(region)) {
		return false;
	}
	tip.label.draw();
	return true;
}

rect manager::screen_location()
{
	// Only the active tip, if any, should be visible.
	if(!active_tooltip) {
		return {};
	} else {
		return tips.at(active_tooltip).loc;
	}
}

void clear_tooltips()
{
	DBG_FT << "clearing all tooltips";
	clear_active();
	tips.clear();
}

void clear_tooltips(const SDL_Rect& r)
{
	for(auto i = tips.begin(); i != tips.end(); ) {
		if(i->second.origin.overlaps(r)) {
			DBG_FT << "clearing tip " << i->first << " at "
				<< i->second.origin << " overlapping " << r;

			if (i->first == active_tooltip) {
				i->second.label.undraw();
				active_tooltip = 0;
			}

			i = tips.erase(i);
		} else {
			++i;
		}
	}
}

bool update_tooltip(int id, const SDL_Rect& origin, const std::string& message)
{
	// TODO: draw_manager - update floating label
	std::map<int, tooltip>::iterator it = tips.find(id);
	if (it == tips.end() ) return false;
	it->second.message = message;
	it->second.origin = origin;
	return true;
}

void remove_tooltip(int id)
{
	DBG_FT << "removing tooltip " << id;
	if(id == active_tooltip) {
		clear_active();
	}
	tips.erase(id);
}

int add_tooltip(const SDL_Rect& origin, const std::string& message, const std::string& action)
{
	// Because some other things are braindead, we have to check we're not
	// just adding the same tooltip over and over every time the mouse moves.
	for(auto& [id, tip] : tips) {
		if(tip.origin == origin && tip.message == message && tip.action == action) {
			return id;
		}
	}
	DBG_FT << "adding tooltip for " << origin;

	// Clear any existing tooltips for this origin
	clear_tooltips(origin);
	// Create and add a new tooltip
	int id = tooltip_id++;
	tips.emplace(id, tooltip(origin, message, action));
	return id;
}

static void raise_to_top()
{
	// Raise the current manager so it will display on top of everything.
	if(!current_manager) {
		throw game::error("trying to show tooltip with no tooltip manager");
	}
	draw_manager::raise_drawable(current_manager);
}

static void select_active(int id)
{
	if(active_tooltip == id) {
		return;
	}
	tooltip& tip = tips.at(id);
	DBG_FT << "showing tip " << id << " for " << tip.origin;
	clear_active();
	active_tooltip = id;
	tip.label.update(SDL_GetTicks());
	raise_to_top();
}

void process(int mousex, int mousey)
{
	point mouseloc{mousex, mousey};
	for(auto& [id, tip] : tips) {
		if(tip.origin.contains(mouseloc)) {
			select_active(id);
			return;
		}
	}

	if(active_tooltip) {
		DBG_FT << "clearing tooltip because none hovered";
		clear_active();
	}
}

bool click(int mousex, int mousey)
{
	for(auto& [id, tip] : tips) { (void)id;
		if(!tip.action.empty() && tip.origin.contains(mousex, mousey)) {
			help::show_help(tip.action);
			return true;
		}
	}
	return false;
}

} // namespace tooltips
