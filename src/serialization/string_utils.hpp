/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file serialization/string_utils.hpp */

#ifndef SERIALIZATION_STRING_UTILS_HPP_INCLUDED
#define SERIALIZATION_STRING_UTILS_HPP_INCLUDED

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/next_prior.hpp>
#include "../tstring.hpp"

#include "SDL_types.h"

class variable_set
{
public:
	virtual ~variable_set();

	virtual const t_string& get_variable_const(const std::string& id) const = 0;
};

/** The type we use to represent Unicode strings. */
typedef std::vector<wchar_t> wide_string;

/** If we append a 0 to that one, we can pass it to SDL_ttf as a const Uint16*. */
typedef std::vector<Uint16> ucs2_string;
typedef std::vector<Uint32> ucs4_string;
typedef std::string utf8_string;

namespace utils {

bool isnewline(const char c);
bool portable_isspace(const char c);
bool notspace(char c);

enum { REMOVE_EMPTY = 0x01,	/**< REMOVE_EMPTY : remove empty elements. */
	  STRIP_SPACES  = 0x02	/**< STRIP_SPACES : strips leading and trailing blank spaces. */
};

std::vector< std::string > split(std::string const &val, char c = ',', int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Splits a string based either on a separator where text within paranthesis
 * is protected from splitting (Note that one can use the same character for
 * both the left and right paranthesis. In this mode it usually makes only
 * sense to have one character for the left and right paranthesis.)
 * or if the separator == 0 it splits a string into an odd number of parts:
 * - The part before the first '(',
 * - the part between the first '('
 * - and the matching right ')', etc ...
 * and the remainder of the string.
 * Note that this will find the first matching char in the left string
 * and match against the corresponding char in the right string.
 * In this mode, a correctly processed string should return with
 * an odd number of elements to the vector and
 * an empty elements are never removed as they are placeholders.
 * hence REMOVE EMPTY only works for the separator split.
 *
 * parenthetical_split("a(b)c{d}e(f{g})h",0,"({",")}") should return
 * a vector of <"a","b","c","d","e","f{g}","h">
 */
std::vector< std::string > paranthetical_split(std::string const &val,
	const char separator = 0 , std::string const &left="(",
	std::string const &right=")",int flags = REMOVE_EMPTY | STRIP_SPACES);

template <typename T>
std::string join(T const &v, char c = ',')
{
        std::stringstream str;
        for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
                str << *i;
                if (boost::next(i) != v.end())
                        str << c;
        }

        return str.str();
}

/**
 * This function is identical to split(), except it does not split
 * when it otherwise would if the previous character was identical to the parameter 'quote'.
 * i.e. it does not split quoted commas.
 * This method was added to make it possible to quote user input,
 * particularly so commas in user input will not cause visual problems in menus.
 *
 * @todo Why not change split()? That would change the methods post condition.
 */
std::vector< std::string > quoted_split(std::string const &val, char c= ',',
                                        int flags = REMOVE_EMPTY | STRIP_SPACES, char quote = '\\');
std::pair< int, int > parse_range(std::string const &str);
std::vector< std::pair< int, int > > parse_ranges(std::string const &str);
int apply_modifier( const int number, const std::string &amount, const int minimum = 0);

/** Prepends a configurable set of characters with a backslash */
std::string escape(const std::string &str, const char *special_chars);

/**
 * Prepend all special characters with a backslash.
 *
 * Special characters are:
 * #@{}+-,\*=
 */
inline std::string escape(const std::string &str)
{ return escape(str, "#@{}+-,\\*="); }

/** Remove all escape characters (backslash) */
std::string unescape(const std::string &str);

/** Remove whitespace from the front and back of the string 'str'. */
std::string &strip(std::string &str);

/** Convert no, false, off, 0, 0.0 to false, empty to def, and others to true */
bool string_bool(const std::string& str,bool def=false);

/**
 * Try to complete the last word of 'text' with the 'wordlist'.
 *
 * @param[in]  'text'     Text where we try to complete the last word of.
 * @param[out] 'text'     Text with completed last word.
 * @param[in]  'wordlist' A vector of strings to complete against.
 * @param[out] 'wordlist' A vector of strings that matched 'text'.
 *
 * @return 'true' iff text is just one word (no spaces)
 */
bool word_completion(std::string& text, std::vector<std::string>& wordlist);

/** Check if a message contains a word. */
bool word_match(const std::string& message, const std::string& word);

/**
 * Match using '*' as any number of characters (including none), and '?' as any
 * one character.
 */
bool wildcard_string_match(const std::string& str, const std::string& match);

/**
 * Check if the username contains only valid characters.
 *
 * (all alpha-numeric characters plus underscore and hyphen)
 */
bool isvalid_username(const std::string &login);

/**
 * Check if the username pattern contains only valid characters.
 *
 * (all alpha-numeric characters plus underscore, hyphen,
 * question mark and asterisk)
 */
bool isvalid_wildcard(const std::string &login);

typedef std::map< std::string, t_string > string_map;

/**
 * Functions for converting Unicode wide-char strings to UTF-8 encoded strings,
 * back and forth.
 */
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

/** Returns a lowercased version of the string. */
utf8_string lowercase(const utf8_string&);

/**
 * Truncates a string.
 *
 * If the string send has more than size utf-8 characters it will be truncated
 * to this size.
 * No assumptions can be made about the actual size of the string.
 *
 * @param[in]  str     String which can be converted to utf-8.
 * @param[out] str     String which contains maximal size utf-8 characters.
 * @param size         The size to truncate at.
 */
void truncate_as_wstring(std::string& str, const size_t size);

}

#endif
