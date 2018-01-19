/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "serialization/ucs4_convert_impl.hpp"
#include "serialization/unicode_cast.hpp"
#include "serialization/unicode.hpp"

#include "log.hpp"

#include <cassert>
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

utf8::string lowercase(const utf8::string& s)
{
	if(!s.empty()) {
		utf8::iterator itor(s);
		utf8::string res;

		for(;itor != utf8::iterator::end(s); ++itor) {
			ucs4::char_t uchar = *itor;
			// If wchar_t is less than 32 bits wide, we cannot apply towlower() to all codepoints
			if(uchar <= static_cast<ucs4::char_t>(std::numeric_limits<wchar_t>::max()))
				uchar = towlower(static_cast<wchar_t>(uchar));
			res += unicode_cast<utf8::string>(uchar);
		}

		res.append(itor.substr().second, s.end());
		return res;
	}
	return s;
}

size_t index(const utf8::string& str, const size_t index)
{
	// chr counts characters, i is the codepoint index
	// remark: several functions rely on the fallback to str.length()
	unsigned int i = 0, len = str.size();
	try {
		for (unsigned int chr=0; chr<index && i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string." << std::endl;
	}
	return i;
}

size_t size(const utf8::string& str)
{
	unsigned int chr, i = 0, len = str.size();
	try {
		for (chr=0; i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string." << std::endl;
	}
	return chr;
}

utf8::string& insert(utf8::string& str, const size_t pos, const utf8::string& insert)
{
	return str.insert(index(str, pos), insert);
}

utf8::string& erase(utf8::string& str, const size_t start, const size_t len)
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

utf8::string& truncate(utf8::string& str, const size_t size)
{
	return erase(str, size);
}

void truncate_as_ucs4(utf8::string &str, const size_t size)
{
	ucs4::string u4_str = unicode_cast<ucs4::string>(str);
	if(u4_str.size() > size) {
		u4_str.resize(size);
		str = unicode_cast<utf8::string>(u4_str);
	}
}

} // end namespace utf8
