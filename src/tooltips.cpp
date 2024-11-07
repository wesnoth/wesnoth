/*
	Copyright (C) 2003 - 2024
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
#define LOG_FT LOG_STREAM(info, log_font)

namespace {

static const int font_size = font::SIZE_SMALL;
static const int text_width = 400;
static const double height_fudge = 0.95;  // An artificial "border" to keep tip text from crowding lower edge of viewing area

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act = "");
	rect origin;
	rect loc = {};
	std::string message;
	std::string action;
	font::floating_label label;

	void init_label();
	void update_label_pos();
};

tooltip::tooltip(const SDL_Rect& r, const std::string& msg, const std::string& act)
	: origin(r), message(msg), action(act), label(msg)
{
	init_label();
	DBG_FT << "created tooltip for " << origin << " at " << loc;
}

void tooltip::init_label()
{
	const color_t bgcolor {0,0,0,192};
	rect game_canvas = video::game_canvas();
	unsigned int border = 10;

	rect huge;
	huge.h=1000000;
	huge.w=1000000;

	label.set_font_size(font_size);
	label.set_color(font::NORMAL_COLOR);
	label.set_clip_rect(huge);
	label.set_width(text_width);   // If tooltip will be too tall for game_canvas, this could be scaled up appropriately
	label.set_alignment(font::LEFT_ALIGN);
	label.set_bg_color(bgcolor);
	label.set_border_size(border);

	label.create_texture();

	point lsize = label.get_draw_size();
	int new_text_width = text_width * static_cast<float>(lsize.y)/game_canvas.h;  // If necessary, scale width to reduce height while preserving area of label
	while((lsize.y > game_canvas.h*height_fudge) && (lsize.x < game_canvas.w)) {
		// Scaling the tip to reduce height is hard, since making a texture wider is no guarantee that there will be fewer lines of text:
		//
		// This block of text is just
		// as tall as the other one.
		//
		// This block of text is just as tall as the other
		// one.
		//
		// Creating this over and over may not be the most efficient route, but it will work and will be quite rare (tip taller than screen).
		bool wont_fit = false;
		if(new_text_width>game_canvas.w) {
			new_text_width=game_canvas.w;
			wont_fit = true;
		}
		DBG_FT << "lsize.x,y = " << lsize.x << "," << lsize.y << ", new_text_width = " << new_text_width;

		label.set_width(new_text_width);
		label.clear_texture();
		label.create_texture();

		lsize = label.get_draw_size();
		DBG_FT << "new label lsize.x,y = " << lsize.x << "," << lsize.y;
		if(wont_fit) {
			break;
		}
		new_text_width *= 1.3;
	}
	// I don't know if it's strictly necessary to create the texture yet again just to make sure the clip_rect is set to game_canvas
	// but it seems like the safe course of action.
	label.set_clip_rect(game_canvas);
	label.clear_texture();
	label.create_texture();

	update_label_pos();
}

void tooltip::update_label_pos()
{
	rect game_canvas = video::game_canvas();

	point lsize = label.get_draw_size();
	loc = {0, 0, lsize.x, lsize.y};

	DBG_FT << "\nupdate_label_pos() Start: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;

	if(origin.y > loc.h) {
		// There is enough room to fit it above the tip area
		loc.y = origin.y - loc.h;
		DBG_FT << "\tAbove: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;
	} else if((origin.y + origin.h + loc.h) <= game_canvas.h*height_fudge) {
		// There is enough room to fit it below the tip area
		loc.y = origin.y + origin.h;
		DBG_FT << "\tBelow: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;
	} else if(((origin.y + origin.h/2 - loc.h/2) >= 0) &&
		  ((origin.y + origin.h/2 + loc.h/2) <= game_canvas.h*height_fudge)) {
		// There is enough room to center it at the tip area
		loc.y = origin.y + origin.h/2 - loc.h/2;
		DBG_FT << "\tCenter: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;
	} else if(loc.h <= game_canvas.h*0.95) {
		// There is enough room to center it
		loc.y = game_canvas.h/2 - loc.h/2;
		DBG_FT << "\tScreen Center: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;
	} else {
		// It doesn't fit
		loc.y = 0;
		DBG_FT << "\tToo big: loc = " << loc.x << "," << loc.y << " origin = " << origin.x << "," << origin.y;
	}

	DBG_FT << "\tBefore x adjust: loc.x,y,w,h = " << loc.x << "," << loc.y << "," << loc.w << "," << loc.h << "  origin = " << origin.x << "," << origin.y;
	// Try to keep it within the screen
	loc.x = origin.x;
	if(loc.x + loc.w > game_canvas.w) {
		loc.x = game_canvas.w - loc.w;
	}
	if(loc.x < 0) {
		loc.x = 0;
	}

	DBG_FT << "\tFinal: loc.x,y,w,h = " << loc.x << "," << loc.y << "," << loc.w << "," << loc.h << "  origin = " << origin.x << "," << origin.y;
	label.set_position(loc.x, loc.y);
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
	tips.at(active_tooltip).label.update(std::chrono::steady_clock::now());
}

bool manager::expose(const rect& region)
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
	LOG_FT << "clearing all tooltips";
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
	std::map<int, tooltip>::iterator it = tips.find(id);
	if (it == tips.end() ) return false;
	tooltip& tip = it->second;
	if(tip.message == message && tip.origin == origin) {
		return false;
	}
	if(tip.message != message) {
		LOG_FT << "updating tooltip " << id << " message";
		tip.message = message;
		tip.label = font::floating_label(message);
		tip.init_label();
	}
	if(tip.origin != origin) {
		DBG_FT << "updating tooltip " << id << " origin " << origin;
		tip.origin = origin;
		tip.update_label_pos();
	}
	return true;
}

void remove_tooltip(int id)
{
	if(!id) { return; }
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
	LOG_FT << "showing tip " << id << " for " << tip.origin;
	clear_active();
	active_tooltip = id;
	tip.label.update(std::chrono::steady_clock::now());
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
		LOG_FT << "clearing tooltip because none hovered";
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
