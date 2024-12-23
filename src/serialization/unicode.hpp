/*
	Copyright (C) 2003 - 2024
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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

#include "ucs4_iterator_base.hpp"
#include "ucs4_convert_impl.hpp"
#include "unicode_cast.hpp"

#include <string>

/**
 * For Win32 API.
 *
 * On Windows, wchar_t is defined as uint16_t.
 * Wide strings are expected to be UTF-16.
 */
namespace utf16 {
	typedef ucs4::iterator_base<std::u16string, ucs4_convert_impl::convert_impl<char16_t>::type> iterator;
}

/**
 * Functions for converting Unicode wide-char strings to UTF-8 encoded strings,
 * back and forth.
 */
namespace utf8 {
	typedef ucs4::iterator_base<std::string_view, ucs4_convert_impl::convert_impl<char>::type> iterator;

	/** Returns a lowercased version of the string. */
	std::string lowercase(std::string_view s);

	/**
	 * Codepoint index corresponding to the nth character in a UTF-8 string.
	 *
	 * @return str.length() if there are less than @p index characters.
	 */
	std::size_t index(std::string_view str, const std::size_t index);

	/** Length in characters of a UTF-8 string. */
	std::size_t size(std::string_view str);

	/** Insert a UTF-8 string at the specified position. */
	std::string& insert(std::string& str, const std::size_t pos, const std::string& insert) ;

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
	std::string& erase(std::string& str, const std::size_t start, const std::size_t len = std::string::npos);

	/**
	 * Truncates a UTF-8 string to the specified number of characters.
	 *
	 * @param str   UTF-8 encoded string.
	 * @param size  Size to truncate to.
	 *
	 * @note This implementation does not check for valid UTF-8. Don't use it
	 *       for user input.
	 */
	std::string& truncate(std::string& str, const std::size_t size);

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
	void truncate_as_ucs4(std::string& str, const std::size_t size);
} // end namespace utf8
