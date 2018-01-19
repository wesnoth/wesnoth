/*
   Copyright (C) 2015 - 2018 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include <string>

namespace font
{

/**
 * Font classes for get_font_families().
 */
enum family_class
{
	FONT_SANS_SERIF,
	FONT_MONOSPACE
};

inline family_class str_to_family_class(const std::string& str)
{
	if(str == "monospace") {
		return FONT_MONOSPACE;
	}

	return FONT_SANS_SERIF;
}

} // end namespace font
