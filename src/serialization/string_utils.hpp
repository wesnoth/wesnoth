/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2007 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/string_utils.hpp
//!

#ifndef SERIALIZATION_STRING_UTILS_HPP_INCLUDED
#define SERIALIZATION_STRING_UTILS_HPP_INCLUDED

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "../tstring.hpp"
#include "../util.hpp"

class variable_set
{
public:
	virtual ~variable_set();

	virtual const t_string& get_variable_const(const std::string& id) const = 0;
};

//! The type we use to represent Unicode strings.
typedef std::vector<wchar_t> wide_string;
// If we append a 0 to that one,
// we can pass it to SDL_ttf as a const Uint16*
typedef std::vector<Uint16> ucs2_string;
typedef std::vector<Uint32> ucs4_string;
typedef std::string utf8_string;

namespace utils {

bool isnewline(const char c);
bool portable_isspace(const char c);
bool notspace(char c);

enum { REMOVE_EMPTY = 0x01,	//!< REMOVE_EMPTY : remove empty elements
	  STRIP_SPACES  = 0x02	//!< STRIP_SPACES : strips leading and trailing blank spaces
};

std::vector< std::string > split(std::string const &val, char c = ',', int flags = REMOVE_EMPTY | STRIP_SPACES);
std::vector< std::string > paranthetical_split(std::string const &val, const char separator = 0 , std::string const &left="(", std::string const &right=")",int flags = REMOVE_EMPTY | STRIP_SPACES);
std::string join(std::vector< std::string > const &v, char c = ',');
std::vector< std::string > quoted_split(std::string const &val, char c= ',',
                                        int flags = REMOVE_EMPTY | STRIP_SPACES, char quote = '\\');
std::pair< int, int > parse_range(std::string const &str);
std::vector< std::pair< int, int > > parse_ranges(std::string const &str);
int apply_modifier( const int number, const std::string &amount, const int minimum = 0);
std::string &escape(std::string &str, const std::string& special_chars);
std::string &escape(std::string &str);
std::string &unescape(std::string &str);
//! Remove whitespace from the front and back of the string 'str'.
std::string &strip(std::string &str);
//! Removes character 'c' from the first and last position of the string 'str'.
std::string& strip_char(std::string &str, const char c);
bool string_bool(const std::string& str,bool def=false);

//! Check if a message contains a word.
bool word_match(const std::string& message, const std::string& word);
//! Match using '*' as any number of characters (including none), 
//! and '?' as any one character.
bool wildcard_string_match(const std::string& str, const std::string& match);
//! Check if the username contains only valid characters.
bool isvalid_username(const std::string &login);

typedef std::map< std::string, t_string > string_map;
//! Function which will interpolate variables, starting with '$' in the string 'str'
//! with the equivalent symbols in the given symbol table.
//! If 'symbols' is NULL, then game event variables will be used instead.
std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols);
std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables);

//! Functions for converting Unicode wide-char strings
//! to UTF-8 encoded strings, back and forth
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

	utf8_iterator(const std::string& str);
	utf8_iterator(std::string::const_iterator const &begin, std::string::const_iterator const &end);

	static utf8_iterator begin(const std::string& str);
	static utf8_iterator end(const std::string& str);

	bool operator==(const utf8_iterator& a) const;
	bool operator!=(const utf8_iterator& a) const { return ! (*this == a); }
	utf8_iterator& operator++();
	wchar_t operator*() const;
	bool next_is_end();
	const std::pair<std::string::const_iterator, std::string::const_iterator>& substr() const;
private:
	void update();

	wchar_t current_char;
	std::string::const_iterator string_end;
	std::pair<std::string::const_iterator, std::string::const_iterator> current_substr;
};

std::string wstring_to_string(const wide_string &);
wide_string string_to_wstring(const std::string &);
std::string wchar_to_string(const wchar_t);

//! Returns a version of the string with the first letter capitalized
utf8_string capitalize(const utf8_string&);
//! Returns an uppercased version of the string
utf8_string uppercase(const utf8_string&);
//! Returns a lowercased version of the string
utf8_string lowercase(const utf8_string&);

//! Truncates a string.
void truncate_as_wstring(std::string& str, const size_t size);

}

// Handy wrappers around interpolate_variables_into_string and gettext
std::string vgettext(const char*, const utils::string_map&);
std::string vngettext(const char*, const char*, int, const utils::string_map&);

#endif
