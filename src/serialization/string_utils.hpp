/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SERIALIZATION_STRING_UTILS_HPP_INCLUDED
#define SERIALIZATION_STRING_UTILS_HPP_INCLUDED

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "SDL_types.h"

//the type we use to represent Unicode strings.
typedef std::vector<wchar_t> wide_string;
//if we append a 0 to that one we can pass it to SDL_ttf
//as a const Uint16*
typedef std::vector<Uint16> ucs2_string;
typedef std::vector<Uint32> ucs4_string;
typedef std::string utf8_string;

namespace utils {

bool isnewline(char c);
bool portable_isspace(char c);
bool notspace(char c);

// REMOVE_EMPTY : remove empty elements
// STRIP_SPACES : strips leading and trailing blank spaces
enum { REMOVE_EMPTY = 0x01, STRIP_SPACES = 0x02 };
std::vector< std::string > split(std::string const &val, char c = ',', int flags = REMOVE_EMPTY | STRIP_SPACES);
std::string join(std::vector< std::string > const &v, char c = ',');
std::vector< std::string > quoted_split(std::string const &val, char c= ',',
                                        int flags = REMOVE_EMPTY | STRIP_SPACES, char quote = '\\');
std::pair< int, int > parse_range(std::string const &str);
bool notspace(char c);
std::string &escape(std::string &str);
std::string &unescape(std::string &str);
std::string &strip(std::string &str);
bool has_value(std::string const &values, std::string const &val);

typedef std::map< std::string, std::string > string_map;
// function which will interpolate variables, starting with '$' in the string 'str' with
// the equivalent symbols in the given symbol table. If 'symbols' is NULL, then game event
// variables will be used instead
std::string interpolate_variables_into_string(std::string const &str, string_map const *symbols = NULL);

//functions for converting Unicode wide-char strings to UTF-8 encoded
//strings, back and forth
class invalid_utf8_exception : public std::exception {
};

class utf8_iterator 
{
public:
	typedef std::input_iterator_tag iterator_category;
	typedef wchar_t value_type;
	typedef ptrdiff_t difference_type;
	typedef wchar_t* pointer;
	typedef wchar_t& reference;

	utf8_iterator();
	utf8_iterator(const std::string& str);
	utf8_iterator(std::string::const_iterator begin, std::string::const_iterator end);

	static utf8_iterator end(const std::string& str);

	bool operator==(const utf8_iterator& a);
	bool operator!=(const utf8_iterator& a) { return ! (*this == a); }
	utf8_iterator& operator++();
	wchar_t operator*();
	const std::pair<std::string::const_iterator, std::string::const_iterator>& substr();
private:
	void update();

	wchar_t current_char;
	std::string::const_iterator string_end;
	std::pair<std::string::const_iterator, std::string::const_iterator> current_substr;
};

std::string wstring_to_string(const wide_string &);
wide_string string_to_wstring(const std::string &);
std::string wchar_to_string(const wchar_t);
ucs2_string utf8_string_to_ucs2_string(const utf8_string& src);
utf8_string ucs2_string_to_utf8_string(const ucs2_string& src);

}

#endif
