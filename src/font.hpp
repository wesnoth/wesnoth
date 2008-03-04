/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FONT_HPP_INCLUDED
#define FONT_HPP_INCLUDED

#include "SDL.h"
#include "SDL_ttf.h"

class CVideo;
#include "sdl_utils.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

namespace font {

//object which initializes and destroys structures needed for fonts
struct manager {
	manager();
	~manager();
	struct error {};
};

//various standard colours
extern const SDL_Color NORMAL_COLOUR, GRAY_COLOUR, LOBBY_COLOUR, GOOD_COLOUR, BAD_COLOUR,
                       BLACK_COLOUR, YELLOW_COLOUR, BUTTON_COLOUR, BIGMAP_COLOUR,
                       STONED_COLOUR, TITLE_COLOUR, DISABLED_COLOUR, LABEL_COLOUR;

// font sizes, to be made theme parameters
#ifdef USE_TINY_GUI
// this is not meant for normal play, just for checking other dimensions get adapted accordingly
const int SIZE_NORMAL = 9;
#else
const int SIZE_NORMAL = 14;
#endif
inline int relative_size(int size)
{
	return (SIZE_NORMAL * size / 14);
}

// automatic computation of other font sizes, to be made a default for theme-provided values
#ifdef USE_TINY_GUI
	const int
	SIZE_TINY	= 8,
	SIZE_SMALL	= 8,
	SIZE_15		= 9,
	SIZE_PLUS	= 9,
	SIZE_LARGE	= 10,
	SIZE_XLARGE	= 10
  ;
#else
const int
	SIZE_TINY       = relative_size(10),
	SIZE_SMALL      = relative_size(12),

	SIZE_15         = relative_size(15),
	SIZE_PLUS       = relative_size(16),
	SIZE_LARGE      = relative_size(18),
	SIZE_XLARGE     = relative_size(24)
  ;
#endif

// Returns a SDL surface containing the text rendered in a given colour.
surface get_rendered_text(const std::string& text, int size, const SDL_Color& colour, int style=0);

SDL_Rect draw_text_line(surface gui_surface, const SDL_Rect& area, int size,
						const SDL_Color& colour, const std::string& text,
						int x, int y, bool use_tooltips, int style);

SDL_Rect draw_text_line(CVideo* gui, const SDL_Rect& area, int size,
                        const SDL_Color& colour, const std::string& text,
                        int x, int y, bool use_tooltips, int style);

// Returns the maximum height of a font, in pixels
int get_max_height(int size);

///
/// Determine the width of a line of text given a certain font size.
/// The font type used is the default wesnoth font type.
///
int line_width(const std::string& line, int font_size, int style=TTF_STYLE_NORMAL);

///
/// Determine the size of a line of text given a certain font size. Similar to
/// line_width, but for both coordinates.
///
SDL_Rect line_size(const std::string& line, int font_size, int style=TTF_STYLE_NORMAL);

///
/// If the text excedes the specified max width, end it with an ellipsis (...)
/// The with_tags can probably always be set to false
///
std::string make_text_ellipsis(const std::string& text, int font_size, int max_width, bool with_tags = true);


/// structure which will hide all current floating labels, and cause floating labels
/// instantiated after it is created to be displayed
struct floating_label_context
{
	floating_label_context();
	~floating_label_context();
};

enum ALIGN { LEFT_ALIGN, CENTER_ALIGN, RIGHT_ALIGN };

enum LABEL_SCROLL_MODE { ANCHOR_LABEL_SCREEN, ANCHOR_LABEL_MAP };

/// add a label floating on the screen above everything else.
/// 'text': the text to display
/// 'font_size': the size to display the text in
/// 'colour': the colour of the text
/// 'xpos,ypos': the location on the screen to display the text.
/// 'xmove,ymove': the amount to move the text each frame
/// 'lifetime': the number of frames to display the text for, or -1 to display until removed
/// 'clip_rect': the rectangle to clip the label to.
///
/// @returns a handle to the label which can be used with other label functions
int add_floating_label(const std::string& text, int font_size, const SDL_Color& colour,
		double xpos, double ypos, double xmove, double ymove, int lifetime,
		const SDL_Rect& clip_rect, ALIGN alignment=CENTER_ALIGN,
		const SDL_Color* bg_colour=NULL, int border_size=0,
		LABEL_SCROLL_MODE scroll_mode=ANCHOR_LABEL_SCREEN);

/// moves the floating label given by 'handle' by (xmove,ymove)
void move_floating_label(int handle, double xmove, double ymove);

/// moves all floating labels that have 'scroll_mode' set to ANCHOR_LABEL_MAP
void scroll_floating_labels(double xmove, double ymove);

/// removes the floating label given by 'handle' from the screen
void remove_floating_label(int handle);

/// hides or shows a floating label
void show_floating_label(int handle, bool show);

SDL_Rect get_floating_label_rect(int handle);

void draw_floating_labels(surface screen);
void undraw_floating_labels(surface screen);

bool load_font_config();

enum CACHE { CACHE_LOBBY, CACHE_GAME };
void cache_mode(CACHE mode);

}

#endif
