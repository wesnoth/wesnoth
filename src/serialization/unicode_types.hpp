/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
#include <vector>
#include <exception>
#include <cstdint>

namespace ucs4 {
	typedef uint32_t char_t;
	typedef std::vector<char_t> string;
}

namespace utf8 {
	typedef char char_t;
	typedef std::string string;

	/**
	 * Thrown by operations encountering invalid UTF-8 data.
	 *
	 * Also used for invalid UTF-16 and UCS-4 data.
	 *
	 * @todo FIXME: This clearly needs a better name for that reason.
	 */
	class invalid_utf8_exception : public std::exception {};
}

/**
 * For Win32 API.
 *
 * On windows, wchar_t is defined as uint16_t.
 * Wide strings are expected to be UTF-16.
 */
namespace utf16 {
	typedef wchar_t char_t;
	typedef std::vector<char_t> string;
}
