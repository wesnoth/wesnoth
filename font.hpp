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

enum COLOUR { NORMAL_COLOUR, GOOD_COLOUR, BAD_COLOUR };

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size, COLOUR colour,
                   const std::string& text, int x, int y, SDL_Surface* bg=NULL);

}

#endif
