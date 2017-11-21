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

#pragma once

#include <string>
#include "sdl/surface.hpp"

struct SDL_Rect;

namespace tooltips {

struct manager
{
	manager();
	~manager();
};

void clear_tooltips();
void clear_tooltips(const SDL_Rect& rect);
int  add_tooltip(const SDL_Rect& rect, const std::string& message, const std::string& action ="", bool use_markup = true, const surface& foreground = surface(nullptr));
bool update_tooltip(int id, const SDL_Rect& rect, const std::string& message,
		const std::string& action, bool use_markup, const surface& foreground);
bool update_tooltip(int id, const SDL_Rect& rect, const std::string& message,
		const std::string& action, bool use_markup);
void remove_tooltip(int id);
void process(int mousex, int mousey);

// Check if we clicked on a tooltip having an action.
// If it is, then execute the action and return true
// (only possible action are opening help page for the moment)
bool click(int mousex, int mousey);

}
