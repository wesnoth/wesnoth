/*
   Copyright (C) 2016 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FONT_SDL_TTF_HPP
#define FONT_SDL_TTF_HPP

#include "font_id.hpp"
#include "font_description.hpp"

#include <string>

#include <SDL_ttf.h>

class surface;

namespace font {

//various standard colors
extern const SDL_Color NORMAL_COLOR, GRAY_COLOR, LOBBY_COLOR, GOOD_COLOR, BAD_COLOR,
                       BLACK_COLOR, YELLOW_COLOR, BUTTON_COLOR, BIGMAP_COLOR,
                       PETRIFIED_COLOR, TITLE_COLOR, DISABLED_COLOR, LABEL_COLOR;

// font sizes, to be made theme parameters
constexpr int SIZE_NORMAL = 14;
// automatic computation of other font sizes, to be made a default for theme-provided values
constexpr int
	SIZE_TINY       = 10 * SIZE_NORMAL / 14,
	SIZE_SMALL      = 12 * SIZE_NORMAL / 14,

	SIZE_15         = 15 * SIZE_NORMAL / 14,
	SIZE_PLUS       = 16 * SIZE_NORMAL / 14,
	SIZE_LARGE      = 18 * SIZE_NORMAL / 14,
	SIZE_TITLE      = 20 * SIZE_NORMAL / 14,
	SIZE_XLARGE     = 24 * SIZE_NORMAL / 14
  ;
// For arbitrary scaling:
// (Not used in defining the SIZE_* consts because of spurious compiler warnings.)
inline int relative_size(int size)
{
	return (SIZE_NORMAL * size / 14);
}

// Returns a SDL surface containing the text rendered in a given color.
surface get_rendered_text(const std::string& text, int size, const SDL_Color& color, int style=0);

SDL_Rect draw_text_line(surface& gui_surface, const SDL_Rect& area, int size,
						const SDL_Color& color, const std::string& text,
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

/**
 * If the text exceeds the specified max width, end it with an ellipsis (...)
 */
std::string make_text_ellipsis(const std::string& text, int font_size, int max_width,
	int style = TTF_STYLE_NORMAL);


enum CACHE { CACHE_LOBBY, CACHE_GAME };
void cache_mode(CACHE mode);




/***
 * RAII object which initializes and destroys SDL_TTF, and manages caches of open fonts
 */
struct sdl_ttf {
  sdl_ttf();
  ~sdl_ttf();

  sdl_ttf(const sdl_ttf &) = delete;
  sdl_ttf & operator = (const sdl_ttf &) = delete;

  // Load a font
  static TTF_Font * get_font(font_id);

  // Set the list of fonts
  static void set_font_list(const std::vector<subset_descriptor>& fontlist);

  // Split a utf8 string into text_chunks
  static std::vector<text_chunk> split_text(const std::string & utf8_text);

};


} // end namespace font

#endif
