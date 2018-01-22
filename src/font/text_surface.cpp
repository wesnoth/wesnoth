/*
   Copyright (C) 2016 - 2018 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/text_surface.hpp"

#include "font/sdl_ttf.hpp"

#include "sdl/surface.hpp"

#include "log.hpp"

#include <SDL_ttf.h>

#include <string>
#include <vector>

#ifdef	HAVE_FRIBIDI
#include <fribidi.h>
#endif

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font {

#ifdef	HAVE_FRIBIDI
void text_surface::bidi_cvt()
{
	char		*c_str = const_cast<char *>(str_.c_str());	// fribidi forgot const...
	FriBidiStrIndex	len = str_.length();
	FriBidiChar	*bidi_logical = new FriBidiChar[len + 2];
	FriBidiChar	*bidi_visual = new FriBidiChar[len + 2];
	char		*utf8str = new char[4*len + 1];	//assume worst case here (all 4 Byte characters)
	FriBidiCharType	base_dir = FRIBIDI_TYPE_ON;
	FriBidiStrIndex n;


	n = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, c_str, len, bidi_logical);
	#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-result"
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	#endif
	fribidi_log2vis(bidi_logical, n, &base_dir, bidi_visual, nullptr, nullptr, nullptr);
	#ifdef __GNUC__
	#pragma GCC diagnostic pop
	#endif

	fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, bidi_visual, n, utf8str);
	is_rtl_ = base_dir == FRIBIDI_TYPE_RTL;
	str_ = std::string(utf8str);
	delete[] bidi_logical;
	delete[] bidi_visual;
	delete[] utf8str;
}
#endif

text_surface::text_surface(const std::string& str, int size,
		color_t color, int style)
	: hash_(0)
	, font_size_(size)
	, color_(color)
	, style_(style)
	, w_(-1)
	, h_(-1)
	, str_(str)
	, initialized_(false)
	, chunks_()
	, surfs_()
#ifdef	HAVE_FRIBIDI
	, is_rtl_(false)
#endif
{
#ifdef	HAVE_FRIBIDI
	bidi_cvt();
#endif
	hash();
}

text_surface::text_surface(int size, color_t color, int style) :
	hash_(0),
	font_size_(size),
	color_(color),
	style_(style),
	w_(-1),
	h_(-1),
	str_(),
	initialized_(false),
	chunks_(),
	surfs_()
#ifdef	HAVE_FRIBIDI
	,is_rtl_(false)
#endif
{
}

void text_surface::set_text(const std::string& str)
{
	initialized_ = false;
	w_ = -1;
	h_ = -1;
	str_ = str;
#ifdef	HAVE_FRIBIDI
	bidi_cvt();
#endif
	hash();
}

void text_surface::hash()
{
	unsigned int h = 0;
	for(const char c : str_) {
		h = ((h << 9) | (h >> (sizeof(int) * 8 - 9))) ^ (c);
    }
	hash_ = h;
}

void text_surface::measure() const
{
	w_ = 0;
	h_ = 0;

	for(const text_chunk& chunk : chunks_)
	{
		TTF_Font* ttfont = sdl_ttf::get_font(font_id(chunk.subset, font_size_, style_));
		if(ttfont == nullptr) {
			continue;
		}

		int w, h;
		TTF_SizeUTF8(ttfont, chunk.text.c_str(), &w, &h);
		w_ += w;
		h_ = std::max<int>(h_, h);
	}
}

size_t text_surface::width() const
{
	if (w_ == -1) {
		if(chunks_.empty())
			chunks_ = sdl_ttf::split_text(str_);
		measure();
	}
	return w_;
}

size_t text_surface::height() const
{
	if (h_ == -1) {
		if(chunks_.empty())
			chunks_ = sdl_ttf::split_text(str_);
		measure();
	}
	return h_;
}

const std::vector<surface>& text_surface::get_surfaces() const
{
	if(initialized_)
		return surfs_;

	initialized_ = true;

	// Impose a maximal number of characters for a text line. Do now draw
	// any text longer that that, to prevent a SDL buffer overflow
	if(width() > max_text_line_width)
		return surfs_;

	for(const text_chunk& chunk : chunks_)
	{
		TTF_Font* ttfont = sdl_ttf::get_font(font_id(chunk.subset, font_size_, style_));

		surface s = surface(TTF_RenderUTF8_Blended(ttfont, chunk.text.c_str(), color_.to_sdl()));
		if(!s.null())
			surfs_.push_back(s);
	}

	return surfs_;
}

bool text_surface::operator==(const text_surface& t) const {
	return hash_ == t.hash_ && font_size_ == t.font_size_
		&& color_ == t.color_ && style_ == t.style_ && str_ == t.str_;
}

} // end namespace font
