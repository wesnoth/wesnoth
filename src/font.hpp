/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FONT_HPP_INCLUDED
#define FONT_HPP_INCLUDED

#include "SDL.h"

#include "display.hpp"
#include "video.hpp"

#include <string>

namespace font {

struct manager {
	manager();
	~manager();
};

const SDL_Color& get_side_colour(int side);

extern const SDL_Color NORMAL_COLOUR, GOOD_COLOUR, BAD_COLOUR, BLACK_COLOUR,
                       BUTTON_COLOUR;

enum MARKUP { USE_MARKUP, NO_MARKUP };

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, SDL_Surface* bg=NULL,
                   bool use_tooltips=false, MARKUP use_markup=USE_MARKUP);

}

#endif
