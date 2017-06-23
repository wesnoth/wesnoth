/*
   Copyright (C) 2015 - 2017 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <boost/optional.hpp>
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
		, present_codepoints()
	{
	}

	explicit subset_descriptor(const config & font)
		: name(font["name"].str())
		, bold_name()
		, italic_name()
		, present_codepoints()
	{
		if (font.has_attribute("bold_name")) {
			bold_name = font["bold_name"].str();
		}

		if (font.has_attribute("italic_name")) {
			italic_name = font["italic_name"].str();
		}

		std::vector<std::string> ranges = utils::split(font["codepoints"]);

		for (const std::string & i : ranges) {
			std::vector<std::string> r = utils::split(i, '-');
			if(r.size() == 1) {
				size_t r1 = lexical_cast_default<size_t>(r[0], 0);
				present_codepoints.emplace_back(r1, r1);
			} else if(r.size() == 2) {
				size_t r1 = lexical_cast_default<size_t>(r[0], 0);
				size_t r2 = lexical_cast_default<size_t>(r[1], 0);

				present_codepoints.emplace_back(r1, r2);
			}
		}
	}

	std::string name;
	boost::optional<std::string> bold_name; //If we are using another font for styled characters in this font, rather than SDL TTF method
	boost::optional<std::string> italic_name;

	typedef std::pair<int, int> range;
	std::vector<range> present_codepoints;
};

} // end namespace font
