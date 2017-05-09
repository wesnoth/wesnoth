/*
   Copyright (C) 2016 - 2017 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include <SDL_ttf.h>

/***
 * Note: This is specific to SDL_TTF code path
 */

namespace font {

// Signed int. Negative values mean "no subset".
typedef int subset_id;

// Used as a key in the font table, which caches the get_font results.
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
		return subset < o.subset || (subset == o.subset && size < o.size) || (subset == o.subset && size == o.size && style < o.style);
	}

	subset_id subset;
	int size;
	int style;
};

/***
 * Text chunk is used by text_surfaces and these are cached sometimes.
 */
struct text_chunk
{
	text_chunk(subset_id subset)
		: subset(subset)
		, text()
	{
	}

	bool operator==(text_chunk const & t) const { return subset == t.subset && text == t.text; }
	bool operator!=(text_chunk const & t) const { return !operator==(t); }

	subset_id subset;
	std::string text;
};

} // end namespace font
