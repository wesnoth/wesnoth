/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>
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

#include "global.hpp"

#include "serialization/unicode.hpp"

#include "log.hpp"
#include "util.hpp"

#include <cassert>
#include <limits>
#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define ERR_GENERAL LOG_STREAM(err, lg::general)

namespace {
size_t byte_size_from_ucs4_codepoint(ucs4::char_t ch)
{
	if(ch < (1u << 7))
		return 1;
	else if(ch < (1u << 11))
		return 2;
	else if(ch < (1u << 16))
		return 3;
	else if(ch < (1u << 21))
		return 4;
	else if(ch < (1u << 26))
		return 5;
	else if(ch < (1u << 31))
		return 6;
	else
		throw utf8::invalid_utf8_exception(); // Invalid UCS-4
}
} // anonymous namespace

namespace implementation {
std::string ucs4string_to_string(const ucs4::string &src)
{
	std::string ret;

	try {
		for(ucs4::string::const_iterator i = src.begin(); i != src.end(); ++i) {
			unsigned int count;
			ucs4::char_t ch = *i;

			// Determine the bytes required
			count = byte_size_from_ucs4_codepoint(ch);

			if(count == 1) {
				ret.push_back(static_cast<char>(ch));
			} else {
				for(int j = static_cast<int>(count) - 1; j >= 0; --j) {
					unsigned char c = (ch >> (6 * j)) & 0x3f;
					c |= 0x80;
					if(j == static_cast<int>(count) - 1) {
						c |= 0xff << (8 - count);
					}
					ret.push_back(c);
				}
			}

		}

		return ret;
	}
	catch(utf8::invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UCS-4 character string\n";
		return ret;
	}
}

std::string ucs4char_to_string(const ucs4::char_t c)
{
	ucs4::string s;
	s.push_back(c);
	return ucs4string_to_string(s);
}

ucs4::string string_to_ucs4string(const std::string &src)
{
	ucs4::string res;

	try {
		utf8::iterator i1(src);
		const utf8::iterator i2(utf8::iterator::end(src));

		// Equivalent to res.insert(res.end(),i1,i2) which doesn't work on VC++6.
		while(i1 != i2) {
			res.push_back(*i1);
			++i1;
		}
	}
	catch(utf8::invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string: \"" << src << "\"\n";
		return res;
	}

	return res;
}

utf16::string ucs4string_to_utf16string(const ucs4::string &src)
{
	utf16::string res;
	const Uint32 bit17 = 0x10000;
	BOOST_FOREACH(const ucs4::char_t &u4, src) {
		if(u4 < bit17)
			res.push_back(static_cast<wchar_t>(u4));
		else {
			const Uint32 char20 = u4 - bit17;
			assert(char20 < (1 << 20));
			const ucs4::char_t lead = 0xD800 + (char20 >> 10);
			const ucs4::char_t trail = 0xDC00 + (char20 & 0x3FF);
			assert(lead < bit17);
			assert(trail < bit17);
			res.push_back(static_cast<wchar_t>(lead));
			res.push_back(static_cast<wchar_t>(trail));
		}
	}

	return res;
}

} // implementation namespace

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

iterator::iterator(const std::string& str) :
	current_char(0),
	string_end(str.end()),
	current_substr(std::make_pair(str.begin(), str.begin()))
{
	update();
}

iterator::iterator(std::string::const_iterator const &beg,
		std::string::const_iterator const &end) :
	current_char(0),
	string_end(end),
	current_substr(std::make_pair(beg, beg))
{
	update();
}

iterator iterator::begin(std::string const &str)
{
	return iterator(str.begin(), str.end());
}

iterator iterator::end(const std::string& str)
{
	return iterator(str.end(), str.end());
}

bool iterator::operator==(const utf8::iterator& a) const
{
	return current_substr.first == a.current_substr.first;
}

iterator& iterator::operator++()
{
	current_substr.first = current_substr.second;
	update();
	return *this;
}

ucs4::char_t iterator::operator*() const
{
	return current_char;
}

bool iterator::next_is_end()
{
	if(current_substr.second == string_end)
		return true;
	return false;
}

const std::pair<std::string::const_iterator, std::string::const_iterator>& iterator::substr() const
{
	return current_substr;
}

void iterator::update()
{
	// Do not try to update the current unicode char at end-of-string.
	if(current_substr.first == string_end)
		return;

	size_t size = byte_size_from_utf8_first(*current_substr.first);
	current_substr.second = current_substr.first + size;

	current_char = static_cast<unsigned char>(*current_substr.first);

	// Convert the first character
	if(size != 1) {
		current_char &= 0xFF >> (size + 1);
	}

	// Convert the continuation bytes
	for(std::string::const_iterator c = current_substr.first+1;
			c != current_substr.second; ++c) {
		// If the string ends occurs within an UTF8-sequence, this is bad.
		if (c == string_end)
			throw invalid_utf8_exception();

		if ((*c & 0xC0) != 0x80)
			throw invalid_utf8_exception();

		current_char = (current_char << 6) | (static_cast<unsigned char>(*c) & 0x3F);
	}

	// Check for non-shortest-form encoding
	// This has been forbidden in Unicode 3.1 for security reasons
	if (size > ::byte_size_from_ucs4_codepoint(current_char))
		throw invalid_utf8_exception();
}


utf8::string lowercase(const utf8::string& s)
{
	if(!s.empty()) {
		utf8::iterator itor(s);
		utf8::string res;

		for(;itor != utf8::iterator::end(s); ++itor) {
			ucs4::char_t uchar = *itor;
			// If wchar_t is less than 32 bits wide, we cannot apply towlower() to all codepoints
			if(uchar <= static_cast<ucs4::char_t>(std::numeric_limits<wchar_t>::max()) &&
			   uchar >= static_cast<ucs4::char_t>(std::numeric_limits<wchar_t>::min()))
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
	unsigned int chr, i = 0, len = str.size();
	try {
		for (chr=0; chr<index && i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string.\n";
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
		ERR_GENERAL << "Invalid UTF-8 string.\n";
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
