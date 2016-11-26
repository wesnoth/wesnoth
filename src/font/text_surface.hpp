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

#ifndef FONT_TEXT_SURFACE_HPP
#define FONT_TEXT_SURFACE_HPP

#include "font_id.hpp" // for text_chunk
#include "sdl/utils.hpp"

#include <SDL_ttf.h>

#include <string>
#include <vector>

/***
 * Note: This is specific to the SDL_TTF codepath.
 */

namespace font {

class text_surface
{
public:
	text_surface(const std::string& str, int size, SDL_Color color, int style);
	text_surface(int size, SDL_Color color, int style);
	void set_text(const std::string& str);

	void measure() const;
	size_t width() const;
	size_t height() const;
#ifdef	HAVE_FRIBIDI
	bool is_rtl() const { return is_rtl_; }	// Right-To-Left alignment
#endif
	std::vector<surface> const & get_surfaces() const;

	bool operator==(text_surface const &t) const {
		return hash_ == t.hash_ && font_size_ == t.font_size_
			&& color_ == t.color_ && style_ == t.style_ && str_ == t.str_;
	}
	bool operator!=(text_surface const &t) const { return !operator==(t); }
private:
	int hash_;
	int font_size_;
	SDL_Color color_;
	int style_;
	mutable int w_, h_;
	std::string str_;
	mutable bool initialized_;
	mutable std::vector<text_chunk> chunks_;
	mutable std::vector<surface> surfs_;
#ifdef	HAVE_FRIBIDI
	bool is_rtl_;
	void bidi_cvt();
#endif
	void hash();
};

} // end namespace font

#endif
