/*
   Copyright (C) 2013 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CAMPAIGN_SERVER_ADDON_UTILS_HPP_INCLUDED
#define CAMPAIGN_SERVER_ADDON_UTILS_HPP_INCLUDED

#include <string>

class config;

namespace campaignd {

/**
 * Markup characters recognized by GUI1 code.
 *
 * These must be the same as the constants defined in marked-up_text.cpp.
 */
extern const std::string illegal_markup_chars;

inline bool is_text_markup_char(char c)
{
	return illegal_markup_chars.find(c) != std::string::npos;
}

/**
 * Format a feedback URL for an add-on.
 *
 * @param format        The format string for the URL, presumably obtained
 *                      from the add-ons server identification.
 *
 * @param params        The URL format parameters table.
 *
 * @return A string containing a feedback URL or an empty string if that
 *         is not possible (e.g. empty or invalid @a format, empty
 *         @a params table, or a result that is identical in content to
 *         the @a format suggesting that the @a params table contains
 *         incorrect data).
 */
std::string format_addon_feedback_url(const std::string& format, const config& params);

}

#endif
