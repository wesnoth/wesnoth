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
}

namespace utf8 {

	/**
	* Functions for converting Unicode wide-char strings to UTF-8 encoded strings,
	* back and forth.
	*/
	class invalid_utf8_exception : public std::exception {};

	class iterator
	{
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef ucs4::char_t value_type;
		typedef ptrdiff_t difference_type;
		typedef ucs4::char_t* pointer;
		typedef ucs4::char_t& reference;

		iterator(const std::string& str);
		iterator(std::string::const_iterator const &begin, std::string::const_iterator const &end);

		static iterator begin(const std::string& str);
		static iterator end(const std::string& str);

		bool operator==(const utf8::iterator& a) const;
		bool operator!=(const utf8::iterator& a) const { return ! (*this == a); }
		iterator& operator++();
		ucs4::char_t operator*() const;
		bool next_is_end();
		const std::pair<std::string::const_iterator, std::string::const_iterator>& substr() const;
	private:
		void update();

		ucs4::char_t current_char;
		std::string::const_iterator string_end;
		std::pair<std::string::const_iterator, std::string::const_iterator> current_substr;
	};

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

} // end namespace utf8

namespace utils {
	std::string ucs4string_to_string(const ucs4::string &);
	ucs4::string string_to_ucs4string(const std::string &);
	std::string ucs4char_to_string(const ucs4::char_t);

	utf16::string ucs4string_to_utf16string(const ucs4::string &);

	/**
	 * Truncates a string.
	 *
	 * If the string send has more than size utf-8 characters it will be truncated
	 * to this size.
	 * No assumptions can be made about the actual size of the string.
	 *
	 * @param[in, out] str The parameter's usage is:
	 *                     - Input: String which can be converted to utf-8.
	 *                     - Output: String which contains maximal size utf-8
	 *                       characters.
	 * @param size         The size to truncate at.
	 */
	void truncate_as_ucs4string(std::string& str, const size_t size);

	/**
	 * Truncates a string to a given utf-8 character count and then appends an ellipsis.
	 */
	void ellipsis_truncate(std::string& str, const size_t size);

} // end namespace utils

#endif
