/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include <sstream>

namespace font
{
/**
 * Escapes the pango markup characters in a text.
 *
 * The markups escaped are the ones used in the pango markup. The special
 * characters are: @verbatim <>'"& @endverbatim
 * The escaping is the same as for HTML.
 *
 * @param text                    The text to escape.
 *
 * @returns                       The escaped text.
 */
inline std::string escape_text(std::string_view text)
{
	std::ostringstream ss;
	for(const char c : text) {
		switch(c) {
		case '&':  ss << "&amp;";  break;
		case '<':  ss << "&lt;";   break;
		case '>':  ss << "&gt;";   break;
		case '\'': ss << "&apos;"; break;
		case '"':  ss << "&quot;"; break;
		default:   ss << c;
		}
	}

	return ss.str();
}

// Escape only the ampersands. This is used by pango_text to try to recover from
// markup parsing failure.
inline std::string semi_escape_text(std::string_view text)
{
	std::ostringstream ss;
	for(const char c : text) {
		if(c == '&') {
			ss << "&amp;";
		} else {
			ss << c;
		}
	}

	return ss.str();
}

} // end namespace font
