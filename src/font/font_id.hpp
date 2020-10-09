/*
   Copyright (C) 2016 - 2018 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/***
 * Note: Specific to SDL_TTF code path
 */

#pragma once

#include <string>
#include <tuple>

#include <SDL2/SDL_ttf.h>

/***
 * Note: This is specific to SDL_TTF code path
 */

namespace font
{
/**
 * Font family, acts an an enumeration with each font loaded by stl_ttf::set_font_list getting an
 * individual value. The values do not necessarily correspond to the order of the list passed to
 * stl_ttf::set_font_list, all positive values should be treated as opaque data.
 *
 * Negative values are returned by sdl_ttf::split_text to denote chunks which can't be handled with
 * the available fonts.
 */
typedef int subset_id;

// Used as a key in requests to the functions in sdl_text.hpp (and the font table in sdl_text.cpp's implementation)
struct font_id
{
	explicit font_id(subset_id subset, int size) : subset(subset), size(size), style(TTF_STYLE_NORMAL) {}
	explicit font_id(subset_id subset, int size, int style) : subset(subset), size(size), style(style) {}

	bool operator==(const font_id& o) const
	{
		return subset == o.subset && size == o.size && style == o.style;
	}
	bool operator<(const font_id& o) const
	{
		return std::tie(subset, size, style) < std::tie(o.subset, o.size, o.style);
	}

	subset_id subset;
	int size;
	/**
	 * Bitmask of the values TTF_STYLE_BOLD, TTF_STYLE_ITALIC.
	 */
	int style;
};

/**
 * A string that should be rendered with a single font. Longer texts that need
 * characters from multiple fonts are cut into these sub-strings.
 *
 * Text chunk is used by text_surfaces and these are cached sometimes.
 */
struct text_chunk
{
	text_chunk(subset_id subset, std::string&& text)
		: subset(subset)
		, text(std::move(text))
	{
	}

	bool operator==(const text_chunk& t) const { return subset == t.subset && text == t.text; }
	bool operator!=(const text_chunk& t) const { return !operator==(t); }

	subset_id subset;
	std::string text;
};

} // end namespace font
