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

//object which initializes and destroys structures needed for fonts
struct manager {
	manager();
	~manager();
};

//function which, given a 1-based side, will return the colour of
//that side.
const SDL_Color& get_side_colour(int side);

//various standard colours
extern const SDL_Color NORMAL_COLOUR, GOOD_COLOUR, BAD_COLOUR, BLACK_COLOUR,
                       DARK_COLOUR, YELLOW_COLOUR, BUTTON_COLOUR;

enum MARKUP { USE_MARKUP, NO_MARKUP };

//function to draw text on the screen. The text will be clipped to area.
//If the text runs outside of area horizontally, an ellipsis will be displayed
//at the end of it. If use_tooltips is true, then text with an ellipsis will
//have a tooltip set for it equivalent to the entire contents of the text.
//
//if use_markup is equal to USE_MARKUP, then some very basic 'markup' will
//be done on the text:
// - any line beginning in # will be displayed in BAD_COLOUR
// - any line beginning in @ will be displayed in GOOD_COLOUR
// - any line beginning in + will be displayed with size increased by 2
// - any line beginning in - will be displayed with size decreased by 2
// - any line beginning with 0x0n will be displayed in the colour of side n
//
//a bounding rectangle of the text is returned. If gui is NULL, then the
//text will not be drawn, and a bounding rectangle only will be returned.
SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, SDL_Surface* bg=NULL,
                   bool use_tooltips=false, MARKUP use_markup=USE_MARKUP);

bool is_format_char(char c);

}

#endif
