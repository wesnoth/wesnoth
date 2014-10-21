/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERIALIZATION_UNICODE_HPP_INCLUDED
#define SERIALIZATION_UNICODE_HPP_INCLUDED

#include "ucs4_iterator_base.hpp"
#include "unicode_types.hpp"
#include "unicode_cast.hpp"

#include <boost/static_assert.hpp>
#include <string>
#include <vector>
#include <SDL_types.h>

namespace ucs4 {
	typedef Uint32 char_t;
	typedef std::vector<char_t> string;
}
namespace utf8 {
	typedef std::string string;
}
/**
 * For win32 API.
 * On windows, wchar_t is defined as Uint16
 * Wide strings are expected to be UTF-16
 */
namespace utf16 {
	typedef std::vector<wchar_t> string;
	
	struct iterator_implementation
	{
		static ucs4::char_t get_next_char(utf16::string::const_iterator& start, const utf16::string::const_iterator& end);
	};

	typedef ucs4::iterator_base<utf16::string, iterator_implementation> iterator;
}

namespace utf8 {

	/**
	* Functions for converting Unicode wide-char strings to UTF-8 encoded strings,
	* back and forth.
	*/
	struct iterator_implementation
	{
		static ucs4::char_t get_next_char(std::string::const_iterator& start, const std::string::const_iterator& end);
	};

	typedef ucs4::iterator_base<std::string, iterator_implementation> iterator;

	/** Returns a lowercased version of the string. */
	utf8::string lowercase(const utf8::string&);

	/**
	 * codepoint index corresponing to the ...th character in an UTF-8 encoded string
	 * if there are less than index characters, return str.length()
	 */
	size_t index(const utf8::string& str, const size_t index);

	/** length in characters of an UTF-8 encoded string */
	size_t size(const utf8::string& str);

	/** insert at position pos into an UTF-8 encoded string */
	utf8::string& insert(utf8::string& str, const size_t pos, const utf8::string& insert);

	/**
	 * erase len characters at position start from an UTF-8 encoded string
	 * this implementation doesn't check for valid UTF-8, don't use for user input
	 */
	utf8::string& erase(utf8::string& str, const size_t start, const size_t len = std::string::npos);

	/**
	* truncate an UTF-8 encoded string after size characters
	* this implementation doesn't check for valid UTF-8, don't use for user input
	*/
	utf8::string& truncate(utf8::string& str, const size_t size);

	/**
	 * Truncate a UTF-8 encoded string.
	 *
	 * If the string has more than @p size UTF-8 characters it will be truncated
	 * to this size.
	 * The output is guaranteed to be valid UTF-8.
	 *
	 * @param[in, out] str The parameter's usage is:
	 *                     - Input: String encoded in UTF-8.
	 *                     - Output: String encoded UTF-8 that contains at most @p size
	 *                       codepoints.
	 * @param size         The size to truncate at.
	 */
	void truncate_as_ucs4(utf8::string& str, const size_t size);
} // end namespace utf8

#endif
