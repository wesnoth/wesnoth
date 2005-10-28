/* $Id: font.hpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MARKED_UP_TEXT_HPP_INCLUDED
#define MARKED_UP_TEXT_HPP_INCLUDED

struct CVideo;

#include <SDL_video.h>
#include <string>

namespace font {

//standard markups
extern const char LARGE_TEXT, SMALL_TEXT, GOOD_TEXT, BAD_TEXT, NORMAL_TEXT, BLACK_TEXT, BOLD_TEXT, IMAGE, NULL_MARKUP;


//function to draw text on the screen. The text will be clipped to area.
//If the text runs outside of area horizontally, an ellipsis will be displayed
//at the end of it. If use_tooltips is true, then text with an ellipsis will
//have a tooltip set for it equivalent to the entire contents of the text.
//
//some very basic 'markup' will be done on the text:
// - any line beginning in # will be displayed in BAD_COLOUR
// - any line beginning in @ will be displayed in GOOD_COLOUR
// - any line beginning in + will be displayed with size increased by 2
// - any line beginning in - will be displayed with size decreased by 2
// - any line beginning with 0x0n will be displayed in the colour of side n
//
//the above special characters can be quoted using a C-style backslash.
//
//a bounding rectangle of the text is returned. If gui is NULL, then the
//text will not be drawn, and a bounding rectangle only will be returned.

SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, bool use_tooltips = false, int style = 0);

//function which returns the size of text if it were to be drawn.
SDL_Rect text_area(const std::string& text, int size, int style=0);

bool is_format_char(char c);

///
/// If the text exceedes the specified max width, wrap it one a word basis.
/// If the is not possible, e.g. the word is too big to fit, wrap it on a
/// char basis.
///
std::string word_wrap_text(const std::string& unwrapped_text, int font_size, int max_width, int max_height=-1, int max_lines=-1);

///
/// Draw text on the screen. This method makes sure that the text
/// fits within a given maximum width. If a line exceedes this width it
/// will be wrapped on a word basis if possible, otherwise on a char
/// basis. This method is otherwise similar to the draw_text method,
/// but it doesn't support special markup or tooltips.
///
/// @return a bounding rectangle of the text.
///
SDL_Rect draw_wrapped_text(CVideo* gui, const SDL_Rect& area, int font_size,
			     const SDL_Color& colour, const std::string& text,
			     int x, int y, int max_width);

//function to chop up one long string of text into lines
size_t text_to_lines(std::string& text, size_t max_length);

}

#endif // MARKED_UP_TEXT_HPP_INCLUDED
