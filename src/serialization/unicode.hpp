/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "ucs4_iterator_base.hpp"
#include "unicode_types.hpp"
#include "ucs4_convert_impl.hpp"
#include "unicode_cast.hpp"

#include <string>
#include <vector>

/**
 * For Win32 API.
 *
 * On Windows, wchar_t is defined as Uint16.
 * Wide strings are expected to be UTF-16.
 */
namespace utf16 {
	typedef ucs4::iterator_base<utf16::string, ucs4_convert_impl::convert_impl<char_t>::type> iterator;
}

/**
 * Functions for converting Unicode wide-char strings to UTF-8 encoded strings,
 * back and forth.
 */
namespace utf8 {
	typedef ucs4::iterator_base<std::string, ucs4_convert_impl::convert_impl<char_t>::type> iterator;

	/** Returns a lowercased version of the string. */
	utf8::string lowercase(const utf8::string& s);

	/**
	 * Codepoint index corresponding to the nth character in a UTF-8 string.
	 *
	 * @return str.length() if there are less than @p index characters.
	 */
	size_t index(const utf8::string& str, const size_t index);

	/** Length in characters of a UTF-8 string. */
	size_t size(const utf8::string& str);

	/** Insert a UTF-8 string at the specified position. */
	utf8::string& insert(utf8::string& str, const size_t pos, const utf8::string& insert);

	/**
	 * Erases a portion of a UTF-8 string.
	 *
	 * @param str    UTF-8 encoded string.
	 * @param start  Start position.
	 * @param len    Number of characters to erase.
	 *
	 * @note This implementation does not check for valid UTF-8. Don't use it
	 *       for user input.
	 */
	utf8::string& erase(utf8::string& str, const size_t start, const size_t len = std::string::npos);

	/**
	 * Truncates a UTF-8 string to the specified number of characters.
	 *
	 * @param str   UTF-8 encoded string.
	 * @param size  Size to truncate to.
	 *
	 * @note This implementation does not check for valid UTF-8. Don't use it
	 *       for user input.
	 */
	utf8::string& truncate(utf8::string& str, const size_t size);

	/**
	 * Truncates a UTF-8 string to the specified number of characters.
	 *
	 * If the string has more than @p size UTF-8 characters it will be
	 * truncated to this size.
	 *
	 * The output is guaranteed to be valid UTF-8.
	 *
	 * @param[in,out]  str   [in]  String encoded in UTF-8.
	 *                       [out] String encoded UTF-8 that contains at most @p size
	 *                             codepoints.
	 * @param      size  The size to truncate to.
	 */
	void truncate_as_ucs4(utf8::string& str, const size_t size);
} // end namespace utf8
