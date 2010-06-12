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

#ifndef TOOLTIPS_HPP_INCLUDED
#define TOOLTIPS_HPP_INCLUDED

#include "SDL.h"

#include <string>

class CVideo;

namespace tooltips {

struct manager
{
	manager(CVideo& video);
	~manager();
};

void clear_tooltips();
void clear_tooltips(const SDL_Rect& rect);
void add_tooltip(const SDL_Rect& rect, const std::string& message, const std::string& action ="");
void process(int mousex, int mousey);

// Check if we clicked on a tooltip having an action.
// If it is, then execute the action and return true
// (only possible action are opening help page for the moment)
bool click(int mousex, int mousey);

}

#endif
