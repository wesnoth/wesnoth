/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

namespace font {

// Helper functions for link-aware text feature

inline bool looks_like_url(const std::string & str)
{
	return (str.size() >= 8) && ((str.substr(0,7) == "http://") || (str.substr(0,8) == "https://"));
}

inline std::string format_as_link(const std::string & link, const std::string & color) {
	return "<span underline=\'single\' color=\'" + color + "\'>" + link + "</span>";
}

} // end namespace font
