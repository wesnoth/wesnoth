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

/**
 * @file
 * Unicode support functions.
 */

#include "serialization/unicode_cast.hpp"
#include "serialization/unicode.hpp"

#include "log.hpp"

#include <limits>

static lg::log_domain log_engine("engine");
#define ERR_GENERAL LOG_STREAM(err, lg::general())

namespace utf8 {

static int byte_size_from_utf8_first(const unsigned char ch)
{
	if (!(ch & 0x80)) {
		return 1;  // US-ASCII character, 1 byte
	}
	/* first bit set: character not in US-ASCII, multiple bytes
	 * number of set bits at the beginning = bytes per character
	 * e.g. 11110xxx indicates a 4-byte character */
	int count = count_leading_ones(ch);
	if (count == 1 || count > 6) {		// count > 4 after RFC 3629
		throw invalid_utf8_exception(); // Stop on invalid characters
	}
	return count;
}

std::string lowercase(std::string_view s)
{
	if(!s.empty()) {
		utf8::iterator itor(s);
		std::string res;

		for(;itor != utf8::iterator::end(s); ++itor) {
			char32_t uchar = *itor;
			// If wchar_t is less than 32 bits wide, we cannot apply towlower() to all codepoints
			if(uchar <= static_cast<char32_t>(std::numeric_limits<wchar_t>::max()))
				uchar = towlower(static_cast<wchar_t>(uchar));
			res += unicode_cast<std::string>(uchar);
		}

		res.append(itor.substr().second, s.end());
		return res;
	}
	return std::string();
}

std::size_t index(std::string_view str, const std::size_t index)
{
	// chr counts characters, i is the codepoint index
	// remark: several functions rely on the fallback to str.length()
	unsigned int i = 0, len = str.size();
	try {
		for (unsigned int chr=0; chr<index && i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(const invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string.";
	}
	return i;
}

std::size_t size(std::string_view str)
{
	unsigned int chr, i = 0, len = str.size();
	try {
		for (chr=0; i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(const invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string.";
	}
	return chr;
}

std::string& insert(std::string& str, const std::size_t pos, const std::string& insert)
{
	return str.insert(index(str, pos), insert);
}

std::string& erase(std::string& str, const std::size_t start, const std::size_t len)
{
	if (start > size(str)) return str;
	unsigned pos = index(str, start);

	if (len == std::string::npos) {
		// without second argument, std::string::erase truncates
		return str.erase(pos);
	} else {
		return str.erase(pos, index(str,start+len) - pos);
	}
}

std::string& truncate(std::string& str, const std::size_t size)
{
	return erase(str, size);
}

void truncate_as_ucs4(std::string &str, const std::size_t size)
{
	std::u32string u4_str = unicode_cast<std::u32string>(str);
	if(u4_str.size() > size) {
		u4_str.resize(size);
		str = unicode_cast<std::string>(u4_str);
	}
}

} // end namespace utf8
