/*
	Copyright (C) 2015 - 2024
	by Chris Beck<render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"
#include <optional>

#include <string>
#include <utility>
#include <vector>

namespace font {

// structure used to describe a font, and the subset of the Unicode character
// set it covers.
//
// used by font_config interface (not specific to sdl_ttf or pango)
struct subset_descriptor
{
	subset_descriptor()
		: name()
		, bold_name()
		, italic_name()
	{
	}

	explicit subset_descriptor(const config & font)
		: name(font["name"].str())
		, bold_name()
		, italic_name()
	{
		if (font.has_attribute("bold_name")) {
			bold_name = font["bold_name"].str();
		}

		if (font.has_attribute("italic_name")) {
			italic_name = font["italic_name"].str();
		}
	}

	std::string name;
	std::optional<std::string> bold_name; //If we are using another font for styled characters in this font, rather than SDL TTF method
	std::optional<std::string> italic_name;
};

} // end namespace font
