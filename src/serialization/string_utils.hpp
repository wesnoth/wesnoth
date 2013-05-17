/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2013 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERIALIZATION_STRING_UTILS_HPP_INCLUDED
#define SERIALIZATION_STRING_UTILS_HPP_INCLUDED

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/next_prior.hpp>

#include "SDL_types.h"

/** The type we use to represent Unicode strings. */
typedef std::vector<wchar_t> wide_string;

/** If we append a 0 to that one, we can pass it to SDL_ttf as a const Uint16*. */
typedef std::vector<Uint16> ucs2_string;
typedef std::vector<Uint32> ucs4_string;
typedef std::string utf8_string;

class t_string;

namespace utils {

extern const std::string unicode_minus;
extern const std::string unicode_en_dash;
extern const std::string unicode_em_dash;
extern const std::string unicode_figure_dash;
extern const std::string unicode_multiplication_sign;
extern const std::string unicode_bullet;

bool isnewline(const char c);
bool portable_isspace(const char c);
bool notspace(char c);

enum { REMOVE_EMPTY = 0x01,	/**< REMOVE_EMPTY : remove empty elements. */
	  STRIP_SPACES  = 0x02	/**< STRIP_SPACES : strips leading and trailing blank spaces. */
};

std::vector< std::string > split(std::string const &val, const char c = ',', const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Splits a string based on two separators into a map.
 * major: the separator between elements of the map
 * minor: the separator between keys and values in one element
 *
 * For example, the string 'a:b,c:d,e:f' would be parsed into:
 *  a => b
 *  c => d
 *  e => f
*/
std::map< std::string, std::string > map_split(std::string const &val, char major = ',', char minor = ':', int flags = REMOVE_EMPTY | STRIP_SPACES, std::string const default_value = "");

/**
 * Splits a string based either on a separator where text within parenthesis
 * is protected from splitting (Note that one can use the same character for
 * both the left and right parenthesis. In this mode it usually makes only
 * sense to have one character for the left and right parenthesis.)
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
std::vector< std::string > parenthetical_split(std::string const &val,
	const char separator = 0 , std::string const &left="(",
	std::string const &right=")",const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Similar to parenthetical_split, but also expands embedded square brackets.
 * Separator must be specified and number of entries in each square bracket
 * must match in each section.
 * Leading zeros are preserved if specified between square brackets.
 * An asterisk as in [a*n] indicates to expand 'a' n times
 *
 * This is useful to expand animation WML code.
 * Examples:
 * square_parenthetical_split("a[1-3](1,[5,6,7]),b[8,9]",",") should return
 * <"a1(1,5)","a2(1,6)","a3(1,7)","b8","b9">
 * square_parenthetical_split("abc[07-10]") should return
 * <"abc07","abc08","abc09","abc10">
 * square_parenthetical_split("a[1,2]b[3-4]:c[5,6]") should return
 * <"a1b3:c5","a2b4:c6">
 * square_parenthetical_split("abc[3,1].png") should return
 * <"abc3.png","abc2.png","abc1.png">
 * square_parenthetical_split("abc[de,xyz]") should return
 * <"abcde","abcxyz">
 * square_parenthetical_split("abc[1*3]") should return
 * <"abc1","abc1","abc1">
 */
std::vector< std::string > square_parenthetical_split(std::string const &val,
	const char separator = ',' , std::string const &left="([",
	std::string const &right=")]",const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Generates a new string joining container items in a list.
 *
 * @param v A container with elements.
 * @param s List delimiter.
 */
template <typename T>
std::string join(T const &v, const std::string& s = ",")
{
        std::stringstream str;
        for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
                str << *i;
                if (boost::next(i) != v.end())
                        str << s;
        }

        return str.str();
}

template <typename T>
std::string join_map(T const &v, std::string major = ",", std::string minor = ":")
{
        std::stringstream str;
        for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
                str << i->first << minor << i->second;
                if (boost::next(i) != v.end())
                        str << major;
        }

        return str.str();
}

/**
 * Generates a new string containing a bullet list.
 *
 * List items are preceded by the indentation blanks, a bullet string and
 * another blank; all but the last item are followed by a newline.
 *
 * @param v A container with elements.
 * @param indent Number of indentation blanks.
 * @param bullet The leading bullet string.
 */
template<typename T>
std::string bullet_list(const T& v, size_t indent = 4, const std::string& bullet = unicode_bullet)
{
	std::ostringstream str;

	for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
		if(i != v.begin()) {
			str << '\n';
		}

		str << std::string(indent, ' ') << bullet << ' ' << *i;
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

/* add a "+" or replace the "-" par Unicode minus */
inline std::string print_modifier(const std::string &mod)
{ return mod[0] == '-' ?
	(unicode_minus + std::string(mod.begin()+1, mod.end())) : ("+" + mod);}

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

/** Replace all instances of src in str with dst */
std::string replace(std::string str, const std::string &src, const std::string &dst);

/** Remove whitespace from the front and back of the string 'str'. */
std::string &strip(std::string &str);

/** Remove whitespace from the back of the string 'str'. */
std::string &strip_end(std::string &str);

/** Convert no, false, off, 0, 0.0 to false, empty to def, and others to true */
bool string_bool(const std::string& str,bool def=false);

/** Convert into a signed value (using the Unicode "−" and +0 convention */
std::string signed_value(int val);

/** Sign with Unicode "−" if negative */
std::string half_signed_value(int val);

/** Convert into a percentage (using the Unicode "−" and +0% convention */
inline std::string signed_percent(int val) {return signed_value(val) + "%";}

/**
 * Convert into a string with an SI-postfix.
 *
 * If the unit is to be translatable,
 * a t_string should be passed as the third argument.
 * _("unit_byte^B") is suggested as standard.
 *
 * There are no default values because they would not be translatable.
 */
std::string si_string(double input, bool base2, std::string unit);

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

/**
 * Truncates a string to a given utf-8 character count and then appends an ellipsis.
 */
void ellipsis_truncate(std::string& str, const size_t size);

}

#endif
