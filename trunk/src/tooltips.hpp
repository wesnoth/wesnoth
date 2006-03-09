/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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
void add_tooltip(const SDL_Rect& rect, const std::string& message);
void process(int mousex, int mousey);


//a function exactly the same as font::draw_text, but will also register
//a tooltip
SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y);

}

#endif
